#ifndef ghoul_compiler_h
#define ghoul_compiler_h

#include "object.h"
#include "vm.h"

ObjFunction *compile(const char *source, const char *file);
void markCompilerRoots();
void eval(const char *source);

#endif
