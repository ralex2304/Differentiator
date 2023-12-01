#include "stack.h"

const Elem_t Stack::POISON = ELEM_T_POISON;

#ifdef DEBUG

#include "log/log.h"

extern LogFileData log_file;

#endif //< #ifdef DEBUG

//==================================================================================================

#ifdef DEBUG
int stk_ctor_debug(Stack* stk, const VarCodeData var_data, const size_t min_capacity) {
    assert(stk);

    stk->var_data = var_data;

    return stk_ctor(stk, min_capacity);
}
#endif //< #ifdef DEBUG

//==================================================================================================

int stk_ctor(Stack* stk, const size_t init_capacity) {
    assert(stk);

    int res = stk->OK;

    if (stk_is_initialised(stk)) {
        res |= stk->ALREADY_INITIALISED;
        STK_OK(stk, res);
        return res;
    }

    if (init_capacity == 0) {
        res |= stk->INVALID_CAPACITY;
        STK_OK(stk, res);
        return res;
    }

    stk->data = (Elem_t*)calloc(calc_stk_data_size(init_capacity), 1);

    if (stk->data == nullptr) {
        res |= stk->ALLOC_ERR;
        STK_OK(stk, res);
        return res;
    }

    stk->capacity = init_capacity;
    stk->size = 0;

#ifdef CANARY_PROTECT
    stk->data = (Elem_t*)(((Canary_t*)stk->data) + 1);

     *left_data_canary_ptr(stk) = CANARY_VAL;
    *right_data_canary_ptr(stk) = CANARY_VAL;
#endif //< #ifdef CANARY_PROTECT

    STK_FILL_POISON(stk, 0, stk->capacity);

#ifdef HASH_PROTECT
    stk->data_hash   =   stk_data_hash_calc(stk);
    stk->struct_hash = stk_struct_hash_calc(stk);
#endif //< #ifdef HASH_PROTECT

    res |= STK_VERIFY(stk);
    STK_OK(stk, res);

    return res;
}

//==================================================================================================

int stk_dtor(Stack* stk) {
    assert(stk);

    int res = STK_VERIFY(stk);
    STK_OK(stk, res);

    STK_FILL_POISON(stk, 0, stk->capacity);

#ifdef CANARY_PROTECT
    free((Canary_t*)stk->data - 1);
    stk->data = nullptr;
#else  //< #ifndef CANARY_PROTECT
    FREE(stk->data);
#endif //< #ifdef CANARY_PROTECT

    stk->capacity = stk->UNITIALISED_CAPACITY;
    stk->size     = stk->UNITIALISED_CAPACITY;

#ifdef HASH_PROTECT
    stk->struct_hash = 0;
    stk->data_hash   = 0;
#endif //< #ifdef HASH_PROTECT

    return res;
}

//==================================================================================================

int stk_resize(Stack* stk, size_t new_size) {
    int res = STK_ASSERT(stk);

    if (new_size == 0) {
        res |= stk->INVALID_CAPACITY;
        STK_OK(stk, res);
        return res;
    }

    Elem_t* tmp = stk->data;

    stk->data = (Elem_t*)recalloc((void*)((Canary_t*)stk->data ON_CANARY_PROTECT(- 1)),
             // We need to clear right canary and free space before it -> + 1
                         stk->capacity * sizeof(Elem_t) ON_CANARY_PROTECT(+ 1 * sizeof(CANARY_VAL)),
                         calc_stk_data_size(new_size));

    if (!stk->data) {
        stk->data = tmp;
        res |= stk->ALLOC_ERR;
        STK_OK(stk, res);
        return res;
    }

    ON_CANARY_PROTECT(stk->data = (Elem_t*)((Canary_t*)stk->data + 1));

    STK_FILL_POISON(stk, stk->capacity, (ssize_t)new_size);

    stk->capacity = new_size;

    ON_CANARY_PROTECT(*right_data_canary_ptr(stk) = CANARY_VAL);

    ON_HASH_PROTECT(stk->struct_hash = stk_struct_hash_calc(stk));

    res |= STK_VERIFY(stk);
    STK_OK(stk, res);

    return res;
}

//==================================================================================================

