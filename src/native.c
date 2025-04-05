#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <curl/curl.h>
#include <readline/readline.h>

#include "../vendor/cJSON/cJSON.h"

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

struct NetworkData {
  char *response;
  size_t size;
};

typedef enum {
  ARG_ANY,
  ARG_NUMBER,
  ARG_STRING,
  ARG_LIST,
  ARG_MAP,
  ARG_CLOSURE,
  ARG_KLASS,
  ARG_INSTANCE,
  ARG_FILE,
} ArgTypes;

typedef enum {
  NATIVE_VARIADIC,
  NATIVE_NORMAL,
} NativeType;

static bool checkArgCount(int argCount, int expectedCount) {
  if (argCount != expectedCount) {
    runtimeError("Expected %d argument but got %d.", expectedCount - 1,
                 argCount - 1);
    vm.shouldPanic = true;
    return false;
  }
  return true;
}

static bool checkArgs(int argCount, int expectedCount, Value *args,
                      NativeType type, ...) {
  if (expectedCount == 0) {
    if (argCount != 1 && type != NATIVE_VARIADIC) {
      runtimeError("Expected no arguments but got %d.", argCount - 1);
      vm.shouldPanic = true;
      return false;
    }
    return true;
  }
  if (type == NATIVE_NORMAL && !checkArgCount(argCount, expectedCount)) {
    return false;
  } else if (type == NATIVE_VARIADIC && argCount < expectedCount) {
    runtimeError("Expected %d argument but got %d.", expectedCount - 1,
                 argCount - 1);
    vm.shouldPanic = true;
    return false;
  }

  va_list expectedArgs;
  va_start(expectedArgs, type);

  for (int i = 0; i < expectedCount; i++) {
    ArgTypes argType = va_arg(expectedArgs, ArgTypes);
    switch (argType) {
    case ARG_NUMBER:
      if (!IS_NUMBER(args[i])) {
        runtimeError("Expected argument %d to be a number.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_STRING:
      if (!IS_STRING(args[i])) {
        runtimeError("Expected argument %d to be a string.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_LIST:
      if (!IS_LIST(args[i])) {
        runtimeError("Expected argument %d to be a list.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_MAP:
      if (!IS_MAP(args[i])) {
        runtimeError("Expected argument %d to be a map.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_CLOSURE:
      if (!IS_CLOSURE(args[i])) {
        runtimeError("Expected argument %d to be a closure.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_KLASS:
      if (!IS_KLASS(args[i])) {
        runtimeError("Expected argument %d to be a class.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      va_end(expectedArgs);
      break;
    case ARG_INSTANCE:
      if (!IS_INSTANCE(args[i])) {
        runtimeError("Expected argument %d to be an instance.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_FILE:
      if (!IS_FILE(args[i])) {
        runtimeError("Expected argument %d to be an instance.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
    case ARG_ANY:
    default:
      continue;
    }
  }
  va_end(expectedArgs);
  return true;
}

static void defineNative(const char *name, int len, NativeFn function) {
  push(OBJ_VAL(copyString(name, len, &vm.strings)));
  push(OBJ_VAL(newNative(function)));
  tableSet(&vm.globals, AS_STRING(peek(1)), peek(0));
  pop();
  pop();
}

static ObjInstance *defineInstance(ObjKlass *klass, const char *name, int len) {
  push(OBJ_VAL(copyString(name, len, &vm.strings)));
  push(OBJ_VAL(klass));
  push(OBJ_VAL(newInstance(klass)));
  tableSet(&vm.globals, AS_STRING(peek(2)), peek(0));
  ObjInstance *instance = AS_INSTANCE(peek(0));
  pop();
  pop();
  pop();
  return instance;
}

static ObjKlass *defineKlass(const char *name, int len, ObjType base) {
  push(OBJ_VAL(copyString(name, len, &vm.strings)));
  push(OBJ_VAL(newKlass(AS_STRING(peek(0)), base)));
  tableSet(&vm.globals, AS_STRING(peek(1)), peek(0));
  ObjKlass *klass = AS_KLASS(peek(0));
  pop();
  pop();
  return klass;
}

static void defineNativeKlassMethod(ObjKlass *klass, const char *name, int len,
                                    NativeFn function) {
  push(OBJ_VAL(klass));
  push(OBJ_VAL(copyString(name, len, &vm.strings)));
  push(OBJ_VAL(newNative(function)));
  tableSet(&klass->properties, AS_STRING(peek(1)), peek(0));
  pop();
  pop();
  pop();
}

static void defineNativeInstanceMethod(ObjInstance *instance, const char *name,
                                       int len, NativeFn function) {
  push(OBJ_VAL(instance));
  push(OBJ_VAL(copyString(name, len, &vm.strings)));
  push(OBJ_VAL(newNative(function)));
  tableSet(&instance->fields, AS_STRING(peek(1)), peek(0));
  pop();
  pop();
  pop();
}

static void setNativeInstanceField(ObjInstance *instance, ObjString *string,
                                   Value value) {
  tableSet(&instance->fields, string, value);
}
static void defineNativeInstanceField(ObjInstance *instance, const char *string,
                                      int len, Value value) {
  push(OBJ_VAL(copyString(string, len, &vm.strings)));
  setNativeInstanceField(instance, AS_STRING(peek(0)), value);
  pop();
}

static Value tickNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 0, args, NATIVE_NORMAL)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value sleepNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  int sleepTime = AS_NUMBER(args[1]) * 1000000;
  return NUMBER_VAL(usleep(sleepTime));
}

static Value exitNative(int argCount, Value *args) {
  if (argCount == 1) {
    exit(0);
  }
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  exit(AS_NUMBER(args[1]));
}

static Value panicNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  vm.shouldPanic = true;
  if (IS_INSTANCE(args[1])) {
    ObjInstance *err = AS_INSTANCE(args[1]);
    Value isError;
    if (tableGet(&err->fields, vm.string.isError, &isError)) {
      if (IS_BOOL(isError) && AS_BOOL(isError)) {
        Value message;
        if (tableGet(&err->fields, vm.string.message, &message) &&
            IS_STRING(message)) {
          fprintf(stderr, "%s: ", err->klass->name->chars);
          runtimeError(AS_CSTRING(message));
          return NIL_VAL;
        }
      }
    }
  } else if (IS_STRING(args[1])) {
    runtimeError(AS_CSTRING(args[1]));
    return NIL_VAL;
  }
  runtimeError("Panic with unepxected value!");
  return NIL_VAL;
}

static size_t writeDataCallback(char *data, size_t size, size_t nmemb, void *clientp)
{
  size_t realsize = size * nmemb;
  struct NetworkData *mem = (struct NetworkData *)clientp;
 
  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if(!ptr)
    return 0;  /* out of memory */
 
  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;
 
  return realsize;
}

static Value getRequestNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING, ARG_LIST)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };

  CURL *curl = curl_easy_init();
  if (!curl) {
    vm.shouldPanic = true;
    return NIL_VAL;
  }

  struct curl_slist *headers = NULL;

  ObjList *list = AS_LIST(args[2]);
  for (int i = 0; i < list->count; i++) {
    Value item = list->items[i];
    if (!IS_STRING(item)) {
      vm.shouldPanic = true;
      runtimeError("Post header list must be strings only");
      return NIL_VAL;
    }    
    headers = curl_slist_append(headers, AS_CSTRING(item));
  }

  if (headers != NULL) {
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }

  struct NetworkData chunk = {0};
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

  curl_easy_setopt(curl, CURLOPT_URL, AS_CSTRING(args[1]));


  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    runtimeError("Error making get request.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }

  long response_code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

  ObjInstance *pair = newInstance(vm.klass.pair);
  push(OBJ_VAL(pair));
  defineNativeInstanceField(pair, "status", 6, NUMBER_VAL((double)response_code));
  defineNativeInstanceField(pair, "response", 8, OBJ_VAL(copyString(chunk.response, chunk.size, &vm.strings)));

  free(chunk.response);
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  return pop();
}

static Value postRequestNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 4, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING, ARG_STRING, ARG_LIST)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };

  CURL *curl = curl_easy_init();
  if (!curl) {
    vm.shouldPanic = true;
    return NIL_VAL;
  }

  struct curl_slist *headers = NULL;

  ObjList *list = AS_LIST(args[3]);
  for (int i = 0; i < list->count; i++) {
    Value item = list->items[i];
    if (!IS_STRING(item)) {
      vm.shouldPanic = true;
      runtimeError("Post header list must be strings only");
      return NIL_VAL;
    }    
    headers = curl_slist_append(headers, AS_CSTRING(item));
  }

  struct NetworkData chunk = {0};
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  if (headers != NULL) {
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, AS_CSTRING(args[2]));
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

  curl_easy_setopt(curl, CURLOPT_URL, AS_CSTRING(args[1]));


  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    runtimeError("Error making get request.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }

  long response_code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

  ObjInstance *pair = newInstance(vm.klass.pair);
  push(OBJ_VAL(pair));
  defineNativeInstanceField(pair, "status", 6, NUMBER_VAL((double)response_code));
  defineNativeInstanceField(pair, "response", 8, OBJ_VAL(copyString(chunk.response, chunk.size, &vm.strings)));

  free(chunk.response);
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  return pop();
}

static void buildMapFromJson(cJSON *item, ObjMap *map);

static void buildListFromJson(cJSON *item, ObjList *list) {  
  if (cJSON_IsString(item)) {
    pushToList(list, OBJ_VAL(copyString(item->valuestring, strlen(item->valuestring), &vm.strings)));
  } else if (cJSON_IsNumber(item)) {
    pushToList(list, NUMBER_VAL(item->valuedouble));
  } else if (cJSON_IsBool(item)) {      
    pushToList(list, BOOL_VAL(cJSON_IsTrue(item)));
  } else if (cJSON_IsObject(item)) {
    ObjMap *nested_map = newMap(vm.klass.map);
    push(OBJ_VAL(nested_map));
    buildMapFromJson(item->child, nested_map);  
    pushToList(list, pop());
  } else if (cJSON_IsArray(item)) {
    ObjList *nested_list = newList(vm.klass.list);
    push(OBJ_VAL(nested_list));
    int size = cJSON_GetArraySize(item);
    for (int i = 0; i < size; i++) {
      cJSON *elem = cJSON_GetArrayItem(item, i);        
      buildListFromJson(elem, nested_list); 
    }
    pushToList(list, pop());
  }
}

static void buildMapFromJson(cJSON *item, ObjMap *map) {
  while (item) {
    if (!item->string) { 
      item = item->next;
      continue;
    }

    if (cJSON_IsString(item)) {
      if (tableSet(&map->items, copyString(item->string, strlen(item->string), &vm.strings), OBJ_VAL(copyString(item->valuestring, strlen(item->valuestring), &vm.strings)))) {    
        item = item->next;
        continue;
      }
      break;
    } else if (cJSON_IsNumber(item)) {
      if (tableSet(&map->items, copyString(item->string, strlen(item->string), &vm.strings), NUMBER_VAL(item->valuedouble))) {        
        item = item->next;        
        continue;
      }
      break;
    } else if (cJSON_IsBool(item)) {      
      if (tableSet(&map->items, copyString(item->string, strlen(item->string), &vm.strings), BOOL_VAL(cJSON_IsTrue(item)))) {
        item = item->next;
        continue; 
      }
      break;
    } else if (cJSON_IsObject(item)) {
      ObjMap *nested_map = newMap(vm.klass.map);
      push(OBJ_VAL(nested_map));
      buildMapFromJson(item->child, nested_map);      
      if (tableSet(&map->items, copyString(item->string, strlen(item->string), &vm.strings), pop())) {
        item = item->next;
        continue; 
      }
      break;
    } else if (cJSON_IsArray(item)) {
      ObjList *list = newList(vm.klass.list);
      push(OBJ_VAL(list));
      int size = cJSON_GetArraySize(item);
      for (int i = 0; i < size; i++) {
        cJSON *elem = cJSON_GetArrayItem(item, i);        
        buildListFromJson(elem, list); 
      }
      if (tableSet(&map->items, copyString(item->string, strlen(item->string), &vm.strings), pop())) {
        item = item->next;
        continue;
      }
      break;
    }

    item = item->next;
    continue;
  } 
}

static Value parseJsonNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    vm.shouldPanic = true;
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

static Value initFileNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING,
                 ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  FILE *file;
  file = fopen(AS_CSTRING(args[1]), AS_CSTRING(args[2]));
  if (file == NULL) {
    runtimeError("Error opening file.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  ObjFile *file_;
  if (IS_FILE(args[0])) {
    file_ = AS_FILE(args[0]);
  } else if (IS_KLASS(args[0])) {
    file_ = newFile(AS_KLASS(args[0]));
  } else {
    file_ = newFile(vm.klass.file);
  }
  file_->file = file;
  return OBJ_VAL(file_);
}

static Value closeFileNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_FILE)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjFile *file = AS_FILE(args[0]);
  fclose(file->file);
  file->file = NULL;
  return NIL_VAL;
}

static Value writeFileNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_FILE, ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjString *str = AS_STRING(args[1]);
  FILE *file = AS_FILE(args[0])->file;
  for (int i = 0; i < str->length; i++) {
    int status = fputc(str->chars[i], file);
    if (status == EOF) {
      runtimeError("Could not write to file!");
      vm.shouldPanic = true;
      return NIL_VAL;
    }
  }
  return NIL_VAL;
}

static Value readFileNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_FILE, ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  FILE *file = AS_FILE(args[0])->file;
  ObjString *termStr = AS_STRING(args[1]);
  if (termStr->length > 1) {
    runtimeError("Read terminator should be empty for EOF or 1 char long.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  char term;
  if (termStr->length == 0) {
    term = EOF;
  } else {
    term = termStr->chars[0];
  }
  int count = 0;
  int capacity = 8;
  int oldCapacity = 0;
  char *str = NULL;
  str = GROW_ARRAY(char, str, oldCapacity, capacity);
  do {
    char c = fgetc(file);
    if (c == EOF) {
      break;
    }
    if (c == term) {
      break;
    }
    if (capacity < count + 1) {
      oldCapacity = capacity;
      capacity = GROW_CAPACITY(oldCapacity);
      str = GROW_ARRAY(char, str, oldCapacity, capacity);
    }
    str[count] = c;
    count++;
  } while (true);
  push(OBJ_VAL(copyString(str, count, &vm.strings)));
  FREE_ARRAY(char, str, capacity);
  return pop();
}

static Value initListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_VARIADIC, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjList *list = NULL;
  if (IS_LIST(args[0])) {
    list = AS_LIST(args[0]);
  } else if (IS_KLASS(args[0])) {
    list = newList(AS_KLASS(args[0]));
  } else {
    runtimeError("Unexpect base for List init.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  push(OBJ_VAL(list));
  for (int i = 1; i < argCount; i++) {
    pushToList(list, args[i]);
  }
  return pop();
}

static Value pushListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_VARIADIC, ARG_LIST, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjList *list = AS_LIST(args[0]);
  for (int i = 1; i < argCount; i++) {
    Value item = args[i];
    pushToList(list, item);
  }
  return NIL_VAL;
}

static Value popListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_LIST)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjList *list = AS_LIST(args[0]);
  Value value = list->items[list->count - 1];
  deleteFromList(list, list->count - 1, list->count - 1);
  return value;
}

static Value lenListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_LIST)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjList *list = AS_LIST(args[0]);
  double count = (double)list->count;
  return NUMBER_VAL(count);
}

static Value removeListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_LIST, ARG_NUMBER,
                 ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  ObjList *list = AS_LIST(args[0]);
  int start = AS_NUMBER(args[1]);
  int end = AS_NUMBER(args[2]);
  if (!isValidListRange(list, start, end)) {
    runtimeError("List range index is out of range.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  deleteFromList(list, start, end);
  return NIL_VAL;
}

static Value joinListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_LIST, ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  ObjList *list = AS_LIST(args[0]);
  ObjString *delimiter = AS_STRING(args[1]);
  int length = 0;

  for (int i = 0; i < list->count; i++) {
    Value item = list->items[i];
    if (IS_STRING(item)) {
      length += AS_STRING(item)->length;
    } else {
      runtimeError("Can only join a list of strings.");
      vm.shouldPanic = true;
      return NIL_VAL;
    }
    if (i < list->count - 1) {
      length += delimiter->length;
    }
  }

  char *result = ALLOCATE(char, length + 1);
  char *rpos = result;
  for (int i = 0; i < list->count; i++) {
    ObjString *str = AS_STRING(list->items[i]);
    memcpy(rpos, str->chars, str->length);
    rpos += str->length;
    if (i < list->count - 1) {
      memcpy(rpos, delimiter->chars, delimiter->length);
      rpos += delimiter->length;
    }
  }
  *rpos = '\0';
  return OBJ_VAL(takeString(result, length));
}

static Value initMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjMap *map = NULL;
  if (IS_MAP(args[0])) {
    map = AS_MAP(args[0]);
  } else if (IS_KLASS(args[0])) {
    map = newMap(AS_KLASS(args[0]));
  } else {
    runtimeError("Unexpect base for map init.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  return OBJ_VAL(map);
}

static Value keysMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_MAP)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjMap *map = AS_MAP(args[0]);
  ObjList *list = newList(vm.klass.list);
  vm.keep = (Obj *)list;
  for (int i = 0; i < map->items.capacity; i++) {
    Entry entry = map->items.entries[i];
    if (entry.key == NULL) {
      continue;
    }
    pushToList(list, OBJ_VAL(entry.key));
  }
  vm.keep = NULL;
  return OBJ_VAL(list);
}

static Value valuesMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_MAP)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjMap *map = AS_MAP(args[0]);
  ObjList *list = newList(vm.klass.list);
  vm.keep = (Obj *)list;
  for (int i = 0; i < map->items.capacity; i++) {
    Entry entry = map->items.entries[i];
    if (entry.key == NULL) {
      continue;
    }
    pushToList(list, entry.value);
  }
  vm.keep = NULL;
  return OBJ_VAL(list);
}

static Value pairsMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_MAP)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjMap *map = AS_MAP(args[0]);
  ObjList *list = newList(vm.klass.list);
  push(OBJ_VAL(list));
  for (int i = 0; i < map->items.capacity; i++) {
    Entry entry = map->items.entries[i];
    if (entry.key == NULL) {
      continue;
    }
    ObjInstance *pair = newInstance(vm.klass.pair);
    push(OBJ_VAL(pair));
    defineNativeInstanceField(pair, "key", 3, OBJ_VAL(entry.key));
    defineNativeInstanceField(pair, "value", 5, entry.value);
    pushToList(list, pop());
  }
  return pop();
}

