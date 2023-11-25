#include "statuses.h"

#define STATUS_CASE_(status, text)  case status:                                \
                                        fprintf(stderr, "Error. " text "\n");   \
                                        break

Status::Statuses Status::raise(const Statuses status) {
    switch (status) {
        case NORMAL_WORK:
            assert(0 && "Status::raise(): NORMAL_WORK mustn't be raised");
            break;

        STATUS_CASE_(ARGS_ERROR,        "Args error");
        STATUS_CASE_(FILE_ERROR,        "File error");
        STATUS_CASE_(MEMORY_EXCEED,     "Not enough memory");
        STATUS_CASE_(OUTPUT_ERROR,      "Output error");
        STATUS_CASE_(INPUT_ERROR,       "Input error");
        STATUS_CASE_(WRONG_CMD,         "User is hopelessly stupid");
        STATUS_CASE_(TOO_MANY_ATTEMPTS, "User is too stubborn");
        STATUS_CASE_(TREE_ERROR,        "Operation with Tree failed");
        STATUS_CASE_(STACK_ERROR,       "Operation with Stack failed");

        case OK_EXIT:
        case DEFAULT:
            break;
        default:
            assert(0 && "Error::raise(): wrong error");
            break;
    };
    return status;
}
#undef STATUS_CASE_
