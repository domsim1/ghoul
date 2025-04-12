#include "common_native.h"

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