static Value hasKeyMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_MAP, ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjMap *map = AS_MAP(args[0]);
  ObjString *key = AS_STRING(args[1]);
  Value value;
  if (tableGet(&map->items, key, &value)) {
    return TRUE_VAL;
  }
  return FALSE_VAL;
}

static Value getMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_MAP, ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjMap *map = AS_MAP(args[0]);
  ObjString *key = AS_STRING(args[1]);
  Value value;
  if (tableGet(&map->items, key, &value)) {
    return value;
  }
  return NIL_VAL;
}

static Value setMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_VARIADIC, ARG_MAP, ARG_STRING,
                 ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjMap *map = AS_MAP(args[0]);
  ObjString *key = AS_STRING(args[1]);
  if (tableSet(&map->items, key, args[2])) {
    return args[2];
  }
  return NIL_VAL;
}

static Value deleteMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_MAP, ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjMap *map = AS_MAP(args[0]);
  ObjString *key = AS_STRING(args[1]);
  if (tableDelete(&map->items, key)) {
    return TRUE_VAL;
  }
  return FALSE_VAL;
}

static Value initStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_VARIADIC, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjString *string = NULL;
  if (IS_STRING(args[0])) {
    string = AS_STRING(args[0]);
  } else if (IS_KLASS(args[0])) {
    if (argCount == 1) {
      string = copyEscString("", 0, &vm.strings, AS_KLASS(args[0]));
    } else if (argCount == 2) {
      if (IS_STRING(args[1])) {
        string = AS_STRING(args[1]);
        string->klass = AS_KLASS(args[0]);
      } else if (IS_NUMBER(args[1])) {
        int d_len = snprintf(NULL, 0, "%.15g", AS_NUMBER(args[1]));
        char d_str[d_len + 1];
        sprintf(d_str, "%.15g", AS_NUMBER(args[1]));
        string = copyEscString(d_str, d_len, &vm.strings, AS_KLASS(args[0]));

      } else {
        runtimeError("Expected argument to be string or number.");
        vm.shouldPanic = true;
        return NIL_VAL;
      }
    } else {
      runtimeError("Expected 1 argument but got %d.", argCount);
      vm.shouldPanic = true;
      return NIL_VAL;
    }
  } else {
    runtimeError("Unexpect base for String init.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }

  return OBJ_VAL(string);
}

static Value asNumberStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_VARIADIC, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  double num;
  int match = sscanf(AS_CSTRING(args[0]), "%lf", &num);
  if (match) {
    return NUMBER_VAL(num);
  }
  return NIL_VAL;
}

