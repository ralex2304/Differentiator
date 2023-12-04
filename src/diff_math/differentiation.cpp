#include "differentiation.h"

#include "../dsl.h"

static Status::Statuses diff_do_diff_oper_node_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_addition_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_multiplication_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_vdu_udv_copy_(DiffData* diff_data, TreeNode** node, DiffOperNum oper,
                                                   size_t* subtree_size = nullptr);

static Status::Statuses diff_do_diff_copy_and_replace_with_mul_(DiffData* diff_data, TreeNode** original,
                                                                TreeNode** copy_dest, size_t* copy_size);

static Status::Statuses diff_do_diff_division_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_division_create_pow_node_(DiffData* diff_data, TreeNode** dest,
                                                               TreeNode* parent, TreeNode* src);

static Status::Statuses diff_do_diff_pow_simple_v_(DiffData* diff_data, TreeNode** node,
                                TreeNode* u_original, TreeNode* u_copy, const size_t u_size,
                                TreeNode* v_original, TreeNode* v_copy, const size_t v_size);

static Status::Statuses diff_do_diff_pow_not_simple_(DiffData* diff_data, TreeNode** node,
                                TreeNode* u_original, TreeNode* u_copy, const size_t u_size,
                                TreeNode* v_original, TreeNode* v_copy, const size_t v_size);

static Status::Statuses diff_do_diff_pow_create_u_pow_v_(DiffData* diff_data, TreeNode** node,
                                                           TreeNode* u_original, const size_t u_size,
                                                           TreeNode* v_original, const size_t v_size);

static Status::Statuses diff_do_diff_ln_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_sqrt_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_sin_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_cos_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_do_diff_complex_func_with_coeff_(DiffData* diff_data, TreeNode* node,
                                                              DiffOperNum main_oper, const double coeff,
                                                              DiffOperNum diffed_oper);


static Status::Statuses diff_do_diff_pow_(DiffData* diff_data, TreeNode** node);



Status::Statuses diff_do_diff(DiffData* diff_data) {
    assert(diff_data);

    STATUS_CHECK(diff_do_diff_traversal(diff_data, &diff_data->tree.root));

    return Status::NORMAL_WORK;
}

Status::Statuses diff_do_diff_traversal(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);

    if (TYPE_IS_OPER(*node)) {
        STATUS_CHECK(diff_do_diff_oper_node_(diff_data, node));
        return Status::NORMAL_WORK;
    }

    if (VAL_IS_SIMPLE(*node)) {
        *NODE_TYPE(*node) = DiffElemType::NUM;
        *NUM_VAL(*node) = 0;
        return Status::NORMAL_WORK;
    }

    if (!VAL_IS_SIMPLE(*node)) {
        *NODE_TYPE(*node) = DiffElemType::NUM;
        *NUM_VAL(*node) = 1;
        return Status::NORMAL_WORK;
    }

    assert(0 && "Invalid DiffElemType given or vars array is damaged");
    return Status::TREE_ERROR;
}

#define OPER_CASE_(oper_, ...)  case DiffOperNum::oper_:        \
                                    STATUS_CHECK(__VA_ARGS__);  \
                                    break

