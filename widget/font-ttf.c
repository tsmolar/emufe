#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <ft2build.h>  
#include FT_FREETYPE_H 

#ifndef USESDL
#include <allegro.h>
#else
# include <SDL.h>
# include "sdl_allegro.h"
#endif

#include "font.h"

// Experimental TTF support, using freetype
// 
// The hope is that by using freetype rather than SDL_ttf and allegttf,
// that portability will be improved, since they both depend on freetype
// anyway

extern FT_Library library;
FT_Face face; 
int fnt_render;

// Glyph Renderers, see comments for differences between them.

int fnt_blendfunc(int pix, int drw, int alp) {
   int adf;
   if(alp==0) return pix;
   if(alp==255) return drw;
   adf=255-alp;
   return ((pix*adf + drw*alp) / 255);
}

void fnt_ttf_draw_bitmap_blend( FT_Bitmap* fbitmap, BITMAP *b, FT_Int x, FT_Int y, int color) {
   
   // Simple render
   
   FT_Int  i, j, p, q;
   FT_Int  x_max = x + fbitmap->width;
   FT_Int  y_max = y + fbitmap->rows;
   int pix;
   int bmc, bmr, bmg, bmb;
//   int bgr, bgg, bgb, fac;
   int pnr, png, pnb, opr, opg, opb, opc;
   
   pnr=getr(color);
   png=getg(color);
   pnb=getb(color);
   
   for ( i = x, p = 0; i < x_max; i++, p++ ) {
      for ( j = y, q = 0; j < y_max; j++, q++ ) {
//	 if ( i >= WIDTH || j >= HEIGHT )
//	   continue;

	 bmc=getpixel(b,i,j);
	 bmr=getr(bmc);	 bmg=getg(bmc);	 bmb=getb(bmc);
	 
	 pix = fbitmap->buffer[q * fbitmap->width + p];

	 if(pix>0) {
	    opr=fnt_blendfunc(bmr,pnr,pix);
	    opg=fnt_blendfunc(bmg,png,pix);
	    opb=fnt_blendfunc(bmb,pnb,pix);
	    
	    opc=makecol(opr,opg,opb);
	    putpixel(b,i,j,opc);
	 }
      }	
   }   
//#ifdef USESDL
////   if(b==screen)
//     SDL_UpdateRect(b,x,y,x_max-x,y_max-y);
//#endif
}

void fnt_ttf_draw_bitmap_simple( FT_Bitmap* fbitmap, BITMAP *b, FT_Int x, FT_Int y, int color) {
   
   // Simple render
   // Limited/no Anti-aliasing
   
   FT_Int  i, j, p, q;
   FT_Int  x_max = x + fbitmap->width;
   FT_Int  y_max = y + fbitmap->rows;
   int pix;
   int pnr, png, pnb, opc;
   
   
   for ( i = x, p = 0; i < x_max; i++, p++ ) {
      for ( j = y, q = 0; j < y_max; j++, q++ ) {
//	 if ( i >= WIDTH || j >= HEIGHT )
//	   continue;

	 pix = fbitmap->buffer[q * fbitmap->width + p];

	 if(pix>128) {
//	    opc=makecol(pnr,png,pnb);
	    putpixel(b,i,j,color);
	 }
      }	
   }   
//#ifdef USESDL
////   if(b==screen)
//     SDL_UpdateRect(b,x,y,x_max-x,y_max-y);
//#endif
}

#ifdef USESDL
void fnt_ttf_draw_bitmap_sdl( FT_Bitmap* fbitmap, BITMAP *b, FT_Int x, FT_Int y, Uint32 color) {
   
   // This uses SDL per-pixel alpha-blending to provide clean anti-aliasing
   // There may not be a way to reconcile this with allegro's methods, hence
   // the separate function
   // 
   // Probably slow, but looks really good!
   
   FT_Int  i, j, p, q;
   FT_Int  x_max = x + fbitmap->width;
   FT_Int  y_max = y + fbitmap->rows;
   int pix, pnr, png, pnb, opc;
   
   pnr=png=pnb=255;
   SDL_GetRGB(color,screen->format,&pnr,&png,&pnb);
//   SDL_GetRGB(color,b->format,&pnr,&png,&pnb);
//   printf("color:  r:%d   g:%d   b:%d\n",pnr,png,pnb);
   
   for ( i = x, p = 0; i < x_max; i++, p++ ) {
      for ( j = y, q = 0; j < y_max; j++, q++ ) {
//	 if ( i >= WIDTH || j >= HEIGHT )
//	   continue;

	 pix = fbitmap->buffer[q * fbitmap->width + p];

	 opc=SDL_MapRGBA(b->format,pnr,png,pnb,pix);
	 putpixel(b,i,j,opc);
      }	
   }   
//#ifdef USESDL
//   if(b==screen)
//     SDL_UpdateRect(b,x,y,x_max-x,y_max-y);
//#endif
}
#endif

