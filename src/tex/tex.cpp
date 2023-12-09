#include "tex.h"

#include "../dsl.h"

static Status::Statuses tex_write_phrase_(DiffData* diff_data);

static Status::Statuses tex_write_header_(DiffData* diff_data, const DiffMode mode);

static Status::Statuses tex_write_footer_(DiffData* diff_data);

static Status::Statuses tex_dump_traversal_(DiffData* diff_data, TreeNode** node);

static Status::Statuses tex_dump_traversal_write_node_(DiffData* diff_data, TreeNode** node);

static Status::Statuses tex_dump_traversal_write_oper_(DiffData* diff_data, TreeNode** node);

static Status::Statuses tex_print_double_(DiffData* diff_data, const double val);

static Status::Statuses tex_dump_traversal_node_opening_(DiffData* diff_data, TreeNode** node,
                                                         bool* parenthesis,
                                                         const char** node_ending);


#define PRINTF_(...) if (file_printf(diff_data->tex_file, __VA_ARGS__) <= 0) return Status::OUTPUT_ERROR

Status::Statuses tex_dump_add(DiffData* diff_data, bool print_phrase) {
    assert(diff_data);
    assert(diff_data->tex_file);

    TREE_CHANGED = false;

    if (print_phrase)
        STATUS_CHECK(tex_write_phrase_(diff_data));

    STATUS_CHECK(tex_dump_traversal_(diff_data, &diff_data->tree.root));

    return Status::NORMAL_WORK;
}

static Status::Statuses tex_dump_traversal_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(diff_data->tex_file);
    assert(node);

    if (*node == nullptr)
        return Status::NORMAL_WORK;

    if (NODE_IS_ROOT(*node))
        PRINTF_("\n$$ ");

    const char* node_ending = "";
    bool parenthesis = false;

    STATUS_CHECK(tex_dump_traversal_node_opening_(diff_data, node, &parenthesis, &node_ending));

    if (TYPE_IS_OPER(*node) && IS_BINARY(*node))
        STATUS_CHECK(tex_dump_traversal_(diff_data, L(*node)));

    STATUS_CHECK(tex_dump_traversal_write_node_(diff_data, node));

    if (TYPE_IS_OPER(*node))
        STATUS_CHECK(tex_dump_traversal_(diff_data, R(*node)));

    if (node_ending[0] != '\0')
        PRINTF_("%s", node_ending);

    if (parenthesis || (*NODE_WILL_BE_DIFFED(*node) && TYPE_IS_OPER(*node)))
        PRINTF_(")");

    if (*NODE_WILL_BE_DIFFED(*node))
        PRINTF_("'");

    PRINTF_(" ");

    if (NODE_IS_ROOT(*node))
        PRINTF_("$$\n");

    return Status::NORMAL_WORK;
}

static Status::Statuses tex_dump_traversal_node_opening_(DiffData* diff_data, TreeNode** node,
                                                         bool* parenthesis,
                                                         const char** node_ending) {
    assert(diff_data);
    assert(node);
    assert(*node);
    assert(parenthesis);
    assert(node_ending);

    if (TYPE_IS_NUM(*node) || TYPE_IS_VAR(*node))
        return Status::NORMAL_WORK;

    if (!TYPE_IS_OPER(*node)) {
        assert(0 && "Wrong DiffElemType given");
        return Status::TREE_ERROR;
    }

    *parenthesis = false;
    const char* node_opening = "";

    switch (*OPER_NUM(*node)) {
        case DiffOperNum::ADD:
        case DiffOperNum::SUB:
            if (!NODE_IS_ROOT(*node) &&
                ((NEEDS_R_CHILD_PARENTHESIS(PARENT(*node)) && IS_R_CHILD(*node)) ||
                 (NEEDS_L_CHILD_PARENTHESIS(PARENT(*node)) && IS_L_CHILD(*node))))
                *parenthesis = true;
            break;

        case DiffOperNum::DIV:
            node_opening = "\\frac{";
            *node_ending  = "} ";
            break;

        case DiffOperNum::POW:
            node_opening = "{";
            *node_ending  = "} ";

            if (IS_L_CHILD_OF_POW(*node))
                *parenthesis = true;
            break;

        case DiffOperNum::LN:
            node_opening = "\\ln{";
            *node_ending  = "} ";

            if (IS_L_CHILD_OF_POW(*node))
                *parenthesis = true;
            break;
        case DiffOperNum::SQRT:
            node_opening = "\\sqrt{";
            *node_ending = "} ";
            break;
        case DiffOperNum::SIN:
            node_opening = "\\sin{";
            *node_ending  = "} ";
            if (IS_L_CHILD_OF_POW(*node))
                *parenthesis = true;
            break;
        case DiffOperNum::COS:
            node_opening = "\\cos{";
            *node_ending  = "} ";
            if (IS_L_CHILD_OF_POW(*node))
                *parenthesis = true;
            break;


        case DiffOperNum::MUL:
            if (!NODE_IS_ROOT(*node) &&
                ((IS_UNARY(PARENT(*node)) && NEEDS_R_CHILD_PARENTHESIS(PARENT(*node))) ||
                 ((*OPER_NUM(PARENT(*node)) == DiffOperNum::POW && IS_L_CHILD(*node)))))
                *parenthesis = true;
            // Nothing
            break;


        case DiffOperNum::ERR:
        default:
            assert(0 && "Invalid DiffOperNum given");
            return Status::TREE_ERROR;
    }

    if (NODE_IS_ROOT(*node))
        *parenthesis = false;

    if (*parenthesis || *NODE_WILL_BE_DIFFED(*node))
        PRINTF_("( ");

    if (node_opening[0] != '\0')
        PRINTF_("%s", node_opening);

    return Status::NORMAL_WORK;
}

