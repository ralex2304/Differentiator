#include "diff.h"

#include "dsl.h"

static Status::Statuses diff_process_mode_(DiffData* diff_data, const DiffMode mode);

static Status::Statuses diff_process_do_maths_(DiffData* diff_data, DiffMode* mode, const char* tex_dir);

static Status::Statuses diff_process_do_diff_(DiffData* diff_data);

static Status::Statuses diff_process_do_taylor_(DiffData* diff_data);

static Status::Statuses diff_process_simplify_(DiffData* diff_data);

static Status::Statuses diff_process_eval_(DiffData* diff_data, DiffMode* mode);

static Status::Statuses diff_process_begin_and_simplify_(DiffData* diff_data, const char* tex_dir,
                                                         const DiffMode mode);

static Status::Statuses diff_process_plot_(DiffData* diff_data, DiffMode* mode, Tree* tree_copy);

static Status::Statuses diff_process_create_tree_copy_if_needed_(DiffData* diff_data,
                                                        const DiffMode mode, Tree* tree_copy);


#define LOCAL_DTOR_() diff_data.dtor(); \
                      FREE(text)

Status::Statuses diff_process(const char* input_filename, const char* tex_directory) {
    assert(input_filename);
    // tex_directory can be nullptr

    DiffData diff_data = {};
    if (!diff_data.ctor())
        return Status::MEMORY_EXCEED;

    char* text = nullptr;
    STATUS_CHECK(diff_read_tree(&diff_data, &text, input_filename), LOCAL_DTOR_());

    diff_data.vars.search_argument();

    DiffMode mode = {};
    STATUS_CHECK(interface_ask_mode(&mode), LOCAL_DTOR_());

    STATUS_CHECK(diff_process_do_maths_(&diff_data, &mode, tex_directory), LOCAL_DTOR_());

    STATUS_CHECK(tex_dump_end(&diff_data), LOCAL_DTOR_());

    LOCAL_DTOR_();

    return Status::NORMAL_WORK;
}
#undef LOCAL_DTOR_


#define LOCAL_DTOR_()   DSL_TREE_DTOR(&tree_copy)

static Status::Statuses diff_process_do_maths_(DiffData* diff_data, DiffMode* mode, const char* tex_dir) {
    assert(diff_data);
    assert(mode);
    // tex_dir can be nullptr

    Tree tree_copy = {};
    DSL_TREE_CTOR(&tree_copy);

    STATUS_CHECK(diff_process_create_tree_copy_if_needed_(diff_data, *mode, &tree_copy), LOCAL_DTOR_());

    STATUS_CHECK(diff_process_begin_and_simplify_(diff_data, tex_dir, *mode), LOCAL_DTOR_());

    STATUS_CHECK(diff_process_mode_(diff_data, *mode), LOCAL_DTOR_());

    STATUS_CHECK(diff_process_simplify_(diff_data), LOCAL_DTOR_());

    STATUS_CHECK(diff_process_plot_(diff_data, mode, &tree_copy), LOCAL_DTOR_());

    STATUS_CHECK(diff_process_eval_(diff_data, mode), LOCAL_DTOR_());

    LOCAL_DTOR_();

    return Status::NORMAL_WORK;
}
#undef LOCAL_DTOR_

