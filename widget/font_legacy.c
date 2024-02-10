#ifndef USESDL
#include <allegro.h>
//#include <allegro/internal/aintern.h>
#else
#include <SDL.h>
#include "sdl_allegro.h"
#endif

#include "font.h"

/* This implements the old API for the font library.   It should be usable
 * with minimal to no code changes.   However, the new API is more robust
 * and should be used instead.  */

// Legacy Globals (too many!)
int font_fr, font_fg, font_fb;
int fnfgcol, fnbgcol, shdcol;
int font_br, font_bg, font_bb;
int fshadow, cachefont, colortable[8];

// These might not be needed, enable if they are:
// needed for emufe, so uncommented
 int textbgr, textbgg, textbgb, textfgr, textfgg, textfgb;
 int textsdr, textsdg, textsdb, texthlr, texthlg, texthlb;
 int textier, textieg, textieb;
 int descbgr, descbgg, descbgb;
 int shadowr, shadowg, shadowb;
 int banrbgr, banrbgg, banrbgb, banrfgr, banrfgg, banrfgb;

  fnt_t* DefaultFont;
extern fnt_t* ActiveFont;

// Legacy Char Functions
void font_load(char *filen) {
  //   fnt_t* nf;
   fnt_t* f2;
   PALETTE pal;
   fnt_destroy(DefaultFont);
   DefaultFont=fnt_loadfont(filen,BIOS_8X16);
   printf("DefaultFont type %d\n", DefaultFont->type);
#ifdef BLITFONT 
      f2=fnt_convblit(DefaultFont);
   fnt_destroy(DefaultFont);
   DefaultFont=f2;
   if(cachefont == 1) {
      // get_palette(pal);
      // save_pcx("/tmp/fontbmp.pcx", ActiveFont->data, pal);
    }
#endif
   fnt_setactive(DefaultFont);
//   printf("END font_load()\n");
}

void display_char(int x, int y, unsigned char chr) {
   fnt_display_char(screen,x, y, chr,fnfgcol);
}

void shadow_char(int x, int y, unsigned char chr) {
   fnt_display_char(screen,x+1, y+1, chr,shdcol);
   fnt_display_char(screen,x, y, chr,fnfgcol);
}

void solid_char(int x, int y, unsigned char chr) {
   int fh=ActiveFont->height-1;
   rectfill(screen,x,y,x+7,y+fh,fnbgcol);
   fnt_display_char(screen,x, y, chr,fnfgcol);
}
// End Legacy Char Functions

// Legacy Routines

void set_font_fcolor(int r, int g, int b) {
   font_fr = r; font_fg = g; font_fb = b;
   fnfgcol=makecol(r,g,b);
}

void set_font_bcolor(int r, int g, int b) {
   font_br = r; font_bg = g; font_bb = b;
   fnbgcol=makecol(r,g,b);
}

void set_font_scolor(int r, int g, int b) {
   /* Seems reversed, but why? */
   shdcol=makecol(r,g,b);
}

void solid_string(int x, int y, char *stt) {
   if(fshadow==1)
     fnt_print_string(screen,x,y,stt,fnfgcol,fnbgcol,shdcol);
		      else
     fnt_print_string(screen,x,y,stt,fnfgcol,fnbgcol,-1);
}

void show_string(int x, int y, char *stt) {
   if(fshadow==1)
     fnt_print_string(screen,x,y,stt,fnfgcol,-1,shdcol);
		      else
     fnt_print_string(screen,x,y,stt,fnfgcol,-1,-1);
}
// End Legacy Routines

// Legacy Bitmap Routines
int find_fontcolor_idx(int fcolor) {
   /* When the font is loaded, it is stored in several different colors
    * The color is passed to this function, and the color table will be 
    * searched, returning the bitmap index offset for that color
    * (if available) */
   int i,r;
   
   r=0;
   for(i=0;i<8;i++) {
      if(ActiveFont->color[i] == fcolor) {
	 r=i*16;
	 break;
      }
   }
   return(r);
}

#ifdef BLITFONT
bmp_font_load(char *filen) {
   /* Load cached bitmap fonts rather than bios fonts */
   char fontname[120];
   PALETTE pal;
   
   get_palette(pal);
   sprintf(fontname, "%s/%s",fontdir, filen);
   
   load_colortbl();
   if((fntbmp = load_pcx(fontname,pal)) == NULL) {
      destroy_bitmap(fntbmp);
      font_load(tfont);
   } else {
      ActiveFont->data=fntbmp;
   }
   
   strcpy(currfont,tfont);
}
#endif

// Probably this is no longer needed!
//load_colortbl() {   
//   ActiveFont->color[0]=makecol16(textfgr,textfgg,textfgb);
//   ActiveFont->color[1]=makecol16(shadowr,shadowg,shadowb);
//   ActiveFont->color[2]=makecol16(banrfgr,banrfgg,banrfgb);
//   ActiveFont->color[3]=makecol16(textier,textieg,textieb);
//   ActiveFont->color[4]=makecol16(textsdr,textsdg,textsdb);
//   ActiveFont->color[5]=makecol16(255,255,255);
//}
// End Legacy Bitmap routines

