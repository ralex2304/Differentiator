#include "diff.h"

#include "dsl.h"

#define LOCAL_DTOR_() diff_data.dtor();\
                      FREE(text);

Status::Statuses diff_proccess(const char* input_filename, const char* tex_filename,
                               bool substitute_vals) {
    assert(input_filename);
    // tex_filename can be nullptr

    DiffData diff_data = {};
    if (!diff_data.ctor())
        return Status::MEMORY_EXCEED;

    char* text = nullptr;
    STATUS_CHECK(diff_read_tree(&diff_data, &text, input_filename), LOCAL_DTOR_());

    diff_data.vars.search_argument();

    if (substitute_vals)
        STATUS_CHECK(interface_ask_vars_values(&diff_data));

    // TODO no tex file check

    STATUS_CHECK(tex_dump_begin(&diff_data, tex_filename), LOCAL_DTOR_());

    STATUS_CHECK(tex_dump_add(&diff_data), LOCAL_DTOR_());

    STATUS_CHECK(diff_simplify(&diff_data), LOCAL_DTOR_());

    STATUS_CHECK(tex_dump_add(&diff_data), LOCAL_DTOR_());

    STATUS_CHECK(diff_do_diff(&diff_data), LOCAL_DTOR_());

    STATUS_CHECK(tex_dump_add(&diff_data), LOCAL_DTOR_());

    STATUS_CHECK(diff_simplify(&diff_data), LOCAL_DTOR_());

    STATUS_CHECK(tex_dump_add(&diff_data), LOCAL_DTOR_());

    STATUS_CHECK(tex_dump_end(&diff_data, tex_filename), LOCAL_DTOR_());

    LOCAL_DTOR_();

    return Status::NORMAL_WORK;
}
#undef LOCAL_DTOR_

Status::Statuses diff_read_tree(DiffData* diff_data, char** text, const char* input_filename) {
    assert(diff_data);
    assert(input_filename);

    STATUS_CHECK(file_open_read_close(input_filename, text));

    STATUS_CHECK(text_tree_parser(diff_data, *text));

    return Status::NORMAL_WORK;
}

