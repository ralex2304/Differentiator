#ifndef DIFF_H_
#define DIFF_H_

#include <assert.h>

#include "utils/statuses.h"
#include "diff_objects.h"
#include "file/file.h"
#include "text_tree_parser/text_tree_parser.h"
#include "diff_math/differentiation.h"
#include "diff_math/simplification.h"
#include "interface.h"
#include "tex/tex.h"

#include "config.h"
#include TREE_INCLUDE


Status::Statuses diff_proccess(const char* input_filename, const char* tex_filename,
                               bool substitute_vals);

Status::Statuses diff_read_tree(DiffData* diff_data, char** text, const char* input_file);

#endif //< #ifndef DIFF_H_
