#include<SDL.h>
#include"sdl_allegro.h"

/* Graphics (primitives) routines */
int getr(int c) {
   Uint8 red,green,blue;
   SDL_GetRGB(c,screen->format,&red,&green,&blue);
   return(red);
}
int getg(int c) {
   Uint8 red,green,blue;
   SDL_GetRGB(c,screen->format,&red,&green,&blue);
   return(green);
}
int getb(int c) {
   Uint8 red,green,blue;
   SDL_GetRGB(c,screen->format,&red,&green,&blue);
   return(blue);
}
int geta(int c) {
   Uint8 red,green,blue,alpha;
   SDL_GetRGBA(c,screen->format,&red,&green,&blue,&alpha);
   return(alpha);
}

Uint32 getpixel(SDL_Surface *surface, int x, int y) {
   // Taken directly from SDL documentation
   int bpp;
   Uint8 *p;
//   int bpp = surface->format->BytesPerPixel;
   /* Here p is the address to the pixel we want to retrieve */
//   Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

   bpp = surface->format->BytesPerPixel;
   p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

//   printf("getpixel bpp=%d\n",bpp);
   switch(bpp) {
    case 1:
      return *p;
      
    case 2:
      return *(Uint16 *)p;
      
    case 3:
      if(SDL_BYTEORDER != SDL_BIG_ENDIAN)
	return p[0] << 16 | p[1] << 8 | p[2];
      else
	return p[0] | p[1] << 8 | p[2] << 16;
      
    case 4:
      return *(Uint32 *)p;
      
    default:
      return 0;       /* shouldn't happen, but avoids warnings */
   }
}

void putpixel(SDL_Surface *bmp,int x, int y, Uint32 color) {
   // Note:  Calling routines should perform the locks and flips for perf
   //   if(SDL_MUSTLOCK(screen)){
   //      if(SDL_LockSurface(screen) < 0) return;
   //   }
   
   
   int bpp = bmp->format->BytesPerPixel;
//   printf("putpixel bpp=%d\n",bpp);
   /* Get Address */
   Uint8 *p = (Uint8 *)bmp->pixels +y * bmp->pitch +x *bpp;
   
   switch(bpp) {
    case 1:
      *p = color;
      break;
      
    case 2:
      *(Uint16 *)p = color;
      break;
      
    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
	 p[0] = (color >> 16) & 0xff;
	 p[1] = (color >> 8) & 0xff;
	 p[2] = color & 0xff;
      } else {
	 p[0] = color & 0xff;
	 p[1] = (color >> 8) & 0xff;
	 p[2] = (color >> 16) & 0xff;
      }
      break;
      
    case 4:
      *(Uint32 *)p = color;
      break;
   }
   
   /* Set Pixel */
//   *(Uint32 *)p = color;
   //   if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
   // 
   //   SDL_Flip(screen);
   
}
	 
void rectfill(SDL_Surface *bmp, int x1, int y1, int x2, int y2, int color) {
   SDL_Rect srect;
   
   srect.x = x1;
   srect.y = y1;
   srect.w = x2 - (x1 -1);
   srect.h = y2 - (y1 -1);
   
   SDL_FillRect(bmp, &srect, color); 
   SDL_UpdateRect(bmp, x1, y1, srect.w, srect.h);
   
}

void rect(SDL_Surface *bmp, int x1, int y1, int x2, int y2, int color) {
   /* This creates a filled rect, need to change that ! */
   SDL_Rect srect;
   
   // Top Line
   srect.x = x1; srect.y = y1;
   srect.w = x2 - (x1 -1); srect.h = 1;
   SDL_FillRect(bmp, &srect, color); 
   // Left Line
   srect.x = x1; srect.y = y1;
   srect.w = 1; srect.h = y2 - (y1 -1);
   SDL_FillRect(bmp, &srect, color); 
   // Right Line
   srect.x = x2; srect.y = y1;
   srect.w = 1; srect.h = y2 - (y1 -1);
   SDL_FillRect(bmp, &srect, color); 
   // Bottom Line
   srect.x = x1; srect.y = y2;
   srect.w = x2 - (x1 -1); srect.h = 1;
   SDL_FillRect(bmp, &srect, color); 

   srect.x = x1; srect.y = y1;
   srect.w = x2 - (x1 -1); srect.h = y2 - (y1 -1);
   
//   SDL_FillRect(bmp, &srect, color); 
   SDL_UpdateRect(bmp, x1, y1, srect.w, srect.h);
   
}

