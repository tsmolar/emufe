#include <stdio.h>
#ifdef USESDL
#include <SDL.h>
#include "sdl_allegro.h"
#else
#include <allegro.h>
#endif
#include "font.h"
#include "button.h"
//#include "graphics.h"
//#include "platform.h"
//#include "colordefs.h"

#define COLOR253 92,92,92
#define COLOR254 160,160,160
#define COLOR255 240,16,16
#define in_box(x1,y1,x2,y2,mx,my) ((mx)>(x1) && (mx)<(x2) && (my)>(y1) && (my)<(y2))

int mcolor;

//extern int just_popped; /* from widget.c */

/*
 * 
 * Following are functions, that implement buttons
 * 
 */

//BITMAP* getwdgbmp(Widget *b) {
//   BITMAP* useb;
//
//   useb=screen;
//   if(b->parent!=NULL) {
//      if(b->parent->extra!=NULL) {
//	 useb=b->parent->extra;
//	 printf("got bmp from indx:%d  %s\n", b->parent->indx,b->parent->id);
//      }
//   }
//   return useb;
//}

up_bmp_button(Widget *b,int x,int y) /* called by mouse_depress */
{
   BITMAP* useb;
   useb=getwdgbmp(b);
   if(just_popped==0)     
     scare_once();
//        rem_pointer(x,y);
//   printf("ubb handler cb check %d\n", b->handler);
        if(in_box(b->x1,b->y1,b->x2,b->y2,x,y) && b->handler) 
            (*(b->handler))(b,x-b->x1,y-b->y1,-1);
   
//   printf("ubb handler cb success\n");
// I get it now, if just popped, then this button is destroyed, and executing
// anything below might cause a segfault.
   	if(just_popped) return;

//   printf("didn't just pop\n");
	restore_under(b);
//   printf("did restore under\n");
//        masked_blit(b->btbmp.btnupbmp,screen,b->btbmp.up_x1,b->btbmp.up_y1,b->x1,b->y1,b->btbmp.up_x2,b->btbmp.up_y2);
        masked_blit(b->btbmp.btnupbmp,useb,b->btbmp.up_x1,b->btbmp.up_y1,b->x1,b->y1,b->btbmp.up_x2,b->btbmp.up_y2);
//   printf("did blit\n");
//#ifdef USESDL
//        SDL_UpdateRect(screen,b->x1,b->y1,b->x2-1,b->y2-1);
//        SDL_Flip(screen);
////   printf("Update Rect! %d,%d   %d,%d\n",b->x1,b->y1,b->x2-1,b->y2-1);
//#endif
   unscare_once();
   // What's this for?
   if(just_popped) return;
//        draw_pointer(x,y);	
}

down_bmp_button(Widget *b,int x,int y,int m) /* called by mouse_press */
{
   BITMAP* useb;
#ifdef SDL2
   SDL_Rect brect;
#endif
   
   useb=getwdgbmp(b);
   scare_once();
   //        rem_pointer(x,y);
   //printf("x1\n");
   restore_under(b);
   masked_blit(b->btbmp.btndnbmp,useb,b->btbmp.dn_x1,b->btbmp.dn_y1,b->x1,b->y1,b->btbmp.dn_x2,b->btbmp.dn_y2);
//printf("x2\n");
#ifdef USESDL
# ifdef SDL1
   SDL_UpdateRect(screen,b->x1,b->y1,b->x2-1,b->y2-1);
# endif
# ifdef SDL2
   brect.x = b->x1;
   brect.y = b->y1;
   brect.w = b->x2-1;
   brect.h = b->y2-1;
   SDL_RenderCopy(sdlRenderer,screen,&brect,&brect);
   SDL_RenderPresent(sdlRenderer);
# endif
#endif
   // experimental
   if(b->draggable)
     if(in_box(b->x1,b->y1,b->x2,b->y2,x,y) && b->handler) 
       (*(b->handler))(b,x-b->x1,y-b->y1,-1);
   //  end experimental
   unscare_once();
//        draw_pointer(x,y);	
}

