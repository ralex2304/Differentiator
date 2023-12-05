#ifndef DIFF_OBJECTS_H_
#define DIFF_OBJECTS_H_

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "file/file.h"

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

    ssize_t argument = -1;

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

        argument = -1;
    };
    inline void search_argument() {
        for (size_t i = 0; i < size; i++)
            if (strcmp(DIFF_VAR_NAME, arr[i].name) == 0) {
                argument = i;
                break;
            }
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

    bool will_be_diffed = false; //< flag for tex dump
};

bool diff_elem_dtor(void* elem);

bool diff_elem_verify(void* elem);

char* diff_elem_str_val(const void* elem);

struct DiffData {
    static const size_t MAX_PATH_LEN = 256;

    Tree tree = {};
    DiffVars vars = {};

    FILE* tex_file = {};
    char tex_filename[MAX_PATH_LEN] = {};

    bool simplify_substitute_vars = false;
    bool tree_changed = false;

    inline bool ctor() {
        bool res = TREE_CTOR(&tree, sizeof(DiffElem), &diff_elem_dtor,
                             &diff_elem_verify, &diff_elem_str_val) == Tree::OK;
        res |= vars.ctor();
        tree_changed = true;
        return res;
    };
    inline void dtor() {
        vars.dtor();
        tree_dtor(&tree);
        if (tex_file != nullptr) {
            fclose(tex_file);
            tex_file = nullptr;
            tex_filename[0] = '\0';
        }
        tree_changed = false;
        simplify_substitute_vars = false;
    };
    inline bool open_tex(const char* directory, const char* mode = "wb") {
        assert(tex_file == nullptr);
        assert(directory);
        assert(mode);

        snprintf(tex_filename, MAX_PATH_LEN, "%s/%s", directory, "article.tex");

        return file_open(&tex_file, tex_filename, mode);
    }
    inline bool close_tex() {
        assert(tex_file);

        bool res = file_close(tex_file);
        tex_file = nullptr;
        return res;
    }
};

bool diff_math_add_variable(const char* text, long* const i, DiffVars* vars, size_t* var_num);

inline bool is_var_char(const char c) {
    return c == '_' || isalnum(c);
}

#endif //< #ifndef DIFF_OBJECTS_H_
