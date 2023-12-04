#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <assert.h>

#include "diff_objects.h"
#include "utils/statuses.h"
#include "ui/ui.h"

Status::Statuses interface_ask_vars_values(DiffData* diff_data);

Status::Statuses interface_throw_err_var_value_not_given(const char* var_name);

#endif //< #ifndef INTERFACE_H_
