#ifndef FONT_H
#define FONT_H


int fnfgcol;
int fnbgcol;
int shdcol;
int fshadow;
int cachefont;
char fontdir[90];
char tfont[30];
char currfont[30];
int textbgr, textbgg, textbgb;
int textfgr, textfgg, textfgb;
int textsdr, textsdg, textsdb;
int texthlr, texthlg, texthlb;
int textier, textieg, textieb;
int descbgr, descbgg, descbgb;
int banrbgr, banrbgg, banrbgb;
int banrfgr, banrfgg, banrfgb;
int shadowr, shadowg, shadowb;

/* This is for displaying text in graphics modes */

void font_load(char *filen);
void set_font_fcolor(int r, int g, int b); // depricated
void set_font_bcolor(int r, int g, int b); // depricated
void show_string(int x, int y, char *stt); // depricated
void solid_string(int x, int y, char *stt); // depricated
int calc_width(char *stt);
int calc_height(char *stt);

#endif /* font.h */
