#include "text_tree_parser.h"

static Status::Statuses text_tree_parser_new_node_(DiffData* diff_data, TreeNode** node,
                                                   TreeNode* parent,
                                                   char* text, long* const i);

static Status::Statuses text_tree_parser_new_node_children_(DiffData* diff_data, TreeNode** node,
                                                            char* text, long* i,
                                                            TreeChildSide side);

static Status::Statuses text_tree_parser_insert_node_(Tree* tree, TreeNode** node,
                                                      TreeNode* parent);

static Status::Statuses text_tree_parser_read_node_oper_(DiffData* diff_data, TreeNode** node,
                                                         char* text, long* const i);

static Status::Statuses text_tree_parser_read_node_num_var_(DiffData* diff_data, TreeNode** node,
                                                            char* text, long* const i);

static TreeNodeActionRes text_tree_write_prefix_action_(Tree* tree, TreeNode** node, va_list* args,
                                                        size_t depth, const TreeChildSide side);

static TreeNodeActionRes text_tree_write_postfix_action_(Tree* tree, TreeNode** node, va_list* args,
                                                         size_t depth, const TreeChildSide side);

static TreeNodeActionRes text_tree_write_nil_action_(Tree* tree, TreeNode** node, va_list* args,
                                                     size_t depth, const TreeChildSide side);

#define PRINT_ERR_(...) printf("Tree file err: " __VA_ARGS__)

inline static Status::Statuses skip_to_char_(const char* text, long* const i, const char c) {
    assert(text);
    assert(i);

    while (text[*i] && isspace(text[*i])) (*i)++;

    if (text[*i] != c) {
        PRINT_ERR_("'%c' expected\n", c);
        return Status::INPUT_ERROR;
    }
    return Status::NORMAL_WORK;
}

inline static Status::Statuses skip_spaces_(const char* text, long* const i) {
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

    long i = 0;

    while (text[i] && isspace(text[i])) i++;

    if (text[i] == '\0') return Status::NORMAL_WORK; //< Empty file is empty tree
    if (text[i++] != '(') {
        PRINT_ERR_("file must begin with '('\n");
        return Status::INPUT_ERROR;
    }

    STATUS_CHECK(skip_spaces_(text, &i));

    STATUS_CHECK(text_tree_parser_new_node_(diff_data, &diff_data->tree.root, nullptr, text, &i));

    STATUS_CHECK(skip_to_char_(text, &i, ')'));

    return Status::NORMAL_WORK;
}

static Status::Statuses text_tree_parser_new_node_(DiffData* diff_data, TreeNode** node,
                                                   TreeNode* parent,
                                                   char* text, long* const i) {
    assert(diff_data);
    assert(node);
    assert(text);
    assert(i);

    STATUS_CHECK(text_tree_parser_insert_node_(&diff_data->tree, node, parent));
    assert(*node);

    if (text[*i] != '(') {
        STATUS_CHECK(text_tree_parser_read_node_num_var_(diff_data, node, text, i));

        return Status::NORMAL_WORK;
    }

    STATUS_CHECK(text_tree_parser_new_node_children_(diff_data, node, text, i, TreeChildSide::LEFT));
    (*i)++;

    STATUS_CHECK(skip_spaces_(text, i));

    STATUS_CHECK(text_tree_parser_read_node_oper_(diff_data, node, text, i));

    STATUS_CHECK(skip_spaces_(text, i));

    STATUS_CHECK(text_tree_parser_new_node_children_(diff_data, node, text, i, TreeChildSide::RIGHT));
    (*i)++;

    return Status::NORMAL_WORK;
}

static Status::Statuses text_tree_parser_new_node_children_(DiffData* diff_data, TreeNode** node,
                                                            char* text, long* i,
                                                            TreeChildSide side) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(text);
    assert(i);
    assert(side == TreeChildSide::LEFT || side == TreeChildSide::RIGHT);

    STATUS_CHECK(skip_spaces_(text, i));

    if (text[*i] == '(') {
        (*i)++;
        if (side == TreeChildSide::LEFT)
            STATUS_CHECK(text_tree_parser_new_node_(diff_data, &(*node)->left, *node, text, i));
        else if (side == TreeChildSide::RIGHT)
            STATUS_CHECK(text_tree_parser_new_node_(diff_data, &(*node)->right, *node, text, i));
        else
            assert(0 && "Wrong TreeChildSide given");

        STATUS_CHECK(skip_to_char_(text, i, ')'));

    } else if (strncmp("nil", text + *i, 3) == 0) {
        (*i) += 3 - 1;
    } else {
        PRINT_ERR_("Unexpected symbol '%c'. Expected '(' or 'nil'\n", text[*i]);
        return Status::INPUT_ERROR;
    }

    return Status::NORMAL_WORK;
}

