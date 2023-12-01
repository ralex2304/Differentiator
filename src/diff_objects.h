#ifndef DIFF_OBJECTS_H_
#define DIFF_OBJECTS_H_

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "config.h"
#include TREE_INCLUDE

enum DiffOperType {
    OPER_TYPE_ERR = 0,
    UNARY         = 1,
    BINARY        = 2,
};

enum class DiffOperNum {
    ERR  = -1,
    ADD  =  0,
    SUB  =  1,
    MUL  =  2,
    DIV  =  3,
    POW  =  4,
    LN   =  5,
    SQRT =  6,
    SIN  =  7,
    COS  =  8,
};

struct DiffOper {
    DiffOperNum num = DiffOperNum::ERR;
    const char* str = nullptr;
    DiffOperType type = OPER_TYPE_ERR;
};

constexpr DiffOper DIFF_OPERS[] = {
    {DiffOperNum::ADD,  "+",    BINARY},
    {DiffOperNum::SUB,  "-",    BINARY},
    {DiffOperNum::MUL,  "*",    BINARY},
    {DiffOperNum::DIV,  "/",    BINARY},
    {DiffOperNum::POW,  "^",    BINARY},
    {DiffOperNum::LN,   "ln",   UNARY},
    {DiffOperNum::SQRT, "sqrt", UNARY},
    {DiffOperNum::SIN,  "sin",  UNARY},
    {DiffOperNum::COS,  "cos",  UNARY},
};

constexpr size_t OPERS_NUM = sizeof(DIFF_OPERS) / sizeof(*DIFF_OPERS);

static_assert((int)(DIFF_OPERS[0].num) == 0);
static_assert((int)(DIFF_OPERS[OPERS_NUM - 1].num) == OPERS_NUM - 1);

struct DiffVar {
    char* name = nullptr;
    double val = NAN;
};

struct DiffVars {
    static const size_t INITIAL_CAP = 256;

    size_t size     = 0;
    size_t capacity = 0;
    DiffVar* arr = nullptr;

    inline bool ctor() {
        arr = (DiffVar*)calloc(INITIAL_CAP, sizeof(*arr));
        if (!arr)
            return false;

        capacity = INITIAL_CAP;
        return true;
    };
    inline void dtor() {
        for (size_t i = 0; i < size; i++)
            FREE(arr[i].name);
        FREE(arr);
    };
    inline bool resize_up() {
        DiffVar* tmp = (DiffVar*)recalloc(arr, capacity, capacity * 2);
        if (tmp == nullptr)
            return false;
        capacity *= 2;
        return true;
    }
};

enum class DiffElemType {
    ERR  = 0,
    OPER = 1,
    NUM  = 2,
    VAR  = 3,
};

union DiffElemData {
    DiffOperNum oper = DiffOperNum::ERR;
    double num;
    size_t var;
};

struct DiffElem {
    DiffElemType type = DiffElemType::ERR;
    DiffElemData data = {};
};

bool diff_elem_dtor(void* elem);

bool diff_elem_verify(void* elem);

char* diff_elem_str_val(const void* elem);

struct DiffData {
    Tree tree = {};
    DiffVars vars = {};

    inline bool ctor() {
        bool res = TREE_CTOR(&tree, sizeof(DiffElem), &diff_elem_dtor,
                             &diff_elem_verify, &diff_elem_str_val) == Tree::OK;
        res |= vars.ctor();
        return res;
    };
    inline void dtor() {
        vars.dtor();
        tree_dtor(&tree);
    };
};

bool diff_math_add_variable(const char* text, long* const i, DiffVars* vars, size_t* var_num);

inline bool is_var_char(const char c) {
    return c == '_' || isalnum(c);
}

#endif //< #ifndef DIFF_OBJECTS_H_
