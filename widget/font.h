#ifndef FONT_H
#define FONT_H

// Font Types
#define BIOS_8X8 0
#define BIOS_8X16 1
#define BLIT_8X8 2
#define BLIT_8X16 3
#define TTF 4

#define FNT_DATA 0
#define FNT_BLIT 1

typedef struct fnt_t {
   char name[240];
   int width,height,type;
   int scale_w,scale_h;  // for TTF fonts now, other later
   int sox, soy;  // shadow offset
   int size;
   int color[8]; // Array of colors, only used for blitfonts
   int lru; // least recently created color (round robin currently)
   void* data;  // Pointer to font data
} fnt_t;

// #ifdef COMPFONT
extern fnt_t* cf8x16;
// #endif

extern char fontdir[90];
extern char tfont[30];
extern char currfont[130];
extern fnt_t* ActiveFont;

/* This is for displaying text in graphics modes */
extern fnt_t* fnt_loadfont(char*, int);

extern void fnt_destroy(fnt_t* font);
extern void fnt_setactive(fnt_t* font);
extern fnt_t* fnt_getactive();
extern fnt_t* fnt_loadfont(char *filen, int ftype);
extern fnt_t* fnt_convblit(fnt_t* font);
//extern void fnt_display_char(BITMAP *b,int x, int y, unsigned char chr,int fcolor);
//extern void fnt_print_string(BITMAP *b, int x, int y, char *str, long fg, long bg, long sd);


#ifdef USE_FREETYPE
#define RENDER_SIMPLE 0  /* No Antialiasing */
#define RENDER_BLEND 1   /* Use Internal Blending */
#define RENDER_NATIVE 2  /* Use SDL or Allegro to Blend */
#endif


#endif /* font.h */
