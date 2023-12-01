#include "diff_objects.h"

static bool diff_math_add_variable_search_(DiffVars* vars, const char* name, const size_t name_size,
                                           size_t* var_num);

static bool diff_math_add_variable_is_banned_name_(const char* name, const size_t name_size);


bool diff_math_add_variable(const char* text, long* const i, DiffVars* vars, size_t* var_num) {
    assert(text);
    assert(i);
    assert(vars);
    assert(var_num);

    const char* name_begin = text + *i;

    while (is_var_char(text[*i])) (*i)++;

    ssize_t name_size = text + *i - name_begin;

    if (name_size <= 0 || diff_math_add_variable_is_banned_name_(name_begin, name_size))
        return false;

    if (diff_math_add_variable_search_(vars, name_begin, name_size, var_num))
        return true;

    while (vars->size >= vars->capacity) {
        if (!vars->resize_up())
            return false;
    }

    vars->arr[vars->size].name = strndup(name_begin, name_size);
    if (vars->arr[vars->size].name == nullptr)
        return false;

    *var_num = vars->size;

    vars->size++;

    return true;
}

static bool diff_math_add_variable_search_(DiffVars* vars, const char* name, const size_t name_size,
                                           size_t* var_num) {
    assert(vars);
    assert(name);
    assert(var_num);

    for (size_t i = 0; i < vars->size; i++) {
        if (strncmp(name, vars->arr[i].name, name_size) == 0) {
            *var_num = i;
            return true;
        }
    }
    return false;
}

static bool diff_math_add_variable_is_banned_name_(const char* name, const size_t name_size) {
    assert(name);
    assert(name_size >= 1);
    // banned format: A, A123

    if (!('A' <= name[0] && name[0] <= 'Z'))
        return false;

    for (size_t i = 1; i < name_size; i++)
        if (!('0' <= name[i] && name[i] <= '9'))
            return false;

    for (size_t i = 0; i < OPERS_NUM; i++)
        if (strncmp(name, DIFF_OPERS[i].str, name_size) == 0)
            return false;

    return true;
}

bool diff_elem_dtor(void* elem) {
    assert(elem);

#ifdef DEBUG
    ((DiffElem*)elem)->type      = DiffElemType::ERR;
    ((DiffElem*)elem)->data.oper = DiffOperNum::ERR;
#endif //< #ifdef DEBUG

    return true;
}

bool diff_elem_verify(void* elem) {
    assert(elem);

    if (((DiffElem*)elem)->type == DiffElemType::ERR) {
        if (((DiffElem*)elem)->data.oper != DiffOperNum::ERR)
            return false;

    } else if (((DiffElem*)elem)->type == DiffElemType::OPER) {
        if (((DiffElem*)elem)->data.oper == DiffOperNum::ERR)
            return false;

    } else if (((DiffElem*)elem)->type == DiffElemType::NUM) {
        if (!isfinite(((DiffElem*)elem)->data.num))
            return false;

    } else if (((DiffElem*)elem)->type == DiffElemType::VAR) {

    } else {
        return false;
    }

    return true;
}

#define OPER_CASE_(enum_, symbol_)  case DiffOperNum::enum_:                        \
                                        snprintf(str, MAX_ELEM_STR_LEN, symbol_);   \
                                        break;

char* diff_elem_str_val(const void* elem) {
    assert(elem);

    static const size_t MAX_ELEM_STR_LEN = 128;

    char* str = (char*)calloc(MAX_ELEM_STR_LEN, sizeof(char));
    if (str == nullptr)
        return nullptr;

    if (((const DiffElem*)elem)->type == DiffElemType::NUM) {
        snprintf(str, MAX_ELEM_STR_LEN, "%lf", ((const DiffElem*)elem)->data.num);

    } else if (((const DiffElem*)elem)->type == DiffElemType::OPER) {

        if ((ssize_t)((const DiffElem*)elem)->data.oper < 0 ||
            (size_t)((const DiffElem*)elem)->data.oper >= OPERS_NUM) {

            snprintf(str, MAX_ELEM_STR_LEN, "Unknown operation: %d",
                                        (int)((const DiffElem*)elem)->data.oper);
        } else {
            snprintf(str, MAX_ELEM_STR_LEN, "%s",
                     DIFF_OPERS[(ssize_t)((const DiffElem*)elem)->data.oper].str);
        }
    } else if (((const DiffElem*)elem)->type == DiffElemType::VAR) {
        snprintf(str, MAX_ELEM_STR_LEN, "Var: %zu", ((const DiffElem*)elem)->data.var);

    } else {
        snprintf(str, MAX_ELEM_STR_LEN, "Unknown type: %d", (int)((const DiffElem*)elem)->type);
    }

    return str;
}
#undef OPER_CASE_
