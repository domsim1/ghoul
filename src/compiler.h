#ifndef ghoul_compiler_h
#define ghoul_compiler_h

#include "object.h"

ObjFunction *compile(const char *source, const char *file);
void markCompilerRoots();

#endif
