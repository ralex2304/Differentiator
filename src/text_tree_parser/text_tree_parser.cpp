#include "text_tree_parser.h"

static Status::Statuses text_tree_parser_new_node_(DiffData* diff_data, TreeNode** node,
                                                   TreeNode* parent,
                                                   char* text, size_t* const i);

static Status::Statuses text_tree_parser_new_node_children_(DiffData* diff_data, TreeNode** node,
                                                            char* text, size_t* const i,
                                                            TreeChildSide side);

static Status::Statuses text_tree_parser_insert_node_(Tree* tree, TreeNode** node,
                                                      TreeNode* parent);

static Status::Statuses text_tree_parser_read_node_oper_(DiffData* diff_data, TreeNode** node,
                                                         char* text, size_t* const i);

static Status::Statuses text_tree_parser_new_node_num_var_or_unary_oper_(DiffData* diff_data,
                            TreeNode** node, char* text, size_t* const i);

static DiffOperNum text_tree_parse_get_oper_by_text_(char* text, size_t* const i);

#define PRINT_ERR_(...) printf("Tree file err: " __VA_ARGS__)

inline static Status::Statuses skip_spaces_and_char_(const char* text, size_t* const i, const char c) {
    assert(text);
    assert(i);

    while (text[*i] && isspace(text[*i])) (*i)++;

    if (text[*i] != c) {
        PRINT_ERR_("'%c' expected\n", c);
        return Status::INPUT_ERROR;
    }

    (*i)++;
    return Status::NORMAL_WORK;
}

inline static Status::Statuses skip_spaces_(const char* text, size_t* const i) {
    assert(text);
    assert(i);

    while (text[*i] && isspace(text[*i])) (*i)++;

    if (text[*i] == '\0') {
        PRINT_ERR_("File ended unexpectedly\n");
        return Status::INPUT_ERROR;
    }
    return Status::NORMAL_WORK;
}

Status::Statuses text_tree_parser(DiffData* diff_data, char* text) {
    assert(diff_data);
    assert(text);

    size_t i = 0;

    while (text[i] && isspace(text[i])) i++;

    if (text[i] == '\0') return Status::NORMAL_WORK; //< Empty file is empty tree
    if (text[i++] != '(') {
        PRINT_ERR_("file must begin with '('\n");
        return Status::INPUT_ERROR;
    }

    STATUS_CHECK(text_tree_parser_new_node_(diff_data, &diff_data->tree.root, nullptr, text, &i));


    STATUS_CHECK(skip_spaces_and_char_(text, &i, ')'));

    while (text[i] != '\0') {
        if (!isspace(text[i])) {
            PRINT_ERR_("File doesn't end at the end of the tree\n");
            return Status::INPUT_ERROR;
        }
        i++;
    }

    return Status::NORMAL_WORK;
}

static Status::Statuses text_tree_parser_new_node_(DiffData* diff_data, TreeNode** node,
                                                   TreeNode* parent,
                                                   char* text, size_t* const i) {
    assert(diff_data);
    assert(node);
    assert(text);
    assert(i);

    STATUS_CHECK(text_tree_parser_insert_node_(&diff_data->tree, node, parent));
    assert(*node);

    STATUS_CHECK(skip_spaces_(text, i));

    if (text[*i] != '(') {
        STATUS_CHECK(text_tree_parser_new_node_num_var_or_unary_oper_(diff_data, node, text, i));

        if (((DiffElem*)((*node)->elem))->type == DiffElemType::OPER) {
            STATUS_CHECK(text_tree_parser_new_node_children_(diff_data, node, text, i, TreeChildSide::RIGHT));
        }

        return Status::NORMAL_WORK;
    }

    STATUS_CHECK(text_tree_parser_new_node_children_(diff_data, node, text, i, TreeChildSide::LEFT));

    STATUS_CHECK(text_tree_parser_read_node_oper_(diff_data, node, text, i));

    STATUS_CHECK(text_tree_parser_new_node_children_(diff_data, node, text, i, TreeChildSide::RIGHT));

    return Status::NORMAL_WORK;
}

