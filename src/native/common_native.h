#ifndef ghoul_common_native
#define ghoul_common_native
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <readline/readline.h>
#include <curl/curl.h>

#include "../memory.h"
#include "../object.h"
#include "../table.h"
#include "../value.h"
#include "../vm.h"

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
  ARG_BOOL,
} ArgTypes;

typedef enum {
  NATIVE_VARIADIC,
  NATIVE_NORMAL,
} NativeType;

bool checkArgCount(int argCount, int expectedCount);
bool checkArgs(int argCount, int expectedCount, Value *args, NativeType type, ...); 
void defineNative(const char *name, int len, NativeFn function); 
ObjInstance *defineInstance(ObjKlass *klass, const char *name, int len);
ObjKlass *defineKlass(const char *name, int len, ObjType base);
void defineNativeKlassMethod(ObjKlass *klass, const char *name, int len, NativeFn function); 
void defineNativeInstanceMethod(ObjInstance *instance, const char *name, int len, NativeFn function);
void setNativeInstanceField(ObjInstance *instance, ObjString *string, Value value); 
void defineNativeInstanceField(ObjInstance *instance, const char *string, int len, Value value); 
Value readNativeInstanceField(ObjInstance *instance, const char *name, int len);
#endif
