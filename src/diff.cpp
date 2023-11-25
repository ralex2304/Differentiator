#include "diff.h"

#define LOCAL_DTOR_() diff_data.dtor();

Status::Statuses diff_proccess(const char* input_filename) {
    assert(input_filename);

    DiffData diff_data = {};
    if (!diff_data.ctor())
        return Status::MEMORY_EXCEED;

    STATUS_CHECK(diff_read_tree(&diff_data, input_filename), LOCAL_DTOR_());

    TREE_DUMP(&diff_data.tree);

    LOCAL_DTOR_();

    return Status::NORMAL_WORK;
}
#undef LOCAL_DTOR_

Status::Statuses diff_read_tree(DiffData* diff_data, const char* input_filename) {
    assert(diff_data);
    assert(input_filename);

    char* input = nullptr;

    STATUS_CHECK(file_open_read_close(input_filename, &input));

    STATUS_CHECK(text_tree_parser(diff_data, input));

    return Status::NORMAL_WORK;
}

