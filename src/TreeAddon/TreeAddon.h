#ifndef TREE_ADDON_H_
#define TREE_ADDON_H_

#include <assert.h>

#include "../utils/statuses.h"
#include "../config.h"
#include TREE_INCLUDE

#include "../diff_objects.h"


Status::Statuses tree_copy_subtree(DiffData* diff_data, TreeNode* src, TreeNode** dest, size_t* size,
                                   bool* is_simple = nullptr);

Status::Statuses tree_dtor_untied_subtree(TreeNode** node);

Status::Statuses tree_reconnect_node(DiffData* diff_data, TreeNode** dest, TreeNode* src);

#endif //< #ifndef TREE_ADDON_H_
