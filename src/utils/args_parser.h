#ifndef ARGS_PARSER_H_
#define ARGS_PARSER_H_

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "statuses.h"

/**
 * @brief Specifies int main() returns
 */
enum class ArgsMode {
    ERR      = -1,    ///< Error
    EXIT     =  0,    ///< Continue reading args, but then normal exit
    CONTINUE =  1,    ///< Continue reading args
};

/**
 * @brief Specifies variables, that are set from console arguments
 */
struct ArgsVars {
    const char* input_filename = nullptr;
};

/**
 * @brief Specifies console argument parametres: name, function and description
 *
 */
struct Argument {
    const char* arg;                                                              ///< Argument name
    ArgsMode (*func) (const Argument args_dict[], const int args_dict_len,
                      int* arg_i, int argc, char* argv[], ArgsVars* args_vars);   ///< Function
    const char* description;                                                      ///< Description
};

/**
 * @brief Prints help
 *
 * @param[in] args_dict
 * @param[in] args_dict_len
 * @param[in] arg_i number of current argument
 * @param[in] argc  int main() argc
 * @param[in] argv  int main() argv
 * @param[in] args_vars
 * @return ArgsMode
 */
ArgsMode print_help(const Argument args_dict[], const int args_dict_len,
                    int* arg_i, int argc, char* argv[], ArgsVars* args_vars);

/**
 * @brief Reads input file name from console
 *
 * @param[in] args_dict
 * @param[in] args_dict_len
 * @param[in] arg_i number of current argument
 * @param[in] argc  int main() argc
 * @param[in] argv  int main() argv
 * @param[out] args_vars
 * @return ArgsMode
 */
ArgsMode read_input_filename(const Argument args_dict[], const int args_dict_len,
                             int* arg_i, int argc, char* argv[], ArgsVars* args_vars);

/**
 * @brief Parses console arguments
 *
 * @param[in] argc
 * @param[in] argv
 * @param[out] args_vars
 * @return Status::Statuses
 */
Status::Statuses args_parse(int argc, char* argv[], ArgsVars* args_vars);

#endif // #ifndef ARGS_PARSER_H_