up_button(Widget *b,int x,int y) /* called by mouse_depress */
{
   BITMAP* useb;
#ifdef SDL2
   SDL_Rect brect;
#endif

   
//   printf("BTN: UP\n");
   useb=getwdgbmp(b);
   
   if(just_popped==0)
         scare_once();
   //        rem_pointer(x,y);
   if(in_box(b->x1,b->y1,b->x2,b->y2,x,y) && b->handler) 
     (*(b->handler))(b,x-b->x1,y-b->y1,-1);
   
//   printf("handler cb success\n");
// Should this be here?   
   	if(just_popped) return;
	
	restore_under(b);
        mcolor=makecol(COLOR254);
	rectfill(useb,b->x1,b->y1,b->x2-1,b->y2-1,mcolor);
        mcolor=makecol(COLOR253);
	line(useb,b->x2,b->y1+1,b->x2,b->y2,mcolor);
	line(useb,b->x1+1,b->y2,b->x2,b->y2,mcolor);
//        set_font_fcolor(255,255,255);
	fnt_print_string(useb,b->x1+3,b->y1+1,b->text,makecol(255,255,255),-1,-1);
#ifdef USESDL
   if(useb==screen) {
# ifdef SDL1
     SDL_UpdateRect(screen,b->x1,b->y1,b->x2-1,b->y2-1);
# endif
# ifdef SDL2
      brect.x = b->x1;
      brect.y = b->y1;
      brect.w = b->x2-1;
      brect.h = b->y2-1;
      SDL_RenderCopy(sdlRenderer,screen,&brect,&brect);
      SDL_RenderPresent(sdlRenderer);
# endif
      
   }
#endif
        unscare_once();
   // What's this for?
	if(just_popped) return;
//        draw_pointer(x,y);	
}

down_button(Widget *b,int x,int y,int m) /* called by mouse_press */
{
   BITMAP* useb;
#ifdef SDL2
   SDL_Rect brect;
#endif
 //  printf("BTN: DOWN\n");
   useb=getwdgbmp(b);

//   printf("x1\n");
        scare_once();
//        rem_pointer(x,y);
	restore_under(b);
//   printf("x2:\n");
        mcolor=makecol(COLOR254);
        mcolor=makecol(160,160,16);
//   printf("x2.5:rectfill %d,%d,%d,%d,%d\n",b->x1+1,b->y1+1,b->x2,b->y2,mcolor);
	rectfill(useb,b->x1+1,b->y1+1,b->x2,b->y2,mcolor);
//   printf("x3\n");
//        set_font_fcolor(225,225,225);
	fnt_print_string(useb,b->x1+4,b->y1+2,b->text,makecol(225,225,225),-1,-1);
//   printf("x4\n");
#ifdef USESDL
   if(useb==screen) {
# ifdef SDL1
     SDL_UpdateRect(screen,b->x1,b->y1,b->x2-1,b->y2-1);
# endif
# ifdef SDL2
      brect.x = b->x1;
      brect.y = b->y1;
      brect.w = b->x2-1;
      brect.h = b->y2-1;
      SDL_RenderCopy(sdlRenderer,screen,&brect,&brect);
      SDL_RenderPresent(sdlRenderer);
# endif
   }
#endif
        unscare_once();
//        draw_pointer(x,y);	
}

void destroy_button(Widget* w){
//   printf("In destroy button\n");
   if(w->under) free(w->under);
//   printf("d2\n");
   if(w->extra) free(w->extra);
//   printf("d3\n");
   if(w->text) free(w->text);
//   printf("d4\n");
   // TODO: The btbmp structure
}

void change_handler_invisible(Widget* w, int(*handler)(Widget*,int,int,int)) {
   /* Change the handler for an invisible button */
   w->press=handler;
}

void change_handler(Widget* w, int(*handler)(Widget*,int,int,int)) {
   /* Change the handler, same as activate button*/
   w->press=&down_button;
   w->depress=&up_button;
   w->handler=handler;
}

void deactivate_button(Widget* w) {
   /* Change the handler for an invisible button */
/*   down_button(w,0,0,0); */
   w->press=NULL;
   w->depress=NULL;
   w->handler=NULL;
}

