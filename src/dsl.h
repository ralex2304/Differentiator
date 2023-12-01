#ifndef DIFF_DSL_H_
#define DIFF_DSL_H_

#include "diff_objects.h"
#include "config.h"
#include TREE_INCLUDE

inline DiffElem* dsl_node_elem(TreeNode* node) {
    assert(node);

    return (DiffElem*)node->elem;
}

inline DiffElemType* dsl_node_type(TreeNode* node) {
    return &dsl_node_elem(node)->type;
}

inline bool dsl_node_is_oper(TreeNode* node) {
    return *dsl_node_type(node) == DiffElemType::OPER;
}

inline bool dsl_node_is_num(TreeNode* node) {
    return *dsl_node_type(node) == DiffElemType::NUM;
}

inline bool dsl_node_is_var(TreeNode* node) {
    return *dsl_node_type(node) == DiffElemType::VAR;
}

inline DiffElemData* dsl_node_data(TreeNode* node) {
    return &dsl_node_elem(node)->data;
}

inline double* dsl_node_num_val(TreeNode* node) {
    assert(dsl_node_is_num(node));

    return &dsl_node_data(node)->num;
}

inline size_t* dsl_node_var_index(TreeNode* node) {
    assert(dsl_node_is_var(node));

    return &dsl_node_data(node)->var;
}

inline DiffOperNum* dsl_node_oper_index(TreeNode* node) {
    assert(dsl_node_is_oper(node));

    return &dsl_node_data(node)->oper;
}

inline const DiffOper* dsl_node_oper(TreeNode* node) {
    return DIFF_OPERS + (size_t)(*dsl_node_oper_index(node));
}

inline DiffVar* dsl_node_var_ptr(const DiffData* diff_data, TreeNode* node) {
    assert(*dsl_node_var_index(node) < diff_data->vars.size);

    return &diff_data->vars.arr[*dsl_node_var_index(node)];
}

inline double* dsl_node_var_value(const DiffData* diff_data, TreeNode* node) {
    return &dsl_node_var_ptr(diff_data, node)->val;
}

inline const char* dsl_node_var_name(const DiffData* diff_data, TreeNode* node) {
    return dsl_node_var_ptr(diff_data, node)->name;
}

inline TreeNode** dsl_node_left_child(TreeNode* node) {
    return &node->left;
}

inline TreeNode** dsl_node_right_child(TreeNode* node) {
    return &node->right;
}

inline void dsl_node_set_elem_num(TreeNode* node, double val) {
    *dsl_node_elem(node) = {.type = DiffElemType::NUM, .data = {.num = val}};
}

inline TreeNode** dsl_vdu_udv_get_udv(TreeNode* node) {
    return dsl_node_right_child(node);
}

inline TreeNode** dsl_vdu_udv_get_vdu(TreeNode* node) {
    return dsl_node_left_child(node);
}

inline TreeNode** dsl_vdu_udv_get_u(TreeNode* node) {
    return dsl_node_right_child(*dsl_vdu_udv_get_udv(node));
}

inline TreeNode** dsl_vdu_udv_get_v(TreeNode* node) {
    return dsl_node_right_child(*dsl_vdu_udv_get_vdu(node));
}

inline TreeNode** dsl_vdu_udv_get_dv(TreeNode* node) {
    return dsl_node_left_child(*dsl_vdu_udv_get_udv(node));
}

inline TreeNode** dsl_vdu_udv_get_du(TreeNode* node) {
    return dsl_node_left_child(*dsl_vdu_udv_get_vdu(node));
}

#endif //< #ifndef DIFF_DSL_H_
