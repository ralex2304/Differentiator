#include "eval.h"

#include "../dsl.h"

static Status::Statuses diff_eval_oper_(DiffData* diff_data, TreeNode** node, double* result);


Status::Statuses diff_eval(DiffData* diff_data, TreeNode** node, double* result) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(result);

    switch (*NODE_TYPE(*node)) {
        case DiffElemType::NUM:
            *result = *NUM_VAL(*node);
            break;
        case DiffElemType::VAR:
            *result = *VAR_VAL(*node);

            if (isnan(*result)) {
                STATUS_CHECK(interface_throw_err_var_value_not_given(*VAR_NAME(*node)));
                return Status::INPUT_ERROR;
            }
            break;
        case DiffElemType::OPER:
            STATUS_CHECK(diff_eval_oper_(diff_data, node, result));
            break;
        case DiffElemType::ERR:
        default:
            assert(0 && "Invalid DiffElemType given");
            return Status::TREE_ERROR;
    }
    return Status::NORMAL_WORK;
}

static Status::Statuses diff_eval_oper_(DiffData* diff_data, TreeNode** node, double* result) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(TYPE_IS_OPER(*node));
    assert(result);

    double  left_res = NAN;
    double right_res = NAN;

    if (IS_BINARY(*node))
        STATUS_CHECK(diff_eval(diff_data, L(*node), &left_res));

    STATUS_CHECK(diff_eval(diff_data, R(*node), &right_res));

    STATUS_CHECK(diff_eval_calc(left_res, right_res, *OPER_NUM(*node), result));

    return Status::NORMAL_WORK;
}

#define OPER_CASE_(oper_, result_)  case DiffOperNum::oper_:                \
                                        *result = result_;                  \
                                        break;

Status::Statuses diff_eval_calc(const double left_val, const double right_val,
                                 DiffOperNum oper, double* result) {
    assert(result);

    switch (oper) {

        OPER_CASE_(ADD,  left_val + right_val);
        OPER_CASE_(SUB,  left_val - right_val);
        OPER_CASE_(MUL,  left_val * right_val);
        OPER_CASE_(DIV,  left_val / right_val);
        OPER_CASE_(POW,  pow(left_val, right_val));

        OPER_CASE_(LN,   log(right_val));
        OPER_CASE_(SQRT, sqrt(right_val));
        OPER_CASE_(SIN,  sin(right_val));
        OPER_CASE_(COS,  cos(right_val));

        case DiffOperNum::ERR:
        default:
            assert(0 && "Invalid DiffOperNum given");
            return Status::TREE_ERROR;
    }

    return Status::NORMAL_WORK;
}
#undef OPER_CASE_
