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

#define ARG_VAR_NUM diff_data->vars.argument

#define VAR_NUM(node_)         (&NODE_DATA(node_)->var)
#define VAR(node_)             (&diff_data->vars.arr[*VAR_NUM(node_)])
#define VAR_VAL(node_)         (&VAR(node_)->val)
#define VAR_NAME(node_)        (&VAR(node_)->name)
#define VAR_IS_ARGUMENT(node_) ((ssize_t)*VAR_NUM(node_) == ARG_VAR_NUM)

#define OPER_NUM(node_)     (&NODE_DATA(node_)->oper)
#define OPER_TYPE(node_)    (&OPER(node_)->type)
#define OPER(node_)         (DIFF_OPERS + (size_t)(*OPER_NUM(node_)))

#define IS_BINARY(node_)    (*OPER_TYPE(node_) == BINARY)
#define IS_UNARY(node_)     (*OPER_TYPE(node_) == UNARY)

#define SUBST_VARS diff_data->simplify_substitute_vars

#define VAL_IS_SIMPLE(node_)    (TYPE_IS_NUM(node_) || (TYPE_IS_VAR(node_) && !VAR_IS_ARGUMENT(node_)))
#define VAL_IS_COUNTABLE(node_) (TYPE_IS_NUM(node_) || (SUBST_VARS && TYPE_IS_VAR(node_) && !isnan(*VAR_VAL(node_))))

#define TYPE_IS_NUM(node_)  (*NODE_TYPE(node_) == DiffElemType::NUM)
#define TYPE_IS_VAR(node_)  (*NODE_TYPE(node_) == DiffElemType::VAR)
#define TYPE_IS_OPER(node_) (*NODE_TYPE(node_) == DiffElemType::OPER)

#define NUM_ELEM(val_)      {.type = DiffElemType::NUM,  .data = {.num  = val_}}
#define OPER_ELEM(oper_)    {.type = DiffElemType::OPER, .data = {.oper = oper_}}
#define VAR_ELEM(var_num_)  {.type = DiffElemType::VAR,  .data = {.var = var_num_}}

#define NODE_VAL(node_) (TYPE_IS_NUM(node_) ? NUM_VAL(node_) : (TYPE_IS_VAR(node_) ? VAR_VAL(node_) : nullptr))

#define ROOT (&diff_data->tree.root)

#define NODE_IS_ROOT(node_) (node_ == *ROOT)

#define PARENT(node_) ((node_)->parent)

#define IS_L_CHILD(node_) (PARENT(node_) != nullptr && *L(PARENT(node_)) == node_)
#define IS_R_CHILD(node_) (PARENT(node_) != nullptr && *R(PARENT(node_)) == node_)

#define OPER_NUM_NEEDS_R_CHILD_PARENTHESIS(oper_num_) (oper_num_ == DiffOperNum::SUB || \
                                                       oper_num_ == DiffOperNum::MUL || \
                                                       oper_num_ == DiffOperNum::SIN || \
                                                       oper_num_ == DiffOperNum::COS || \
                                                       oper_num_ == DiffOperNum::LN)

#define OPER_NUM_NEEDS_L_CHILD_PARENTHESIS(oper_num_) (oper_num_ == DiffOperNum::MUL || \
                                                       oper_num_ == DiffOperNum::POW)

#define NEEDS_L_CHILD_PARENTHESIS(node_) (TYPE_IS_OPER(node_) && OPER_NUM_NEEDS_L_CHILD_PARENTHESIS(*OPER_NUM(node_)))
#define NEEDS_R_CHILD_PARENTHESIS(node_) (TYPE_IS_OPER(node_) && OPER_NUM_NEEDS_R_CHILD_PARENTHESIS(*OPER_NUM(node_)))

#define NEEDS_PARENTHESIS(node_) (TYPE_IS_OPER(node_) && OPER_NUM_NEEDS_PARENTHESIS(*OPER_NUM(node_)))