static Status::Statuses text_tree_parser_new_node_num_var_or_unary_oper_(DiffData* diff_data,
                                    TreeNode** node, char* text, size_t* const i) {
    assert(diff_data);
    assert(node);
    assert(text);
    assert(i);

    DiffElem elem = {};
    bool is_found = false;


    char* d_endptr = nullptr;
    double num = strtod(text + *i, &d_endptr);

    if (d_endptr != nullptr && d_endptr - (text + *i) > 0) {

        elem = {.type = DiffElemType::NUM,
                .data = {.num = num}};
        is_found = true;

        (*i) += d_endptr - (text + *i);
    }

    DiffOperNum unary_oper = DiffOperNum::ERR;

    if (!is_found && (unary_oper = text_tree_parse_get_oper_by_text_(text, i)) != DiffOperNum::ERR &&
                      DIFF_OPERS[(ssize_t)unary_oper].type == DiffOperType::UNARY) {

        elem = {.type = DiffElemType::OPER,
                .data = {.oper = unary_oper}};
        is_found = true;
    }

    size_t var = 0;

    if (!is_found && diff_add_variable(text, i, &diff_data->vars, &var) == true) {

        elem = {.type = DiffElemType::VAR,
                .data = {.var = var}};
        is_found = true;
    }

    if (!is_found) {
        PRINT_ERR_("Unexpected symbol: '%c'\n", text[*i]);
        return Status::INPUT_ERROR;
    }

    if (tree_set_elem(&diff_data->tree, *node, &elem, false) != Tree::OK) {
        PRINT_ERR_("Tree error: can't change node elem value\n");
        return Status::TREE_ERROR;
    }

    STATUS_CHECK(skip_spaces_(text, i));

    return Status::NORMAL_WORK;
}

static Status::Statuses text_tree_parser_new_node_children_(DiffData* diff_data, TreeNode** node,
                                                            char* text, size_t* i,
                                                            TreeChildSide side) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(text);
    assert(i);
    assert(side == TreeChildSide::LEFT || side == TreeChildSide::RIGHT);

    STATUS_CHECK(skip_spaces_and_char_(text, i, '('));

    if (side == TreeChildSide::LEFT)
        STATUS_CHECK(text_tree_parser_new_node_(diff_data, &(*node)->left, *node, text, i));
    else if (side == TreeChildSide::RIGHT)
        STATUS_CHECK(text_tree_parser_new_node_(diff_data, &(*node)->right, *node, text, i));
    else
        assert(0 && "Wrong TreeChildSide given");

    STATUS_CHECK(skip_spaces_and_char_(text, i, ')'));

    return Status::NORMAL_WORK;
}

static DiffOperNum text_tree_parse_get_oper_by_text_(char* text, size_t* const i) {
    assert(text);
    assert(i);

    DiffOperNum ret = DiffOperNum::ERR;

    for (size_t op = 0; op < OPERS_NUM; op++) {

        size_t oper_len = strlen(DIFF_OPERS[op].str);
        if (strncmp(DIFF_OPERS[op].str, text + *i, oper_len) == 0 &&
            !is_var_char(text[*i + oper_len])) {

            ret = DIFF_OPERS[op].num;
            (*i) += oper_len;
            return ret;
        }
    }

    return DiffOperNum::ERR;
}

#undef OPER_CASE_
static Status::Statuses text_tree_parser_read_node_oper_(DiffData* diff_data, TreeNode** node,
                                                         char* text, size_t* const i) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(text);
    assert(i);

    STATUS_CHECK(skip_spaces_(text, i));

    DiffElem elem = {.type = DiffElemType::OPER};

    if ((elem.data.oper = text_tree_parse_get_oper_by_text_(text, i)) == DiffOperNum::ERR) {
        PRINT_ERR_("Unknown operator: '%c'\n", text[*i]);
        return Status::INPUT_ERROR;
    }

    if (tree_set_elem(&diff_data->tree, *node, &elem, false) != Tree::OK) {
        PRINT_ERR_("Tree error: can't change node elem value\n");
        return Status::TREE_ERROR;
    }

    STATUS_CHECK(skip_spaces_(text, i));

    return Status::NORMAL_WORK;
}

static Status::Statuses text_tree_parser_insert_node_(Tree* tree, TreeNode** node, TreeNode* parent) {
    assert(tree);
    assert(node);
    assert(*node == nullptr);
    assert(parent || tree->root == nullptr);

    DiffElem empty_elem = {};

    if (tree_insert(tree, node, parent, &empty_elem) != Tree::OK) {
        PRINT_ERR_("Tree error\n");
        return Status::TREE_ERROR;
    }

    return Status::NORMAL_WORK;
}

#undef SKIP_SPACE_
#undef SKIP_TO_CHAR_
#undef PRINT_ERR_