static Value lenStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjString *string = AS_STRING(args[0]);
  double count = (double)string->length;
  return NUMBER_VAL(count);
}

static Value containsStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_STRING, ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };

  char *string = AS_CSTRING(args[0]);
  char *term = AS_CSTRING(args[1]);

  return (strstr(string, term) != NULL) ? TRUE_VAL : FALSE_VAL;
}

static Value splitStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_STRING, ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjString *string = AS_STRING(args[0]);
  char *term = AS_CSTRING(args[1]);
  ObjList *list = newList(vm.klass.list);
  push(OBJ_VAL(list));
  char *lastSplit = string->chars;
  ObjString *str = NULL;

  while (true) {
    char *cp = strstr(lastSplit, term);
    if (cp == NULL) {
      str = copyString(lastSplit,
                       (int)(&string->chars[string->length] - lastSplit),
                       &vm.strings);
      pushToList(list, OBJ_VAL(str));
      return pop();
    }

    str = copyString(lastSplit, (int)(cp - lastSplit), &vm.strings);
    pushToList(list, OBJ_VAL(str));
    lastSplit = cp + 1;
  }
}

static Value isNumberNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_NUMBER(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_STRING(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isInstNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_INSTANCE(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isKlassNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_KLASS(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_LIST(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_MAP(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isBoolNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_BOOL(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isNilNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_NIL(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isFuncNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_CLOSURE(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isInstOfNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };

  if (!IS_INSTANCE(args[1])) {
    return FALSE_VAL;
  }

  if (IS_KLASS(args[2])) {
    return AS_INSTANCE(args[1])->klass == AS_KLASS(args[2]) ? TRUE_VAL
                                                            : FALSE_VAL;
  }
  if (IS_INSTANCE(args[2])) {
    return AS_INSTANCE(args[1])->klass == AS_INSTANCE(args[2])->klass
               ? TRUE_VAL
               : FALSE_VAL;
  }

  return FALSE_VAL;
}

static Value promptNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  char *input = readline(AS_CSTRING(args[1]));
  push(OBJ_VAL(copyString(input, strlen(input), &vm.strings)));
  free(input);
  return pop();
}

static Value isErrorNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };

  if (IS_INSTANCE(args[1])) {
    ObjInstance *err = AS_INSTANCE(args[1]);
    Value isError;
    if (tableGet(&err->fields, vm.string.isError, &isError)) {
      if (IS_BOOL(isError) && AS_BOOL(isError)) {
        return TRUE_VAL;
      }
    }
  }
  return FALSE_VAL;
}

static Value initErrorNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjInstance *err = NULL;
  if (IS_KLASS(args[0])) {
    err = newInstance(AS_KLASS(args[0]));
  } else if (IS_INSTANCE(args[0])) {
    err = AS_INSTANCE(args[0]);
  } else {
    runtimeError("Could not init error.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  push(OBJ_VAL(err));
  setNativeInstanceField(err, vm.string.message, args[1]);
  setNativeInstanceField(err, vm.string.isError, TRUE_VAL);
  return pop();
}

static Value absMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(fabs(AS_NUMBER(args[1])));
}

static Value acosMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(acos(AS_NUMBER(args[1])));
}

static Value acoshMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(acosh(AS_NUMBER(args[1])));
}

static Value asinMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(asin(AS_NUMBER(args[1])));
}

static Value asinhMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(asinh(AS_NUMBER(args[1])));
}

