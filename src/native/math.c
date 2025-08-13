#include "common_native.h"

static Value absMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(fabs(AS_NUMBER(args[1])));
}

static Value acosMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(acos(AS_NUMBER(args[1])));
}

static Value acoshMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(acosh(AS_NUMBER(args[1])));
}

static Value asinMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(asin(AS_NUMBER(args[1])));
}

static Value asinhMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(asinh(AS_NUMBER(args[1])));
}

static Value atanMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(atan(AS_NUMBER(args[1])));
}

static Value atan2MathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER,
                 ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(atan2(AS_NUMBER(args[1]), AS_NUMBER(args[2])));
}

static Value atanhMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(atanh(AS_NUMBER(args[1])));
}

static Value cbrtMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(cbrt(AS_NUMBER(args[1])));
}

static Value ceilMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(ceil(AS_NUMBER(args[1])));
}

static Value cosMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(cos(AS_NUMBER(args[1])));
}

static Value coshMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(cosh(AS_NUMBER(args[1])));
}

static Value expMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(exp(AS_NUMBER(args[1])));
}

static Value expm1MathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(expm1(AS_NUMBER(args[1])));
}

static Value floorMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(floor(AS_NUMBER(args[1])));
}

static Value hypotMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER,
                 ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(hypot(AS_NUMBER(args[1]), AS_NUMBER(args[2])));
}

static Value logMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(log(AS_NUMBER(args[1])));
}

static Value log10MathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(log10(AS_NUMBER(args[1])));
}

static Value log1pMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(log1p(AS_NUMBER(args[1])));
}

static Value log2MathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(log2(AS_NUMBER(args[1])));
}

static Value maxMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_VARIADIC, ARG_ANY)) {
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
    return NIL_VAL;
  };
  return NUMBER_VAL(pow(AS_NUMBER(args[1]), AS_NUMBER(args[2])));
}

static Value randomMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
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
    return NIL_VAL;
  };
  return NUMBER_VAL(sin(AS_NUMBER(args[1])));
}

static Value sinhMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(sinh(AS_NUMBER(args[1])));
}

static Value sqrtMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(sqrt(AS_NUMBER(args[1])));
}

static Value tanMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(tan(AS_NUMBER(args[1])));
}

static Value tanhMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(tanh(AS_NUMBER(args[1])));
}

static Value truncMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(trunc(AS_NUMBER(args[1])));
}

static Value isFiniteMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  double x = AS_NUMBER(args[1]);
  return isfinite(x) ? TRUE_VAL : FALSE_VAL;
}

static Value isNaNMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  double x = AS_NUMBER(args[1]);
  return isnan(x) ? TRUE_VAL : FALSE_VAL;
}

static Value isInfiniteMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  double x = AS_NUMBER(args[1]);
  return isinf(x) ? TRUE_VAL : FALSE_VAL;
}

static Value signMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  double x = AS_NUMBER(args[1]);
  if (isnan(x)) return NUMBER_VAL(x);
  if (x > 0.0) return NUMBER_VAL(1.0);
  if (x < 0.0) return NUMBER_VAL(-1.0);
  return NUMBER_VAL(x); // Returns +0.0 or -0.0
}

static Value fmodMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(fmod(AS_NUMBER(args[1]), AS_NUMBER(args[2])));
}

static Value remainderMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(remainder(AS_NUMBER(args[1]), AS_NUMBER(args[2])));
}

static Value modfMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  double intpart;
  double fracpart = modf(AS_NUMBER(args[1]), &intpart);
  
  ObjList *result = newList(vm.klass.list);
  push(OBJ_VAL(result));
  pushToList(result, NUMBER_VAL(intpart));
  pushToList(result, NUMBER_VAL(fracpart));
  return pop();
}

static Value clampMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 4, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  double x = AS_NUMBER(args[1]);
  double min_val = AS_NUMBER(args[2]);
  double max_val = AS_NUMBER(args[3]);
  
  if (min_val > max_val) {
    runtimeError("clamp: min value cannot be greater than max value.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  if (x < min_val) return NUMBER_VAL(min_val);
  if (x > max_val) return NUMBER_VAL(max_val);
  return NUMBER_VAL(x);
}

static Value lerpMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 4, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  double a = AS_NUMBER(args[1]);
  double b = AS_NUMBER(args[2]);
  double t = AS_NUMBER(args[3]);
  
  return NUMBER_VAL(a + t * (b - a));
}

static Value mapMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 6, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  double x = AS_NUMBER(args[1]);
  double in_min = AS_NUMBER(args[2]);
  double in_max = AS_NUMBER(args[3]);
  double out_min = AS_NUMBER(args[4]);
  double out_max = AS_NUMBER(args[5]);
  
  if (in_min == in_max) {
    runtimeError("map: input range cannot be zero.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  return NUMBER_VAL(out_min + (x - in_min) * (out_max - out_min) / (in_max - in_min));
}

static Value degreesMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  double radians = AS_NUMBER(args[1]);
  return NUMBER_VAL(radians * 180.0 / M_PI);
}

static Value radiansMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  double degrees = AS_NUMBER(args[1]);
  return NUMBER_VAL(degrees * M_PI / 180.0);
}

