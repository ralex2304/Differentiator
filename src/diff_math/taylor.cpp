#include "taylor.h"

#include "../dsl.h"

static Status::Statuses diff_do_taylor_eval_n_derivatives_(DiffData* diff_data, double* values,
                                                           const size_t degree, const double point);

static Status::Statuses diff_do_taylor_create_x_sub_a_(DiffData* diff_data, Tree* x_sub_a,
                                                       const double point);

static Status::Statuses diff_do_taylor_build_sum_(DiffData* diff_data, double* values, Tree* x_sub_a,
                                                  const size_t degree);

#define LOCAL_DTOR_()   FREE(values);           \
                        DSL_TREE_DTOR(&x_sub_a)

Status::Statuses diff_do_taylor(DiffData* diff_data, const size_t degree, const double point) {
    assert(diff_data);

    if (ARG_VAR_NUM < 0) {
        STATUS_CHECK(interface_error_no_argument());
        return Status::INPUT_ERROR;
    }

    if (diff_exists_var_with_no_value(diff_data, true)) {
        STATUS_CHECK(interface_ask_vars_values(diff_data, false, true));
    }

    double* values = (double*)calloc(degree + 1, sizeof(double));
    if (values == nullptr)
        return Status::MEMORY_EXCEED;

    STATUS_CHECK(diff_do_taylor_eval_n_derivatives_(diff_data, values, degree, point), FREE(values));

    CLEAR_TREE(&diff_data->tree, FREE(values));

    Tree x_sub_a = {};
    DSL_TREE_CTOR(&x_sub_a);

    STATUS_CHECK(diff_do_taylor_create_x_sub_a_(diff_data, &x_sub_a, point), LOCAL_DTOR_());

    STATUS_CHECK(diff_do_taylor_build_sum_(diff_data, values, &x_sub_a, degree), LOCAL_DTOR_());

    LOCAL_DTOR_();

    return Status::NORMAL_WORK;
}
#undef LOCAL_DTOR_

static Status::Statuses diff_do_taylor_eval_n_derivatives_(DiffData* diff_data, double* values,
                                                           const size_t degree, const double point) {
    assert(diff_data);
    assert(values);

    for (size_t i = 0; i < degree + 1; i++) {
        if (i >= 1) {
            if (IS_TEX_DUMP_ENABLED)
                STATUS_CHECK(tex_subsection_n_derivative_(diff_data, i));

            DO_DIFF(ROOT);

            STATUS_CHECK(diff_simplify(diff_data));

            TEX_DUMP();
        }

        double tmp_x = diff_data->vars.arr[ARG_VAR_NUM].val;
        diff_data->vars.arr[ARG_VAR_NUM].val = point;

        STATUS_CHECK(diff_eval(diff_data, ROOT, &values[i]));

        diff_data->vars.arr[ARG_VAR_NUM].val = tmp_x;

        if (IS_TEX_DUMP_ENABLED)
            STATUS_CHECK(tex_print_evaled_value(diff_data, values[i]));
    }

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_taylor_create_x_sub_a_(DiffData* diff_data, Tree* x_sub_a,
                                                       const double point) {
    assert(diff_data);
    assert(x_sub_a);

    DiffElem sub_elem = OPER_ELEM(DiffOperNum::SUB);
    DiffElem pow_elem = OPER_ELEM(DiffOperNum::POW);
    assert(ARG_VAR_NUM >= 0);
    DiffElem x_elem = VAR_ELEM((size_t)ARG_VAR_NUM);
    DiffElem point_elem = NUM_ELEM(point);
    DiffElem zero_elem = NUM_ELEM(0);

    if (tree_insert(x_sub_a, &x_sub_a->root, nullptr, &pow_elem) != Tree::OK)
        return Status::TREE_ERROR;

    if (tree_insert(x_sub_a, L(x_sub_a->root), x_sub_a->root, &sub_elem) != Tree::OK)
        return Status::TREE_ERROR;

    if (tree_insert(x_sub_a, L(*L(x_sub_a->root)), *L(x_sub_a->root), &x_elem) != Tree::OK)
        return Status::TREE_ERROR;

    if (tree_insert(x_sub_a, R(*L(x_sub_a->root)), *L(x_sub_a->root), &point_elem) != Tree::OK)
        return Status::TREE_ERROR;

    if (tree_insert(x_sub_a, R(x_sub_a->root), x_sub_a->root, &zero_elem) != Tree::OK)
        return Status::TREE_ERROR;

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_do_taylor_build_sum_(DiffData* diff_data, double* values, Tree* x_sub_a,
                                                  const size_t degree) {
    assert(diff_data);
    assert(values);
    assert(x_sub_a);

    DiffElem add_elem = OPER_ELEM(DiffOperNum::ADD);
    DiffElem mul_elem = OPER_ELEM(DiffOperNum::MUL);
    DiffElem div_elem = OPER_ELEM(DiffOperNum::DIV);

    TreeNode** cur_add_node = ROOT;
    TreeNode* parent = nullptr;
    size_t fact = 1;

    for (size_t i = 0; i <= degree; i++) {
        TreeNode** cur_dest = nullptr;

        if (i < degree) {
            TREE_INSERT(cur_add_node, parent, add_elem);
            cur_dest = L(*cur_add_node);

            TREE_INSERT(cur_dest, *cur_add_node, mul_elem);
        } else {
            cur_dest = cur_add_node;

            TREE_INSERT(cur_dest, parent, mul_elem);
        }

        TREE_INSERT(L(*cur_dest), *cur_dest, div_elem);


        *NUM_VAL(*R(x_sub_a->root)) = (double)i;

        size_t size = 0;
        COPY_SUBTREE(x_sub_a->root, L(*L(*cur_dest)), &size);
        PARENT(*L(*L(*cur_dest))) = *L(*cur_dest);
        diff_data->tree.size += size;


        DiffElem fact_elem = NUM_ELEM((double)fact);
        TREE_INSERT(R(*L(*cur_dest)), *L(*cur_dest), fact_elem);

        DiffElem val_elem = NUM_ELEM(values[i]);
        TREE_INSERT(R(*cur_dest), *cur_dest, val_elem);

        fact *= i + 1;

        if (i >= degree)
            break;

        parent = *cur_add_node;
        cur_add_node = R(*cur_add_node);
    }

    return Status::NORMAL_WORK;
}
