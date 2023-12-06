#include "recursive_fall.h"

#include "../dsl.h"

struct Priority {
    enum Enum {
        ADDITIVE       = 0,
        MULTIPLICATIVE = 1,
        POWER          = 2,
        PRIMARY_EXPR   = 3,
        NUMBER         = 4,
    };
};

static Status::Statuses get_G_(DiffData* diff_data, TreeNode** dest, char* text, size_t* pos, size_t* size);

static Status::Statuses get_Binary_oper_(DiffData* diff_data, TreeNode** dest, char* text, size_t* pos,
                                         size_t* size, Priority::Enum priority);

static Status::Statuses get_Primary_(DiffData* diff_data, TreeNode** dest, char* text, size_t* pos, size_t* size);

static Status::Statuses get_Num_Var_(DiffData* diff_data, TreeNode** dest, char* text, size_t* pos, size_t* size);

static Status::Statuses new_oper_node_(TreeNode** dest, TreeNode* l_child, TreeNode* r_child,
                                       const DiffOperNum oper);

static Status::Statuses new_num_node_(TreeNode** dest, double val);

static Status::Statuses new_var_node_(TreeNode** dest, size_t var_num);

inline void skip_spaces(char** text) {
    while ((*text)[0] && isspace((*text)[0]))
        (*text)++;
}

inline void skip_spaces(char* text, size_t* pos) {
    while (text[*pos] && isspace(text[*pos]))
        (*pos)++;
}

static Status::Statuses syntax_error_(const char* format, ...) {
    assert(format);

    va_list args = {};
    va_start(args, format);

    vfprintf(stderr, format, args);

    va_end(args);

    return Status::INPUT_ERROR;
}

Status::Statuses recursive_fall(DiffData* diff_data, char* text) {
    assert(diff_data);
    assert(text);

    skip_spaces(&text);

    size_t pos = 0;
    size_t size = 0;
    TreeNode* root = nullptr;
    STATUS_CHECK(get_G_(diff_data, &root, text, &pos, &size));

    skip_spaces(&text);

    if (root == nullptr && text[0] == '\0')
        return Status::NORMAL_WORK;
    else if (root == nullptr)
        return Status::INPUT_ERROR;

    *ROOT = INSERT_SUBTREE(nullptr, root, size);

    return Status::NORMAL_WORK;
}

static Status::Statuses get_G_(DiffData* diff_data, TreeNode** dest, char* text, size_t* pos,
                               size_t* size) {
    assert(diff_data);
    assert(dest);
    assert(*dest == nullptr);
    assert(text);
    assert(pos);

    STATUS_CHECK(get_Binary_oper_(diff_data, dest, text, pos, size, Priority::ADDITIVE));
    assert(dest);

    skip_spaces(text, pos);

    if (text[*pos] != '\0') {
        DELETE_UNTIED_SUBTREE(dest, *size);
        return syntax_error_("End of file expected, instead of \"%s\"\n", text + *pos);
    }

    return Status::NORMAL_WORK;
}

static Status::Statuses get_Binary_oper_(DiffData* diff_data, TreeNode** dest, char* text, size_t* pos,
                                         size_t* size, Priority::Enum priority) {
    assert(diff_data);
    assert(dest);
    assert(*dest == nullptr);
    assert(text);
    assert(pos);

    if (Priority::ADDITIVE <= priority && priority + 1 <= Priority::POWER)

        STATUS_CHECK(get_Binary_oper_(diff_data, dest, text, pos, size,
                                      (Priority::Enum)(priority + 1)));

    else if (priority + 1 == Priority::PRIMARY_EXPR)

        STATUS_CHECK(get_Primary_(diff_data, dest, text, pos, size));
    else {
        assert(0 && "Wrong priority given");
        return Status::INPUT_ERROR;
    }
    assert(*dest);

    skip_spaces(text, pos);

    const DiffOper* oper = nullptr;
    size_t pos_after_oper = 0;

    while ((oper = diff_get_oper_by_name(text, pos, &pos_after_oper)) != nullptr) {
        TreeNode* new_node = nullptr;

        if (Priority::ADDITIVE <= priority && priority + 1 <= Priority::POWER) {
            *pos = pos_after_oper;

            STATUS_CHECK(get_Binary_oper_(diff_data, &new_node, text, pos, size,
                                          (Priority::Enum)(priority + 1)),
                                                                tree_dtor_untied_subtree(dest));
        } else if (priority + 1 == Priority::PRIMARY_EXPR) {
            *pos = pos_after_oper;

            STATUS_CHECK(get_Primary_(diff_data, &new_node, text, pos, size),
                                                                tree_dtor_untied_subtree(dest));
        } else
            break;
        assert(new_node);

        TreeNode* dest_tmp = nullptr;
        STATUS_CHECK(new_oper_node_(&dest_tmp, *dest, new_node, oper->num),
                                                                tree_dtor_untied_subtree(dest);
                                                                tree_dtor_untied_subtree(&new_node));
        assert(dest_tmp);
        *dest = dest_tmp;
        (*size)++;

        skip_spaces(text, pos);
    }

    return Status::NORMAL_WORK;
}

