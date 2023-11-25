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