/* New create button */
Widget* wdg_button_add(Widget *parent,int x1, int y1, int x2, int y2, 
		       int(*handler)(Widget*,int,int,int)) {
   Widget *w=new_widget();
   w->draggable=0; /* &down_button is only called once */
   
   strcpy(w->id,"BUTN");
   w->x1=x1; w->x2=x2;
   w->y1=y1; w->y2=y2; 
   w->handler=handler;
   w->destroy=&destroy_button;
//   w->destroy=&wdg_destroy;
   w->extra=NULL;
   w->parent=parent;
   
   save_under(w);
//   up_button(w,-1,-1);
   return w;
}

Widget* wdg_button_txt_add(Widget *parent,int x1, int y1, int x2, int y2, 
		       char* text,int(*handler)(Widget*,int,int,int)) {
   Widget *w=wdg_button_add(parent,x1,y1,x2,y2,handler); 
   
   w->press=&down_button;
   w->depress=&up_button;
   w->text=(char *)malloc(strlen(text)+4);  // Doesn't Crash on Linux 
   strcpy(w->text,text);
   up_button(w,-1,-1);
   return w;
}

Widget* wdg_button_bmp_add(Widget *parent,int x1, int y1, int x2, int y2, 
		       bmpbtn_t bbmp,int(*handler)(Widget*,int,int,int)) {
   Widget *w=wdg_button_add(parent,x1,y1,x2,y2,handler); 
   
   w->press=&down_bmp_button;
   w->depress=&up_bmp_button;
   w->text=w->extra=NULL;
   // do this right (because we don't pass dynamically)
   w->btbmp.btnupbmp = bbmp.btnupbmp; 
   w->btbmp.btndnbmp = bbmp.btndnbmp; 
   w->btbmp.btnhlbmp = bbmp.btnhlbmp;

   w->btbmp.up_x1=bbmp.up_x1; w->btbmp.up_x2=bbmp.up_x2;
   w->btbmp.up_y1=bbmp.up_y1; w->btbmp.up_y2=bbmp.up_y2;
   w->btbmp.dn_x1=bbmp.dn_x1; w->btbmp.dn_x2=bbmp.dn_x2;
   w->btbmp.dn_y1=bbmp.dn_y1; w->btbmp.dn_y2=bbmp.dn_y2;
   w->btbmp.hl_x1=bbmp.hl_x1; w->btbmp.hl_x2=bbmp.hl_x2;
   w->btbmp.hl_y1=bbmp.hl_y1; w->btbmp.hl_y2=bbmp.hl_y2;

   up_bmp_button(w,-1,-1);
   return w;
}

/* Creates a new button */
Widget* add_button
  (int x1,int y1,int x2,int y2,char* text,int(*handler)(Widget*,int,int,int))
{
	Widget *w=new_widget();
	
	w->press=&down_button;
	w->depress=&up_button;
	w->draggable=0; /* &down_button is only called once */
   
	w->x1=x1;
	w->x2=x2;
	w->y1=y1;
	w->y2=y2;
//	w->text=text;  // works on most platforms, but is probably not correct
                       // Crashes on Linux                   
        w->text=(char *)malloc(strlen(text)+4);  // Doesn't Crash on Linux 
        strcpy(w->text,text);                    //
	w->handler=handler;
        strcpy(w->id,"BUTL");
	w->destroy=&destroy_button;
  	w->btbmp.btnupbmp = w->btbmp.btndnbmp = w->btbmp.btnhlbmp =NULL;
        w->extra=NULL;
	
	save_under(w);
	up_button(w,-1,-1);
	return w;
}

Widget* add_bmp_button
  (int x1,int y1,int x2,int y2,bmpbtn_t bbmp,int(*handler)(Widget*,int,int,int))
{
	Widget *w=new_widget();
	
	w->press=&down_bmp_button;
	w->depress=&up_bmp_button;
	w->draggable=0; /* &down_button is only called once */
	
	w->x1=x1;
	w->x2=x2;
	w->y1=y1;
	w->y2=y2;
   
	w->handler=handler;
//   printf("adding handler: %d\n",w->handler);
	w->destroy=&destroy_button;
  	w->btbmp.btnupbmp = w->btbmp.btndnbmp = w->btbmp.btnhlbmp =NULL;
	w->btbmp=bbmp;
        // Make sure this is null to prevent segfaults
        w->text=w->extra=NULL;
     
	save_under(w);
//   printf("pop the button!\n");
	up_bmp_button(w,-1,-1);
//   printf("popped the button!\n");
	return w;
}

