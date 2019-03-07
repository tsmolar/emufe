#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef USE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif
#include <stdio.h>

#ifndef USESDL
#include <allegro.h>
//#include <allegro/internal/aintern.h>
#else
#include <SDL.h>
#include "sdl_allegro.h"
// New for SDL2 support
#ifdef SDL2
#define SA_COLORKEY SDL_TRUE
#endif
#ifdef SDL1
#define SA_COLORKEY SDL_SRCCOLORKEY
#endif
// end SDL2 code
#endif
//unsigned char fontdata[4096];
#include "font.h"

#ifdef USE_FREETYPE
extern void fnt_ttf_init();
extern int fnt_ttf_calcwidth(char *text);
extern int fnt_ttf_calcheight(char *text);
extern void fnt_ttf_loadfont(fnt_t *myfont, char *filen);
extern void fnt_ttf_print_string(BITMAP *b, int x, int y, char *text, int fg, int bg, int sd);
#endif

#ifdef COMPFONT
#include "fontdata.h"
fnt_t* cf8x16;
#endif

#define FONT_VERSION "3.7.5"

fnt_t* ActiveFont;  // Current Active Font

// unsigned char arps_font[16][256];
char fontdir[90], tfont[30], currfont[130];

#ifdef USE_FREETYPE
FT_Library library;
#endif
BITMAP *fntbmp;

void fnt_version_info() {
   printf("Font Library Version %s\n",FONT_VERSION);
#ifdef COMPFONT
   printf("  * Using Compiled-in Font\n");
#else
   printf("  * Using disk-loaded Fonts only\n");
#endif
}

// Improved Font Routines
fnt_t *fnt_newfont(int ftype) {
   fnt_t *f;
   f = malloc(sizeof(fnt_t));

   //printf("fnt_newfont: pointer: %p\n",(void *)f);
//   struct fnt_t *f = malloc(sizeof(struct fnt_t));
#ifdef USESDL
   Uint32 ckey;
#endif
   BITMAP* bfd;
   unsigned char* fd;

   // printf("ftype is %d (1)\n", ftype);
   switch(ftype) {
    case TTF:
      f->width=f->height=f->scale_w=0;
      f->scale_h=14;
//      fd=(unsigned char*)malloc(204800);
//      f->data=fd;
    case BIOS_8X8:
      f->width=8;
      f->height=8;
      f->size=2048;
      fd=(unsigned char*)malloc(2048);
      f->data=fd;
      break;
    case BIOS_8X16: 
      f->width=8;
      f->height=16;
      f->size=4096;
      fd=(unsigned char*)malloc(4096);
      f->data=fd;
      break;
    case BLIT_8X8:
      f->width=8;
      f->height=8;
      bfd=create_bitmap(1024,64);
      rectfill(bfd,0,0,1024,64,makecol(255,0,255));
#ifdef USESDL
      ckey=SDL_MapRGB(bfd->format,255,0,255);
      SDL_SetColorKey(bfd,SA_COLORKEY,ckey);
#endif
      f->data=bfd;
      break;
    case BLIT_8X16: 
      f->width=8;
      f->height=16;
      bfd=create_bitmap(1024,128);
      rectfill(bfd,0,0,1024,128,makecol(255,0,255));
#ifdef USESDL
      ckey=SDL_MapRGB(bfd->format,255,0,255);
      SDL_SetColorKey(bfd,SA_COLORKEY,ckey);
#endif
      f->data=bfd;
      break;
   }
//   printf("ftype is %d (2)\n", ftype);

   f->type=ftype;
//   printf("ftype is %d (3)\n", f->type);
   return f;
}

void fnt_destroy(fnt_t* font) {
  //  if(font->data != NULL) free(font->data);
  if(font != NULL) free(font);
}

void fnt_setactive(fnt_t* font) {
//   printf("fnt_setactive: font_p=%p\n",font);
   ActiveFont=font;
//   printf("fnt_setactive: ActiveFont_p=%p\n",ActiveFont);
//   printf("Setting new active font -> %s\n",ActiveFont->name);
//   printf("--> font size %d\n",ActiveFont->size);
//   printf("--> font type %d\n",ActiveFont->type);
}

fnt_t* fnt_getactive() {
//   printf("Retrieving new active font -> %s\n",ActiveFont->name);
//   printf("--> font size %d\n",ActiveFont->size);
//   printf("--> font type %d\n",ActiveFont->type);
//   printf("fnt_getactive: ActiveFont_p=%p\n",ActiveFont);
   return ActiveFont;
}

