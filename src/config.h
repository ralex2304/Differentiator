#ifndef CONFIG_H_
#define CONFIG_H_

const char* const DIFF_VAR_NAME = "x";

constexpr size_t TEX_DOUBLE_MAX_PRECISION = 3;

#define DEBUG //< enables debug mode

#ifdef DEBUG

#define TREE_INCLUDE "TreeDebug/TreeDebug.h"

#else //< #ifndef DEBUG

#define TREE_INCLUDE "Tree/TreeD.h"

#endif //< #ifdef DEBUG

#endif //< #ifndef CONFIG_H_