static Value atanMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(atan(AS_NUMBER(args[1])));
}

static Value atan2MathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER,
                 ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(atan2(AS_NUMBER(args[1]), AS_NUMBER(args[2])));
}

static Value atanhMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(atanh(AS_NUMBER(args[1])));
}

static Value cbrtMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(cbrt(AS_NUMBER(args[1])));
}

static Value ceilMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(ceil(AS_NUMBER(args[1])));
}

static Value cosMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(cos(AS_NUMBER(args[1])));
}

static Value coshMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(cosh(AS_NUMBER(args[1])));
}

static Value expMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(exp(AS_NUMBER(args[1])));
}

static Value expm1MathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(expm1(AS_NUMBER(args[1])));
}

static Value floorMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(floor(AS_NUMBER(args[1])));
}

static Value hypotMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER,
                 ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(hypot(AS_NUMBER(args[1]), AS_NUMBER(args[2])));
}

static Value logMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(log(AS_NUMBER(args[1])));
}

static Value log10MathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(log10(AS_NUMBER(args[1])));
}

static Value log1pMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(log1p(AS_NUMBER(args[1])));
}

static Value log2MathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(log2(AS_NUMBER(args[1])));
}

static Value maxMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_VARIADIC, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  double max = DBL_MIN;
  for (int i = 1; i < argCount; i++) {
    double x = AS_NUMBER(args[i]);
    max = max < x ? x : max;
  }
  return NUMBER_VAL(max);
}

