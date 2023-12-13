#include "interface.h"

static Status::Statuses interface_ask_bool_(const char* question, bool* answer);

#define PRINT_(...) if (printf(__VA_ARGS__) <= 0)   \
                        return Status::INPUT_ERROR

Status::Statuses interface_ask_vars_values(DiffData* diff_data, bool ask_x, bool require_all) {
    assert(diff_data);

    static const size_t MAX_ATTEMPTS_NUMBER = 5;

    if (diff_data->vars.size == 0) {
        PRINT_("Warning. Expression doesn't contain any variales\n");
        return Status::NORMAL_WORK;
    }

    PRINT_("Enter variables values. Or press \"enter\" to skip this var\n");

    size_t attempts = 0;

    for (size_t i = 0; i < diff_data->vars.size; i++) {
        if (!isnan(diff_data->vars.arr[i].val) || (!ask_x && (ssize_t)i == diff_data->vars.argument))
            continue;

        PRINT_("Enter \"%s\" value: ", diff_data->vars.arr[i].name);

        double res = ui_get_double_or_NAN();

        if (isinf(res) || (require_all && isnan(res))) {
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

Status::Statuses interface_ask_taylor_degree_and_point(size_t* degree, double* point) {
    assert(degree);
    assert(point);

    static const size_t MAX_ATTEMPTS_NUMBER = 5;

    ssize_t attempts = MAX_ATTEMPTS_NUMBER;


    while (attempts-- > 0) {
        PRINT_("Enter degree:\n");

        int res = ui_get_int();
        if (res >= 0) {
            *degree = (size_t)res;
            break;
        }

        PRINT_("Incorrect input. Try again\n");
    }

    if (attempts < 0)
        return Status::INPUT_ERROR;


    attempts = MAX_ATTEMPTS_NUMBER;

    while (attempts-- > 0) {
        PRINT_("Enter x:\n");

        *point = ui_get_double_or_NAN();
        if (isfinite(*point))
            break;

        PRINT_("Incorrect input. Try again\n");
    }

    if (attempts < 0)
        return Status::INPUT_ERROR;

    return Status::NORMAL_WORK;
}

Status::Statuses interface_ask_mode(DiffMode* mode) {
    assert(mode);

    static const size_t MAX_ATTEMPTS_NUMBER = 5;

    ssize_t attempts = MAX_ATTEMPTS_NUMBER;

    while (attempts-- > 0) {
        PRINT_("Choose mode:\n"
               "    s - eval or simplificate\n"
               "    d - differentiate\n"
               "    t - expand into Taylor series\n");

        char res = ui_get_char_no_enter();

        switch (res) {
            case 's':
                mode->action = DiffMode::SIMPLIFICATION;

                return Status::NORMAL_WORK;
            case 'd':
                mode->action = DiffMode::DIFF;

                return Status::NORMAL_WORK;
            case 't':
                mode->action = DiffMode::TAYLOR;

                return Status::NORMAL_WORK;

            default:
                PRINT_("Wrong input. Try again\n");
                break;
        }
    }

    PRINT_("Too many attempts\n");

    return Status::INPUT_ERROR;
}

static Status::Statuses interface_ask_bool_(const char* question, bool* answer) {
    assert(question);
    assert(answer);

    static const size_t MAX_ATTEMPTS_NUMBER = 5;

    ssize_t attempts = MAX_ATTEMPTS_NUMBER;

    while (attempts-- > 0) {

        PRINT_("%s (y/n)\n", question);

        char res = ui_get_char_no_enter();

        switch (res) {
            case 'y':
                *answer = true;

                return Status::NORMAL_WORK;
            case 'n':
                *answer = false;

                return Status::NORMAL_WORK;

            default:
                PRINT_("Wrong input. Try again\n");
                break;
        }
    }

    PRINT_("Too many attempts\n");

    return Status::INPUT_ERROR;
}

Status::Statuses interface_ask_about_plot(DiffMode* mode) {
    assert(mode);

    return interface_ask_bool_("Build plot?", &mode->plot);
}

Status::Statuses interface_ask_about_eval(DiffMode* mode) {
    assert(mode);

    return interface_ask_bool_("Evaluate?", &mode->eval);
}

Status::Statuses interface_error_no_argument() {
    PRINT_("Error. Expression doesn't contain argument (\"x\")\n");

    return Status::NORMAL_WORK;
}

Status::Statuses interface_ask_plot_params(double* min, double* max) {
    assert(min);
    assert(max);

    static const size_t MAX_ATTEMPTS_NUM = 5;

    for (size_t i = 0; i < 2; i++) {
        ssize_t attempts = MAX_ATTEMPTS_NUM;

        while (attempts-- > 0) {
            if (i == 0) {
                PRINT_("Enter min value:\n");
            } else if (i == 1) {
                PRINT_("Enter max value:\n");
            }

            double answ = ui_get_double_or_NAN();

            if (isfinite(answ)) {
                if (i == 0) {
                    *min = answ;
                    break;
                } else if (i == 1) {
                    if (answ > *min) {
                        *max = answ;
                        break;
                    }
                }
            }

            PRINT_("Wrong input. Try again\n");
        }

        if (attempts <= 0) {
            PRINT_("Too many attempts\n");
            return Status::INPUT_ERROR;
        }
    }

    return Status::NORMAL_WORK;
}
