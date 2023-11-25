#ifndef DIFF_MATH_H_
#define DIFF_MATH_H_

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "config.h"

#ifdef DEBUG

#include "TreeDebug/TreeDebug.h"

#else //< #ifndef DEBUG

#include "Tree/Tree.h"

#endif //< #ifdef DEBUG

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

enum class DiffOper {
    ERR = 0,
    ADD = 1,
    SUB = 2,
    MUL = 3,
    DIV = 4,
    POW = 5,
};

enum class DiffElemType {
    ERR  = 0,
    OPER = 1,
    NUM  = 2,
    VAR  = 3,
};

union DiffElemData {
    DiffOper oper = DiffOper::ERR;
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
    inline void dtor() { vars.dtor(); };
};

bool diff_math_add_variable(const char* text, long* const i, DiffVars* vars, size_t* var_num);


#endif //< #ifndef DIFF_MATH_H_