static Value minMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_VARIADIC, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  double min = DBL_MAX;
  for (int i = 1; i < argCount; i++) {
    double x = AS_NUMBER(args[i]);
    min = min > x ? x : min;
  }
  return NUMBER_VAL(min);
}

static Value powMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER,
                 ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(pow(AS_NUMBER(args[1]), AS_NUMBER(args[2])));
}

static Value randomMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL((double)rand() / (double)((unsigned)RAND_MAX + 1));
}

static Value roundMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(round(AS_NUMBER(args[1])));
}

static Value sinMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(sin(AS_NUMBER(args[1])));
}

static Value sinhMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(sinh(AS_NUMBER(args[1])));
}

static Value sqrtMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(sqrt(AS_NUMBER(args[1])));
}

static Value tanMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(tan(AS_NUMBER(args[1])));
}

static Value tanhMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(tanh(AS_NUMBER(args[1])));
}

static Value truncMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL(trunc(AS_NUMBER(args[1])));
}

static ObjKlass *createFileClass() {
  ObjKlass *fileKlass = defineKlass("File", 4, OBJ_FILE);
  defineNativeKlassMethod(fileKlass, "init", 4, initFileNative);
  defineNativeKlassMethod(fileKlass, "close", 5, closeFileNative);
  defineNativeKlassMethod(fileKlass, "write", 5, writeFileNative);
  defineNativeKlassMethod(fileKlass, "read", 4, readFileNative);

  return fileKlass;
}