static Status::Statuses diff_do_diff_oper_node_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(TYPE_IS_OPER(*node));

    switch (*OPER_NUM(*node)) {
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
#undef OPER_CASE_

static Status::Statuses diff_do_diff_addition_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*OPER_NUM(*node) == DiffOperNum::ADD ||
           *OPER_NUM(*node) == DiffOperNum::SUB);

    DO_DIFF(L(*node));
    DO_DIFF(R(*node));

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_multiplication_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*OPER_NUM(*node) == DiffOperNum::MUL);

    VdU_UdV_BUILD(DiffOperNum::ADD);

    // TODO dump

    DO_DIFF(dU(*node));
    DO_DIFF(dV(*node));

    // TODO dump

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_vdu_udv_copy_(DiffData* diff_data, TreeNode** node, DiffOperNum oper,
                                                   size_t* subtree_size) {
    assert(diff_data);
    assert(node);
    assert(*node);

    *OPER_NUM(*node) = oper;

    COPY_AND_REPLACE_WITH_MUL(VdU(*node), u_copy, u_size);

    COPY_AND_REPLACE_WITH_MUL(UdV(*node), v_copy, v_size);

    *V(*node) = INSERT_SUBTREE(*VdU(*node), v_copy, v_size);

    *U(*node) = INSERT_SUBTREE(*UdV(*node), u_copy, u_size);

    if (subtree_size != nullptr)
        // size(u) + size(dv) + size(v) + size(du) + mul_node + mul_node + parent
        *subtree_size = v_size * 2 + u_size * 2 + 2 + 1;

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_copy_and_replace_with_mul_(DiffData* diff_data, TreeNode** original,
                                                                TreeNode** copy_dest, size_t* copy_size) {
    assert(diff_data);
    assert(original);
    assert(*original);
    assert(copy_dest);
    assert(*copy_dest == nullptr);
    assert(copy_size);

    COPY_SUBTREE(*original, copy_dest, copy_size);

    TreeNode* original_tmp = UNTIE_SUBTREE(*original, *copy_size);

    TREE_INSERT(original, original_tmp->parent, OPER_ELEM(DiffOperNum::MUL));

    *L(*original) = INSERT_SUBTREE(*original, original_tmp, *copy_size);

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_division_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*OPER_NUM(*node) == DiffOperNum::DIV);

    size_t subtree_size = 0;
    VdU_UdV_BUILD_AND_COUNT_SIZE(DiffOperNum::SUB, &subtree_size);

    TreeNode* udv_vdu_node = UNTIE_SUBTREE(*node, subtree_size);

    TREE_INSERT(node, udv_vdu_node->parent, OPER_ELEM(DiffOperNum::DIV));

    *L(*node) = INSERT_SUBTREE(*node, udv_vdu_node, subtree_size);


    STATUS_CHECK(diff_do_diff_division_create_pow_node_(diff_data, R(*node), *node,
                                                        *V(udv_vdu_node)));

    // TODO dump

    DO_DIFF(dU(*L(*node)));
    DO_DIFF(dV(*L(*node)));

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

    TREE_INSERT(dest, parent, OPER_ELEM(DiffOperNum::POW));

    TREE_INSERT(R(*dest), *dest, NUM_ELEM(2));

    TreeNode* copy = nullptr;
    size_t copy_size = 0;
    COPY_SUBTREE(src, &copy, &copy_size);

    *L(*dest) = INSERT_SUBTREE(*dest, copy, copy_size);

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_pow_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*OPER_NUM(*node) == DiffOperNum::POW);

    // u^v

    TreeNode* u_copy = nullptr;
    size_t u_size = 0;
    bool u_is_simple = false;
    COPY_SUBTREE_AND_CHECK_SIMPLICITY(*L(*node), &u_copy, &u_size, &u_is_simple);
    TreeNode* u_original = UNTIE_SUBTREE(*L(*node), u_size);


    TreeNode* v_copy = nullptr;
    size_t v_size = 0;
    bool v_is_simple = false;
    COPY_SUBTREE_AND_CHECK_SIMPLICITY(*R(*node), &v_copy, &v_size, &v_is_simple);
    TreeNode* v_original = UNTIE_SUBTREE(*R(*node), v_size);

    if (u_is_simple && v_is_simple) { //< n^n
        DELETE_UNTIED_SUBTREE(&u_copy, u_size);
        DELETE_UNTIED_SUBTREE(&v_copy, v_size);
        DELETE_UNTIED_SUBTREE(&u_original, u_size); //< size value turns to 0 there,
        DELETE_UNTIED_SUBTREE(&v_original, v_size); //< so value isn't important

        TREE_REPLACE_SUBTREE_WITH_NUM(node, 0);

        // TODO dump

        return Status::NORMAL_WORK;
    }

    *OPER_NUM(*node) = DiffOperNum::MUL;

    if (v_is_simple) { //< x^n
        STATUS_CHECK(diff_do_diff_pow_simple_v_(diff_data, node, u_original, u_copy, u_size,
                                                                 v_original, v_copy, v_size));
        return Status::NORMAL_WORK;
    }

    // n^x or x^x
    STATUS_CHECK(diff_do_diff_pow_not_simple_(diff_data, node, u_original, u_copy, u_size,
                                                               v_original, v_copy, v_size));

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_pow_simple_v_(DiffData* diff_data, TreeNode** node,
                                TreeNode* u_original, TreeNode* u_copy, const size_t u_size,
                                TreeNode* v_original, TreeNode* v_copy, const size_t v_size) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(u_original);
    assert(u_copy);
    assert(v_original);
    assert(v_copy);
    // x^n

    *L(*node) = INSERT_SUBTREE(*node, v_copy, v_size);

    TREE_INSERT(R(*node), *node, OPER_ELEM(DiffOperNum::MUL));
    // result: v * (*)

    *R(*R(*node)) = INSERT_SUBTREE(*R(*node), u_copy, u_size);

    TreeNode**  pow_node = L(*R(*node));
    TREE_INSERT(pow_node, *R(*node), OPER_ELEM(DiffOperNum::POW));
    // result: v * ((^) * u')

    *L(*pow_node) = INSERT_SUBTREE(*pow_node, u_original, u_size);

    TREE_INSERT(R(*pow_node), *pow_node, OPER_ELEM(DiffOperNum::SUB));
    // result: v * ((u ^ (-)) * u')

    *L(*R(*pow_node)) = INSERT_SUBTREE(*R(*pow_node), v_original, v_size);

    TREE_INSERT(R(*R(*pow_node)), *R(*pow_node), NUM_ELEM(1));
    // result: v * ((u ^ (v - 1)) * u'

    // TODO DUMP

    DO_DIFF(R(*R(*node)));

    // TODO DUMP

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_pow_not_simple_(DiffData* diff_data, TreeNode** node,
                                TreeNode* u_original, TreeNode* u_copy, const size_t u_size,
                                TreeNode* v_original, TreeNode* v_copy, const size_t v_size) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(u_original);
    assert(u_copy);
    assert(v_original);
    assert(v_copy);
    // n^x or x^x

    STATUS_CHECK(diff_do_diff_pow_create_u_pow_v_(diff_data, node, u_original, u_size,
                                                                   v_original, v_size));

    TreeNode** r_node = R(*node);

    TREE_INSERT(r_node, *node, OPER_ELEM(DiffOperNum::MUL));

    *L(*r_node) = INSERT_SUBTREE(*r_node, v_copy, v_size);


    TreeNode** ln_node = R(*r_node);

    TREE_INSERT(ln_node, *r_node, OPER_ELEM(DiffOperNum::LN));

    *R(*ln_node) = INSERT_SUBTREE(*ln_node, u_copy, u_size);;

    // TODO dump

    DO_DIFF(R(*node));

    // TODO dump

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_pow_create_u_pow_v_(DiffData* diff_data, TreeNode** node,
                                                           TreeNode* u_original, const size_t u_size,
                                                           TreeNode* v_original, const size_t v_size) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(u_original);
    assert(v_original);

    TREE_INSERT(L(*node), *node, OPER_ELEM(DiffOperNum::POW));

    *L(*L(*node)) = INSERT_SUBTREE(*L(*node), u_original, u_size);

    *R(*L(*node)) = INSERT_SUBTREE(*L(*node), v_original, v_size);

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_ln_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*OPER_NUM(*node) == DiffOperNum::LN);

    *OPER_NUM(*node) = DiffOperNum::DIV;

    TreeNode* copy = nullptr;
    size_t copy_size = 0;
    COPY_SUBTREE(*R(*node), &copy, &copy_size);

    *L(*node) = INSERT_SUBTREE(*node, copy, copy_size);

    // TODO dump

    DO_DIFF(L(*node));

    // TODO dump

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_sqrt_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*OPER_NUM(*node) == DiffOperNum::SQRT);

    STATUS_CHECK(diff_do_diff_complex_func_with_coeff_(diff_data, *node, DiffOperNum::DIV, 2,
                                                                         DiffOperNum::SQRT));

    // TODO dump

    DO_DIFF(L(*node));

    // TODO dump

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_sin_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*OPER_NUM(*node) == DiffOperNum::SIN);

    *OPER_NUM(*node) = DiffOperNum::MUL;

    TreeNode* copy = nullptr;
    size_t size = 0;
    COPY_SUBTREE(*R(*node), &copy, &size);

    TreeNode* original = UNTIE_SUBTREE(*R(*node), size);
    *L(*node) = INSERT_SUBTREE(*node, copy, size);

    TREE_INSERT(R(*node), *node, OPER_ELEM(DiffOperNum::COS));

    *R(*R(*node)) = INSERT_SUBTREE(*R(*node), original, size);

    // TODO dump

    DO_DIFF(L(*node));

    // TODO dump

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_diff_cos_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(*OPER_NUM(*node) == DiffOperNum::COS);

    STATUS_CHECK(diff_do_diff_complex_func_with_coeff_(diff_data, *node, DiffOperNum::MUL, -1,
                                                                         DiffOperNum::SIN));

    // TODO dump

    DO_DIFF(L(*node));

    // TODO dump

    return Status::NORMAL_WORK;
}



static Status::Statuses diff_do_diff_complex_func_with_coeff_(DiffData* diff_data, TreeNode* node,
                                                              DiffOperNum main_oper, const double coeff,
                                                              DiffOperNum diffed_oper) {
    assert(diff_data);
    assert(node);

    *OPER_NUM(node) = main_oper;


    TreeNode* copy = nullptr;
    size_t size = 0;
    COPY_SUBTREE(*R(node), &copy, &size);

    TreeNode* original = UNTIE_SUBTREE(*R(node), size);

    *L(node) = INSERT_SUBTREE(node, copy, size);

    TREE_INSERT(R(node), node, OPER_ELEM(DiffOperNum::MUL));


    TREE_INSERT(L(*R(node)), *R(node), NUM_ELEM(coeff));

    TREE_INSERT(R(*R(node)), *R(node), OPER_ELEM(diffed_oper));

    *R(*R(*R(node))) = INSERT_SUBTREE(*R(*R(node)), original, size);

    return Status::NORMAL_WORK;
}

