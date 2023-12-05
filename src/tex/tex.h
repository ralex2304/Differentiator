#ifndef TEX_H_
#define TEX_H_

#include <assert.h>

#include "../diff_objects.h"
#include "../utils/statuses.h"
#include "../config.h"
#include TREE_INCLUDE

#include "phrases.h"
#include "../utils/rand_wrapper.h"


Status::Statuses tex_dump_begin(DiffData* diff_data, const char* tex_directory);

Status::Statuses tex_dump_add(DiffData* diff_data, bool print_phrase = true);

Status::Statuses tex_dump_end(DiffData* diff_data, const char* tex_directory);

Status::Statuses tex_dump_given_expression(DiffData* diff_data);

Status::Statuses tex_dump_section_diff(DiffData* diff_data);

Status::Statuses tex_dump_section_simplify(DiffData* diff_data);

Status::Statuses tex_dump_section_subst(DiffData* diff_data);

#endif //< #ifndef TEX_H_