static Status::Statuses diff_process_mode_(DiffData* diff_data, const DiffMode mode) {
    assert(diff_data);

    switch (mode.action) {
        case DiffMode::DIFF:
            STATUS_CHECK(diff_process_do_diff_(diff_data));
            break;

        case DiffMode::TAYLOR:
            STATUS_CHECK(diff_process_do_taylor_(diff_data));
            break;

        case DiffMode::SIMPLIFICATION: //< Do nothing at this point
            break;

        default:
            assert(0 && "Wrong DiffMode.action given");
            return Status::INPUT_ERROR;
    }

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_process_begin_and_simplify_(DiffData* diff_data, const char* tex_dir,
                                                         const DiffMode mode) {
    assert(diff_data);
    //< tex_dir can be nullptr

    STATUS_CHECK(tex_dump_begin(diff_data, tex_dir, mode));

    STATUS_CHECK(tex_dump_given_expression(diff_data));

    STATUS_CHECK(diff_process_simplify_(diff_data));

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_process_eval_(DiffData* diff_data, DiffMode* mode) {
    assert(diff_data);
    assert(mode);

    STATUS_CHECK(interface_ask_about_eval(mode));

    if (!mode->eval)
        return Status::NORMAL_WORK;

    if (diff_exists_var_with_no_value(diff_data, false))
        STATUS_CHECK(interface_ask_vars_values(diff_data, true, true));

    if (IS_TEX_DUMP_ENABLED)
        STATUS_CHECK(tex_dump_section_subst(diff_data));

    bool tmp = SUBST_VARS;

    SUBST_VARS = true;

    STATUS_CHECK(diff_simplify(diff_data));

    SUBST_VARS = tmp;

    TEX_DUMP();

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_process_simplify_(DiffData* diff_data) {
    assert(diff_data);

    if (IS_TEX_DUMP_ENABLED)
        STATUS_CHECK(tex_dump_section_simplify(diff_data));


    STATUS_CHECK(diff_simplify(diff_data));

    TEX_DUMP();

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_process_do_diff_(DiffData* diff_data) {
    assert(diff_data);

    if (IS_TEX_DUMP_ENABLED)
        STATUS_CHECK(tex_dump_section_diff(diff_data));

    STATUS_CHECK(diff_do_diff(diff_data));

    TEX_DUMP();

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_process_do_taylor_(DiffData* diff_data) {
    assert(diff_data);

    size_t degree = 0;
    double point = NAN;
    STATUS_CHECK(interface_ask_taylor_degree_and_point(&degree, &point));

    if (IS_TEX_DUMP_ENABLED)
        STATUS_CHECK(tex_dump_section_taylor(diff_data, degree, point));

    STATUS_CHECK(diff_do_taylor(diff_data, degree, point));

    TEX_DUMP();

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_process_plot_(DiffData* diff_data, DiffMode* mode, Tree* tree_copy) {
    assert(diff_data);
    assert(mode);
    assert(tree_copy);

    STATUS_CHECK(interface_ask_about_plot(mode));

    if (!mode->plot)
        return Status::NORMAL_WORK;

    if (diff_exists_var_with_no_value(diff_data, true))
        STATUS_CHECK(interface_ask_vars_values(diff_data, false, true));

    Tree* trees[] = {&diff_data->tree, tree_copy};
    const char* titles[] = {"Taylor series", "Function"};
    static_assert(sizeof(trees) / sizeof(*trees) == sizeof(titles) / sizeof(*titles));

    if (mode->action == DiffMode::TAYLOR) {
        assert(tree_copy->root != nullptr);

        STATUS_CHECK(diff_plot(diff_data, trees, titles, 2));
    } else {
        STATUS_CHECK(diff_plot(diff_data, trees, titles + 1, 1));
    }

    if (IS_TEX_DUMP_ENABLED)
        STATUS_CHECK(tex_insert_plot(diff_data, "plot.png"));

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_process_create_tree_copy_if_needed_(DiffData* diff_data,
                                                        const DiffMode mode, Tree* tree_copy) {
    assert(diff_data);
    assert(tree_copy);

    if (mode.action != DiffMode::TAYLOR) //< Copy is needed only in this mode
        return Status::NORMAL_WORK;

    size_t size = 0;
    COPY_SUBTREE(diff_data->tree.root, &tree_copy->root, &size);
    tree_copy->size = size;

    return Status::NORMAL_WORK;
}

Status::Statuses diff_read_tree(DiffData* diff_data, char** text, const char* input_filename) {
    assert(diff_data);
    assert(input_filename);

    STATUS_CHECK(file_open_read_close(input_filename, text));

    STATUS_CHECK(recursive_fall(diff_data, *text));

    return Status::NORMAL_WORK;
}

