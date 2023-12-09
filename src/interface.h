#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <assert.h>

#include "diff_objects.h"
#include "utils/statuses.h"
#include "ui/ui.h"

Status::Statuses interface_ask_vars_values(DiffData* diff_data, bool ask_x, bool require_all);

Status::Statuses interface_throw_err_var_value_not_given(const char* var_name);

Status::Statuses interface_ask_mode(DiffMode* mode);

Status::Statuses interface_ask_taylor_degree_and_point(size_t* degree, double* point);

Status::Statuses interface_ask_about_plot(DiffMode* mode);

Status::Statuses interface_ask_about_eval(DiffMode* mode);

Status::Statuses interface_error_no_argument();

#endif //< #ifndef INTERFACE_H_
