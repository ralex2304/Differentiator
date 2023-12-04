#include "interface.h"

#define PRINT_(...) if (printf(__VA_ARGS__) <= 0)   \
                        return Status::INPUT_ERROR

Status::Statuses interface_ask_vars_values(DiffData* diff_data) {
    assert(diff_data);

    static const size_t MAX_ATTEMPTS_NUMBER = 5;

    if (diff_data->vars.size == 0) {
        PRINT_("Warning. Expression doesn't contain any variales\n");
        return Status::NORMAL_WORK;
    }

    PRINT_("Enter variables values. Or press \"enter\" to skip this var\n");

    size_t attempts = 0;

    for (size_t i = 0; i < diff_data->vars.size; i++) {
        PRINT_("Enter \"%s\" value: ", diff_data->vars.arr[i].name);

        double res = ui_get_double_or_NAN();

        if (isinf(res)) {
            if (++attempts >= MAX_ATTEMPTS_NUMBER) {
                PRINT_("Invalid input. Too many attempts\n");
                return Status::INPUT_ERROR;
            }

            PRINT_("Invalid input. Try again\n");
            i--;
            continue;
        }

        attempts = 0;

        diff_data->vars.arr[i].val = res;
    }

    return Status::NORMAL_WORK;
}

Status::Statuses interface_throw_err_var_value_not_given(const char* var_name) {
    assert(var_name);

    PRINT_("Evaluation error. Value of variable \"%s\" was not given\n", var_name);

    return Status::NORMAL_WORK;
}
