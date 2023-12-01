#include "diff.h"

#define LOCAL_DTOR_() diff_data.dtor();\
                      FREE(text);

Status::Statuses diff_proccess(const char* input_filename) {
    assert(input_filename);

    DiffData diff_data = {};
    if (!diff_data.ctor())
        return Status::MEMORY_EXCEED;

    char* text = nullptr;
    STATUS_CHECK(diff_read_tree(&diff_data, &text, input_filename), LOCAL_DTOR_());

    TREE_DUMP(&diff_data.tree);

    STATUS_CHECK(diff_do_diff(&diff_data), LOCAL_DTOR_());

    TREE_DUMP(&diff_data.tree);

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

