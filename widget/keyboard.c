#include <stdio.h>
#ifdef GFXLIB_ALLEGRO
# ifdef USESDL
#  include <SDL.h>
#  include "sdl_allegro.h"
# else
#  include <allegro.h>
# endif
#endif
//#include "widget.h"
#include "button.h"
// Keyboard and hilighting functions
//    to help make the gui kdb accessable

struct kdbhl_t {
   Widget* hilighted;
   void* bmp;  /* data storage */
   int color;
} kbdhl_curr;

//struct kbdhl_t kbdhl_curr;

wdg_unhilight() {
   BITMAP *mbmp;
   int x1,y1,x2,y2,c;
//   printf("xwu 1\n");
    kbdhl_curr.hilighted=wdg_getlevel(W_HILIGHTED);
//   printf("xwu 2 %d\n",kbdhl_curr.hilighted);
    if(kbdhl_curr.hilighted) {
      x1=kbdhl_curr.hilighted->x1; x2=kbdhl_curr.hilighted->x2;
      y1=kbdhl_curr.hilighted->y1; y2=kbdhl_curr.hilighted->y2;
      mbmp=kbdhl_curr.bmp;
      if(mbmp)
	blit(mbmp,screen,0,0,x1-2,y1-2,(x2-x1)+4,(y2-y1)+4);
   }
//   printf("xwu 3\n");
}

wdg_hilight(int inew) {   
   // Set inew to 1 for a new hilight, 0 for existing
   BITMAP *mbmp;
   int x1,y1,x2,y2,c,c2,r2,g2,b2,hx,hy;

   kbdhl_curr.hilighted=wdg_getlevel(W_HILIGHTED);
   if(kbdhl_curr.hilighted) {
//      printf("whl a %d\n",kbdhl_curr.hilighted);
      x1=kbdhl_curr.hilighted->x1; x2=kbdhl_curr.hilighted->x2;
      y1=kbdhl_curr.hilighted->y1; y2=kbdhl_curr.hilighted->y2;
      if (inew==1) {
	 // destroy previous bitmap
	 if(kbdhl_curr.bmp) {
	    mbmp=kbdhl_curr.bmp;
	    destroy_bitmap(mbmp);
	 }
	 mbmp=create_bitmap((x2-x1)+5,(y2-y1)+5);
	 blit(screen,mbmp,x1-2,y1-2,0,0,(x2-x1)+4,(y2-y1)+4);
	 //      blit(screen,screen,x1-2,0,y1-2,0,(x2-x1)+2,(y2-y1)+2);
	 //      blit(mbmp,screen,0,0,0,0,(x2-x1)+4,(y2-y1)+4);
	 kbdhl_curr.bmp=mbmp;
      }
      if(kbdhl_curr.hilighted->btbmp.btnhlbmp) {
//	 printf("whl b\n");
	 hx=kbdhl_curr.hilighted->btbmp.hl_x1;
//	 printf("whl b2\n");
	 hy=kbdhl_curr.hilighted->btbmp.hl_y1;
//	 printf("whl b3\n");
//	 printf("bmp:%d hx %d hy %d x1 %d y1 %d w %d h %d \n",kbdhl_curr.hilighted->btbmp.btnhlbmp,hx,hy,x1,y1,(x2-x1),(y2-y1));
	 masked_blit(kbdhl_curr.hilighted->btbmp.btnhlbmp,screen,hx,hy,x1,y1,(x2-x1),(y2-y1));
//	 printf("whl b4\n");
      } else {
//	 printf("whl c\n");
	 c=kbdhl_curr.color;
	 rect(screen,x1,y1,x2,y2,c);
	 r2=getr(c)*0.75;g2=getg(c)*0.75;b2=getb(c)*0.75;
	 c2=makecol(r2,g2,b2);
	 rect(screen,x1-1,y1-1,x2+1,y2+1,c2);
	 rect(screen,x1+1,y1+1,x2-1,y2-1,c2);
      }
   }
}

Widget* is_hlwdg(int x, int y) {
   Widget* w;
   Widget* rv;
   int xg=0;
   rv=NULL;
   w=wdg_getlevel(W_FIRST);
   while(w) {
      if(x>w->x1 && x<w->x2 && y>w->y1 && y<w->y2 && w->hilight=='T') {
	 rv=w;
	 break;
      }
//      printf("check wid %d\n",xg++);
      w=w->next;
   }
   return(rv);
}