static Status::Statuses tex_dump_traversal_write_node_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(diff_data->tex_file);
    assert(node);
    assert(*node);

    if (TYPE_IS_NUM(*node)) {
        STATUS_CHECK(tex_print_double_(diff_data, *NUM_VAL(*node)));
        return Status::NORMAL_WORK;
    }

    if (TYPE_IS_VAR(*node)) {
        PRINTF_("%s ", *VAR_NAME(*node));
        return Status::NORMAL_WORK;
    }

    if (TYPE_IS_OPER(*node)) {
        STATUS_CHECK(tex_dump_traversal_write_oper_(diff_data, node));
        return Status::NORMAL_WORK;
    }


    assert(0 && "Wrong DiffOperType given");
    return Status::TREE_ERROR;
}

static Status::Statuses tex_dump_traversal_write_oper_(DiffData* diff_data, TreeNode** node) {
    assert(diff_data);
    assert(diff_data->tex_file);
    assert(node);
    assert(*node);
    assert(TYPE_IS_OPER(*node));

    switch (*OPER_NUM(*node)) {
        case DiffOperNum::ADD:
            PRINTF_("+ ");
            break;
        case DiffOperNum::SUB:
            PRINTF_("- ");
            break;
        case DiffOperNum::MUL:
            PRINTF_("* ");
            break;

        case DiffOperNum::DIV:
            PRINTF_("}{");
            break;

        case DiffOperNum::POW:
            PRINTF_("}^{");
            break;

        case DiffOperNum::LN:
        case DiffOperNum::SQRT:
        case DiffOperNum::SIN:
        case DiffOperNum::COS:
            // Nothing
            break;

        case DiffOperNum::ERR:
        default:
            assert(0 && "Invalid DiffOperNum given");
            return Status::TREE_ERROR;
    }



    return Status::NORMAL_WORK;
}


Status::Statuses tex_dump_begin(DiffData* diff_data, const char* tex_directory, const DiffMode mode) {
    assert(diff_data);
    assert(diff_data->tex_file == nullptr);

    if (tex_directory == nullptr)
        return Status::NORMAL_WORK;

    if (!diff_data->open_tex(tex_directory))
        return Status::FILE_ERROR;

    STATUS_CHECK(tex_write_header_(diff_data, mode));

    return Status::NORMAL_WORK;
}


Status::Statuses tex_dump_end(DiffData* diff_data, const char* tex_directory) {
    assert(diff_data);
    assert(diff_data->tex_file || (diff_data->tex_filename == nullptr && tex_directory == nullptr));

    if (diff_data->tex_filename == nullptr || tex_directory == nullptr) {
        assert(diff_data->tex_file == nullptr && tex_directory == nullptr);
        return Status::NORMAL_WORK;
    }

    STATUS_CHECK(tex_write_footer_(diff_data));

    if (!diff_data->close_tex())
        return Status::FILE_ERROR;

    static const size_t MAX_COMMAND_LEN = diff_data->MAX_PATH_LEN * 2;
    char command[MAX_COMMAND_LEN] = "";

    printf("LaTeX pdf generation:\n");

    snprintf(command, MAX_COMMAND_LEN, "pdflatex -interaction=batchmode -output-directory %s  %s",
                                                           tex_directory, diff_data->tex_filename);

    if (system(command) != 0) {
        fprintf(stderr, "pdf latex error occured\n");
    }

    printf("Generation is ended\n");

    return Status::NORMAL_WORK;
}

static Status::Statuses tex_write_phrase_(DiffData* diff_data) {
    assert(diff_data);
    assert(diff_data->tex_file);

    long long phrase_n = rand_normal_generate_ll(0, TEX_PHRASES_NUM - 1);

    PRINTF_("\n%s\n", TEX_PHRASES[phrase_n]);

    return Status::NORMAL_WORK;
}

