#include "differentiation.h"

static Status::Statuses diff_do_diff_oper_node_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_addition_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_multiplication_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_vdu_udv_copy_(DiffData* diff_data, TreeNode** node, DiffOperNum oper,
                                                   size_t* subtree_size = nullptr);

static Status::Statuses diff_do_diff_vdu_udv_diff_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_set_copy_(DiffData* diff_data, TreeNode** dest, TreeNode* parent,
                                               TreeNode* copy, size_t copy_size);

static Status::Statuses diff_do_diff_copy_and_replace_with_mul_(DiffData* diff_data, TreeNode** original,
                                                                TreeNode** copy_dest, size_t* copy_size);

static Status::Statuses diff_do_diff_subtree_copy_(TreeNode* src, TreeNode** dest, size_t* size);

static Status::Statuses diff_do_diff_division_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_division_create_pow_node_(DiffData* diff_data, TreeNode** dest,
                                                               TreeNode* parent, TreeNode* src);

static Status::Statuses diff_do_diff_pow_create_right_node_(DiffData* diff_data, TreeNode** node,
                                                            TreeNode* f_copy, const size_t f_size,
                                                            TreeNode* g_copy, const size_t g_size);

static Status::Statuses diff_do_diff_pow_create_left_node_(DiffData* diff_data, TreeNode** node,
                                                           TreeNode* f_original, const size_t f_size,
                                                           TreeNode* g_original, const size_t g_size);

static Status::Statuses diff_do_diff_ln_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_sqrt_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_sin_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_cos_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_complex_func_with_coeff_(DiffData* diff_data, TreeNode* node,
                                                              DiffOperNum main_oper, const double coeff,
                                                              DiffOperNum diffed_oper);

static Status::Statuses diff_do_diff_copy_and_untie_original_(DiffData* diff_data, TreeNode** original,
                                            TreeNode** original_ptr, TreeNode** copy_ptr, size_t* size);

static Status::Statuses diff_do_diff_copy_(Tree* tree, TreeNode* src, TreeNode** dest,
                                           TreeNode* parent);

Status::Statuses diff_do_diff_pow_(DiffData* diff_data, TreeNode** node);

Status::Statuses diff_do_diff(DiffData* diff_data) {
    assert(diff_data);

    STATUS_CHECK(diff_do_diff_traversal(diff_data, &diff_data->tree.root));

    return Status::NORMAL_WORK;
}

#include "../dsl.h"

Status::Statuses diff_do_diff_traversal(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);

    if (dsl_node_is_num(*node)) {
        dsl_node_set_elem_num(*node, 0);
        return Status::NORMAL_WORK;
    }

    if (dsl_node_is_var(*node)) {
        dsl_node_set_elem_num(*node, 1);
        return Status::NORMAL_WORK;
    }

    if (dsl_node_is_oper(*node)) {
        STATUS_CHECK(diff_do_diff_oper_node_(diff_data, node));
        return Status::NORMAL_WORK;
    }

    assert(0 && "Invalid DiffElemType");
    return Status::TREE_ERROR;
}





#define OPER_CASE_(oper_, ...)  case DiffOperNum::oper_:        \
                                    STATUS_CHECK(__VA_ARGS__);  \
                                    break