static ObjKlass *createListClass() {
  ObjKlass *listKlass = defineKlass("List", 4, OBJ_LIST);
  defineNativeKlassMethod(listKlass, "init", 4, initListNative);
  defineNativeKlassMethod(listKlass, "push", 4, pushListNative);
  defineNativeKlassMethod(listKlass, "pop", 3, popListNative);
  defineNativeKlassMethod(listKlass, "len", 3, lenListNative);
  defineNativeKlassMethod(listKlass, "remove", 6, removeListNative);
  defineNativeKlassMethod(listKlass, "join", 4, joinListNative);

  return listKlass;
}

static ObjKlass *createMapClass() {
  ObjKlass *mapKlass = defineKlass("Map", 3, OBJ_MAP);
  defineNativeKlassMethod(mapKlass, "init", 4, initMapNative);
  defineNativeKlassMethod(mapKlass, "keys", 4, keysMapNative);
  defineNativeKlassMethod(mapKlass, "values", 6, valuesMapNative);
  defineNativeKlassMethod(mapKlass, "pairs", 5, pairsMapNative);
  defineNativeKlassMethod(mapKlass, "has", 3, hasKeyMapNative);
  defineNativeKlassMethod(mapKlass, "get", 3, getMapNative);
  defineNativeKlassMethod(mapKlass, "set", 3, setMapNative);
  defineNativeKlassMethod(mapKlass, "delete", 6, deleteMapNative);

  return mapKlass;
}