/*
 * 
 * Following are functions, that implement arrow buttons
 * 
 */

up_up_arrow_button(Widget *b,int x,int y) /* called by mouse_depress */
{
	int i;
	restore_under(b);
        mcolor=makecol(COLOR254);
	for(i=0;i<7;i++) {
	   line(screen,b->x1+i,b->y1+6-i,b->x1+12-i,b->y1+6-i,mcolor);
	}
        mcolor=makecol(COLOR254);
	line(screen,b->x1+1,b->y1+7,b->x1+13,b->y1+7,mcolor);
	if(in_box(b->x1,b->y1,b->x2,b->y2,x,y) && b->handler)
		(*(b->handler))(b,x-b->x1,y-b->y1,-1);
}

down_up_arrow_button(Widget *b,int x,int y,int m) /* called by mouse_press */
{
	int i;
	restore_under(b);
        mcolor=makecol(COLOR254);
	for(i=0;i<7;i++) {
	  line(screen,b->x1+i+1,b->y1+7-i,b->x1+13-i,b->y1+7-i,mcolor);
	}
}

up_down_arrow_button(Widget *b,int x,int y) /* called by mouse_depress */
{
	int i;
	restore_under(b);
	for(i=0;i<7;i++) {
          mcolor=makecol(COLOR254);
	  line(screen,b->x1+i,b->y1+i,b->x1+12-i,b->y1+i,mcolor);
          mcolor=makecol(COLOR254);
	  line(screen,b->x1+i+1,b->y1+i+1,b->x1+13-i,b->y1+i+1,mcolor);
	}
	if(in_box(b->x1,b->y1,b->x2,b->y2,x,y) && b->handler)
		(*(b->handler))(b,x-b->x1,y-b->y1,-1);
}

down_down_arrow_button(Widget *b,int x,int y,int m) /* called by mouse_press */
{
	int i;
	restore_under(b);
        mcolor=makecol(COLOR254);
	for(i=0;i<7;i++) {
	  line(screen,b->x1+i+1,b->y1+i+1,b->x1+13-i,b->y1+i+1,mcolor);
	}
}


up_left_arrow_button(Widget *b,int x,int y) /* called by mouse_depress */
{
	int i;
	
	restore_under(b);
	
        mcolor=makecol(COLOR253);
        for(i=0;i<7;i++) {
		line(screen,b->x1+7-i,b->y1+i+1,b->x1+7,b->y1+i+1,mcolor);
		line(screen,b->x1+7-i,b->y1+13-i,b->x1+7,b->y1+13-i,mcolor);
	}
        mcolor=makecol(COLOR254);
	for(i=0;i<7;i++) {
		line(screen,b->x1+6-i,b->y1+i,b->x1+6,b->y1+i,mcolor);
		line(screen,b->x1+6-i,b->y1+12-i,b->x1+6,b->y1+12-i,mcolor);
	}
	if(in_box(b->x1,b->y1,b->x2,b->y2,x,y) && b->handler)
		(*(b->handler))(b,x-b->x1,y-b->y1,-1);
	
}

down_left_arrow_button(Widget *b,int x,int y,int m) /* called by mouse_press */
{
	int i;
	
	restore_under(b);
   
        mcolor=makecol(COLOR254);
	for(i=0;i<7;i++) {
	   line(screen,b->x1+7-i,b->y1+i+1,b->x1+7,b->y1+i+1,mcolor);
	   line(screen,b->x1+7-i,b->y1+13-i,b->x1+7,b->y1+13-i,mcolor);
	}
	

}

up_right_arrow_button(Widget *b,int x,int y) /* called by mouse_depress */
{
	int i;
	
	restore_under(b);
	
        mcolor=makecol(COLOR253);
	for(i=0;i<7;i++) {
		line(screen,b->x1+1,b->y1+i+1,b->x1+i+1,b->y1+i+1,mcolor);
		line(screen,b->x1+1,b->y1+13-i,b->x1+i+1,b->y1+13-i,mcolor);
	}
        mcolor=makecol(COLOR254);
	for(i=0;i<7;i++) {
		line(screen,b->x1,b->y1+i,b->x1+i,b->y1+i,mcolor);
		line(screen,b->x1,b->y1+12-i,b->x1+i,b->y1+12-i,mcolor);
	}
	if(in_box(b->x1,b->y1,b->x2,b->y2,x,y) && b->handler)
		(*(b->handler))(b,x-b->x1,y-b->y1,-1);
	
}