#define IS_ADD_OR_SUB(node_) (TYPE_IS_OPER(node_) && OPER_NUM_ADD_OR_SUB(*OPER_NUM(node_)))

#define IS_L_CHILD_OF_POW(node_) (!NODE_IS_ROOT(node_) && *OPER_NUM(PARENT(node_)) == DiffOperNum::POW && IS_L_CHILD(node_))

#define TREE_INSERT(dest_, parent_, elem_)                                                  \
            do {                                                                            \
                DiffElem new_elem_ = elem_;                                                 \
                if (tree_insert(&diff_data->tree, dest_, parent_, &new_elem_) != Tree::OK)  \
                    return Status::TREE_ERROR;                                              \
                diff_data->tree_changed = true;                                             \
            } while(0)


#define TREE_DELETE_NODE(node_)                                                 \
            do {                                                                \
                if (tree_delete(&diff_data->tree, node_, false) != Tree::OK)    \
                    return Status::NORMAL_WORK;                                 \
                diff_data->tree_changed = true;                                 \
            } while(0)

#define TREE_DELETE_SUBTREE(node_)                                          \
            do {                                                            \
                if (tree_delete(&diff_data->tree, node_, true) != Tree::OK) \
                    return Status::NORMAL_WORK;                             \
                diff_data->tree_changed = true;                             \
            } while(0)


#define COPY_AND_REPLACE_WITH_MUL(target_, copy_, size_)                                            \
            TreeNode* copy_ = nullptr;                                                              \
            size_t size_ = 0;                                                                       \
                                                                                                    \
            STATUS_CHECK(diff_do_diff_copy_and_replace_with_mul_(diff_data, target_, &copy_, &size_))

#define INSERT_SUBTREE(parent_, src_, size_)    \
            /* = */ src_;                       \
            diff_data->tree.size += size_;      \
            (src_)->parent = parent_;           \
            diff_data->tree_changed = true

#define COPY_SUBTREE(src_, dest_, size_)                        \
            STATUS_CHECK(tree_copy_subtree(diff_data, src_, dest_, size_))

#define COPY_SUBTREE_AND_CHECK_SIMPLICITY(src_, dest_, size_, is_simple_)   \
            STATUS_CHECK(tree_copy_subtree(diff_data, src_, dest_, size_, is_simple_))

#define UNTIE_SUBTREE(node_, size_)         \
            /* = */ node_;                  \
            diff_data->tree.size -= size_;  \
            node_ = nullptr;                \
            diff_data->tree_changed = true

#define DELETE_UNTIED_SUBTREE(copy_, size_)                     \
            do {                                                \
                STATUS_CHECK(tree_dtor_untied_subtree(copy_));  \
                size_ = 0;                                      \
            } while(0)


#define RECONNECT(dest_, src_) STATUS_CHECK(tree_reconnect_node(diff_data, dest_, src_))

#define SIMPLIFY_EVAL(node_) diff_simplify_eval_subtree_(diff_data, node_)

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

#define NODE_VAL_EQUALS(node_, val_) (VAL_IS_COUNTABLE(node_) && IS_DOUBLE_EQ(*NODE_VAL(node_), val_))

#define NODE_WILL_BE_DIFFED(node_) (&ELEM(node_)->will_be_diffed)

#define IS_TEX_DUMP_ENABLED (diff_data->tex_file != nullptr)

#define TREE_CHANGED (diff_data->tree_changed)

#define TEX_DUMP()  do {                                            \
                        if (IS_TEX_DUMP_ENABLED && TREE_CHANGED)    \
                            STATUS_CHECK(tex_dump_add(diff_data));  \
                    } while (0)

#define DO_DIFF_AND_TEX_DUMP(node_) do {                                        \
                                        *NODE_WILL_BE_DIFFED(*node_) = true;    \
                                        TEX_DUMP();                             \
                                        DO_DIFF(node_);                         \
                                        TEX_DUMP();                             \
                                    } while (0)

#endif //< #ifndef DIFF_DSL_H_
