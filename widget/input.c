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

int cursor_px; // X position of cursor in focused widget

void destroy_input(Widget* w) {
//   printf("problem area?\n");
   if(w->under) free(w->under);
//   printf("problem area? 2\n");
   if(w->extra) free(w->extra);
//   printf("problem area? 3\n");
//   if(w->text) free(w->text);
   // TODO: The btbmp structure
}

int has_focus(Widget* b) {
   // Does this work properly?
   if(b==wdg_getlevel(W_FOCUS))
     return 1;
   else
     return 0;
}

void unfocus_input(Widget *b,int x, int y, int m) {
   int hlcol,fncol,al,wdt,i;
   char limittxt[190];
   BITMAP* useb;
   
   useb=screen;
   if(b->parent!=NULL) {
      if(b->parent->extra!=NULL)
	useb=b->parent->extra;
   }
   
   scare_once();
   if(b==wdg_getlevel(W_FOCUS)) // remove focus if it belongs to me
     wdg_setlevel(W_FOCUS,NULL);
   // Need condition to unfocus here?

   if(just_popped) return;
   restore_under(b);
   // Draw unfocused input here, leave options for themes/bmps
   hlcol=activestyle.input.bg;
   fncol=activestyle.input.tx;
   al=activestyle.input.al;
   style_rectfill(screen,b->x1,b->y1,b->x2,b->y2,hlcol,al);
//   rectfill(screen,b->x1,b->y1,b->x2,b->y2,makecol(128,0,0));
//   fnt_print_string(screen, b->x1+2, b->y1+2, "FUNK", fncol, -1 , -1);
   wdt=(b->x2-b->x1)/8;
   for(i=0;i<wdt;i++)
     limittxt[i]=b->text[i];
   limittxt[wdt]=0;
   fnt_print_string(screen, b->x1+2, b->y1+2, limittxt, fncol, -1 , -1);
   
   // End Draw
//printf("Unfocused widget with %s at %d,%d %d,%d\n",b->text,b->x1,b->y1,b->x2,b->y2);
   unscare_once();
}

focus_input(Widget *b, int x, int y, int m) {
   // Note ,to 
   int cx,hlcol,fncol,white,dkcol,al;
   Widget* fw;
   
   scare_once();
//   printf("Got Focus,  x is %d\n",x);
   fw=wdg_getlevel(W_FOCUS);
   if(fw) { // If something else has focus, unfocus it
      (*(fw->handler))(fw, -1,-1,-1);
//      printf(" had to unfocus %s\n",fw->text);
   }
   wdg_setlevel(W_FOCUS,b);
   restore_under(b);
   // Draw focused input here, leave options for themes/bmps
   // Adapted from Yaki
   hlcol=activestyle.input.bg;
   //         printf("Did it still work?  abg: %d %d %d\n",
   //getr(activestyle.input.bg),
   //getg(activestyle.input.bg),getb(activestyle.input.bg));
//   printf("What gives?  %d\n",getr(hlcol));
   fncol=activestyle.input.tx;
   white=activestyle.input.hl;
   dkcol=activestyle.input.sh;
   al=activestyle.input.al;
   
   style_rectfill(screen,b->x1,b->y1,b->x2,b->y2,hlcol,al);
   style_rect(screen,b->x1,b->y1,b->x2,b->y2,white,al);
   style_hline(screen,b->x1+1,b->y1,b->x2,dkcol,al);
   style_vline(screen,b->x1+1,b->y1,b->y2,dkcol,al);
   fnt_print_string(screen, b->x1+3, b->y1+3, b->text, fncol, -1 , -1);
   if(((x-b->x1)+2)/8 < strlen(b->text))
     cursor_px=((x-b->x1)+2)/8;
   else
     cursor_px=strlen(b->text);
   cx=(cursor_px*8)+b->x1+2;
//   rect(screen,cx,b->y1+2,cx,b->y2-2,fncol);
   style_vline(screen,cx,b->y1+2,b->y2-2,fncol,al);
   // End Draw
//printf("Focused widget with %s\n",b->text);
   unscare_once();
}

