#include "../external/cJSON/cJSON.h"

#include "common_native.h"

static void buildMapFromJson(cJSON *item, ObjMap *map);

static void buildListFromJson(cJSON *item, ObjList *list) {  
  if (cJSON_IsString(item)) {
    pushToList(list, OBJ_VAL(copyString(item->valuestring, strlen(item->valuestring), &vm.strings)));
  } else if (cJSON_IsNumber(item)) {
    pushToList(list, NUMBER_VAL(item->valuedouble));
  } else if (cJSON_IsBool(item)) {      
    pushToList(list, BOOL_VAL(cJSON_IsTrue(item)));
  } else if (cJSON_IsNull(item)) {
    pushToList(list, NIL_VAL);
  } else if (cJSON_IsObject(item)) {
    ObjMap *nested_map = newMap(vm.klass.map);
    push(OBJ_VAL(nested_map));
    buildMapFromJson(item->child, nested_map);  
    pushToList(list, OBJ_VAL(nested_map));
    pop();
  } else if (cJSON_IsArray(item)) {
    ObjList *nested_list = newList(vm.klass.list);
    push(OBJ_VAL(nested_list));
    int size = cJSON_GetArraySize(item);
    for (int i = 0; i < size; i++) {
      cJSON *elem = cJSON_GetArrayItem(item, i);        
      buildListFromJson(elem, nested_list); 
    }
    pushToList(list, OBJ_VAL(nested_list));
    pop();
  }
}

static void buildMapFromJson(cJSON *item, ObjMap *map) {
  while (item) {
    if (!item->string) { 
      item = item->next;
      continue;
    }

    if (cJSON_IsString(item)) {
      tableSet(&map->items, copyString(item->string, strlen(item->string), &vm.strings), OBJ_VAL(copyString(item->valuestring, strlen(item->valuestring), &vm.strings)));
    } else if (cJSON_IsNumber(item)) {
      tableSet(&map->items, copyString(item->string, strlen(item->string), &vm.strings), NUMBER_VAL(item->valuedouble));
    } else if (cJSON_IsBool(item)) {      
      tableSet(&map->items, copyString(item->string, strlen(item->string), &vm.strings), BOOL_VAL(cJSON_IsTrue(item)));
    } else if(cJSON_IsNull(item)) {
      tableSet(&map->items, copyString(item->string, strlen(item->string), &vm.strings), NIL_VAL);
    } else if (cJSON_IsObject(item)) {
      ObjMap *nested_map = newMap(vm.klass.map);
      push(OBJ_VAL(nested_map));
      buildMapFromJson(item->child, nested_map);      
      tableSet(&map->items, copyString(item->string, strlen(item->string), &vm.strings), OBJ_VAL(nested_map));
      pop();
    } else if (cJSON_IsArray(item)) {
      ObjList *list = newList(vm.klass.list);
      push(OBJ_VAL(list));
      int size = cJSON_GetArraySize(item);
      for (int i = 0; i < size; i++) {
        cJSON *elem = cJSON_GetArrayItem(item, i);        
        buildListFromJson(elem, list); 
      }
      tableSet(&map->items, copyString(item->string, strlen(item->string), &vm.strings), OBJ_VAL(list));
      pop();
    }
    item = item->next;
  } 
}

static Value parseJsonNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    return NIL_VAL;
  };
  cJSON *root = cJSON_Parse(AS_CSTRING(args[1])); 
  if (!root) {
    runtimeError("failed to parse JSON: [%s]\n", cJSON_GetErrorPtr());
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  ObjMap *map = newMap(vm.klass.map);
  push(OBJ_VAL(map));
  buildMapFromJson(root->child, map);
  cJSON_Delete(root);
  return pop();
}

static cJSON *valueToJson(Value value);

static cJSON *mapToJson(ObjMap *map) {
  cJSON *object = cJSON_CreateObject();
  for (int i = 0; i < map->items.capacity; i++) {
    Entry *entry = &map->items.entries[i];
    if (entry->key) {
      const char *key = entry->key->chars;
      cJSON *jsonVal = valueToJson(entry->value);
      cJSON_AddItemToObject(object, key, jsonVal);
    }
  }
  return object;
}

static cJSON *listToJson(ObjList *list) {
  cJSON *array = cJSON_CreateArray();
  for (int i = 0; i < list->count; i++) {
    cJSON *jsonElem = valueToJson(list->items[i]);
    cJSON_AddItemToArray(array, jsonElem); 
  }
  return array;
}

static cJSON *valueToJson(Value value) {
  if (IS_STRING(value)) {
    return cJSON_CreateString(AS_CSTRING(value));
  } else if (IS_NUMBER(value)) {
    return cJSON_CreateNumber(AS_NUMBER(value));
  } else if (IS_BOOL(value)) {
    return cJSON_CreateBool(AS_BOOL(value));
  } else if (IS_NIL(value)) {
    return cJSON_CreateNull();
  } else if (IS_OBJ(value)) {
    if (OBJ_TYPE(value) == OBJ_MAP) {
      return mapToJson((ObjMap *)AS_OBJ(value));
    } else if (OBJ_TYPE(value) == OBJ_LIST) {
      return listToJson((ObjList *)AS_OBJ(value));
    }
  }

  return cJSON_CreateString("<unsupported>");
}

static Value stringifyJsonNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_MAP, ARG_BOOL)) {
    return NIL_VAL;
  }
  ObjMap *map = AS_MAP(args[1]);
  bool shouldFormat = AS_BOOL(args[2]);

  cJSON *json = mapToJson(map);
  char *jsonStr;
  if (shouldFormat) {
    jsonStr = cJSON_Print(json);
  } else {
    jsonStr = cJSON_PrintUnformatted(json);
  }
  cJSON_Delete(json);
  push(OBJ_VAL(copyString(jsonStr, strlen(jsonStr), &vm.strings)));
  free(jsonStr);
  return pop();
}

void registerJsonNatives() {
  static bool isRegistered = false;
  if (isRegistered)
    return;
  isRegistered = true;

  ObjInstance *jsonInstance =
      defineInstance(defineKlass("JSON", 4, OBJ_INSTANCE), "JSON", 4); 
  defineNativeInstanceMethod(jsonInstance, "parse", 5, parseJsonNative);
  defineNativeInstanceMethod(jsonInstance, "stringify", 9, stringifyJsonNative);
}
