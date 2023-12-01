#ifndef TEXT_TREE_PARSER_H_
#define TEXT_TREE_PARSER_H_

#include <assert.h>
#include <ctype.h>

#include "../utils/statuses.h"
#include "../file/file.h"
#include "../diff_objects.h"

#include "../config.h"
#include TREE_INCLUDE

Status::Statuses text_tree_parser(DiffData* diff_data, char* text);

Status::Statuses text_tree_save(Tree* tree, const char* filename);

#endif //< #ifndef TEXT_TREE_PARSER_H_
