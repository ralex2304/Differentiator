#include "simplification.h"

#include "../dsl.h"

static Status::Statuses diff_simplify_oper_(DiffData* diff_data, TreeNode** node, bool* is_countable);

static Status::Statuses diff_simplify_eval_subtree_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_simplify_eval_subtree_oper_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_simplify_neutral_(DiffData* diff_data, TreeNode** node);

static Status::Statuses diff_simplify_oper_const_eval_(DiffData* diff_data, TreeNode** node,
                                                       bool* is_parent_countable);


Status::Statuses diff_simplify(DiffData* diff_data) {
    assert(diff_data);

    bool is_countable = false;
    STATUS_CHECK(diff_simplify_traversal(diff_data, &diff_data->tree.root, &is_countable));

    return Status::NORMAL_WORK;
}

Status::Statuses diff_simplify_traversal(DiffData* diff_data, TreeNode** node, bool* is_countable) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(is_countable);

    switch (*NODE_TYPE(*node)) {
        case DiffElemType::NUM:
        case DiffElemType::VAR:
            *is_countable = VAL_IS_COUNTABLE(*node);
            break;

        case DiffElemType::OPER:
            STATUS_CHECK(diff_simplify_oper_(diff_data, node, is_countable));
            break;

        case DiffElemType::ERR:
        default:
            assert(0 && "Invalid DiffElemType given");
            return Status::TREE_ERROR;
    }

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_simplify_oper_(DiffData* diff_data, TreeNode** node, bool* is_countable) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(is_countable);
    assert(TYPE_IS_OPER(*node));

    STATUS_CHECK(diff_simplify_oper_const_eval_(diff_data, node, is_countable));

    if (!(*is_countable))
        STATUS_CHECK(diff_simplify_neutral_(diff_data, node));

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_simplify_oper_const_eval_(DiffData* diff_data, TreeNode** node,
                                                       bool* is_parent_countable) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(TYPE_IS_OPER(*node));

    bool is_l_countable = false;
    bool is_r_countable = false;

    if (IS_BINARY(*node))
        STATUS_CHECK(diff_simplify_traversal(diff_data, L(*node), &is_l_countable));
    else
        is_l_countable = true;

    STATUS_CHECK(diff_simplify_traversal(diff_data, R(*node), &is_r_countable));


    *is_parent_countable = is_l_countable && is_r_countable;

    if (is_l_countable && IS_BINARY(*node))
        SIMPLIFY_EVAL(L(*node));

    if (is_r_countable)
        SIMPLIFY_EVAL(R(*node));

    if (*is_parent_countable)
        SIMPLIFY_EVAL(node);

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_simplify_eval_subtree_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(VAL_IS_COUNTABLE(*node) || TYPE_IS_OPER(*node));

    if (TYPE_IS_OPER(*node)) {
        STATUS_CHECK(diff_simplify_eval_subtree_oper_(diff_data, node));
        return Status::NORMAL_WORK;
    }

    if (TYPE_IS_VAR(*node) && SUBST_VARS) {
        *ELEM(*node) = NUM_ELEM(*VAR_VAL(*node));

        TREE_CHANGED = true;
        TEX_DUMP();
        return Status::NORMAL_WORK;
    }

    assert(TYPE_IS_NUM(*node) || (TYPE_IS_VAR(*node) && !SUBST_VARS));
    // if TYPE_IS_NUM - do nothing

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_simplify_eval_subtree_oper_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(TYPE_IS_OPER(*node));

    if (OPER(*node)->type == BINARY)
        STATUS_CHECK(diff_simplify_eval_subtree_(diff_data, L(*node)));

    STATUS_CHECK(diff_simplify_eval_subtree_(diff_data, R(*node)));

    const double left  = OPER(*node)->type == BINARY ? *NUM_VAL(*L(*node)) : NAN;
    const double right = *NUM_VAL(*R(*node));
    double result = NAN;

    STATUS_CHECK(diff_eval_calc(left, right, *OPER_NUM(*node), &result));
    assert(isfinite(result));


    if (OPER(*node)->type == BINARY)
        TREE_DELETE_NODE(L(*node));

    TREE_DELETE_NODE(R(*node));

    *NODE_TYPE(*node) = DiffElemType::NUM;
    *NUM_VAL(*node) = result;

    TREE_CHANGED = true;
    TEX_DUMP();

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_simplify_neutral_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(VAL_IS_COUNTABLE(*node) || TYPE_IS_OPER(*node));

    if (VAL_IS_COUNTABLE(*node))
        return Status::NORMAL_WORK;

    switch (*OPER_NUM(*node)) {
        case DiffOperNum::ADD:
            if (NODE_VAL_EQUALS(*R(*node), 0)) {
                RECONNECT(node, *L(*node));
                break;
            }

            if (NODE_VAL_EQUALS(*L(*node), 0))
                RECONNECT(node, *R(*node));
            break;

        case DiffOperNum::SUB:
            if (NODE_VAL_EQUALS(*R(*node), 0))
                RECONNECT(node, *L(*node));
            break;

        case DiffOperNum::MUL:
            if (NODE_VAL_EQUALS(*L(*node), 0) || NODE_VAL_EQUALS(*R(*node), 0)) {
                TREE_REPLACE_SUBTREE_WITH_NUM(node, 0);
                break;
            }

            if (NODE_VAL_EQUALS(*L(*node), 1)) {
                RECONNECT(node, *R(*node));
                break;
            }

            if (NODE_VAL_EQUALS(*R(*node), 1))
                RECONNECT(node, *L(*node));
            break;

        case DiffOperNum::DIV:
            if (NODE_VAL_EQUALS(*L(*node), 0)) {
                TREE_REPLACE_SUBTREE_WITH_NUM(node, 0);
                break;
            }

            if (NODE_VAL_EQUALS(*R(*node), 1))
                RECONNECT(node, *L(*node));
            break;

        case DiffOperNum::POW:
            if (NODE_VAL_EQUALS(*L(*node), 1)) {
                TREE_REPLACE_SUBTREE_WITH_NUM(node, 1);
                break;
            }

            if (NODE_VAL_EQUALS(*L(*node), 0)) {
                TREE_REPLACE_SUBTREE_WITH_NUM(node, 1);
                break;
            }

            if (NODE_VAL_EQUALS(*R(*node), 1))
                RECONNECT(node, *L(*node));
            break;

        case DiffOperNum::LN:
        case DiffOperNum::SQRT:
        case DiffOperNum::SIN:
        case DiffOperNum::COS:
            assert(OPER(*node)->type == UNARY);

            if (VAL_IS_COUNTABLE(*R(*node)))
                SIMPLIFY_EVAL(node);
            break;

        case DiffOperNum::ERR:
        default:
            assert(0 && "Wrong DiffOperNum given");
            return Status::TREE_ERROR;
    }

    TEX_DUMP();

    return Status::NORMAL_WORK;
}
#undef OPER_CASE_
