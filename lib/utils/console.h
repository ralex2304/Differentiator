#ifndef CONSOLE_H_
#define CONSOLE_H_

// Reset style:
#define STYLE_RESET       "\033[0m"

// Text styles:
#define STYLE_BOLD        "\033[1m"
#define STYLE_FAINT       "\033[2m"
#define STYLE_ITALIC      "\033[3m"
#define STYLE_UNDERLINED  "\033[4m"
#define STYLE_SLOW_BLINK  "\033[5m"
#define STYLE_FAST_BLINK  "\033[6m"
#define STYLE_BLINK_OFF   "\033[25m"

// Text colors:
#define COLOR_BLACK       "\033[30m"
#define COLOR_RED         "\033[31m"
#define COLOR_GREEN       "\033[32m"
#define COLOR_YELLOW      "\033[93m"
#define COLOR_BLUE        "\033[94m"
#define COLOR_WHITE       "\033[97m"

// Background colors:
#define BACK_COLOR_BLACK  "\033[40m"
#define BACK_COLOR_RED    "\033[41m"
#define BACK_COLOR_GREEN  "\033[42m"
#define BACK_COLOR_YELLOW "\033[103m"
#define BACK_COLOR_BLUE   "\033[104m"
#define BACK_COLOR_WHITE  "\033[107m"

// Macros:
#define CONSOLE_RED(str)   COLOR_RED str STYLE_RESET
#define CONSOLE_GREEN(str) COLOR_GREEN str STYLE_RESET

#endif // #ifndef COSOLE_H_