int hl_next_dn(Widget *tw, int x, int y, int m) {
   // Search down for another hilightable widget
   Widget* w;
   int sx,sy,cx,cy,sw;

   wdg_unhilight();
   if(kbdhl_curr.hilighted) {
      sy=(kbdhl_curr.hilighted->y2)+1;
      sx=(kbdhl_curr.hilighted->x1+kbdhl_curr.hilighted->x2)/2;
      sw=10;
   } else {
      sy=0;
      sx=VIRTUAL_W/2;
      sw=VIRTUAL_W/2-25;
   }
   for(cy=sy;cy<VIRTUAL_H;cy=cy+4) {
      for(cx=(sx-sw);cx<(sx+sw);cx=cx+6) {
         w=is_hlwdg(cx,cy);
//	 putpixel(screen,cx,cy,makecol(0,0,0));
	 if(w) break;
      }
      //      for(cy=sy;cy<(sy+sw);cy=cy+6) {
      //	 putpixel(screen,cx,cy,makecol(0,0,0));
      //         w=is_hlwdg(cx,cy);
      //	 if(w) break;
      //      }
      if(w) break;
   }
//   SDL_Flip(screen);
   if(w) {
      wdg_setlevel(W_HILIGHTED,w);
      wdg_hilight(1);
   } else 
      wdg_hilight(0);
}

   
int hl_next_up(Widget *tw, int x, int y, int m) {
   // Search up for another hilightable widget
   Widget* w;
   int sx,sy,cx,cy,sw;

   wdg_unhilight();
   if(kbdhl_curr.hilighted) {
      sy=(kbdhl_curr.hilighted->y1)-1;
      sx=(kbdhl_curr.hilighted->x1+kbdhl_curr.hilighted->x2)/2;
      sw=10;
   } else {
      sy=VIRTUAL_H-1;
      sx=VIRTUAL_W/2;
      sw=VIRTUAL_W/2-25;
   }
   for(cy=sy;cy>0;cy=cy-4) {
      for(cx=(sx-sw);cx<(sx+sw);cx=cx+6) {
//	 printf("cx:%d cy:%d\n",cx,cy);
         w=is_hlwdg(cx,cy);
//	 putpixel(screen,cx,cy,makecol(0,0,0));
	 if(w) break;
      }
      //      for(cy=sy;cy<(sy+sw);cy=cy+6) {
      //	 putpixel(screen,cx,cy,makecol(0,0,0));
      //         w=is_hlwdg(cx,cy);
      //	 if(w) break;
      //      }
      if(w) break;
   }   
         
//   SDL_Flip(screen);
   if(w) {
      wdg_setlevel(W_HILIGHTED,w);
      wdg_hilight(1);
   } else 
      wdg_hilight(0);
}

int hl_next_rt(Widget *tw, int x, int y, int m) {
   // Search to the right for another hilightable widget
   Widget* w;
   int sx,sy,cx,cy,sw;
   wdg_unhilight();
//   wdg_setlevel(W_HILIGHTED,NULL);
   if(kbdhl_curr.hilighted) {
      sx=(kbdhl_curr.hilighted->x2)+1;
      sy=(kbdhl_curr.hilighted->y1+kbdhl_curr.hilighted->y2)/2;
      sw=10;
   } else {
      sy=VIRTUAL_H/2;
      sx=0;
      sw=VIRTUAL_H/2-5;
   }
   for(cx=sx;cx<VIRTUAL_W;cx=cx+4) {
      for(cy=sy;cy>(sy-sw);cy=cy-6) {
         w=is_hlwdg(cx,cy);
//	 putpixel(screen,cx,cy,makecol(0,0,0));
	 if(w) break;
      }
      if(w) break;
      for(cy=sy;cy<(sy+sw);cy=cy+6) {
//	 putpixel(screen,cx,cy,makecol(0,0,0));
         w=is_hlwdg(cx,cy);
	 if(w) break;
      }
      if(w) break;
   }
         
//   SDL_Flip(screen);
   if(w) {
      wdg_setlevel(W_HILIGHTED,w);
      wdg_hilight(1);
   } else 
      wdg_hilight(0);
}

int hl_next_lf(Widget *tw, int x, int y, int m) {
   // Search to the left for another hilightable widget
   Widget* w;
   int sx,sy,cx,cy,sw;

//   printf("IN LF\n");
   wdg_unhilight();
//   printf("IN LF2\n");
   if(kbdhl_curr.hilighted) {
      sx=(kbdhl_curr.hilighted->x1)-1;
      sy=(kbdhl_curr.hilighted->y1+kbdhl_curr.hilighted->y2)/2;
      sw=10;
   } else {
      sy=VIRTUAL_H/2;
      sx=VIRTUAL_W;
      sw=VIRTUAL_H/2-5;
   }
//   printf("IN LF3: sx:%d  sy:%d  sw:%d\n",sx,sy,sw);
   for(cx=sx;cx>0;cx=cx-4) {
      for(cy=sy;cy>(sy-sw);cy=cy-6) {
//	 printf("checking is_hl (1)\n");
         w=is_hlwdg(cx,cy);
//	 printf("Dont hl (1)\n");
//	 printf("cx:%d cy:%d\n",cx,cy);
//	 putpixel(screen,cx,cy,makecol(0,0,0));
	 if(w) break;
      }
      if(w) break;
      for(cy=sy;cy<(sy+sw);cy=cy+6) {
//	 putpixel(screen,cx,cy,makecol(0,0,0));
//	 printf("checking is_hl (2)\n");
         w=is_hlwdg(cx,cy);
//	 printf("Done hl (2)\n");
	 if(w) break;
      }
      if(w) break;
   }
//   printf("broken out\n");      
   
//   SDL_Flip(screen);
   if(w) { 
//      printf("doing setlevel\n");
      wdg_setlevel(W_HILIGHTED,w);
      //      printf("doing hilight\n");
      wdg_hilight(1);
      // printf("dont both\n");
   } else 
      wdg_hilight(0);
}