static ObjKlass *createPairClass() {
  ObjKlass *pairKlass = defineKlass("Pair", 4, OBJ_INSTANCE);

  return pairKlass;
}

static ObjKlass *createStringClass() {
  ObjKlass *stringKlass = defineKlass("String", 6, OBJ_STRING);
  defineNativeKlassMethod(stringKlass, "init", 4, initStringNative);
  defineNativeKlassMethod(stringKlass, "len", 3, lenStringNative);
  defineNativeKlassMethod(stringKlass, "contains", 8, containsStringNative);
  defineNativeKlassMethod(stringKlass, "split", 5, splitStringNative);
  defineNativeKlassMethod(stringKlass, "asnum", 5, asNumberStringNative);

  return stringKlass;
}

static ObjKlass *createErrorClass() {
  ObjKlass *errKlass = defineKlass("Error", 5, OBJ_INSTANCE);
  defineNativeKlassMethod(errKlass, "init", 4, initErrorNative);

  return errKlass;
}

void registerNatives() {
  defineNative("tick", 4, tickNative);
  defineNative("sleep", 5, sleepNative);
  defineNative("exit", 4, exitNative);
  defineNative("open", 4, initFileNative);
  defineNative("iserr", 5, isErrorNative);
  defineNative("instof", 6, isInstOfNative);
  defineNative("panic", 5, panicNative);
  defineNative("isnum", 5, isNumberNative);
  defineNative("isstr", 5, isStringNative);
  defineNative("isinst", 6, isInstNative);
  defineNative("isclass", 7, isKlassNative);
  defineNative("islist", 6, isListNative);
  defineNative("ismap", 5, isMapNative);
  defineNative("isbool", 6, isBoolNative);
  defineNative("isnil", 5, isNilNative);
  defineNative("isfn", 4, isFuncNative);
  defineNative("prompt", 6, promptNative);
}

void registerBuiltInKlasses() {
  vm.klass.list = createListClass();
  vm.klass.file = createFileClass();
  vm.klass.string = createStringClass();
  vm.klass.error = createErrorClass();
  vm.klass.map = createMapClass();
  vm.klass.pair = createPairClass();
}

