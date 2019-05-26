#ifndef __ANSICOLOR_H__
#define __ANSICOLOR_H__



// COLORS //

// set color
#define COLOR(color_str_seq) COLORA(CO_AND color_str_seq)
#define COLORA(color_str_seq) "\033[" color_str_seq "m"

#define CO_AND ";"


#define CO_DEFAULT "0"
#define CO_BOLD "1"
#define CO_ITALIC "3"
#define CO_UNDERLINE "4"
#define CO_BLINK "5"
#define CO_REV "7"
#define CO_STRIKE "9"


#define FG_DEFAULT "39"

#define FG_BLACK "30"
#define FG_RED "31"
#define FG_GREEN "32"
#define FG_YELLOW "33"
#define FG_BLUE "34"
#define FG_MARGENTA "35"
#define FG_CYAN "36"
#define FG_GREY "37"

#define FG_BLACK_L "90"
#define FG_RED_L "91"
#define FG_GREEN_L "92"
#define FG_YELLOW_L "93"
#define FG_BLUE_L "94"
#define FG_MARGENTA_L "95"
#define FG_CYAN_L "96"
#define FG_GREY_L "97"


#define BG_DEFAULT "49"

#define BG_BLACK "40"
#define BG_RED "41"
#define BG_GREEN "42"
#define BG_YELLOW "43"
#define BG_BLUE "44"
#define BG_MARGENTA "45"
#define BG_CYAN "46"
#define BG_GREY "47"

#define BG_BLACK_L "100"
#define BG_RED_L "101"
#define BG_GREEN_L "102"
#define BG_YELLOW_L "103"
#define BG_BLUE_L "104"
#define BG_MARGENTA_L "105"
#define BG_CYAN_L "106"
#define BG_GREY_L "107"


#define FG_WHITE FG_GREY_L // alias
#define BG_WHITE BG_GREY_L // alias




// for Windows 10 compatibility //
#ifdef _WIN32

#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

// Enable ANSI Sequence on Windows Console
void set_windows_ansi_ready()
{
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD console_mode;
    GetConsoleMode(console_handle, &console_mode);
    console_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    
    SetConsoleMode(console_handle, console_mode);
}

#else

// Avoid compiler error on non-windows
void set_windows_ansi_ready()
{
}

#endif


#endif