int hl_next(Widget *tw, int x, int y, int m) {
   Widget* w;
   int wn=0;
   // 'Tab' key callback
//   printf("TAB NEXT WIDGET!!!!\n");
   wdg_unhilight();
   // Find next hilightable widget
   w=NULL;
   if(key_shifts & KB_SHIFT_FLAG || key_shifts & KB_CTRL_FLAG) {
      // Reverse search
      if(kbdhl_curr.hilighted)
	w=kbdhl_curr.hilighted->prev;
      
      if(w==NULL)
	w=wdg_getlevel(W_LAST);
      
      while(w!=kbdhl_curr.hilighted) {
	 if(w->hilight=='T') {
	    wdg_setlevel(W_HILIGHTED,w);
	    break;
	 }
	 if(w==wdg_getlevel(W_FIRST))
	   w=wdg_getlevel(W_LAST);
	 else
	   w=w->prev;
      }
      // End Reverse search
   } else {
      // Forward search
      if(kbdhl_curr.hilighted)
	w=kbdhl_curr.hilighted->next;
      
      if(w==NULL)
	w=wdg_getlevel(W_FIRST);
      
      while(w!=kbdhl_curr.hilighted) {
	 if(w->hilight=='T') {
	    wdg_setlevel(W_HILIGHTED,w);
	    break;
	 }
	 if(w==wdg_getlevel(W_LAST))
	   w=wdg_getlevel(W_FIRST);
	 else
	   w=w->next;
      }
      // End Forward search
   }
   wdg_hilight(1);
}

int hl_select(Widget *tw, int x, int y, int m) {
   // Called when <enter> is pressed
   int fx,fy;
//   Widget* w;
   kbdhl_curr.hilighted=wdg_getlevel(W_HILIGHTED);
//   printf("Im called\n");
   if(kbdhl_curr.hilighted) {
      fx=kbdhl_curr.hilighted->x1 +1;  // Buttons don't work unless the mouse is in them
      fy=kbdhl_curr.hilighted->y1 +1;  // so we fake it
      rem_pointer(x,y);
//      printf("Im working\n");
      if(kbdhl_curr.hilighted->press)
	(*(kbdhl_curr.hilighted->press))(kbdhl_curr.hilighted,fx,fy,1);
      if(kbdhl_curr.hilighted->depress)
	(*(kbdhl_curr.hilighted->depress))(kbdhl_curr.hilighted,fx,fy);
      draw_pointer(x,y);
   }
}

wdg_bind_key(Widget *w, int ksym, int kasc, int hl) {
   w->ksym=ksym;
   w->kascii=kasc;
   if(hl==1)
     w->hilight='T';
   else
     w->hilight='F';
}

void wdg_install_hilight(int color,int tab,int select, int up, int down, int left, int right) {
   /* hlall -> The hilight flag will be set on future widgets
    *        Others are keysyms that will be used, set to -1 
    *        if you don't want that feature */
   Widget *w;
	
   kbdhl_curr.color=color;
   if(tab>-1) {	
      w=add_invisible_button(-1,-1,-1,-1,&hl_next);
      if(tab==0)
	wdg_bind_key(w,KEY_TAB,-1,0);
      else
	wdg_bind_key(w,tab,-1,0);
   }
   if(select>-1) {	
      w=add_invisible_button(-1,-5,-1,-5,&hl_select);
      if(select==0)
	wdg_bind_key(w,KEY_ENTER,-1,0);
      else
	wdg_bind_key(w,select,-1,0);
   }
   if(up>-1) {
      w=add_invisible_button(-1,-10,-1,-10,&hl_next_up);
      if(up==0)
	wdg_bind_key(w,KEY_UP,-1,0);
      else
	wdg_bind_key(w,up,-1,0);
   }
   if(down>-1) {
      w=add_invisible_button(-1,-15,-1,-15,&hl_next_dn);
      if(down==0)
	wdg_bind_key(w,KEY_DOWN,-1,0);
      else
	wdg_bind_key(w,down,-1,0);
   }
   if(left>-1) {
      w=add_invisible_button(-1,-20,-1,-20,&hl_next_lf);
      if(left==0)
	wdg_bind_key(w,KEY_LEFT,-1,0);
      else
	wdg_bind_key(w,left,-1,0);
   }
   if(right>-1) {
      w=add_invisible_button(-1,-25,-1,-25,&hl_next_rt);
      if(right==0)
	wdg_bind_key(w,KEY_RIGHT,-1,0);
      else
	wdg_bind_key(w,right,-1,0);
   }
}


