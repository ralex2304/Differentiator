#ifndef SIMPLIFICATION_H_
#define SIMPLIFICATION_H_

#include <assert.h>

#include "../diff_objects.h"
#include "../utils/statuses.h"
#include "eval.h"

#include "../config.h"
#include TREE_INCLUDE
#include "../TreeAddon/TreeAddon.h"

Status::Statuses diff_simplify(DiffData* diff_data);

Status::Statuses diff_simplify_traversal(DiffData* diff_data, TreeNode** node, bool* is_simple);

#endif //< #ifndef SIMPLIFICATION_H_
