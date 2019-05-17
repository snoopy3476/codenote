#ifndef __CODENOTE_ANSISEQ_H__
#define __CODENOTE_ANSISEQ_H__

#define MOVE_CURSOR "\033[%d;%dH"
#define NEWPAGE "\033[2J"
#define CLEAR "\033[J"


#define COLOR(color_str_seq) "\033[" color_str_seq "m"

#define FC_DEFAULT ";39"

#define FC_BLACK ";30"
#define FC_RED ";31"
#define FC_GREEN ";32"
#define FC_YELLOW ";33"
#define FC_BLUE ";34"
#define FC_MARGENTA ";35"
#define FC_CYAN ";36"
#define FC_GREY ";37"

#define FC_BLACK_L ";90"
#define FC_RED_L ";91"
#define FC_GREEN_L ";92"
#define FC_YELLOW_L ";93"
#define FC_BLUE_L ";94"
#define FC_MARGENTA_L ";95"
#define FC_CYAN_L ";96"
#define FC_GREY_L ";97"


#define BC_DEFAULT ";49"

#define BC_BLACK ";40"
#define BC_RED ";41"
#define BC_GREEN ";42"
#define BC_YELLOW ";43"
#define BC_BLUE ";44"
#define BC_MARGENTA ";45"
#define BC_CYAN ";46"
#define BC_GREY ";47"

#define BC_BLACK_L ";100"
#define BC_RED_L ";101"
#define BC_GREEN_L ";102"
#define BC_YELLOW_L ";103"
#define BC_BLUE_L ";104"
#define BC_MARGENTA_L ";105"
#define BC_CYAN_L ";106"
#define BC_GREY_L ";107"


#endif