down_right_arrow_button(Widget *b,int x,int y,int m) /* called by mouse_press */
{
	int i;
	
	restore_under(b);
	
        mcolor=makecol(COLOR254);
	for(i=0;i<7;i++) {
		line(screen,b->x1+1,b->y1+i+1,b->x1+i+1,b->y1+i+1,mcolor);
		line(screen,b->x1+1,b->y1+13-i,b->x1+i+1,b->y1+13-i,mcolor);
	}
	

}

Widget* wdg_button_arrow_add(Widget *parent,int x,int y,int dir,
			     int(*handler)(Widget*,int,int,int)) {
   Widget *w;
   
   if (dir == UP) {
      w=wdg_button_add(parent,x-6,y,x+7,y+7,handler); 
      w->press=&down_up_arrow_button;
      w->depress=&up_up_arrow_button;
      w->text=(char *)malloc(9);  // Doesn't Crash on Linux 
      strcpy(w->text,"UP");
   } else if (dir == DOWN) {
      w=wdg_button_add(parent,x-6,y-7,x+7,y,handler); 
      w->press=&down_down_arrow_button;
      w->depress=&up_down_arrow_button;
      w->text=(char *)malloc(9);  // Doesn't Crash on Linux 
      strcpy(w->text,"DOWN");
   } else if (dir == LEFT) {
      w=wdg_button_add(parent,x,y-6,x+7,y+7,handler); 
      w->press=&down_left_arrow_button;
      w->depress=&up_left_arrow_button;
      w->text=(char *)malloc(9);  // Doesn't Crash on Linux 
      strcpy(w->text,"LEFT");
   } else if (dir == RIGHT) {
      w=wdg_button_add(parent,x-6,y-6,x+1,y+7,handler); 
      w->press=&down_right_arrow_button;
      w->depress=&up_right_arrow_button;
      w->text=(char *)malloc(9);  // Doesn't Crash on Linux 
      strcpy(w->text,"RIGHT");
   }
   (*(w->depress))(w,-1,-1);
   return w;
} // wdg_button_add_arrow(...)

/* Creates a new button */
Widget* add_arrow_button
  (int x,int y,int dir,int(*handler)(Widget*,int,int,int))
{
   Widget *w=wdg_button_arrow_add(NULL,x,y,dir,handler);
}
   
Widget* add_arrow_button0
  (int x,int y,int dir,int(*handler)(Widget*,int,int,int))
{
        Widget *w=new_widget();
        w->text=(char *)malloc(9);  // Doesn't Crash on Linux 

	if (dir == UP) {
		w->press=&down_up_arrow_button;
		w->depress=&up_up_arrow_button;
		w->x1=x-6;
		w->x2=x+7;
		w->y1=y;
		w->y2=y+7;
	        strcpy(w->text,"UP");
//		w->text="UP";
	} else if (dir == DOWN) {
		w->press=&down_down_arrow_button;
		w->depress=&up_down_arrow_button;
		w->x1=x-6;
		w->x2=x+7;
		w->y1=y-7;
		w->y2=y;
	        strcpy(w->text,"DOWN");
//	        w->text="DOWN";
	} else if (dir == LEFT) {
		w->press=&down_left_arrow_button;
		w->depress=&up_left_arrow_button;
		w->x1=x;
		w->x2=x+7;
		w->y1=y-6;
		w->y2=y+7;
	        strcpy(w->text,"LEFT");
//	        w->text="RIGHT";
	} else if (dir == RIGHT) {
		w->press=&down_right_arrow_button;
		w->depress=&up_right_arrow_button;
		w->x1=x-6;
		w->x2=x+1;
		w->y1=y-6;
		w->y2=y+7;
	        strcpy(w->text,"RIGHT");
//	        w->text="RIGHT";
	}
	
	w->draggable=0; /* &down_button is only called once */
	
	w->handler=handler;
	w->destroy=&destroy_button;
        w->extra=NULL;
	
	save_under(w);
	(*(w->depress))(w,-1,-1);
	return w;
}

