#ifndef STATUSES_H_
#define STATUSES_H_

#include <stdio.h>
#include <assert.h>

/**
 * @brief Specifies functions status
 */
struct Status {
    enum Statuses {
        DEFAULT           = -2,  //< Poison value
        NORMAL_WORK       = -1,  //< Normal mode system value (isn't returned in main())
        OK_EXIT           =  0,  //< OK and exit
        ARGS_ERROR        =  1,  //< Console arguments error
        FILE_ERROR        =  2,  //< File opening or reading error
        MEMORY_EXCEED     =  3,  //< Memory allocation failed
        OUTPUT_ERROR      =  4,  //< Output writing error
        INPUT_ERROR       =  5,  //< Input reading error
        WRONG_CMD         =  6,  //< Wrong cmd was recieved from user
        TOO_MANY_ATTEMPTS =  7,  //< User made too many attempts
        TREE_ERROR        =  8,  //< Tree error
        STACK_ERROR       =  9,  //< Stack error
    };

    /**
     * @brief Prints and returns given status
     *
     * @param[in] status
     * @return Statuses
     */
    static Statuses raise(const Statuses status);
};

/**
 * @brief Macros for validating function return in main() with raise() call
 *
 * @param func function
 * @param ... fallback operations
 */
#define STATUS_CHECK_RAISE(func, ...)   do {                                    \
                                            Status::Statuses res = func;        \
                                            if (res != Status::NORMAL_WORK) {   \
                                                __VA_ARGS__;                    \
                                                return Status::raise(res);      \
                                            }                                   \
                                        } while (0)

/**
 * @brief Macros for validating function return
 *
 * @param func function
 * @param ... fallback operations
 */
#define STATUS_CHECK(func, ...) do {                                            \
                                    Status::Statuses status_res_ = func;        \
                                    if (status_res_ != Status::NORMAL_WORK) {   \
                                        __VA_ARGS__;                            \
                                        return status_res_;                     \
                                    }                                           \
                                } while (0)

#endif // #ifndef STATUSES_H_
