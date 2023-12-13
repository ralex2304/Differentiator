#ifndef TEX_H_
#define TEX_H_

#include <assert.h>

#include "../diff_objects.h"
#include "../utils/statuses.h"
#include "../config.h"
#include TREE_INCLUDE

#include "phrases.h"
#include "../utils/rand_wrapper.h"


Status::Statuses tex_dump_begin(DiffData* diff_data, const char* tex_directory, const DiffMode mode);

Status::Statuses tex_dump_add(DiffData* diff_data, bool print_phrase = true);

Status::Statuses tex_dump_end(DiffData* diff_data);

Status::Statuses tex_dump_given_expression(DiffData* diff_data);

Status::Statuses tex_dump_section_diff(DiffData* diff_data);

Status::Statuses tex_dump_section_simplify(DiffData* diff_data);

Status::Statuses tex_dump_section_subst(DiffData* diff_data);

Status::Statuses tex_dump_section_taylor(DiffData* diff_data, const size_t degree, const double point);

Status::Statuses tex_subsection_n_derivative_(DiffData* diff_data, const size_t n);

Status::Statuses tex_print_evaled_value(DiffData* diff_data, const double value);

Status::Statuses tex_insert_plot(DiffData* diff_data, const char* filename);

#endif //< #ifndef TEX_H_
