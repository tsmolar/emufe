#include <stdio.h>
#ifdef GFXLIB_ALLEGRO
# ifdef USESDL
#  include <SDL.h>
#  include "sdl_allegro.h"
# else
#  include <allegro.h>
# endif
#endif
#include "widget.h"

void wdgst_default() {
   // Sets default style
   int bg,tx,hl,sh;
//   bg=makeacol(184,184,224,255);
//   tx=makeacol(64,64,64,255);
//   hl=makeacol(255,255,255,255);
//   sh=makeacol(128,128,192,255);
   bg=makecol(184,184,224);
   tx=makecol(64,64,64);
   hl=makecol(255,255,255);
   sh=makecol(128,128,192);
   activestyle.input.bg=bg;
   activestyle.input.hl=hl;
   activestyle.input.sh=sh;
   activestyle.input.tx=tx;
   activestyle.input.al=255;
   activestyle.button.bg=bg;
   activestyle.button.hl=hl;
   activestyle.button.sh=sh;
   activestyle.button.tx=tx;
   activestyle.button.al=255;
   activestyle.window.bg=makecol(128,128,128);
   activestyle.window.sh=makecol(64,64,64);
   activestyle.window.al=230;
}

void wdgst_apply(style_t newstyle) {
   activestyle=newstyle;
//   printf("inDid it work?  abg: %d %d %d\n",
//	  getr(activestyle.input.bg),
//	  getg(activestyle.input.bg),getb(activestyle.input.bg));
}

style_t wdgst_getactive() {
   return(activestyle);
}

style_rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color,int ba) {
   int x,y,c;
   int sr,sg,sb,sa;   // src
   int br,bg,bb;
   int nr,ng,nb;
#ifdef SDL2
   SDL_Rect srect;
#endif

   br=getr(color); bg=getg(color); bb=getb(color);
//   printf("NEW: I am called with trans:%d  red:%d\n",ba,br);
   if(ba==255 || bmp!=screen) {
//      printf("NEW: Do it the old way!\n");
//      printf("rectfill(bmp, %d, %d, %d, %d, makecol(%d,%d,%d))\n",x1,y1,x2,y2,br,bg,bb);
      rectfill(bmp, x1, y1, x2, y2, makecol(br,bg,bb));
   } else {
//      printf("NEW: Do it the new way!\n");
#ifdef USESDL
# ifdef SDL1
      if(SDL_LockSurface(screen) < 0) return;
# endif
#endif
      sa=255-ba;

      rectfill(screen, x1, y1, x2, y2, makecol(br,bg,bb));
      
      for(y=y1;y<=y2;y++) {
	 for(x=x1;x<=x2;x++) {
	    c=getpixel(screen,x,y);
	    sr=getr(c);
	    sg=getg(c);
	    sb=getb(c);
	    nr=(((sr*sa)+(br*ba))/255);
	    ng=(((sg*sa)+(bg*ba))/255);
	    nb=(((sb*sa)+(bb*ba))/255);
	    putpixel(screen,x,y,makecol(nr,ng,nb));
//	    if(nr>255)
//	      printf("NEW uhoh!  %d + %d\n",sr,br);
	 }
      }
      
#ifdef USESDL
# ifdef SDL1
      SDL_UnlockSurface(screen);
      SDL_UpdateRect(screen,x1,y1,(x2-x1),(y2-y1));
# endif
# ifdef SDL2
      srect.x = x1;
      srect.y = y1;
      srect.w = x2-x1;
      srect.h = y2-y1;
      printf("NEW RECT:  %d,%d - %d,%d\n",srect.x, srect.y, srect.w, srect.h);
      SDL_RenderCopy(sdlRenderer,screen,&srect,&srect);
      SDL_RenderPresent(sdlRenderer);
# endif
#endif   
   }
}

void style_hline(BITMAP *bmp, int x1, int y, int x2, int color,int ba) {
   int x,c;
   int sr,sg,sb,sa;   // src
   int br,bg,bb;
   int nr,ng,nb;
#ifdef SDL2
   SDL_Rect srect;
#endif

   br=getr(color); bg=getg(color); bb=getb(color);
   if(ba==255 || bmp != screen) {
      hline(bmp,x1,y,x2,makecol(br,bg,bb));
   } else {
#ifdef USESDL
# ifdef SDL1
      if(SDL_LockSurface(screen) < 0) return;
# endif
#endif
      
   sa=255-ba;
   
   for(x=x1;x<=x2;x++) {
      c=getpixel(screen,x,y);
      sr=getr(c);
      sg=getg(c);
      sb=getb(c);
      nr=(((sr*sa)+(br*ba))/255);
      ng=(((sg*sa)+(bg*ba))/255);
      nb=(((sb*sa)+(bb*ba))/255);
      putpixel(screen,x,y,makecol(nr,ng,nb));
   }
     
#ifdef USESDL
# ifdef SDL1
   SDL_UnlockSurface(screen);
   SDL_UpdateRect(screen,x1,y,(x2-x1),y);
# endif
# ifdef SDL2
      srect.x = x1;
      srect.y = y;
      srect.w = x2-x1;
      srect.h = y;
      SDL_RenderCopy(sdlRenderer,screen,&srect,&srect);
      SDL_RenderPresent(sdlRenderer);
# endif
#endif   
   }
}

void style_vline(BITMAP *bmp, int x, int y1, int y2, int color,int ba) {
   int y,c;
   int sr,sg,sb,sa;   // src
   int br,bg,bb;
   int nr,ng,nb;
#ifdef SDL2
   SDL_Rect srect;
#endif

   br=getr(color); bg=getg(color); bb=getb(color);
   if(ba==255 || bmp != screen) {
      vline(bmp,x,y1,y2,makecol(br,bg,bb));
   } else {
#ifdef USESDL
# ifdef SDL1
   if(SDL_LockSurface(screen) < 0) return;
# endif
#endif

   sa=255-ba;
   
   for(y=y1;y<=y2;y++) {
      c=getpixel(screen,x,y);
      sr=getr(c);
      sg=getg(c);
      sb=getb(c);
      nr=(((sr*sa)+(br*ba))/255);
      ng=(((sg*sa)+(bg*ba))/255);
      nb=(((sb*sa)+(bb*ba))/255);
      putpixel(screen,x,y,makecol(nr,ng,nb));
   }
   
#ifdef USESDL
# ifdef SDL1
   SDL_UnlockSurface(screen);
   SDL_UpdateRect(screen,x,y1,x,(y2-y1));
# endif
# ifdef SDL2
      srect.x = x;
      srect.y = y1;
      srect.w = x;
      srect.h = y2-y1;
      SDL_RenderCopy(sdlRenderer,screen,&srect,&srect);
      SDL_RenderPresent(sdlRenderer);
# endif
#endif   
   }
}
   
void style_rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color,int ba) {
   style_hline(bmp,x1,y1,x2,color,ba);
   style_vline(bmp,x1,y1+1,y2-1,color,ba);
   style_hline(bmp,x1,y2,x2,color,ba);
   style_vline(bmp,x2,y1+1,y2-1,color,ba);
}
