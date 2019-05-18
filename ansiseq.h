#ifndef __CODENOTE_ANSISEQ_H__
#define __CODENOTE_ANSISEQ_H__

#define MOVE_CURSOR "\033[%d;%dH"
#define NEWPAGE "\033[2J"
#define CLEAR "\033[J"


#define FILL_LINE "\033[K"





// COLORS //

#define COLOR(color_str_seq) "\033[" color_str_seq "m"


#define CO_DEFAULT ";0"

#define FG_DEFAULT ";39"

#define FG_BLACK ";30"
#define FG_RED ";31"
#define FG_GREEN ";32"
#define FG_YELLOW ";33"
#define FG_BLUE ";34"
#define FG_MARGENTA ";35"
#define FG_CYAN ";36"
#define FG_GREY ";37"

#define FG_BLACK_L ";90"
#define FG_RED_L ";91"
#define FG_GREEN_L ";92"
#define FG_YELLOW_L ";93"
#define FG_BLUE_L ";94"
#define FG_MARGENTA_L ";95"
#define FG_CYAN_L ";96"
#define FG_GREY_L ";97"


#define BG_DEFAULT ";49"

#define BG_BLACK ";40"
#define BG_RED ";41"
#define BG_GREEN ";42"
#define BG_YELLOW ";43"
#define BG_BLUE ";44"
#define BG_MARGENTA ";45"
#define BG_CYAN ";46"
#define BG_GREY ";47"

#define BG_BLACK_L ";100"
#define BG_RED_L ";101"
#define BG_GREEN_L ";102"
#define BG_YELLOW_L ";103"
#define BG_BLUE_L ";104"
#define BG_MARGENTA_L ";105"
#define BG_CYAN_L ";106"
#define BG_GREY_L ";107"


#define FG_WHITE FG_GREY_L // alias
#define BG_WHITE BG_GREY_L // alias


#endif