static Status::Statuses diff_do_diff_oper_node_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(dsl_node_is_oper(*node));

    switch (*dsl_node_oper_index(*node)) {
        OPER_CASE_(ADD,  diff_do_diff_addition_(diff_data, node));
        OPER_CASE_(SUB,  diff_do_diff_addition_(diff_data, node));
        OPER_CASE_(MUL,  diff_do_diff_multiplication_(diff_data, node));
        OPER_CASE_(DIV,  diff_do_diff_division_(diff_data, node));
        OPER_CASE_(POW,  diff_do_diff_pow_(diff_data, node));

        OPER_CASE_(LN,   diff_do_diff_ln_(diff_data, node));
        OPER_CASE_(SQRT, diff_do_diff_sqrt_(diff_data, node));
        OPER_CASE_(SIN,  diff_do_diff_sin_(diff_data, node));
        OPER_CASE_(COS,  diff_do_diff_cos_(diff_data, node));

        case DiffOperNum::ERR:
        default:
            assert(0 && "Invalid DiffOperNum given");
            return Status::TREE_ERROR;
    }

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_addition_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*dsl_node_oper_index(*node) == DiffOperNum::ADD ||
           *dsl_node_oper_index(*node) == DiffOperNum::SUB);

    STATUS_CHECK(diff_do_diff_traversal(diff_data, dsl_node_left_child(*node)));
    STATUS_CHECK(diff_do_diff_traversal(diff_data, dsl_node_right_child(*node)));

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_multiplication_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*dsl_node_oper_index(*node) == DiffOperNum::MUL);

    STATUS_CHECK(diff_do_diff_vdu_udv_copy_(diff_data, node, DiffOperNum::ADD));

    // TODO dump

    STATUS_CHECK(diff_do_diff_vdu_udv_diff_(diff_data, node));

    // TODO dump

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_vdu_udv_diff_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);

    STATUS_CHECK(diff_do_diff_traversal(diff_data, dsl_vdu_udv_get_du(*node)));
    STATUS_CHECK(diff_do_diff_traversal(diff_data, dsl_vdu_udv_get_dv(*node)));

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_vdu_udv_copy_(DiffData* diff_data, TreeNode** node, DiffOperNum oper,
                                                   size_t* subtree_size) {
    assert(diff_data);
    assert(node);
    assert(*node);

    *dsl_node_oper_index(*node) = oper;

    TreeNode* u_copy = nullptr;
    size_t u_copy_size = 0;

    STATUS_CHECK(diff_do_diff_copy_and_replace_with_mul_(diff_data, dsl_vdu_udv_get_vdu(*node),
                                                         &u_copy, &u_copy_size));

    TreeNode* v_copy = nullptr;
    size_t v_copy_size = 0;

    STATUS_CHECK(diff_do_diff_copy_and_replace_with_mul_(diff_data, dsl_vdu_udv_get_udv(*node),
                                                         &v_copy, &v_copy_size));

    STATUS_CHECK(diff_do_diff_set_copy_(diff_data, dsl_vdu_udv_get_v(*node), *dsl_vdu_udv_get_vdu(*node),
                                                   v_copy, v_copy_size));

    STATUS_CHECK(diff_do_diff_set_copy_(diff_data, dsl_vdu_udv_get_u(*node), *dsl_vdu_udv_get_udv(*node),
                                                   u_copy, u_copy_size));

    if (subtree_size != nullptr)
        // size(u) + size(dv) + size(v) + size(du) + mul_node + mul_node + parent
        *subtree_size = v_copy_size * 2 + u_copy_size * 2 + 2 + 1;

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_set_copy_(DiffData* diff_data, TreeNode** dest, TreeNode* parent,
                                               TreeNode* copy, size_t copy_size) {
    assert(diff_data);
    assert(dest);
    assert(*dest == nullptr);
    assert(copy);

    copy->parent = parent;
    *dest = copy;
    diff_data->tree.size += copy_size;

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_copy_and_replace_with_mul_(DiffData* diff_data, TreeNode** original,
                                                                TreeNode** copy_dest, size_t* copy_size) {

    TreeNode* original_tmp = *original;

    STATUS_CHECK(diff_do_diff_subtree_copy_(original_tmp, copy_dest, copy_size));

    diff_data->tree.size -= *copy_size;
    *original = nullptr;

    DiffElem mul_node = {.type = DiffElemType::OPER, .data = {.oper = DiffOperNum::MUL}};

    if (tree_insert(&diff_data->tree, original, original_tmp->parent, &mul_node) != Tree::OK) {
        return Status::TREE_ERROR;
    }

    STATUS_CHECK(diff_do_diff_set_copy_(diff_data, dsl_node_left_child(*original), *original,
                                        original_tmp, *copy_size));

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_division_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*dsl_node_oper_index(*node) == DiffOperNum::DIV);

    size_t subtree_size = 0;
    STATUS_CHECK(diff_do_diff_vdu_udv_copy_(diff_data, node, DiffOperNum::SUB, &subtree_size));

    TreeNode* udv_vdu_node = *node;
    diff_data->tree.size -= subtree_size;
    *node = nullptr;

    DiffElem div_elem = {.type = DiffElemType::OPER, .data = {.oper = DiffOperNum::DIV}};
    if (tree_insert(&diff_data->tree, node, udv_vdu_node->parent, &div_elem) != Tree::OK)
        return Status::TREE_ERROR;

    STATUS_CHECK(diff_do_diff_set_copy_(diff_data, dsl_node_left_child(*node), *node,
                                        udv_vdu_node, subtree_size));

    STATUS_CHECK(diff_do_diff_division_create_pow_node_(diff_data, dsl_node_right_child(*node),
                                                        *node, *dsl_vdu_udv_get_v(udv_vdu_node)));

    // TODO dump

    STATUS_CHECK(diff_do_diff_vdu_udv_diff_(diff_data, dsl_node_left_child(*node)));

    // TODO dump

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_division_create_pow_node_(DiffData* diff_data, TreeNode** dest,
                                                               TreeNode* parent, TreeNode* src) {
    assert(diff_data);
    assert(dest);
    assert(*dest == nullptr);
    assert(parent);
    assert(src);

    DiffElem pow_elem     = {.type = DiffElemType::OPER, .data = {.oper = DiffOperNum::POW}};
    DiffElem pow_num_elem = {.type = DiffElemType::NUM,  .data = {.num = 2}};

    if (tree_insert(&diff_data->tree, dest, parent, &pow_elem) != Tree::OK)
        return Status::TREE_ERROR;

    if (tree_insert(&diff_data->tree, dsl_node_right_child(*dest), *dest, &pow_num_elem) != Tree::OK)
        return Status::TREE_ERROR;

    TreeNode* copy = nullptr;
    size_t copy_size = 0;
    STATUS_CHECK(diff_do_diff_subtree_copy_(src, &copy, &copy_size));

    STATUS_CHECK(diff_do_diff_set_copy_(diff_data, dsl_node_left_child(*dest), *dest, copy, copy_size));

    return Status::NORMAL_WORK;
}

Status::Statuses diff_do_diff_pow_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*dsl_node_oper_index(*node) == DiffOperNum::POW);

    // f^g

    TreeNode* f_original = nullptr;
    TreeNode* f_copy = nullptr;
    size_t f_size = 0;

    STATUS_CHECK(diff_do_diff_copy_and_untie_original_(diff_data, dsl_node_left_child(*node),
                                                       &f_original, &f_copy, &f_size));

    TreeNode* g_original = nullptr;
    TreeNode* g_copy = nullptr;
    size_t g_size = 0;

    STATUS_CHECK(diff_do_diff_copy_and_untie_original_(diff_data, dsl_node_right_child(*node),
                                                       &g_original, &g_copy, &g_size));

    *dsl_node_oper_index(*node) = DiffOperNum::MUL;

    STATUS_CHECK(diff_do_diff_pow_create_left_node_(diff_data, node, f_original, f_size,
                                                                     g_original, g_size));

    STATUS_CHECK(diff_do_diff_pow_create_right_node_(diff_data, node, f_copy, f_size,
                                                                      g_copy, g_size));

    // TODO dump

    STATUS_CHECK(diff_do_diff_traversal(diff_data, dsl_node_right_child(*node)));

    // TODO dump

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_pow_create_right_node_(DiffData* diff_data, TreeNode** node,
                                                            TreeNode* f_copy, const size_t f_size,
                                                            TreeNode* g_copy, const size_t g_size) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(f_copy);
    assert(g_copy);

    TreeNode* r_node = *dsl_node_right_child(*node);

    DiffElem mul_elem = {.type = DiffElemType::OPER, .data = {.oper = DiffOperNum::MUL}};

    if (tree_insert(&diff_data->tree, dsl_node_right_child(*node), *node, &mul_elem) != Tree::OK) {
        return Status::TREE_ERROR;
    }

    STATUS_CHECK(diff_do_diff_set_copy_(diff_data, dsl_node_left_child(r_node), r_node,
                                        g_copy, g_size));

    DiffElem ln_elem = {.type = DiffElemType::OPER, .data = {.oper = DiffOperNum::LN}};

    if (tree_insert(&diff_data->tree, dsl_node_right_child(r_node), r_node, &ln_elem) != Tree::OK) {
        return Status::TREE_ERROR;
    }

    TreeNode* ln_node = *dsl_node_right_child(r_node);

    STATUS_CHECK(diff_do_diff_set_copy_(diff_data, dsl_node_right_child(ln_node), ln_node,
                                        f_copy, f_size));

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_pow_create_left_node_(DiffData* diff_data, TreeNode** node,
                                                           TreeNode* f_original, const size_t f_size,
                                                           TreeNode* g_original, const size_t g_size) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(f_original);
    assert(g_original);

    TreeNode* l_node = *dsl_node_left_child(*node);

    DiffElem pow_node = {.type = DiffElemType::OPER, .data = {.oper = DiffOperNum::POW}};

    if (tree_insert(&diff_data->tree, dsl_node_left_child(*node), *node, &pow_node) != Tree::OK) {
        return Status::TREE_ERROR;
    }

    STATUS_CHECK(diff_do_diff_set_copy_(diff_data, dsl_node_left_child(l_node), l_node,
                                        f_original, f_size));

    STATUS_CHECK(diff_do_diff_set_copy_(diff_data, dsl_node_right_child(l_node), l_node,
                                        g_original, g_size));

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_copy_and_untie_original_(DiffData* diff_data, TreeNode** original,
                                            TreeNode** original_ptr, TreeNode** copy_ptr, size_t* size) {
    assert(diff_data);
    assert(original);
    assert(original_ptr);
    assert(*original_ptr == nullptr);
    assert(copy_ptr);
    assert(*copy_ptr == nullptr);
    assert(size);

    STATUS_CHECK(diff_do_diff_subtree_copy_(*original, copy_ptr, size));

    *original_ptr = *original;
    *original = nullptr;
    diff_data->tree.size -= *size;

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_ln_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*dsl_node_oper_index(*node) == DiffOperNum::LN);

    *dsl_node_oper_index(*node) = DiffOperNum::DIV;

    TreeNode* copy = nullptr;
    size_t copy_size = 0;
    STATUS_CHECK(diff_do_diff_subtree_copy_(*dsl_node_right_child(*node), &copy, &copy_size));

    STATUS_CHECK(diff_do_diff_set_copy_(diff_data, dsl_node_left_child(*node), *node,
                                        copy, copy_size));

    // TODO dump

    STATUS_CHECK(diff_do_diff_traversal(diff_data, dsl_node_left_child(*node)));

    // TODO dump

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_sqrt_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*dsl_node_oper_index(*node) == DiffOperNum::SQRT);

    STATUS_CHECK(diff_do_diff_complex_func_with_coeff_(diff_data, *node, DiffOperNum::DIV, 2,
                                                                         DiffOperNum::SQRT));

    // TODO dump

    STATUS_CHECK(diff_do_diff_traversal(diff_data, dsl_node_left_child(*node)));

    // TODO dump

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_sin_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*dsl_node_oper_index(*node) == DiffOperNum::SIN);

    *dsl_node_oper_index(*node) = DiffOperNum::MUL;

    TreeNode* original = nullptr;
    TreeNode* copy = nullptr;
    size_t size = 0;
    STATUS_CHECK(diff_do_diff_copy_and_untie_original_(diff_data, dsl_node_right_child(*node),
                                                       &original, &copy, &size));

    STATUS_CHECK(diff_do_diff_set_copy_(diff_data, dsl_node_left_child(*node), *node,
                                        copy, size));

    DiffElem cos_elem = {.type = DiffElemType::OPER, .data = {.oper = DiffOperNum::COS}};
    if (tree_insert(&diff_data->tree, dsl_node_right_child(*node), *node, &cos_elem) != Tree::OK)
        return Status::NORMAL_WORK;

    TreeNode* r_node = *dsl_node_right_child(*node);

    STATUS_CHECK(diff_do_diff_set_copy_(diff_data, dsl_node_right_child(r_node), r_node,
                                        original, size));
    // TODO dump

    STATUS_CHECK(diff_do_diff_traversal(diff_data, dsl_node_left_child(*node)));

    // TODO dump

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_cos_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*dsl_node_oper_index(*node) == DiffOperNum::COS);

    STATUS_CHECK(diff_do_diff_complex_func_with_coeff_(diff_data, *node, DiffOperNum::MUL, -1,
                                                                         DiffOperNum::SIN));

    // TODO dump

    STATUS_CHECK(diff_do_diff_traversal(diff_data, dsl_node_left_child(*node)));

    // TODO dump

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_subtree_copy_(TreeNode* src, TreeNode** dest, size_t* size) {
    assert(src);
    assert(dest);
    assert(*dest == nullptr);
    assert(size);

    Tree copy = {};
    if (TREE_CTOR(&copy, sizeof(DiffElem), &diff_elem_dtor,
                                           &diff_elem_verify,
                                           &diff_elem_str_val) != Tree::OK)
        return Status::TREE_ERROR;

    STATUS_CHECK(diff_do_diff_copy_(&copy, src, &copy.root, nullptr));

    *size = copy.size;
    *dest = copy.root;

    copy.size = 0;
    copy.root = nullptr;

    if (tree_dtor(&copy) != Tree::OK)
        return Status::TREE_ERROR;

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_complex_func_with_coeff_(DiffData* diff_data, TreeNode* node,
                                                              DiffOperNum main_oper, const double coeff,
                                                              DiffOperNum diffed_oper) {
    assert(diff_data);
    assert(node);

    *dsl_node_oper_index(node) = main_oper;

    TreeNode* original = nullptr;
    TreeNode* copy = nullptr;
    size_t size = 0;
    STATUS_CHECK(diff_do_diff_copy_and_untie_original_(diff_data, dsl_node_right_child(node),
                                                       &original, &copy, &size));

    STATUS_CHECK(diff_do_diff_set_copy_(diff_data, dsl_node_left_child(node), node, copy, size));

    DiffElem mul_elem = {.type = DiffElemType::OPER, .data = {.oper = DiffOperNum::MUL}};
    if (tree_insert(&diff_data->tree, dsl_node_right_child(node), node, &mul_elem) != Tree::OK)
        return Status::TREE_ERROR;

    TreeNode* r_node = *dsl_node_right_child(node);

    DiffElem mul_num_elem = {.type = DiffElemType::NUM, .data = {.num = coeff}};
    if (tree_insert(&diff_data->tree, dsl_node_left_child(r_node), r_node, &mul_num_elem) != Tree::OK)
        return Status::TREE_ERROR;

    DiffElem oper_elem = {.type = DiffElemType::OPER, .data = {.oper = diffed_oper}};
    if (tree_insert(&diff_data->tree, dsl_node_right_child(r_node), r_node, &oper_elem) != Tree::OK)
        return Status::TREE_ERROR;

    TreeNode* diffed_node = *dsl_node_right_child(r_node);

    STATUS_CHECK(diff_do_diff_set_copy_(diff_data, dsl_node_right_child(diffed_node), diffed_node,
                                        original, size));

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_copy_(Tree* tree, TreeNode* src, TreeNode** dest,
                                           TreeNode* parent) {
    assert(tree);
    assert(src);
    assert(dest);
    assert(*dest == nullptr);

    if (tree_insert(tree, dest, parent, dsl_node_elem(src)) != Tree::OK)
        return Status::TREE_ERROR;

    if (dsl_node_is_num(src) || dsl_node_is_var(src))
        return Status::NORMAL_WORK;


    if (dsl_node_is_oper(src) && dsl_node_oper(src)->type != UNARY)
        STATUS_CHECK(diff_do_diff_copy_(tree, src->left, &(*dest)->left, *dest));

    STATUS_CHECK(diff_do_diff_copy_(tree, src->right, &(*dest)->right, *dest));

    return Status::NORMAL_WORK;
}
