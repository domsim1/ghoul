#include "common_native.h"

struct NetworkData {
  char *response;
  size_t size;
};

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
