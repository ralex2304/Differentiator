#include "log/log.h"
#include "utils/statuses.h"
#include "utils/args_parser.h"
#include "diff.h"

#include "config.h"
#include TREE_INCLUDE

LogFileData log_file = {"log"};

int main(int argc, char* argv[]) {

    ArgsVars args_vars = {};
    args_vars.input_filename  = "trees/example.txt"; //< default value
    args_vars.tex_directory    = "tex";              //< default value
    args_vars.substitute_vals = false;               //< default value

    STATUS_CHECK_RAISE(args_parse(argc, argv, &args_vars));

    STATUS_CHECK_RAISE(diff_proccess(args_vars.input_filename, args_vars.tex_directory,
                                     args_vars.substitute_vals));

    return Status::OK_EXIT;
}