Widget* wdg_input_add (Widget *parent,int x1,int y1,int wc,char* text) {
   Widget *w=new_widget();
   int x2=x1+(wc*8)+4;
   int y2=y1+20;

   // Note: Probably should put unfocus on handler rather than depress
   //       Depress should do nothing basically.

   w->press=&focus_input;
   w->depress=NULL;  // If I don't set this then it points to unfocus?
//   w->depress=&unfocus_input;
   w->draggable=0; /* &down_button is only called once */

   w->x1=x1;
   w->x2=x2;
   w->y1=y1;
   w->y2=y2;
   w->parent=parent;
   // Note, a persistant problem might be that the widget gets destroyed
   // before the text data can be copied out.  Perhaps this should be a 
   // pointer to another string the user provides (like it was originally)
   // That may solve this problem, and make the malloc unnecessary.
   //   w->text=(char *)malloc(wc+2);
   // strcpy(w->text,text);
   // w->text[strlen(w->text)]=0;
   // Lets try this instead:
   w->text=text;
   w->handler=&unfocus_input;
   w->destroy=&destroy_input;
   w->under = w->extra=NULL;
   save_under(w);
   unfocus_input(w,-1,-1,-1);
   return w;
}

Widget* add_input (int x1,int y1,int wc,char* text) { // legacy
   Widget *w=wdg_input_add(NULL,x1,y1,wc,text);
   return w;
}

wdg_input_data(Widget* b, int *ksym, char *kasc) {
   int hlcol, fncol, cx,slen,i,al;
   char nc[3];
   slen=(((b->x2-b->x1)-4)/8);
   
//   printf("in wdg_input_data:  ksym %d  kasc '%c' (%d)\n",*ksym,*kasc,*kasc);
//   printf("cursor_px is %d\n",cursor_px);
//   hlcol=makecol(184,184,224);
//   fncol=makecol(64,64,64);
   hlcol=activestyle.input.bg;
   fncol=activestyle.input.tx;
   al=activestyle.input.al;
   
   if(*ksym==KEY_BACKSPACE) {
      if(cursor_px>0) {
	 cursor_px--;
	 for(i=cursor_px;i<strlen(b->text);i++)
	   b->text[i]=b->text[i+1];
      }
//      b->text[cursor_px]=0; // Needs to be fixed
      *ksym=0;*kasc=0;
   }
   if(*ksym==KEY_DEL) {
      if(cursor_px<strlen(b->text)) {
	 for(i=cursor_px;i<strlen(b->text);i++)
	   b->text[i]=b->text[i+1];
      }
      *ksym=0;*kasc=0;
   }
   
   if(*ksym==KEY_LEFT) {
      if(cursor_px>0)
	cursor_px--;
      *ksym=0;*kasc=0;
   }

   if(*ksym==KEY_RIGHT) {
      if(cursor_px<strlen(b->text))
	cursor_px++;
      *ksym=0;*kasc=0;
   }
   if(*ksym==KEY_UP || *ksym==KEY_DOWN)  {
     *ksym=0;*kasc=0;  // We'd want this for an editor box
   }

//   printf("still in wdg_input_data:  ksym %d  kasc '%c'(%d)  slen %d\n",*ksym,*kasc,*kasc,slen);
//   printf("problem w/ text field? len is  %d content is %s\n",strlen(b->text),b->text);
   
   if(*kasc>31 && *kasc<127) {
//      printf("Ok, got in here\n");
      if(strlen(b->text)<slen) {
//	 printf("adding '%c'\n",*kasc);
	 for(i=strlen(b->text);i>=cursor_px;i--)
	   b->text[i+1]=b->text[i];
	 b->text[cursor_px]=*kasc;
// This works, but can only append to the end
//	 sprintf(nc,"%c",*kasc);
//	 strcat(b->text,nc);
	 cursor_px++;
      }
      *ksym=0;*kasc=0;
   }
   // Draw Text & Cursor

   style_rectfill(screen,b->x1+2,b->y1+2,b->x2-2,b->y2-2,hlcol,al);
   fnt_print_string(screen, b->x1+3, b->y1+3, b->text, fncol, -1 , -1);
   cx=(cursor_px*8)+b->x1+2;
   style_vline(screen,cx,b->y1+2,b->y2-2,fncol,al);
   //End Text and Cursor
   if(*ksym==KEY_ENTER) // unfocus on ENTER,  non editor only  
     (*(b->handler))(b, -1,-1,-1);
}
