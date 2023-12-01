#include "simplification.h"

Status::Statuses diff_simplify(DiffData* diff_data) {
    assert(diff_data);

    bool is_simple = false;
    STATUS_CHECK(diff_simplify_traversal(diff_data, &diff_data->tree.root, &is_simple));

    return Status::NORMAL_WORK;
}

Status::Statuses diff_simplify_traversal(DiffData* diff_data, TreeNode** node, bool* is_simple) {
    assert(diff_data);
    assert(node);
    assert(is_simple);



    return Status::NORMAL_WORK;
}
