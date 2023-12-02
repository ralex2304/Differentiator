#ifndef DIFF_MATH_EVAL_H_
#define DIFF_MATH_EVAL_H_

#include <assert.h>

#include "../utils/statuses.h"
#include "../diff_objects.h"
#include "../config.h"
#include TREE_INCLUDE

Status::Statuses diff_eval(DiffData* diff_data, TreeNode** node, double* result);

Status::Statuses diff_eval_calc(const double left_val, const double right_val,
                                DiffOperNum oper, double* result);

#endif //< #ifndef DIFF_MATH_EVAL_H_