void hline(SDL_Surface *bmp, int x1, int y, int x2, int color) {
   SDL_Rect srect;
   
   srect.x = x1;
   srect.y = y;
   srect.w = x2 - (x1 -1);
   /* Should this be 1? */
   srect.h = 1;
   
   SDL_FillRect(bmp, &srect, color); 
   SDL_UpdateRect(bmp, x1, y, srect.w, srect.h);
   
}

void vline(BITMAP *bmp, int x, int y1, int y2, int color) {
   SDL_Rect srect;

   srect.x = x;
   srect.y = y1;
   srect.w = 1;
   srect.h = y2-y1 ;
   SDL_FillRect(bmp, &srect, color); 
   SDL_UpdateRect(bmp, x, y1, srect.w, srect.h);
}

void fastline(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
   SDL_Rect srect;
   int ix, iy, dx, isx, iex; 
   float iif; 

//   printf("line : %d %d %d %d\n",x1,y1,x2,y2);
   if ( x1 == x2 ) {
      /* Vertical Line */
      vline(bmp,x1,y1,y2,color);
   } else { 
      if( y1 == y2 ) {
	 /* Horizontal Line */
	 hline(bmp,x1,y1,x2,color);
      } else {
	 if (abs(x2-x1) >= abs(y2-y1)) {
	    if (x2 > x1) {
	       isx = x1;
	       iex = x2;
	       dx=1;  /* Increase x */
	       iy = y1;
	    } else {
	       isx = x2;
	       iex = x1;
	       dx=-1; /* Decrease x */
	       iy = y2;
	    }	    
            srect.x=isx;
            srect.y=iy;
	    srect.w=iex-isx;
	    iif = (y2-y1) / (x2-x1) * dx;
//	    printf("x line report: (%d,%d) (%d,%d) isx:%d iex:%d  iif:%f\n",x1,y1,x2,y2,isx,iex,iif);
	    for (ix=isx; ix<iex; ix++) {
	       putpixel(bmp,ix,iy,color);
	       iy = iy + iif;  // possibly mixing integer and float bad
	    }
	    srect.h=iy-srect.y;
	 } else {
	    if (y2 > y1) {
	       isx = y1;
	       iex = y2;
	       dx=1;  /* Increase x */
	       ix = x1;
	    } else {
	       isx = y2;
	       iex = y1;
	       dx=-1; /* Decrease x */	    
	       ix = x2;
	    }
            srect.x=ix;
            srect.y=isx;
	    srect.h=iex-isx;
	    iif = (x2-x1) / (y2-y1) * dx;
//	    printf("y line report: (%d,%d) (%d,%d) isx:%d iex:%d  iif:%f\n",x1,y1,x2,y2,isx,iex,iif);
	    for (iy=isx; iy<iex; iy++) {
	       putpixel(bmp,ix,iy,color);
	       ix = ix + iif;
	    }
	    srect.w=ix-srect.x;
	 }	 
	 // Why was this here?  It causes problems if lines are drawn in
	 // certain directions
//	 SDL_UpdateRect(bmp, srect.x, y1, srect.w, srect.h);
	 SDL_UpdateRect(bmp, srect.x, srect.y, srect.w, srect.h);
      } /* hline */
   } /* vline */
}
   
void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
   fastline(bmp,x1,y1,x2,y2,color);
}

void triangle(BITMAP *bmp,int x1,int y1,int x2, int y2, int x3, int y3, int color) {
   /* TODO, this needs to be filled,  might be able to steal code from arps */
   fastline(bmp,x1,y1,x2,y2,color);
   fastline(bmp,x2,y2,x3,y3,color);
   fastline(bmp,x3,y3,x1,y1,color);
}

	 
	 