#define OPER_CASE_(oper_, enum_)    case oper_:                             \
                                        elem.data.oper = DiffOper::enum_;   \
                                        break;

static Status::Statuses text_tree_parser_read_node_oper_(DiffData* diff_data, TreeNode** node,
                                                         char* text, long* const i) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(text);
    assert(i);

    DiffElem elem = {.type = DiffElemType::OPER};

    switch (text[*i]) {
        OPER_CASE_('+', ADD);
        OPER_CASE_('-', SUB);
        OPER_CASE_('*', MUL);
        OPER_CASE_('/', DIV);
        OPER_CASE_('^', POW);
        default:
            PRINT_ERR_("Unknown operator: '%c'\n", text[*i]);
            return Status::INPUT_ERROR;
    }

    (*i)++;

    if (tree_set_elem(&diff_data->tree, *node, &elem, false) != Tree::OK) {
        PRINT_ERR_("Tree error: can't change node elem value\n");
        return Status::TREE_ERROR;
    }

    return Status::NORMAL_WORK;
}
#undef OPER_CASE_

static Status::Statuses text_tree_parser_read_node_num_var_(DiffData* diff_data, TreeNode** node,
                                                            char* text, long* const i) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(text);
    assert(i);

    double num = NAN;
    int sscanf_n = -1;

    DiffElem elem = {};
    bool is_found = false;

    if (sscanf(text + *i, "%lf%n", &num, &sscanf_n) == 1) {

        elem = {.type = DiffElemType::NUM,
                .data = {.num = num}};
        is_found = true;

        (*i) += sscanf_n;
    }

    size_t var = 0;

    if (!is_found && diff_math_add_variable(text, i, &diff_data->vars, &var) == true) {

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

Status::Statuses text_tree_save(Tree* tree, const char* filename) {
    assert(tree);
    assert(filename);

    FILE* file = {};
    if (!file_open(&file, filename, "wb"))
        return Status::FILE_ERROR;

    TreeNodeAction* actions[4] = {&text_tree_write_prefix_action_, nullptr,
                                  &text_tree_write_postfix_action_, &text_tree_write_nil_action_};

    if (tree_traversal(tree, &tree->root, actions, file) != Tree::OK)
        return Status::TREE_ERROR;

    if (!file_close(file))
        return Status::FILE_ERROR;

    return Status::NORMAL_WORK;
}
#undef PRINT_ERR_

static TreeNodeActionRes text_tree_write_prefix_action_(Tree* tree, TreeNode** node, va_list* args,
                                                        size_t depth, const TreeChildSide side) {
    (void) tree;
    (void) depth;
    (void) side;
    assert(node);
    assert(*node);
    assert(args);

    va_list args_dup = {};
    va_copy(args_dup, *args);

    FILE* file = va_arg(args_dup, FILE*);

    va_end(args_dup);

    if (fprintf(file, "( \"%s\" ", "123") <= 0) // FIXME
        return TreeNodeActionRes::ERR;

    return TreeNodeActionRes::OK;
}

static TreeNodeActionRes text_tree_write_nil_action_(Tree* tree, TreeNode** node, va_list* args,
                                                     size_t depth, const TreeChildSide side) {
    (void) tree;
    (void) depth;
    (void) side;
    assert(node);
    assert(*node == nullptr);
    assert(args);

    va_list args_dup = {};
    va_copy(args_dup, *args);

    FILE* file = va_arg(args_dup, FILE*);

    va_end(args_dup);

    if (fprintf(file, "nil ") <= 0)
        return TreeNodeActionRes::ERR;

    return TreeNodeActionRes::OK;
}

static TreeNodeActionRes text_tree_write_postfix_action_(Tree* tree, TreeNode** node, va_list* args,
                                                         size_t depth, const TreeChildSide side) {
    (void) tree;
    (void) depth;
    (void) side;
    assert(node);
    assert(*node);
    assert(args);

    va_list args_dup = {};
    va_copy(args_dup, *args);

    FILE* file = va_arg(args_dup, FILE*);

    va_end(args_dup);

    if (fprintf(file, ") ") <= 0)
        return TreeNodeActionRes::ERR;

    return TreeNodeActionRes::OK;
}

