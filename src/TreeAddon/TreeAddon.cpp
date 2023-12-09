#include "TreeAddon.h"

#include "../dsl.h"

static Status::Statuses tree_copy_subtree_traversal_(DiffData* diff_data, Tree* tree, TreeNode* src,
                                                     TreeNode** dest, TreeNode* parent,
                                                     bool* is_simple);

static Status::Statuses tree_copy_subtree_traversal_(DiffData* diff_data, Tree* tree, TreeNode* src,
                                                     TreeNode** dest, TreeNode* parent,
                                                     bool* is_simple) {
    assert(diff_data);
    assert(tree);
    assert(src);
    assert(dest);
    assert(*dest == nullptr);
    assert(is_simple);

    // Not macros, because inserting not in diff_data.tree
    if (tree_insert(tree, dest, parent, ELEM(src)) != Tree::OK)
        return Status::TREE_ERROR;

    if (VAL_IS_SIMPLE(src)) {
        *is_simple = true;
        return Status::NORMAL_WORK;

    } else if (!TYPE_IS_OPER(src)) {
        *is_simple = false;
        return Status::NORMAL_WORK;
    }

    bool is_l_simple = false;
    bool is_r_simple = false;

    if (TYPE_IS_OPER(src) && IS_BINARY(src))
        STATUS_CHECK(tree_copy_subtree_traversal_(diff_data, tree, src->left, L(*dest), *dest,
                                                  &is_l_simple));
    else
        is_l_simple = true;

    STATUS_CHECK(tree_copy_subtree_traversal_(diff_data, tree, src->right, R(*dest), *dest,
                                              &is_r_simple));

    *is_simple = is_l_simple && is_r_simple;

    return Status::NORMAL_WORK;
}

Status::Statuses tree_copy_subtree(DiffData* diff_data, TreeNode* src, TreeNode** dest, size_t* size,
                                   bool* is_simple) {
    assert(diff_data);
    assert(src);
    assert(dest);
    assert(*dest == nullptr);
    assert(size);
    // is_simple can be nullptr

    Tree copy = {};
    DSL_TREE_CTOR(&copy);

    bool placeholder = false;
    STATUS_CHECK(tree_copy_subtree_traversal_(diff_data, &copy, src, &copy.root, nullptr,
                                              is_simple != nullptr ? is_simple
                                                                   : &placeholder));

    *size = copy.size;
    *dest = copy.root;

    copy.size = 0;
    copy.root = nullptr;

    DSL_TREE_DTOR(&copy);

    return Status::NORMAL_WORK;
}


Status::Statuses tree_dtor_untied_subtree(TreeNode** node) {
    assert(node);

    if (*node == nullptr) return Status::NORMAL_WORK;

    STATUS_CHECK(tree_dtor_untied_subtree(L(*node)));
    STATUS_CHECK(tree_dtor_untied_subtree(R(*node)));

    assert(*L(*node) == nullptr);
    assert(*R(*node) == nullptr);

    (*node)->parent = nullptr;
    diff_elem_dtor(ELEM(*node));

    FREE(*node);

    return Status::NORMAL_WORK;
}

Status::Statuses tree_reconnect_node(DiffData* diff_data, TreeNode** dest, TreeNode* src) {
    assert(diff_data);
    assert(dest);
    assert(*dest);
    assert(src);

    TreeNode* node_tmp = *dest;

    if (L(node_tmp) != nullptr && *L(node_tmp) != src) TREE_DELETE_NODE(L(node_tmp));
    if (R(node_tmp) != nullptr && *R(node_tmp) != src) TREE_DELETE_NODE(R(node_tmp));

    src->parent = (*dest)->parent;
    *dest = src;

    *L(node_tmp) = nullptr;
    *R(node_tmp) = nullptr;

    if (tree_node_dtor(&diff_data->tree, &node_tmp) != Tree::OK)
        return Status::TREE_ERROR;

    diff_data->tree.size -= 1;

    TREE_CHANGED = true;

    return Status::NORMAL_WORK;
}
