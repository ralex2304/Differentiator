#include "eval.h"

static Status::Statuses diff_eval_oper_(DiffData* diff_data, TreeNode** node, double* result);

Status::Statuses diff_eval(DiffData* diff_data, TreeNode** node, double* result) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(result);

    switch (((DiffElem*)(*node)->elem)->type) {
        case DiffElemType::NUM:
            *result = ((DiffElem*)(*node)->elem)->data.num;
            break;
        case DiffElemType::VAR:
            *result = diff_data->vars.arr[((DiffElem*)(*node)->elem)->data.var].val;
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


#define OPER_CASE_(oper_, result_)  case DiffOperNum::oper_:                \
                                        *result = result_;                  \
                                        break;

static Status::Statuses diff_eval_oper_(DiffData* diff_data, TreeNode** node, double* result) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(((DiffElem*)(*node)->elem)->type == DiffElemType::OPER);
    assert(((DiffElem*)(*node)->elem)->data.oper != DiffOperNum::ERR);
    assert(result);

    double  left_res = NAN;
    double right_res = NAN;

    if (DIFF_OPERS[(size_t)(((DiffElem*)(*node)->elem)->data.oper)].type != UNARY)
        STATUS_CHECK(diff_eval(diff_data, &(*node)->left, &left_res));

    STATUS_CHECK(diff_eval(diff_data, &(*node)->right, &right_res));

    switch ((((DiffElem*)(*node)->elem)->data.oper)) {

        OPER_CASE_(ADD,  left_res + right_res);
        OPER_CASE_(SUB,  left_res - right_res);
        OPER_CASE_(MUL,  left_res * right_res);
        OPER_CASE_(DIV,  left_res / right_res);
        OPER_CASE_(POW,  pow(left_res, right_res));

        OPER_CASE_(LN,   log(right_res));
        OPER_CASE_(SQRT, sqrt(right_res));
        OPER_CASE_(SIN,  sin(right_res));
        OPER_CASE_(COS,  cos(right_res));

        case DiffOperNum::ERR:
        default:
            assert(0 && "Invalid DiffOperNum given");
            return Status::TREE_ERROR;
    }

    return Status::NORMAL_WORK;
}