int fnt_blitnewcolor(fnt_t* cfont,int fcolor) {
   int ix,iy,h,yo,oc=makecol(0,0,0);
   
   printf("creating a new color!\n");
   h=cfont->height;
   cfont->lru++;
   if(cfont->lru>7)
     cfont->lru=1;
   cfont->color[cfont->lru]=fcolor;
   yo=cfont->lru*h;
   for(iy=0;iy<h;iy++) {
      for(ix=0;ix<1024;ix++) {
	 if(getpixel(cfont->data,ix,iy)==oc)
	   putpixel(cfont->data,ix,iy+yo,fcolor);
      }
   }
   return cfont->lru;
}

int fnt_blitcoloridx(fnt_t* cfont,int fcolor) {
   int i,rv=-1;
   for(i=0;i<8;i++) {
      if(cfont->color[i]==fcolor)
	rv=i;
   }
   if(rv==-1)
     rv=fnt_blitnewcolor(cfont, fcolor);
   return(rv);
}

fnt_t* fnt_loadfont(char *filen, int ftype) {
   // Improved font load that doesn't try to append a directory
   // This will eventually return a pointer to a font.
   FILE *fp;
   char a;
   unsigned char* w;
   int ix,iy;
   int char_h;
   fnt_t *lfont;
#ifdef DEBUG
   printf("creating font with %d\n",ftype);
#endif
   
   lfont=fnt_newfont(ftype);
//printf("fnt_loadfont pointer is %p\n",(void *)lfont);
//   printf("f-type is %d (1)\n", lfont->type);
   w=lfont->data;
   char_h=lfont->height;
//   printf("f-type is %d (1.1)\n", lfont->type);
   strcpy(lfont->name,filen);
//   printf("f-type is %d (1.2)\n", lfont->type);
//#ifdef DEBUG
//   printf("fontname is %s\n", fontname);
//#endif
   if(ftype==TTF) {
#ifdef USE_FREETYPE
      printf("loading TTF: %s\n",filen);
      fnt_ttf_loadfont(lfont,filen);
#else
      printf("Error: TrueType support not compiled in\n");
#endif
   } else { 
      // NOTE: Chaned fontname to filen b/c it's having trouble
      //     with paths.   Probably a cygwin/mingwin issue
      if ((fp = fopen(filen,"rb"))==NULL)  {
	 printf("Error opening file %s",filen);
	 exit(1);
      }
#ifdef DEBUG
      printf("Loading fonts....");
#endif	
      for(iy=0;iy<256;iy++)  {
	 for(ix=0;ix<char_h;ix++) {
	    a=(unsigned char)fgetc(fp);
	    //	 fontdata[(iy*16)+ix]=a;
	    w[(iy*16)+ix]=a;
	 }
      }
      //   printf("Is this the offensive line? %d\n",lfont->data);
      //   memcpy(lfont->data,&fontdata[(iy*16)+ix],lfont->size);
      //   printf("It wasn't\n");
#ifdef DEBUG
      printf("Done.\n");
#endif
      fclose(fp);
   }
   strcpy(currfont,filen);

   return lfont;
}

void fnt_display_char(BITMAP *b,int x, int y, unsigned char chr,int fcolor) {
   int t,c,dc,cx;
   int fh=ActiveFont->height;
   unsigned char* fdata;
   BITMAP* bfdata;
#ifdef USESDL
   SDL_Rect srect, drect;
#endif

   if(ActiveFont->type==BIOS_8X8 || ActiveFont->type==BIOS_8X16) {
      fdata=ActiveFont->data;
      for(t=0; t<fh; t++) {
	 //      c=fontdata[(chr*16)+t];
	 c=fdata[(chr*fh)+t];
	 for(cx=0;cx<8;cx++) {
	    c<<=1;
	    if(c>255) {
	       //	    putpixel(b, x+cx, y+t, fnfgcol);
	       putpixel(b, x+cx, y+t, fcolor);
	       c=c-256;  
	    }
	 }
      }
   }
   if(ActiveFont->type==BLIT_8X8 || ActiveFont->type==BLIT_8X16) {
      c=fnt_blitcoloridx(ActiveFont,fcolor);
#ifdef USESDL
      srect.x = (chr*8); srect.y = (c*fh);
      srect.w = 8;       srect.h = fh;
      drect.x = x;       drect.y = y;
      drect.w = 8;       drect.h = fh;
      
      SDL_BlitSurface(ActiveFont->data, &srect, screen, &drect);
#else
      masked_blit(ActiveFont->data,screen,(chr*8),c*fh,x,y,8,fh);
#endif
   }
} // fnt_display_char

