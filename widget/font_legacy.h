#ifndef FONT_H
#include "font.h"
#endif

/* This is for the old font API.  If using the new api, then include 
 * font.h instead! */

extern unsigned int fnfgcol;
extern unsigned int fnbgcol;
extern int shdcol;
extern int fshadow;
extern int cachefont;
extern int textbgr, textbgg, textbgb;
extern int textfgr, textfgg, textfgb;
extern int textsdr, textsdg, textsdb;
extern int texthlr, texthlg, texthlb;
extern int textier, textieg, textieb;
extern int descbgr, descbgg, descbgb;
extern int banrbgr, banrbgg, banrbgb;
extern int banrfgr, banrfgg, banrfgb;
extern int shadowr, shadowg, shadowb;
extern fnt_t* DefaultFont;

// void font_load(char*);
// void set_font_fcolor(int, int, int); // depricated
// void set_font_bcolor(int, int, int); // depricated
void show_string(int, int, char*); // depricated
// void solid_string(int, int, char*); // depricated
// int calc_width(char*);
// int calc_height(char*);