void registerRequestNatives() {  
  static bool isRegistered = false;
  if (isRegistered)
    return;
  isRegistered = true;
  curl_global_init(CURL_GLOBAL_DEFAULT);

  ObjInstance *requestInstance =
      defineInstance(defineKlass("Request", 7, OBJ_INSTANCE), "Request", 7);
  defineNativeInstanceMethod(requestInstance, "get", 3, getRequestNative);
  defineNativeInstanceMethod(requestInstance, "post", 4, postRequestNative);
}

void registerJsonNatives() {
  static bool isRegistered = false;
  if (isRegistered)
    return;
  isRegistered = true;

  ObjInstance *jsonInstance =
      defineInstance(defineKlass("JSON", 4, OBJ_INSTANCE), "JSON", 4); 
  defineNativeInstanceMethod(jsonInstance, "parse", 5, parseJsonNative);
}

void registerMathNatives() {
  static bool isRegistered = false;
  if (isRegistered)
    return;
  isRegistered = true;

  ObjInstance *mathInstance =
      defineInstance(defineKlass("Math", 4, OBJ_INSTANCE), "Math", 4);
  defineNativeInstanceField(mathInstance, "E", 1, NUMBER_VAL(2.718));
  defineNativeInstanceField(mathInstance, "LN10", 4, NUMBER_VAL(2.303));
  defineNativeInstanceField(mathInstance, "LN2", 3, NUMBER_VAL(0.693));
  defineNativeInstanceField(mathInstance, "LOG10E", 6, NUMBER_VAL(0.434));
  defineNativeInstanceField(mathInstance, "LOG2E", 5, NUMBER_VAL(1.443));
  defineNativeInstanceField(mathInstance, "PI", 2, NUMBER_VAL(3.14159));
  defineNativeInstanceField(mathInstance, "SQRT1_2", 7, NUMBER_VAL(0.707));
  defineNativeInstanceField(mathInstance, "SQRT2", 5, NUMBER_VAL(1.414));

  defineNativeInstanceMethod(mathInstance, "abs", 3, absMathNative);
  defineNativeInstanceMethod(mathInstance, "acos", 4, acosMathNative);
  defineNativeInstanceMethod(mathInstance, "acosh", 5, acoshMathNative);
  defineNativeInstanceMethod(mathInstance, "asin", 4, asinMathNative);
  defineNativeInstanceMethod(mathInstance, "asinh", 5, asinhMathNative);
  defineNativeInstanceMethod(mathInstance, "atan", 4, atanMathNative);
  defineNativeInstanceMethod(mathInstance, "atan2", 5, atan2MathNative);
  defineNativeInstanceMethod(mathInstance, "atanh", 5, atanhMathNative);
  defineNativeInstanceMethod(mathInstance, "cbrt", 4, cbrtMathNative);
  defineNativeInstanceMethod(mathInstance, "ceil", 4, ceilMathNative);
  defineNativeInstanceMethod(mathInstance, "cos", 3, cosMathNative);
  defineNativeInstanceMethod(mathInstance, "cosh", 4, coshMathNative);
  defineNativeInstanceMethod(mathInstance, "exp", 3, expMathNative);
  defineNativeInstanceMethod(mathInstance, "expm1", 5, expm1MathNative);
  defineNativeInstanceMethod(mathInstance, "floor", 5, floorMathNative);
  defineNativeInstanceMethod(mathInstance, "hypot", 5, hypotMathNative);
  defineNativeInstanceMethod(mathInstance, "log", 3, logMathNative);
  defineNativeInstanceMethod(mathInstance, "log10", 5, log10MathNative);
  defineNativeInstanceMethod(mathInstance, "log1p", 5, log1pMathNative);
  defineNativeInstanceMethod(mathInstance, "log2", 4, log2MathNative);
  defineNativeInstanceMethod(mathInstance, "max", 3, maxMathNative);
  defineNativeInstanceMethod(mathInstance, "min", 3, minMathNative);
  defineNativeInstanceMethod(mathInstance, "pow", 3, powMathNative);
  defineNativeInstanceMethod(mathInstance, "random", 6, randomMathNative);
  defineNativeInstanceMethod(mathInstance, "round", 5, roundMathNative);
  defineNativeInstanceMethod(mathInstance, "sin", 3, sinMathNative);
  defineNativeInstanceMethod(mathInstance, "sinh", 4, sinhMathNative);
  defineNativeInstanceMethod(mathInstance, "sqrt", 4, sqrtMathNative);
  defineNativeInstanceMethod(mathInstance, "tan", 3, tanMathNative);
  defineNativeInstanceMethod(mathInstance, "tanh", 4, tanhMathNative);
  defineNativeInstanceMethod(mathInstance, "trunc", 5, truncMathNative);
}
