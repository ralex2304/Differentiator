#include "ui.h"

int ui_clear_console() {
#ifdef unix
    return system("clear");
#elif _WIN32
    return system("cls");
#else
    return 0;
#endif
}

char* ui_get_string(FILE* stream) {
    assert(stream);

    char* input = nullptr;
    size_t input_size = 0;

#ifdef unix
    ssize_t input_len = getline(&input, &input_size, stream);
#else //< #ifndef unix
    ssize_t input_len = cus_getline(&input, &input_size, stream);
#endif //< #ifdef unix

    if (input_len <= 0) {
        FREE(input);
        return nullptr;
    }

    input[input_len - 1] = '\0'; //< \n removal

    return input;
}

char ui_get_char_no_enter(FILE* stream) {
    assert(stream);

#ifdef unix
    struct termios oldt = {};
    tcgetattr(STDIN_FILENO, &oldt);

    struct termios newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int c = getc(stream);
#elif _WIN32
    int c = (stream == stdin) ? _getch() : getc(stream);
#endif //< #ifdef unix

    if (c == EOF || feof(stream) || ferror(stream))
        return '\0';

#ifdef unix
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
#endif //< #ifdef unix

    return (char)c;
}

double ui_get_double_or_NAN(FILE* stream) {
    assert(stream);

    static const size_t MAX_INPUT_LEN = 256;
    char input[MAX_INPUT_LEN] = {};

    if (fgets(input, MAX_INPUT_LEN, stream) == nullptr)
        return INFINITY;

    if (input[0] == '\n' && input[1] == '\0')
        return NAN;

    double res = INFINITY;

    if (sscanf(input, "%lf", &res) != 1)
        return INFINITY;

    return res;
}

int ui_get_int(FILE* stream) {
    assert(stream);

    int res = -__INT_MAX__;

    if (fscanf(stream, "%d", &res) != 1)
        return -__INT_MAX__;

    if (!input_flush())
        return -__INT_MAX__;

    return res;
}

bool input_flush(FILE* stream) {
    assert(stream);

    int c = ' ';
    bool non_space = true;

    do {
        if (!isspace(c))
            non_space = false;

    } while ((c = fgetc(stream)) != '\n' && c != EOF && !feof(stream) && !ferror(stream));

    return non_space;
}
