#include "diff_objects.h"

static bool diff_add_variable_search_(DiffVars* vars, const char* name, const size_t name_size,
                                           size_t* var_num);

static bool diff_add_variable_is_banned_name_(const char* name, const size_t name_size);


const DiffOper* diff_get_oper_by_name(const char* text, const size_t* const pos, size_t* const new_pos) {
    assert(text);
    assert(pos);
    assert(new_pos);

    for (size_t op = 0; op < OPERS_NUM; op++) {

        size_t oper_len = strlen(DIFF_OPERS[op].str);
        if (strncmp(DIFF_OPERS[op].str, text + *pos, oper_len) == 0 &&
            !is_var_char(text[*pos + oper_len])) {

            *new_pos = *pos + oper_len;

            return DIFF_OPERS + op;
        }
    }

    return nullptr;
}

bool diff_add_variable(const char* text, size_t* const pos, DiffVars* vars, size_t* var_num) {
    assert(text);
    assert(pos);
    assert(vars);
    assert(var_num);

    size_t local_pos = *pos;

    if (!isalpha(text[local_pos])) //< Var name must start with alpha
        return false;

    while (is_var_char(text[local_pos])) local_pos++;

    ssize_t name_size = local_pos - *pos;

    if (name_size <= 0 || diff_add_variable_is_banned_name_(text + *pos, name_size))
        return false;

    if (diff_add_variable_search_(vars, text + *pos, name_size, var_num)) {
        *pos = local_pos;
        return true;
    }

    while (vars->size >= vars->capacity) {
        if (!vars->resize_up())
            return false;
    }

    vars->arr[vars->size].name = strndup(text + *pos, name_size);
    if (vars->arr[vars->size].name == nullptr)
        return false;

    vars->arr[vars->size].val = NAN;

    *var_num = vars->size;

    vars->size++;

    *pos = local_pos;

    return true;
}

bool diff_exists_var_with_no_value(DiffData* diff_data, bool except_x) {
    assert(diff_data);

    for (size_t i = 0; i < diff_data->vars.size; i++) {
        if (except_x && (ssize_t)i == diff_data->vars.argument)
            continue;

        if (isnan(diff_data->vars.arr[i].val))
            return true;
    }

    return false;
}

static bool diff_add_variable_search_(DiffVars* vars, const char* name, const size_t name_size,
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

static bool diff_add_variable_is_banned_name_(const char* name, const size_t name_size) {
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

    ((DiffElem*)elem)->will_be_diffed = false;
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

    char* str = (char*)calloc(MAX_ELEM_STR_LEN + 1, sizeof(char)); //< +1 for '
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

    if (((const DiffElem*)elem)->will_be_diffed)
        strcat(str, "'");

    return str;
}
#undef OPER_CASE_
