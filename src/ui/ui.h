#ifndef UI_H_
#define UI_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#ifdef unix

#include <termios.h>
#include <unistd.h>

#elif _WIN32

#include <windows.h>
#include <conio.h>

#include "cus_str.h"

#endif //< #ifdef unix

#include "utils/macros.h"

/**
 * @brief Clears console. Returns system() call result
 *
 * @return int -1 if error
 */
int ui_clear_console();

/**
 * @brief Gets one line from input stream
 *
 * @param stream
 * @return char*
 */
char* ui_get_string(FILE* stream = stdin);

/**
 * @brief Gets one char from stream
 *
 * @param stream
 * @return char
 */
char ui_get_char_no_enter(FILE* stream = stdin);

/**
 * @brief Gets double from stream. If empty, returns NAN. If error returns INF
 *
 * @param stream
 * @return double empty - NAN, error - INF
 */
double ui_get_double_or_NAN(FILE* stream = stdin);

/**
 * @brief Gets int from stream. If error returns -__INT_MAX__
 *
 * @param stream
 * @return int
 */
int ui_get_int(FILE* stream = stdin);

/**
 * @brief Flushes input stream
 * @param stream
 *
 * @return bool false - if there are non space symbols in stream left
 */
bool input_flush(FILE* stream = stdin);

#endif //< #ifndef UI_H_