static Status::Statuses get_Primary_(DiffData* diff_data, TreeNode** dest, char* text, size_t* pos,
                                     size_t* size) {
    assert(size);
    assert(dest);
    assert(*dest == nullptr);
    assert(text);
    assert(pos);

    skip_spaces(text, pos);

    if (text[*pos] == '(') {
        (*pos)++;

        STATUS_CHECK(get_Binary_oper_(diff_data, dest, text, pos, size, Priority::ADDITIVE));
        assert(dest);

        skip_spaces(text, pos);
        if (text[*pos] != ')') {
            DELETE_UNTIED_SUBTREE(dest, *size);
            return syntax_error_("')' expected instead of \"%s\"\n", text + *pos);
        }
        (*pos)++;

        return Status::NORMAL_WORK;
    }

    size_t pos_after_oper = 0;

    const DiffOper* oper = diff_get_oper_by_name(text, pos, &pos_after_oper);

    if (oper != nullptr && oper->type == UNARY) {
        *pos = pos_after_oper;

        skip_spaces(text, pos);

        if (text[*pos] != '(')
            return syntax_error_("'(' expected instead of \"%s\"\n", text + *pos);
        (*pos)++;

        TreeNode* child = nullptr;
        STATUS_CHECK(get_Binary_oper_(diff_data, &child, text, pos, size, Priority::ADDITIVE));
        assert(child);

        skip_spaces(text, pos);
        if (text[*pos] != ')') {
            DELETE_UNTIED_SUBTREE(&child, *size);
            return syntax_error_("')' expected instead of \"%s\"\n", text + *pos);
        }
        (*pos)++;

        STATUS_CHECK(new_oper_node_(dest, nullptr, child, oper->num),
                                                          tree_dtor_untied_subtree(&child));
        (*size)++;
        return Status::NORMAL_WORK;
    }

    STATUS_CHECK(get_Num_Var_(diff_data, dest, text, pos, size));
    assert(*dest);

    return Status::NORMAL_WORK;
}

static Status::Statuses get_Num_Var_(DiffData* diff_data, TreeNode** dest, char* text, size_t* pos,
                                     size_t* size) {
    assert(diff_data);
    assert(dest);
    assert(*dest == nullptr);
    assert(text);
    assert(pos);

    skip_spaces(text, pos);

    size_t var_num = 0;
    if (diff_add_variable(text, pos, &diff_data->vars, &var_num) == true) {

        STATUS_CHECK(new_var_node_(dest, var_num));
        (*size)++;

        return Status::NORMAL_WORK;
    }

    double val = NAN;
    int len = 0;
    if (sscanf(text + *pos, "%lf%n", &val, &len) != 1 || !isfinite(val))
        return syntax_error_("Double number or variable name expected instead of \"%s\"", text + *pos);

    *pos += len;

    STATUS_CHECK(new_num_node_(dest, val));
    (*size)++;

    return Status::NORMAL_WORK;
}

static Status::Statuses new_oper_node_(TreeNode** dest, TreeNode* l_child, TreeNode* r_child,
                                       const DiffOperNum oper) {
    assert(dest);
    assert(*dest == nullptr);
    assert(r_child);
    assert(oper != DiffOperNum::ERR);

    DiffElem new_elem = OPER_ELEM(oper);
    if (tree_node_ctor(dest, &new_elem, sizeof(DiffElem), nullptr) != Tree::OK)
        return Status::TREE_ERROR;

    if (l_child != nullptr)
        l_child->parent = *dest;

    r_child->parent = *dest;

    *L(*dest) = l_child;
    *R(*dest) = r_child;

    return Status::NORMAL_WORK;
}

static Status::Statuses new_num_node_(TreeNode** dest, double val) {
    assert(dest);
    assert(*dest == nullptr);
    assert(isfinite(val));

    DiffElem new_elem = NUM_ELEM(val);
    if (tree_node_ctor(dest, &new_elem, sizeof(DiffElem), nullptr) != Tree::OK)
        return Status::TREE_ERROR;

    return Status::NORMAL_WORK;
}

static Status::Statuses new_var_node_(TreeNode** dest, size_t var_num) {
    assert(dest);
    assert(*dest == nullptr);

    DiffElem new_elem = VAR_ELEM(var_num);
    if (tree_node_ctor(dest, &new_elem, sizeof(DiffElem), nullptr) != Tree::OK)
        return Status::TREE_ERROR;

    return Status::NORMAL_WORK;
}