static Value randomIntMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  int min_val = (int)AS_NUMBER(args[1]);
  int max_val = (int)AS_NUMBER(args[2]);
  
  if (min_val > max_val) {
    runtimeError("randomInt: min value cannot be greater than max value.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  int range = max_val - min_val + 1;
  int result = min_val + (rand() % range);
  return NUMBER_VAL((double)result);
}

static Value randomRangeMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  double min_val = AS_NUMBER(args[1]);
  double max_val = AS_NUMBER(args[2]);
  
  if (min_val > max_val) {
    runtimeError("randomRange: min value cannot be greater than max value.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  double range = max_val - min_val;
  double random_01 = (double)rand() / (double)RAND_MAX;
  return NUMBER_VAL(min_val + random_01 * range);
}

static Value seedMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  unsigned int seed = (unsigned int)AS_NUMBER(args[1]);
  srand(seed);
  return NIL_VAL;
}

static Value clz32MathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  uint32_t x = (uint32_t)AS_NUMBER(args[1]);
  if (x == 0) return NUMBER_VAL(32);
  
  int count = 0;
  uint32_t mask = 0x80000000;
  while ((x & mask) == 0 && count < 32) {
    count++;
    mask >>= 1;
  }
  return NUMBER_VAL((double)count);
}

static Value imulMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  int32_t a = (int32_t)AS_NUMBER(args[1]);
  int32_t b = (int32_t)AS_NUMBER(args[2]);
  
  int64_t result = (int64_t)a * (int64_t)b;
  int32_t truncated = (int32_t)(result & 0xFFFFFFFF);
  return NUMBER_VAL((double)truncated);
}

static Value logbMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  double x = AS_NUMBER(args[1]);
  double base = AS_NUMBER(args[2]);
  
  if (x <= 0.0) {
    runtimeError("logb: argument must be positive.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  if (base <= 0.0 || base == 1.0) {
    runtimeError("logb: base must be positive and not equal to 1.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  return NUMBER_VAL(log(x) / log(base));
}

static Value gammaMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  double x = AS_NUMBER(args[1]);
  return NUMBER_VAL(tgamma(x));
}

static Value factorialMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  double n = AS_NUMBER(args[1]);
  
  if (n < 0 || n != floor(n)) {
    runtimeError("factorial: argument must be a non-negative integer.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  if (n > 170) {
    runtimeError("factorial: argument too large (would overflow).");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  return NUMBER_VAL(tgamma(n + 1));
}

static Value gcdMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  long long a = (long long)AS_NUMBER(args[1]);
  long long b = (long long)AS_NUMBER(args[2]);
  
  a = a < 0 ? -a : a;
  b = b < 0 ? -b : b;
  
  while (b != 0) {
    long long temp = b;
    b = a % b;
    a = temp;
  }
  
  return NUMBER_VAL((double)a);
}

static Value lcmMathNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  long long a = (long long)AS_NUMBER(args[1]);
  long long b = (long long)AS_NUMBER(args[2]);
  
  if (a == 0 || b == 0) {
    return NUMBER_VAL(0.0);
  }
  
  a = a < 0 ? -a : a;
  b = b < 0 ? -b : b;
  
  long long orig_a = a;
  long long orig_b = b;
  
  while (b != 0) {
    long long temp = b;
    b = a % b;
    a = temp;
  }
  
  long long gcd = a;
  long long lcm = (orig_a / gcd) * orig_b;
  
  return NUMBER_VAL((double)lcm);
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
  
  // Number classification functions
  defineNativeInstanceMethod(mathInstance, "isFinite", 8, isFiniteMathNative);
  defineNativeInstanceMethod(mathInstance, "isNaN", 5, isNaNMathNative);
  defineNativeInstanceMethod(mathInstance, "isInfinite", 10, isInfiniteMathNative);
  defineNativeInstanceMethod(mathInstance, "sign", 4, signMathNative);
  
  // Advanced rounding functions
  defineNativeInstanceMethod(mathInstance, "fmod", 4, fmodMathNative);
  defineNativeInstanceMethod(mathInstance, "remainder", 9, remainderMathNative);
  defineNativeInstanceMethod(mathInstance, "modf", 4, modfMathNative);
  
  // Statistical functions
  defineNativeInstanceMethod(mathInstance, "clamp", 5, clampMathNative);
  defineNativeInstanceMethod(mathInstance, "lerp", 4, lerpMathNative);
  defineNativeInstanceMethod(mathInstance, "map", 3, mapMathNative);
  
  // Angle conversion functions
  defineNativeInstanceMethod(mathInstance, "degrees", 7, degreesMathNative);
  defineNativeInstanceMethod(mathInstance, "radians", 7, radiansMathNative);
  
  // Enhanced random functions
  defineNativeInstanceMethod(mathInstance, "randomInt", 9, randomIntMathNative);
  defineNativeInstanceMethod(mathInstance, "randomRange", 11, randomRangeMathNative);
  defineNativeInstanceMethod(mathInstance, "seed", 4, seedMathNative);
  
  // Bitwise math functions
  defineNativeInstanceMethod(mathInstance, "clz32", 5, clz32MathNative);
  defineNativeInstanceMethod(mathInstance, "imul", 4, imulMathNative);
  
  // Logarithm with arbitrary base
  defineNativeInstanceMethod(mathInstance, "logb", 4, logbMathNative);
  
  // Gamma and special functions
  defineNativeInstanceMethod(mathInstance, "gamma", 5, gammaMathNative);
  defineNativeInstanceMethod(mathInstance, "factorial", 9, factorialMathNative);
  defineNativeInstanceMethod(mathInstance, "gcd", 3, gcdMathNative);
  defineNativeInstanceMethod(mathInstance, "lcm", 3, lcmMathNative);
}
