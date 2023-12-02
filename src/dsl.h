#ifndef DIFF_DSL_H_
#define DIFF_DSL_H_

#include "diff_objects.h"
#include "config.h"
#include TREE_INCLUDE

#include <math.h>


#define L(node_) (&(node_)->left)
#define R(node_) (&(node_)->right)

#define VdU(node_) L(node_)
#define UdV(node_) R(node_)

#define V(node_)  R(*VdU(node_))
#define U(node_)  R(*UdV(node_))
#define dU(node_) L(*VdU(node_))
#define dV(node_) L(*UdV(node_))

#define VdU_UdV_BUILD(type_) STATUS_CHECK(diff_do_diff_vdu_udv_copy_(diff_data, node, type_))

#define VdU_UdV_BUILD_AND_COUNT_SIZE(type_, size_ptr_)                                  \
            STATUS_CHECK(diff_do_diff_vdu_udv_copy_(diff_data, node, type_, size_ptr_))

#define DO_DIFF(node_) STATUS_CHECK(diff_do_diff_traversal(diff_data, node_))

#define ELEM(node_)  ((DiffElem*)((node_)->elem))

#define NODE_DATA(node_) (&ELEM(node_)->data)
#define NODE_TYPE(node_) (&ELEM(node_)->type)

#define NUM_VAL(node_) (&NODE_DATA(node_)->num)

#define VAR_NUM(node_) (&NODE_DATA(node_)->var)
#define VAR_VAL(node_) (&diff_data->vars.arr[*VAR_NUM(node_)].val)

#define OPER_NUM(node_) (&NODE_DATA(node_)->oper)
#define OPER(node_)     (DIFF_OPERS + (size_t)(*OPER_NUM(node_)))

#define NODE_IS_NUM(node_)  (*NODE_TYPE(node_) == DiffElemType::NUM)
#define NODE_IS_VAR(node_)  (*NODE_TYPE(node_) == DiffElemType::VAR)
#define NODE_IS_OPER(node_) (*NODE_TYPE(node_) == DiffElemType::OPER)

#define NUM_ELEM(val_)      {.type = DiffElemType::NUM,  .data = {.num  = val_}}
#define OPER_ELEM(oper_)    {.type = DiffElemType::OPER, .data = {.oper = oper_}}

#define TREE_INSERT(dest_, parent_, elem_)                                                  \
            do {                                                                            \
                DiffElem new_elem_ = elem_;                                                 \
                if (tree_insert(&diff_data->tree, dest_, parent_, &new_elem_) != Tree::OK)  \
                    return Status::TREE_ERROR;                                              \
            } while(0)


#define TREE_DELETE_NODE(node_)                                             \
            if (tree_delete(&diff_data->tree, node_, false) != Tree::OK)    \
                return Status::NORMAL_WORK

#define TREE_DELETE_SUBTREE(node_)                                      \
            if (tree_delete(&diff_data->tree, node_, true) != Tree::OK) \
                return Status::NORMAL_WORK

#define COPY_AND_REPLACE_WITH_MUL(target_, copy_, size_)                                            \
            TreeNode* copy_ = nullptr;                                                              \
            size_t size_ = 0;                                                                       \
                                                                                                    \
            STATUS_CHECK(diff_do_diff_copy_and_replace_with_mul_(diff_data, target_, &copy_, &size_))

#define INSERT_SUBTREE(parent_, src_, size_)    \
            /* = */ src_;                       \
            diff_data->tree.size += size_;      \
            (src_)->parent = parent_

#define COPY_SUBTREE(src_, dest_, size_)                        \
            STATUS_CHECK(tree_copy_subtree(src_, dest_, size_))

#define COPY_SUBTREE_AND_CHECK_SIMPLICITY(src_, dest_, size_, is_simple_)   \
            STATUS_CHECK(tree_copy_subtree(src_, dest_, size_, is_simple_))

#define UNTIE_SUBTREE(node_, size_)         \
            /* = */ node_;                  \
            diff_data->tree.size -= size_;  \
            node_ = nullptr

#define DELETE_SUBTREE_COPY(copy_, size_)                                       \
            do {                                                                \
                STATUS_CHECK(tree_dtor_subtree_copy(&diff_data->tree, copy_));  \
                size_ = 0;                                                      \
            } while(0)


#define RECONNECT(dest_, src_) STATUS_CHECK(tree_reconnect_node(diff_data, dest_, src_))

#define TREE_REPLACE_SUBTREE_WITH_NUM(node_, val_)          \
            do {                                            \
                TreeNode* parent_ = (*(node_))->parent;     \
                                                            \
                TREE_DELETE_SUBTREE(node_);                 \
                TREE_INSERT(node_, parent_, NUM_ELEM(val_));\
            } while(0)

inline bool dsl_is_double_equal(const double a, const double b) {
    static const double EPSILON = 0.000001;

    return abs(a - b) < EPSILON;
}

#define IS_DOUBLE_EQ(a_, b_) dsl_is_double_equal(a_, b_)

#define NODE_VAL_EQUALS(node_, val_) (NODE_IS_NUM(node_) && IS_DOUBLE_EQ(*NUM_VAL(node_), val_))

#endif //< #ifndef DIFF_DSL_H_