void fnt_ttf_init() {
   FT_Error error;
#ifdef DEBUG
   printf("in fnt_ttf_init()\n");
#endif
//   fnt_ttf_setrender(RENDER_SIMPLE);
   fnt_ttf_setrender(RENDER_BLEND);
//   fnt_ttf_setrender(RENDER_NATIVE);
   error = FT_Init_FreeType( &library ); 
   if ( error ) {
      printf("... an error occurred during freetype initialization ...\n"); 
   }
#ifdef DEBUG
   printf("done fnt_ttf_init\n");
#endif
}

void fnt_ttf_loadfont(fnt_t *myfont ,char *filen) {
   FT_Error error;
#ifdef DEBUG
   printf("load ttf font:%s\n",filen);
#endif
   error = FT_New_Face( library, filen, 0, &face ); 
#ifdef DEBUG
   printf("Face Family: %s\n",face->family_name);
#endif
   if ( error == FT_Err_Unknown_File_Format ) {
      printf("unsupported font format\n"); 
   }
   else if ( error ) {
#ifdef DEBUG
      printf("could not open font\n"); 
#endif
   }
//printf("EXIT\n");
}

void fnt_ttf_print_string(BITMAP *b, int x, int y, char *text, int fg, int bg, int sd) {
   // Refined Version from tutorial
   FT_GlyphSlot slot = face->glyph; /* a small shortcut */  
   FT_Error error;
   fnt_t* cfont;
   BITMAP* line1,line0;

   int pen_x, pen_y, n; 
#ifdef USESDL
   Uint32 rmask,gmask,bmask,amask;
#else
   int rmask,gmask,bmask,amask;
#endif
   // ... initialize library ... 
   //  ... create face object ... 

   // printf("fnt_ttf_print_string\n");
   cfont=fnt_getactive();
   // printf("scale_h=%d\n",cfont->scale_w);
   error = FT_Set_Pixel_Sizes(face, /* handle to face object */  
			      cfont->scale_w,    /* pixel_width */
			     cfont->scale_h );  /* pixel_height */
   pen_x = x; pen_y = y+cfont->scale_h-1;
   pen_x = 0; pen_y = cfont->scale_h-1;

   if(fnt_render==RENDER_NATIVE) {
      // for SDL, you can use this
#ifdef USESDL	
# if SDL_BYTEORDER == SDL_BIG_ENDIAN
      rmask=0xff000000; gmask=0x00ff0000; bmask=0x0000ff00; amask=0x000000ff;
# else
      rmask=0x000000ff; gmask=0x0000ff00; bmask=0x00ff0000; amask=0xff000000;
# endif
      line1=SDL_CreateRGBSurface(SDL_SWSURFACE,640,cfont->scale_h*2,32,rmask,gmask,bmask,amask);
#endif
      rectfill(line1,0,0,640,cfont->scale_h*2,makeacol(0,0,0,0));
   } else {
      // For generic sdl/allegro, use this:
      line1=create_bitmap(640,cfont->scale_h*2);
      if(bg==-1) 
	blit(screen,line1,x,y,0,0,640-x,y+cfont->scale_h*2);
      else
	rectfill(line1,0,0,640,cfont->scale_h*2,bg);
   }

//   printf ("x4\n");
//   printf("rendering: %s\n",text);
   for ( n = 0; n < strlen(text); n++ ) {
      if(text[n] != 10 ) {
	 error = FT_Load_Char( face, text[n], FT_LOAD_RENDER ); 
	 if ( error ) continue; /* ignore errors */

	 // LOOK HERE
	 if(fnt_render==RENDER_SIMPLE) {
	    if(sd>-1)
	      fnt_ttf_draw_bitmap_simple( &slot->bitmap, line1, pen_x + slot->bitmap_left+cfont->sox, pen_y - slot->bitmap_top+cfont->soy, sd ); /* increment pen position */   
	    fnt_ttf_draw_bitmap_simple( &slot->bitmap, line1, pen_x + slot->bitmap_left, pen_y - slot->bitmap_top, fg ); /* increment pen position */   
	 }
	 if(fnt_render==RENDER_BLEND) {
	    if(sd>-1)
	      fnt_ttf_draw_bitmap_blend( &slot->bitmap, line1, pen_x + slot->bitmap_left+cfont->sox, pen_y - slot->bitmap_top+cfont->soy, sd ); /* increment pen position */   
	    fnt_ttf_draw_bitmap_blend( &slot->bitmap, line1, pen_x + slot->bitmap_left, pen_y - slot->bitmap_top, fg ); /* increment pen position */   
	 }
	 if(fnt_render==RENDER_NATIVE) {
#ifdef USESDL
	    if(sd>-1)
	      fnt_ttf_draw_bitmap_sdl( &slot->bitmap, line1, pen_x + slot->bitmap_left+cfont->sox, pen_y - slot->bitmap_top+cfont->soy, sd ); /* increment pen position */   
	    fnt_ttf_draw_bitmap_sdl( &slot->bitmap, line1, pen_x + slot->bitmap_left, pen_y - slot->bitmap_top, fg ); /* increment pen position */   
#endif
	 }
      }
      pen_x += slot->advance.x >> 6; pen_y += slot->advance.y >> 6; /* not useful for now */  
   }
//   printf ("x5\n");
if(fnt_render==RENDER_NATIVE && bg>-1)
     rectfill(screen,x,y,pen_x+x,pen_y+y,bg);
   masked_blit(line1,screen,0,0,x,y,pen_x,cfont->scale_h+4);
   destroy_bitmap(line1);
//   destroy_bitmap(line0);
//   printf("%s\n",text);
}