int stk_push(Stack* stk, const Elem_t elem) {
    int res = STK_ASSERT(stk);

    res |= stk_resize_up(stk);

    if (res != stk->OK)
        return res;

    stk->data[stk->size++] = elem;

#ifdef HASH_PROTECT
    stk->data_hash = stk_data_hash_push(stk, stk->data[stk->size - 1], stk->data_hash);
    stk->struct_hash = stk_struct_hash_calc(stk);
#endif //< #ifdef HASH_PROTECT

    res |= STK_VERIFY(stk);
    STK_OK(stk, res);

    return res;
}

//==================================================================================================

int stk_pop(Stack* stk, Elem_t *const elem) {
    assert(elem);

    int res = STK_ASSERT(stk);

    if (stk->size <= 0) {
        res |= stk->NOTHING_TO_POP;
        STK_OK(stk, res);
        return res;
    }

    *elem = stk->data[--stk->size];

#ifdef HASH_PROTECT
    stk->data_hash = stk_data_hash_calc(stk);
    stk->struct_hash = stk_struct_hash_calc(stk);
#endif //< #ifdef HASH_PROTECT

    ON_DEBUG(stk->data[stk->size] = stk->POISON);

    res |= stk_resize_down(stk);

    res |= STK_VERIFY(stk);
    STK_OK(stk, res);

    return res;
}

//==================================================================================================

#ifdef DEBUG

