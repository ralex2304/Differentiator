#include "simplification.h"

static Status::Statuses diff_simplify_oper_(DiffData* diff_data, TreeNode** node, bool* is_simple);

static Status::Statuses diff_simplify_eval_subtree_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_simplify_eval_subtree_oper_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_simplify_neutral_(DiffData* diff_data, TreeNode** node, bool* is_simple);

static Status::Statuses diff_simplify_oper_const_eval_(DiffData* diff_data, TreeNode** node,
                                                       bool* is_parent_simple,
                                                       bool* is_l_simple, bool* is_r_simple);


Status::Statuses diff_simplify(DiffData* diff_data) {
    assert(diff_data);

    bool is_simple = false;
    STATUS_CHECK(diff_simplify_traversal(diff_data, &diff_data->tree.root, &is_simple));

    return Status::NORMAL_WORK;
}

#include "../dsl.h"

Status::Statuses diff_simplify_traversal(DiffData* diff_data, TreeNode** node, bool* is_simple) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(is_simple);

    switch (((DiffElem*)(*node)->elem)->type) {
        case DiffElemType::NUM:
            *is_simple = true;
            break;
        case DiffElemType::VAR:
            *is_simple = false;
            break;
        case DiffElemType::OPER:
            STATUS_CHECK(diff_simplify_oper_(diff_data, node, is_simple));
            break;
        case DiffElemType::ERR:
        default:
            assert(0 && "Invalid DiffElemType given");
            return Status::TREE_ERROR;
    }

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_simplify_oper_(DiffData* diff_data, TreeNode** node, bool* is_simple) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(is_simple);
    assert(NODE_IS_OPER(*node));

    bool is_left_simple = false;
    bool is_right_simple = false;

    STATUS_CHECK(diff_simplify_oper_const_eval_(diff_data, node, is_simple, &is_left_simple,
                                                                            &is_right_simple));

    if (!(*is_simple))
        STATUS_CHECK(diff_simplify_neutral_(diff_data, node, is_simple));

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_simplify_oper_const_eval_(DiffData* diff_data, TreeNode** node,
                                                       bool* is_parent_simple,
                                                       bool* is_l_simple, bool* is_r_simple) {
    assert(diff_data);
    assert(node);
    assert(*node);

    if (OPER(*node)->type == BINARY)
        STATUS_CHECK(diff_simplify_traversal(diff_data, L(*node), is_l_simple));
    else
        *is_r_simple = true;

    STATUS_CHECK(diff_simplify_traversal(diff_data, R(*node), is_r_simple));

    *is_parent_simple = *is_l_simple && *is_r_simple;

    if (*is_l_simple != *is_r_simple) {
        STATUS_CHECK(diff_simplify_eval_subtree_(diff_data, *is_l_simple ? L(*node)
                                                                         : R(*node)));
    }

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_simplify_eval_subtree_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(!NODE_IS_VAR(*node)); // TODO var value eval

    if (NODE_IS_OPER(*node)) {
        STATUS_CHECK(diff_simplify_eval_subtree_oper_(diff_data, node));
    }

    // dsl_node_is_num - do nothing

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_simplify_eval_subtree_oper_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(NODE_IS_OPER(*node));

    if (OPER(*node)->type == BINARY)
        STATUS_CHECK(diff_simplify_eval_subtree_(diff_data, L(*node)));

    STATUS_CHECK(diff_simplify_eval_subtree_(diff_data, R(*node)));

    const double left  = *NUM_VAL(*L(*node));
    const double right = *NUM_VAL(*R(*node));
    double result = NAN;

    STATUS_CHECK(diff_eval_calc(left, right, *OPER_NUM(*node), &result));
    assert(isfinite(result));

    TREE_DELETE_NODE(L(*node));
    TREE_DELETE_NODE(R(*node));

    *NODE_TYPE(*node) = DiffElemType::NUM;
    *NUM_VAL(*node) = result;

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_simplify_neutral_(DiffData* diff_data, TreeNode** node, bool* is_simple) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(is_simple);
    assert(!NODE_IS_VAR(*node));

    if (NODE_IS_NUM(*node))
        return Status::NORMAL_WORK;

    // TODO Equal simplification for sub and div

    switch (*OPER_NUM(*node)) {
        case DiffOperNum::ADD:
            if (NODE_VAL_EQUALS(*L(*node), 0)) {
                RECONNECT(node, *R(*node));
                break;
            }

            if (NODE_VAL_EQUALS(*R(*node), 0))
                RECONNECT(node, *L(*node));
            break;

        case DiffOperNum::SUB:
            if (NODE_VAL_EQUALS(*R(*node), 0))
                RECONNECT(node, *L(*node));
            break;

        case DiffOperNum::MUL:
            if (NODE_VAL_EQUALS(*L(*node), 1)) {
                RECONNECT(node, *R(*node));
                break;
            }

            if (NODE_VAL_EQUALS(*R(*node), 1)) {
                RECONNECT(node, *L(*node));
                break;
            }

            if (NODE_VAL_EQUALS(*L(*node), 0) || NODE_VAL_EQUALS(*R(*node), 0))
                TREE_REPLACE_SUBTREE_WITH_NUM(node, 0);
            break;

        case DiffOperNum::DIV:
            if (NODE_VAL_EQUALS(*R(*node), 1)) {
                RECONNECT(node, *L(*node));
                break;
            }

            if (NODE_VAL_EQUALS(*L(*node), 0))
                TREE_REPLACE_SUBTREE_WITH_NUM(node, 0);
            break;

        case DiffOperNum::POW:
            if (NODE_VAL_EQUALS(*R(*node), 1)) {
                RECONNECT(node, *L(*node));
                break;
            }

            if (NODE_VAL_EQUALS(*L(*node), 1)) {
                TREE_REPLACE_SUBTREE_WITH_NUM(node, 1);
                break;
            }

            if (NODE_VAL_EQUALS(*L(*node), 0))
                TREE_REPLACE_SUBTREE_WITH_NUM(node, 1);
            break;

        case DiffOperNum::LN:
        case DiffOperNum::SQRT:
        case DiffOperNum::SIN:
        case DiffOperNum::COS:
            break;

        case DiffOperNum::ERR:
        default:
            assert(0 && "Wrong DiffOperNum given");
            return Status::TREE_ERROR;
    }

    return Status::NORMAL_WORK;
}
#undef OPER_CASE_