fnt_t* fnt_convblit(fnt_t* font) {
   // Make a blit font (for speed on some platforms)
   fnt_t* lfont;
   fnt_t* cact;
   int ntype,ix;
   int black=makecol(0,0,0);
   unsigned char* sf=font->data;
   BITMAP *df;

   if(font->type==BIOS_8X8) ntype=BLIT_8X8;
   if(font->type==BIOS_8X16) ntype=BLIT_8X16;
   lfont=fnt_newfont(ntype);
   df=lfont->data;
   lfont->color[0]=black;
   lfont->lru=0;
   cact=ActiveFont;
   ActiveFont=font;
   for(ix=0;ix<128;ix++) {
      fnt_display_char(df,ix*8, 0, ix,black);
   }
   // restore active font
   ActiveFont=cact;
   return lfont;
}

// More Improved Font Routines
void fnt_print_string(BITMAP *b, int x, int y, char *str, long fg, long bg, long sd) {
   // New method that can draw on any bitmap, and hand solid, shadow and plain
   int c=0, l=0, cx,cy, sl=strlen(str)*8;
   int fh=ActiveFont->height-1;

#ifdef SDL2
   SDL_Rect frect;
#endif

   if(ActiveFont->type==TTF ) {
#ifdef USE_FREETYPE
      fnt_ttf_print_string(b,x,y,str,fg,bg,sd);
#else
      printf("Error: TrueType support not compiled in\n");
#endif
   } else {
      // bg is now a 'long', and -1 maps to a positive number
      // so I added a cast
      // changed >-1 to != -1 since no matter what I do, these show up as 
      // negative signed in 32 bit depth
      if((int)bg!=-1)
	 rectfill(b,x,y,x+sl,y+fh,bg);
      
#ifdef USESDL
      // Put here for speed optimization
      //   if(SDL_MUSTLOCK(screen)){
      if(b==screen && ActiveFont->type<2) {
	if(SDL_LockSurface(b) < 0) return;
      }
#endif
      
      while(*str) {
	 if (*str == '\n'){
	    l++;
	    str++;
	    c=0;
	 } else {
	    cx=x+(c++)*8;
	    cy=y+l*9;
	    if(sd>-1) // shadow
	      fnt_display_char(b, cx+1, cy+1, *(str),sd);
	    fnt_display_char(b, cx, cy, *(str++),fg);
	 }
      }
      
#ifdef USESDL
      if(b==screen) {
	 if(ActiveFont->type<2)
	   SDL_UnlockSurface(b);
	 if(SA_AUTOUPDATE==1) {
# ifdef SDL1	      
	    SDL_UpdateRect(screen,x,y,sl,16);
	    //      printf("hokee: %d %d %d\n",x,y,sl);
# endif
# ifdef SDL2
	    frect.x=x; frect.y=y;
	    frect.w=sl; frect.h=16;
	    SDL_RenderCopy(sdlRenderer,screen,&frect,&frect);
	    SDL_RenderPresent(sdlRenderer);
# endif
	 }
      }
#endif
   }   
} // fnt_print_string
   
/* Calculates the width and height of the string in pixels.
 * width and height have to be non-constant pointers to int.	
 */
 
int calc_width(char *stt) {
   int ffw=ActiveFont->width;
   int tmp_width=ffw;
   int width=ffw;
   
   if(ActiveFont->type==TTF) {
#ifdef USE_FREETYPE
      width=fnt_ttf_calcwidth(stt);
#else
      printf("Error: Truetype not compiled-in\n");
#endif      
   } else {
      while(*stt) {
	 if (*stt == '\n') {
	    if(tmp_width > width)
	      width=tmp_width;
	    tmp_width=ffw;
	 } else
	   tmp_width+=ffw;
	 stt++;
      }
      if(tmp_width > width)
	width=tmp_width;
   }      
   return width;
}

int calc_height(char *stt) {
   int ffh=ActiveFont->height;
   int height=ffh;
   
   if(ActiveFont->type==TTF) {
#ifdef USE_FREETYPE
      height=fnt_ttf_calcheight(stt);
#else
      printf("Error: Truetype not compiled-in\n");
#endif      
   } else {
      while(*stt) {
	 if (*stt == '\n')
	   height+=ffh;
	 stt++;
      }
   }
   return height ;  
}

void fnt_init() {
   // right now, only sets up compiled-in fonts
   int i;
#ifdef COMPFONT
   cf8x16=fnt_newfont(BIOS_8X16);
   strcpy(cf8x16->name,"COMPFONT8x16");
   //   for(i=0;i<4096;i++)
   //   cf8x16->data[i]=fontdata[i];
   cf8x16->data=fontdata;
   fnt_setactive(cf8x16);
#endif
#ifdef USE_FREETYPE
   fnt_ttf_init();
#endif
#ifdef DEBUG
   printf("done fnt_init()\n");
#endif
}

void fnt_setscale(fnt_t *myfont, int w, int h) {
   myfont->scale_w=w;
   myfont->scale_h=h;
}
