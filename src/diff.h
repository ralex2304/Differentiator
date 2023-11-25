#ifndef DIFF_H_
#define DIFF_H_

#include <assert.h>

#include "utils/statuses.h"
#include "config.h"
#include "diff_math.h"
#include "file/file.h"
#include "text_tree_parser/text_tree_parser.h"

#ifdef DEBUG

#include "TreeDebug/TreeDebug.h"

#else //< #ifndef DEBUG

#include "Tree/Tree.h"

#endif //< #ifdef DEBUG

Status::Statuses diff_proccess(const char* input_filename);

Status::Statuses diff_read_tree(DiffData* diff_data, const char* input_file);

#endif //< #ifndef DIFF_H_