void fnt_ttf_setrender(int r) {
   fnt_render=r;
}

int fnt_ttf_calcwidth(char *text) {
   // Refined Version from tutorial
   FT_GlyphSlot slot = face->glyph; /* a small shortcut */  
   FT_Error error;
   fnt_t* cfont;
   BITMAP* line1;

   int pen_x, pen_y, n; 
#ifdef USESDL
   Uint32 rmask,gmask,bmask,amask;
#else
   int rmask,gmask,bmask,amask;
#endif

   cfont=fnt_getactive();
   error = FT_Set_Pixel_Sizes(face, /* handle to face object */  
			      cfont->scale_w,    /* pixel_width */
			     cfont->scale_h );  /* pixel_height */
   
   pen_x = 0; pen_y = cfont->scale_h-1;
   line1=create_bitmap(640,cfont->scale_h*2);
   for ( n = 0; n < strlen(text); n++ ) {
      if(text[n] != 10 ) {
	 error = FT_Load_Char( face, text[n], FT_LOAD_RENDER ); 
	 if ( error ) continue; /* ignore errors */  
	 
      }
      pen_x += slot->advance.x >> 6; pen_y += slot->advance.y >> 6; /* not useful for now */  
   }
   destroy_bitmap(line1);
   return(pen_x);
}

int fnt_ttf_calcheight(char *text) {
   // Refined Version from tutorial
   FT_GlyphSlot slot = face->glyph; /* a small shortcut */  
   FT_Error error;
   fnt_t* cfont;
   BITMAP* line1;

   int pen_x, pen_y, n; 
#ifdef USESDL
   Uint32 rmask,gmask,bmask,amask;
#else
   int rmask,gmask,bmask,amask;
#endif
   
   cfont=fnt_getactive();
   error = FT_Set_Pixel_Sizes(face, /* handle to face object */  
			      cfont->scale_w,    /* pixel_width */
			     cfont->scale_h );  /* pixel_height */
   
   pen_x = 0; pen_y = cfont->scale_h-1;
   line1=create_bitmap(640,cfont->scale_h*2);
   for ( n = 0; n < strlen(text); n++ ) {
      if(text[n] != 10 ) {
	 error = FT_Load_Char( face, text[n], FT_LOAD_RENDER ); 
	 if ( error ) continue; /* ignore errors */  
	 
      }
      pen_x += slot->advance.x >> 6; pen_y += slot->advance.y >> 6; /* not useful for now */  
   }
   destroy_bitmap(line1);
   return(pen_y);
}