/*
 * 
 * Following are functions, that implement invisible buttons
 * (to implement the icon editor and icon chooser, etc...)
 *
 */


Widget* add_invisible_button
   (int x1,int y1,int x2,int y2,int(*handler)(Widget*,int,int,int))
{
	Widget *w;
	w=new_widget();
	
	w->press=handler;
	w->depress=NULL; /* no action on mouse-up */
	w->draggable=0; /* initially, the handler is only called once
			   on mouse-press */
	
	w->x1=x1;
	w->x2=x2;
	w->y1=y1;
	w->y2=y2;
  	w->btbmp.btnupbmp = w->btbmp.btndnbmp = w->btbmp.btnhlbmp =NULL;
        w->extra=NULL;
	w->handler=NULL; /* the handler is called directly on mouse-press,
			    so it's assigned to w->press instead */
	
	
	return w;
}

up_checkbox(Widget *b,int x,int y) /* called by mouse_depress */
{
   BITMAP* useb;
   useb=getwdgbmp(b);
     // until I can figure out how this style thing is supposed to work
   int i;
   
   restore_under(b);
   rectfill(useb,b->x1,b->y1,b->x2,b->y2,makecol(210,210,210));
   if(strcmp(b->text,"1")==0) {
      // draw X
//      rectfill(useb,b->x1,b->y1,b->x2,b->y2,makecol(25,25,25));
      line(useb,b->x1+1,b->y2-1,b->x2-1,b->y1+1,makecol(0,0,0));
      line(useb,b->x1+2,b->y2-1,b->x2-1,b->y1+2,makecol(80,80,80));
      line(useb,b->x1+1,b->y2-2,b->x2-2,b->y1+1,makecol(80,80,80));
      line(useb,b->x1+2,b->y1+2,b->x2-1,b->y2-1,makecol(0,0,0));
      line(useb,b->x1+3,b->y1+2,b->x2-1,b->y2-2,makecol(80,80,80));
      line(useb,b->x1+2,b->y1+3,b->x2-2,b->y2-1,makecol(80,80,80));
   }
   vline(useb,b->x1,b->y1,b->y2,makecol(128,128,128));
   hline(useb,b->x1,b->y1,b->x2,makecol(128,128,128));
   vline(useb,b->x2,b->y1,b->y2,makecol(255,255,255));
   hline(useb,b->x1,b->y2,b->x2,makecol(255,255,255));
   
   if(in_box(b->x1,b->y1,b->x2,b->y2,x,y) && b->handler)
     (*(b->handler))(b,x-b->x1,y-b->y1,-1);
}

down_checkbox(Widget *b,int x,int y,int m) /* called by mouse_press */
{  
   if(strcmp(b->text,"1")!=0)
     strcpy(b->text,"1");
   else
     strcpy(b->text,"0");
}

Widget* wdg_checkbox_add
  (Widget *parent,int x,int y,int checked,int(*handler)(Widget*,int,int,int)) 
{
   Widget *w=new_widget();
	
	w->press=&down_checkbox;
	w->depress=&up_checkbox;
	w->draggable=0; /* &down_button is only called once */
	
	w->x1=x-2;
	w->x2=x+8;
	w->y1=y-2;
	w->y2=y+8;
//	w->text=text;  // works on most platforms, but is probably not correct
                       // Crashes on Linux                   
        w->text=(char *)malloc(6);  // Doesn't Crash on Linux 
        sprintf(w->text,"%d",checked);
	w->handler=handler;
	w->destroy=&destroy_button;
  	w->btbmp.btnupbmp = w->btbmp.btndnbmp = w->btbmp.btnhlbmp =NULL;
        w->extra=NULL;
        w->parent=parent;
	
	save_under(w);
	up_checkbox(w,-1,-1);
	return w;
}

Widget* add_checkbox    // legacy
  (int x,int y,int checked,int(*handler)(Widget*,int,int,int)) {
     Widget *w=wdg_checkbox_add(NULL,x,y,checked,handler);
     return w;
  }