static Status::Statuses tex_write_header_(DiffData* diff_data, const DiffMode mode) {
    assert(diff_data);
    assert(diff_data->tex_file);

    const char* title = "Wrong mode given";

    switch (mode.action) {
        case DiffMode::SIMPLIFICATION:
            title = "Упрощение математического выражения";
            break;

        case DiffMode::DIFF:
            title = "Дифференцирование математического выражения";
            break;

        case DiffMode::TAYLOR:
            title = "Разложение функции в ряд Тейлора";
            break;

        default:
            break;
    }

    PRINTF_("\\documentclass[12pt]{article}\n"
            "\\usepackage[T2A]{fontenc}\n"
            "\\usepackage{mathtools}\n"
            "\\usepackage[utf8]{inputenc}\n"
            "\\usepackage[english, russian]{babel}\n"
            "\\usepackage{fancyhdr}\n"
            "\\usepackage{graphicx}\n"
            "\\usepackage{float}\n"
            "\\usepackage{booktabs}\n"
            "\\usepackage{amsmath}\n"
            "\\usepackage{amssymb}\n"
            "\\usepackage{indentfirst}\n"
            "\n"
            "\\title{Работа X.X.X. %s}\n"
            "\\author{Скайнет и Кожаный мешок (Александр Рожков)}\n"
            "\\date{29 августа 1997 года - 11 июля 2029 года}\n"
            "\n"
            "\\begin{document}\n"
            "\n"
            "\\maketitle\n"
            "\\newpage\n", title);

    return Status::NORMAL_WORK;
}

static Status::Statuses tex_write_footer_(DiffData* diff_data) {
    assert(diff_data);
    assert(diff_data->tex_file);

    static const char* footer_text = "\n\\end{document}";

    PRINTF_("%s\n", footer_text);

    return Status::NORMAL_WORK;
}

Status::Statuses tex_dump_section_taylor(DiffData* diff_data, const size_t degree, const double point) {
    assert(diff_data);
    assert(diff_data->tex_file);

    PRINTF_("\n\\section{Разложим по формуле Тейлора}\n\n"
            "Точность: до $о((x - %0.2lf)^%zu)$\n", point, degree);

    STATUS_CHECK(tex_dump_add(diff_data, false));

    return Status::NORMAL_WORK;
}

Status::Statuses tex_dump_given_expression(DiffData* diff_data) {
    assert(diff_data);
    assert(diff_data->tex_file);

    PRINTF_("\n\\section{Дано}\n");

    STATUS_CHECK(tex_dump_add(diff_data, false));

    return Status::NORMAL_WORK;
}

Status::Statuses tex_dump_section_diff(DiffData* diff_data) {
    assert(diff_data);
    assert(diff_data->tex_file);

    PRINTF_("\n\\section{Продифференцируем}\n");

    *NODE_WILL_BE_DIFFED(*ROOT) = true;

    STATUS_CHECK(tex_dump_add(diff_data, false));

    return Status::NORMAL_WORK;
}

Status::Statuses tex_dump_section_simplify(DiffData* diff_data) {
    assert(diff_data);
    assert(diff_data->tex_file);

    PRINTF_("\n\\section{Упростим выражение}\n");

    STATUS_CHECK(tex_dump_add(diff_data, false));

    return Status::NORMAL_WORK;
}

Status::Statuses tex_subsection_n_derivative_(DiffData* diff_data, const size_t n) {
    PRINTF_("\n\\subsection{Найдём %zu-ую производную}\n", n);

    return Status::NORMAL_WORK;
}

Status::Statuses tex_dump_section_subst(DiffData* diff_data) {
    assert(diff_data);
    assert(diff_data->tex_file);

    PRINTF_("\n\\section{Подставим численные значения}\n");

    STATUS_CHECK(tex_dump_add(diff_data, false));

    return Status::NORMAL_WORK;
}

Status::Statuses tex_print_evaled_value(DiffData* diff_data, const double value) {
    assert(diff_data);

    PRINTF_("\nПодставив знчения переменных, получаем:\n"
            "$$");

    STATUS_CHECK(tex_print_double_(diff_data, value));

    PRINTF_("$$\n");

    return Status::NORMAL_WORK;
}

static Status::Statuses tex_print_double_(DiffData* diff_data, const double val) {
    assert(diff_data);
    assert(diff_data->tex_file);

    int precision = 2;

    if (IS_DOUBLE_EQ(val, (ssize_t)val))
        precision = 0;

    PRINTF_("%.*lf ", precision, val);

    return Status::NORMAL_WORK;
}
