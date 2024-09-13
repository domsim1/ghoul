#ifndef ghoul_compiler_h
#define ghoul_compiler_h

#include "object.h"
#include "vm.h"

ObjFunction *compile(const char *source);
void markCompilerRoots();

#endif