#define PRINT_ERR_(code, descr)  if ((err_code) & Stack::code)                                    \
                                    stk_log_printf(&log_file,                                         \
                                               HTML_TEXT(HTML_RED("!!! " #code ": " descr "\n")));

void stk_print_error(const int err_code) {
    if (err_code == Stack::OK) {
        stk_log_printf(&log_file, HTML_TEXT(HTML_GREEN("No error\n")));
    } else {
        PRINT_ERR_(ALREADY_INITIALISED,  "Constructor called for already initialised or corrupted stack");
        PRINT_ERR_(UNITIALISED,          "Stack is not initialised");
        PRINT_ERR_(DATA_INVALID_PTR,     "stk.data pointer is not valid for writing");
        PRINT_ERR_(ALLOC_ERR,            "Can't allocate memory");
        PRINT_ERR_(NOTHING_TO_POP,       "Program is trying to pop empty stack");
        PRINT_ERR_(POISON_VAL_FOUND,     "There is poison value in stack");
        PRINT_ERR_(NON_POISON_EMPTY,     "Empty element is not poison");
        PRINT_ERR_(STRUCT_HASH_ERR,      "Stack struct was modified (wrong hash)");
        PRINT_ERR_(DATA_HASH_ERR,        "Stack data was modified (wrong hash)");
        PRINT_ERR_(LOW_CAPACITY,         "stk.size > stk.capacity");
        PRINT_ERR_(NEGATIVE_CAPACITY,    "Negative stk.capacity");
        PRINT_ERR_(INVALID_CAPACITY,     "Invalid capacity given");
        PRINT_ERR_(NEGATIVE_SIZE,        "Negative stk.size");
        PRINT_ERR_(STRUCT_L_CANARY_ERR,  "Left struct canary is corrupt");
        PRINT_ERR_(STRUCT_R_CANARY_ERR,  "Right struct canary is corrupt");
        PRINT_ERR_(DATA_L_CANARY_ERR,    "Left data canary is corrupt");
        PRINT_ERR_(DATA_R_CANARY_ERR,    "Right data canary is corrupt");
    }
}
#undef PRINT_ERR_

#endif //< #ifdef DEBUG

//==================================================================================================

#ifdef DEBUG
void stk_dump(const Stack* stk, const VarCodeData call_data) {
    assert(stk);

    stk_log_printf(&log_file, HTML_BEGIN);

    stk_log_printf(&log_file, "    stk_dump() called from %s:%d %s\n"
                          "    %s[%p] initialised in %s:%d %s \n",
                          call_data.file, call_data.line, call_data.func,
                          stk->var_data.name, stk,
                          stk->var_data.file, stk->var_data.line, stk->var_data.func);

    stk_log_printf(&log_file, "    {\n");

#ifdef CANARY_PROTECT
    stk_log_printf(&log_file, "        L_canary = 0x" CANARY_T_PRINTF "\n", stk->canary_left);
    stk_log_printf(&log_file, "        R_canary = 0x" CANARY_T_PRINTF "\n", stk->canary_right);
#endif //< #ifdef CANARY_PROTECT

    stk_log_printf(&log_file, "        size     = %zd\n"
                          "        capacity = %zd\n", stk->size, stk->capacity);

#ifdef HASH_PROTECT
    stk_log_printf(&log_file, "        struct hash = 0x%X\n"
                          "        data   hash = 0x%X\n", stk->struct_hash, stk->data_hash);
#endif //< #ifdef HASH_PROTECT

    stk_log_printf(&log_file, "        data[%p] {\n", stk->data);

    if (!is_ptr_valid(stk->data)) {

        if (stk_is_initialised(stk))
            stk_log_printf(&log_file, HTML_RED("        can't read (invalid pointer)\n"));

        stk_log_printf(&log_file, "        }\n"
                              "    }\n" HTML_END);
        return;
    }

#ifdef CANARY_PROTECT
    stk_log_printf(&log_file, "            L_canary = 0x" CANARY_T_PRINTF "\n",
               *left_data_canary_ptr(stk));
#endif //< #ifdef CANARY_PROTECT

    int index_len = 1;
    ssize_t index = MAX(stk->size, stk->capacity);
    while ((index /= 10) > 0)
        index_len++;

    for (ssize_t i = 0; i < stk->capacity; i++) {
        if (i < stk->size)
            stk_log_printf(&log_file, "            *");
        else
            stk_log_printf(&log_file, "             ");

        stk_log_printf(&log_file, "[%*zu] ", index_len, i);
        if (stk->data[i] == stk->POISON)
            stk_log_printf(&log_file, "(poison)\n");
        else
            stk_log_printf(&log_file, ELEM_T_PRINTF "\n", stk->data[i]);
    }

#ifdef CANARY_PROTECT
    stk_log_printf(&log_file, "            R_canary = 0x" CANARY_T_PRINTF "\n",
               *right_data_canary_ptr(stk));
#endif //< #ifdef CANARY_PROTECT

    stk_log_printf(&log_file, "        }\n");

    stk_log_printf(&log_file, "    }\n" HTML_END);
}

//==================================================================================================

#define CHECK_ERR_(clause, err) if (clause) res |= err
int stk_verify(Stack* stk) {
    assert(stk);

    int res = stk->OK;

    if (!stk_is_initialised(stk)) {
        res |= stk->UNITIALISED;
        return res;
    }

    bool is_data_valid = true;

    if (!is_ptr_valid(stk->data)) {
        res |= stk->DATA_INVALID_PTR;
        is_data_valid = false;
    }

    if (is_data_valid) {
        for (ssize_t i = 0; i < stk->size; i++)
            if (stk->data[i] == stk->POISON) {
                res |= stk->POISON_VAL_FOUND;
                break;
            }

        for (ssize_t i = stk->size; i < stk->capacity; i++)
            if (stk->data[i] != stk->POISON) {
                res |= stk->NON_POISON_EMPTY;
                break;
            }
    }

#ifdef HASH_PROTECT
    CRC32_t tmp_struct_hash = stk->struct_hash;

    if (is_data_valid) {
        CRC32_t data_hash = stk_data_hash_calc(stk);
        CHECK_ERR_(stk->data_hash != data_hash, stk->DATA_HASH_ERR);
    }

    CRC32_t struct_hash = stk_struct_hash_calc(stk);
    stk->struct_hash = tmp_struct_hash;

    CHECK_ERR_(stk->struct_hash != struct_hash, stk->STRUCT_HASH_ERR);
#endif //< #ifdef HASH_PROTECT

    CHECK_ERR_(stk->capacity < stk->size, stk->LOW_CAPACITY);
    CHECK_ERR_(stk->capacity < 0, stk->NEGATIVE_CAPACITY);
    CHECK_ERR_(stk->size < 0, stk->NEGATIVE_SIZE);

#ifdef CANARY_PROTECT
    CHECK_ERR_(stk->canary_left  != CANARY_VAL, stk->STRUCT_L_CANARY_ERR);
    CHECK_ERR_(stk->canary_right != CANARY_VAL, stk->STRUCT_R_CANARY_ERR);

    if (is_data_valid) {
        CHECK_ERR_( *left_data_canary_ptr(stk) != CANARY_VAL, stk->DATA_L_CANARY_ERR);
        CHECK_ERR_(*right_data_canary_ptr(stk) != CANARY_VAL, stk->DATA_R_CANARY_ERR);
    }
#endif //< #ifdef CANARY_PROTECT

    return res;
}
#undef CHECK_ERR_

#endif //< #ifdef DEBUG
