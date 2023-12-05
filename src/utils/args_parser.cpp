#include "args_parser.h"

Status::Statuses args_parse(int argc, char* argv[], ArgsVars* args_vars) {
    assert(argv);
    assert(argc);
    assert(args_vars);

    /**
     * @brief This array contains console options, their functions and descriptions
     */
    static Argument args_dict[] = {
        {"-h", print_help,             "#   -h - prints help information\n"},             //< Help option

        {"-i", read_input_filename,    "#   -i - specify input file name after this\n"},  //< Input filename

        {"-o", read_tex_directory,    "#   -o - specify tex output file name after this\n"},  //< Output filename

        {"-v", enable_substitute_vals, "#   -v - will ask about variable values\n"},      //< exact mode

    };

    static const int ARGS_DICT_LEN = sizeof(args_dict) / sizeof(args_dict[0]); //< args_dict array len

    bool exit = false;
    for (int i = 1; i < argc; i++) {
        bool is_found = false;

        for (int j = 0; j < ARGS_DICT_LEN; j++) {
            if (strcmp(argv[i], args_dict[j].arg) == 0) {
                switch (args_dict[j].func(args_dict, ARGS_DICT_LEN, &i, argc, argv, args_vars)) {
                    case ArgsMode::CONTINUE:
                        break;
                    case ArgsMode::EXIT:
                        exit = true;
                        break;
                    case ArgsMode::ERR:
                        return Status::ARGS_ERROR;
                    default:
                        assert(0 && "args[j].func() returned wrong ArgsMode");
                        break;
                }
                is_found = true;
                break;
            }
        }
        if (is_found)
            continue;

        printf("Wrong console argument: %s\n"
               "Use -h for help\n", argv[i]);
        return Status::ARGS_ERROR;
    }
    if (exit)
        return Status::OK_EXIT;

    return Status::NORMAL_WORK;
}

ArgsMode print_help(const Argument args_dict[], const int args_dict_len,
                    int* arg_i, const int argc, char* argv[], ArgsVars* args_vars) {
    assert(args_dict);
    assert(arg_i);
    assert(argc);
    assert(argv);
    assert(args_vars);

    printf("# Differentiator\n"
           "# Console args:\n");

    for (int i = 0; i < args_dict_len; i++)
        printf("%s", args_dict[i].description);

    printf("# End of help\n");
    return ArgsMode::EXIT;
}

ArgsMode read_input_filename(const Argument args_dict[], const int args_dict_len,
                             int* arg_i, int argc, char* argv[], ArgsVars* args_vars) {
    (void) args_dict_len;

    assert(args_dict);
    assert(arg_i);
    assert(argv);
    assert(args_vars);

    if (++(*arg_i) >= argc) {
        printf("No input file name found\n");
        return ArgsMode::ERR;
    }

    args_vars->input_filename = argv[*arg_i];
    return ArgsMode::CONTINUE;
}

ArgsMode read_tex_directory(const Argument args_dict[], const int args_dict_len,
                           int* arg_i, int argc, char* argv[], ArgsVars* args_vars) {
    (void) args_dict_len;

    assert(args_dict);
    assert(arg_i);
    assert(argv);
    assert(args_vars);

    if (++(*arg_i) >= argc) {
        printf("No tex file name found\n");
        return ArgsMode::ERR;
    }

    args_vars->tex_directory = argv[*arg_i];
    return ArgsMode::CONTINUE;
}

ArgsMode enable_substitute_vals(const Argument args_dict[], const int args_dict_len,
                                int* arg_i, int argc, char* argv[], ArgsVars* args_vars) {
    (void) args_dict_len;
    (void) argc;

    assert(args_dict);
    assert(arg_i);
    assert(argv);
    assert(args_vars);

    args_vars->substitute_vals = true;

    return ArgsMode::CONTINUE;
}
