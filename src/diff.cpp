#include "diff.h"

#include "dsl.h"

static Status::Statuses diff_do_mode_(DiffData* diff_data, const DiffMode mode);

static Status::Statuses diff_process_do_diff_(DiffData* diff_data);

static Status::Statuses diff_process_do_taylor_(DiffData* diff_data);

static Status::Statuses diff_process_simplify_(DiffData* diff_data);

static Status::Statuses diff_process_substitute_(DiffData* diff_data);

static Status::Statuses diff_process_begin_and_simplify_(DiffData* diff_data);


#define LOCAL_DTOR_() diff_data.dtor();\
                      FREE(text);

Status::Statuses diff_proccess(const char* input_filename, const char* tex_directory) {
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

    STATUS_CHECK(tex_dump_begin(&diff_data, tex_directory, mode), LOCAL_DTOR_());

    STATUS_CHECK(diff_process_begin_and_simplify_(&diff_data), LOCAL_DTOR_());

    STATUS_CHECK(diff_do_mode_(&diff_data, mode), LOCAL_DTOR_());

    STATUS_CHECK(diff_process_simplify_(&diff_data), LOCAL_DTOR_());

    //STATUS_CHECK(interface_ask_about_plot(&mode), LOCAL_DTOR_());

    /*if (mode.plot)
        STATUS_CHECK(diff_plot(&diff_data), LOCAL_DTOR_());*/

    STATUS_CHECK(interface_ask_about_eval(&mode), LOCAL_DTOR_());

    if (mode.eval)
        STATUS_CHECK(diff_process_substitute_(&diff_data), LOCAL_DTOR_());

    STATUS_CHECK(tex_dump_end(&diff_data, tex_directory), LOCAL_DTOR_());

    LOCAL_DTOR_();

    return Status::NORMAL_WORK;
}
#undef LOCAL_DTOR_



static Status::Statuses diff_do_mode_(DiffData* diff_data, const DiffMode mode) {
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

static Status::Statuses diff_process_begin_and_simplify_(DiffData* diff_data) {
    assert(diff_data);

    STATUS_CHECK(tex_dump_given_expression(diff_data));

    STATUS_CHECK(diff_process_simplify_(diff_data));

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_process_substitute_(DiffData* diff_data) {
    assert(diff_data);

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

Status::Statuses diff_read_tree(DiffData* diff_data, char** text, const char* input_filename) {
    assert(diff_data);
    assert(input_filename);

    STATUS_CHECK(file_open_read_close(input_filename, text));

    STATUS_CHECK(recursive_fall(diff_data, *text));

    return Status::NORMAL_WORK;
}

