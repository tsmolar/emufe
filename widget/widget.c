#include <stdio.h>
#ifdef GFXLIB_SDL
#include <SDL.h>
#endif
#ifdef GFXLIB_ALLEGRO
#ifdef USESDL
#include <SDL.h>
#include "sdl_allegro.h"
#else
#include <allegro.h>
#endif /* ifdef USESDL */
#endif /* if GFX_LIB==ALLEGRO */
#include <sys/time.h>
#include <sys/types.h>
#include "widget.h"

#define WIDGETVER "2.1.9"

#define in_box(x1,y1,x2,y2,mx,my) ((mx)>(x1) && (mx)<(x2) && (my)>(y1) && (my)<(y2))

// Shouldn't this be globalized?
struct wid_level {
	Widget* last;		/* the last created widget in this level */
	Widget* first;		/* The start of the list of widgets */
	Widget* pressed;	/* The widget, that currently is held down */
	Widget* focus;		/* The widget, that is receiving keyboard
				   input */
        Widget* hilighted;      /* The widget that is hilighted */
	void*	data;		/* Some data, which could be used to restore
				   the appearence of the screen, when returning
				   to this level. */
	int x1,x2,y1,y2;
};


struct wid_level level[MAX_LEVELS];
style_t activestyle;

int mx,my,mk;
int is_scared=0;
int curr_level=-1;
int just_popped=0;
int wflag_clickprocessed=0;

BITMAP* drawbmp;

void widget_version_info() {
   printf("Widget library version %s\n",WIDGETVER);
}

// Support for interogatting the level[]
Widget* wdg_getlevel(int v) {
   if(curr_level<0) return(NULL);
   switch(v) {
    case W_LAST: 
      return(level[curr_level].last);
    case W_FIRST: 
      return(level[curr_level].first);
    case W_PRESSED: 
      return(level[curr_level].pressed);
    case W_FOCUS: 
      return(level[curr_level].focus);
    case W_HILIGHTED: 
      return(level[curr_level].hilighted);
   }
}
   
void wdg_setlevel(int v,Widget* w) {
   switch(v) {
    case W_LAST: 
      level[curr_level].last=w;
      break;
    case W_FIRST: 
      level[curr_level].first=w;
      break;
    case W_PRESSED: 
      level[curr_level].pressed=w;
      break;
    case W_FOCUS: 
      level[curr_level].focus=w;
      break;
    case W_HILIGHTED: 
//      printf("setting hilighted %d.  x1=%d  y1=%d\n hx:",w,w->x1,w->y1,w->btbmp.hl_x1);
      level[curr_level].hilighted=w;
      break;
   }
}
// End level[] Support

//int hl_next(Widget *w, int x, int y, int m) {
//   printf("TAB NEXT WIDGET!!!!\n");
//}

void scare_once() {
   // For allegro, scares that come from different parts of the program 
   // Can make unscaring difficult, so we will only scare the mouse once
   if(is_scared==0) {
      scare_mouse();
      is_scared=1;
   }
}

void unscare_once() {
   if(is_scared) {
      unscare_mouse();
      is_scared=0;
   }
}

void rem_pointer(int x, int y) {
//   printf("BEE: rem scare mouse:  BWAAAA!\n");
   scare_once();
//   printf("wu1\n");
   wdg_unhilight();
//   printf("wu2\n");
}

void draw_pointer(int x, int y) {
//   printf("BEE: restore mouse:\n");
   unscare_once();
   wdg_hilight(0);
}

void widget_setbmp(BITMAP *newbmp) {
   drawbmp=newbmp;
}

void widget_init() /* Initializes the widget library */
{
   printf("Initializing Widget library v. %s...\n",WIDGETVER);
   wdgst_default();
   drawbmp=screen;
   // only init once
   if(curr_level<0) {
      printf("about to push, curr level=%d\n",curr_level);
      push_level(0,0,0,0);
      printf("widget_init(): curr level: %d\n",curr_level);
   }
}

int widget_clear_level() {
   printf("widget_clear_level: about to pop \n");
   pop_level();
   printf("pop ok\n");
   push_level(0,0,0,0);
   printf("push ok\n");
}

void wdg_adjust(int id, int ax, int ay) {
/*  This is for when the screen gets resized, and buttons need to
 * be moved.   Can accept an ID for special processing */
   
      Widget *w1,*w2;
   
      w1=level[curr_level].first; /* Get first widget in this level */
      while(w1) {
	 /* Delete all widgets in this level */
	 w2=w1;
	 printf("WMOV: type: %s\n",w2->id);
	 if(strcmp(w2->id,"BUTL")==0 && (id==-1 || w2->id==id)) {
	    w2->x1+=ax;
	    w2->x2+=ax;
	    w2->y1+=ay;
	    w2->y2+=ay;
	 }
	 w1=w2->next;
      }
}

int wdg_refresh() {
   // If widgets need to be redrawn, call this!
   Widget *w1,*w2;
   
   w1=level[curr_level].first; /* Get first widget in this level */
   while(w1) {		/* Delete all widgets in this level */
      w2=w1;		
      printf("WREF: type: %s\n",w2->id);
      if(strcmp(w2->id,"BUTL")==0) { save_under(w2); up_button(w2,-1,-1); }
      w1=w2->next;
   }
}


int push_level(int x1,int y1,int x2,int y2) /* Makes current level temporary inactive and creates a
		    new one */
{
#ifdef GFXLIB_ALLEGRO
   BITMAP *bmp;
#endif
#ifdef GFXLIB_SDL
   SDL_Surface *bmp;
#endif
//   printf("pushlevel: curr_level:%d\n",curr_level);
   rem_pointer(mx,my);
//   printf("pl 2 (removed pointer)\n");
	if (curr_level > -1){
		level[curr_level].data=(void*)malloc((x2-x1+1)*(y2-y1+1)*4);
		level[curr_level].x1=x1;
		level[curr_level].x2=x2;
		level[curr_level].y1=y1;
		level[curr_level].y2=y2;
	        bmp=create_bitmap(x2-x1+1,y2-y1+1);
		blit(screen,bmp,x1,y1,0,0,x2-x1+1,y2-y1+1);
		level[curr_level].data=bmp;
//	   printf("NOTE: lv %d data = %d\n",curr_level,level[curr_level].data);
//		arps_getbox32(x1,y1,x2-x1+1,y2-y1+1,level[curr_level].data);
	}
//   printf("pl 3\n");
	if(++curr_level == MAX_LEVELS) {
#ifndef WIN32
		fprintf(stderr,"Too many levels. MAX=%d\n",MAX_LEVELS);
#endif
		exit(-1);
	}
//   printf("pl 4\n");
	level[curr_level].first=NULL;
	level[curr_level].pressed=NULL;
        level[curr_level].focus=NULL;
        level[curr_level].hilighted=NULL; // Safer?
   
        just_popped=0;
//   printf("pl 5\n");
/* Will this screw up SVGA?   added for SDL */
   draw_pointer(mx,my);
//   printf("pl 6 (displayed pointer) cl:%d\n",curr_level);
   return curr_level;
}

int pop_level() /* Deletes all widgets in the current level (from memory)
		   and returns to the previous level */
{
      Widget *w1,*w2;
#ifdef GFXLIB_ALLEGRO
   BITMAP *bmp;
#endif
#ifdef GFXLIB_SDL
   SDL_Surface *bmp;
#endif
	rem_pointer(mx,my);
//   printf("pop: curr level: %d\n",curr_level);
   // Note, this was <= 0, but then you can't pop the lowest level,
   // is this desired?
	if(curr_level < 0) {
#ifndef WIN32
	   fprintf(stderr,"No more levels to pop.\n");
#endif
		exit(-1);
	}
//   printf("pop2\n");
	w1=level[curr_level].first; /* Get first widget in this level */
//   printf("dump the widgies\n");

	while(w1) {		/* Delete all widgets in this level */
		w2=w1;		
		w1=w2->next;
//	   printf("xxx: %d\n",w2->destroy);
		if(w2->destroy) (*(w2->destroy))(w2);
//	   printf("free:\n");
		free(w2);
	}
//   printf("done the widgies\n");

	just_popped=1;
	curr_level--;

//   printf("pop 3\n");
        if(curr_level>-1 && level[curr_level].data) {
#ifdef FUNC_DEBUG
	  printf("going to pop a level in pop_level");
#endif
//	      printf("having a blit fit1\n");
	        bmp=level[curr_level].data;
//	      printf("having a blit fit2:%d\n",curr_level);
	        blit(bmp,screen, 0,0, level[curr_level].x1,
		   level[curr_level].y1,
		   level[curr_level].x2-level[curr_level].x1+1,
		   level[curr_level].y2-level[curr_level].y1+1);
//	   printf("having a blit fit3\n");
 		free(level[curr_level].data);
 		level[curr_level].data=NULL;
	        level[curr_level].hilighted=NULL; // Safer?
	}
   /* Will this screw up SVGA?   added for SDL */
//   printf("pop 4\n");
        draw_pointer(mx,my);
//   printf("pop 5\n");

	return (curr_level);
}

void wdg_destroy(Widget *w) {
   printf("wdgd1\n");
   // could this be a generic destory function that overrides others?
   if(w->extra!=NULL && strcmp(w->id,"WINF"))
     destroy_bitmap(w->extra);
   printf("wdgd2\n");
   if(w->under!=NULL)
     free(w->under); // do we need to restore?
   printf("wdgd3\n");
   if(w->text!=NULL) free(w->text);
   printf("wdgd4\n");
   free(w);
}


void wdg_scanlist() {
   // for debugging widget linked list
   Widget *w1;
   
   printf("LIST SCAN\n------------\n");
   w1=level[curr_level].first;
   while(w1) {
      printf("widget indx:%d  id:%s\n",w1->indx,w1->id);
      w1=w1->next;
   }
   printf("------------\n");
}

void wdg_tree_close(Widget *win) {
   Widget *w1,*w2;
   
   wdg_scanlist();
   w1=level[curr_level].first;
   // what about the children?
   printf("tree close\n");
   while(w1) {
      w2=w1;
      w1=w1->next;
      if(w2->parent==win) {
	 printf("recurse on: index:%d,  id:%s\n",w2->indx,w2->id);
	 // recurse here
	 w1=level[curr_level].first; // get out of the way!
	 wdg_window_close(w2);
      }
   }
   printf("parent: idx:%d  id%s\n",win->indx,win->id);
   if(level[curr_level].pressed==win) level[curr_level].pressed=NULL;
   if(level[curr_level].focus==win) level[curr_level].focus=NULL;
   if(level[curr_level].hilighted==win) level[curr_level].hilighted=NULL;

   if(win->prev!=NULL) win->prev->next=win->next; // remove from list
   if(win->next!=NULL) win->next->prev=win->prev; // remove from list
   if(level[curr_level].last==win) { 
      printf("LAST :%d\n",win->indx);
      level[curr_level].last=win->prev;
      printf("NEW LAST is %d\n",win->prev->indx);
      win->prev->next=NULL;
   }
   if(win->destroy) {
      printf("calling destroy callback\n");
      (*(win->destroy))(win);
   } else  {
     printf("destroy\n");
     wdg_destroy(win);
   }
   

//   free(w2);
   wdg_scanlist();
} // wdg_tree_close 

BITMAP* getwdgbmp(Widget *b) {
   BITMAP* useb;
   
   useb=screen;
   if(b->parent!=NULL) {
      if(b->parent->extra!=NULL) {
	 useb=b->parent->extra;
	 printf("got bmp from indx:%d  %s\n", b->parent->indx,b->parent->id);
      }
   }
   return useb;
}

void restore_under(Widget *w) {
#ifdef GFXLIB_ALLEGRO
   BITMAP *bmp;
   BITMAP *useb;
#endif
#ifdef GFXLIB_SDL
   SDL_Surface *bmp;
   SDL_Surface *useb;
#endif
   useb=getwdgbmp(w);
   bmp=w->under;
   if(bmp)
     blit(bmp,useb,0,0,w->x1,w->y1,(w->x2-w->x1)+1,(w->y2-w->y1)+1);
//        printf("restore_under()\n");
}

void save_under(Widget *w) {
#ifdef GFXLIB_ALLEGRO
   BITMAP *bmp;
   BITMAP *useb;
//        printf("save_under()\n");
   useb=getwdgbmp(w);
   bmp=create_bitmap((w->x2-w->x1)+1,(w->y2-w->y1)+1);
   blit(useb,bmp,w->x1,w->y1,0,0,(w->x2-w->x1)+1,(w->y2-w->y1)+1);
   w->under=bmp;
#endif
#ifdef GFXLIB_SDL
   // This is wrong!
	w->under=(void*)malloc((w->x2-w->x1+1)*(w->y2-w->y1+1)*4);
	arps_getbox32(w->x1,w->y1,(w->x2-w->x1)+1,(w->y2-w->y1)+1,w->under);
#endif
}	

Widget* new_widget() /* Creates a new widget in the current level */
{
	Widget *w = (Widget*) malloc(sizeof(Widget));
	if (!w) {
#ifndef WIN32
		fprintf(stderr,"Malloc failed!\n");
#endif
		exit(-1);
	}
	if(level[curr_level].first==NULL) {
		level[curr_level].first=w;
		w->prev=NULL;
	        w->indx=1;
	} else {
		level[curr_level].last->next=w;
		w->prev=level[curr_level].last;
	        w->indx=level[curr_level].last->indx+1;
	}
        w->ksym=-1;
        w->kascii=-1;
        w->hilight='F';
	w->under=NULL;
	w->next=NULL;
	w->destroy=NULL;
        w->parent=NULL;
        w->text=NULL;
        strcpy(w->id,"UNKN");
	level[curr_level].last=w;
	return w;
}

void kbd_loop(int halt_on_pop) {
   Widget* w;
   int mw, key,ksym,fx,fy;
   char kasc;
   
//   printf("in kbd_loop\n");
//   level[curr_level].pressed=NULL;
   while(!(halt_on_pop && just_popped)) {
//            printf("in event loop:  %d\n", mk);  
//            printf("mid event loop:  %d\n", mk); 
      // 
      if(keypressed()) {   // Allegro
	 key=readkey();
	 kasc=key & 0xff;
	 ksym=key>>8;
	 // printf("got key press: ksym: %d  asc: %c  level:%d\n",ksym,kasc,curr_level);
	 w=level[curr_level].first;
	 while(w) {
	    // printf("1: ksym:%d kasc:%d\n",ksym,kasc);
	    // It might be better to have a callback to handle input data
	    // to support different input types
//	    printf("Is there focus? %d\n",level[curr_level].focus);
	    if(level[curr_level].focus) {
//	       printf("have focus, sending data %d, %d\n",ksym, kasc);
	       wdg_input_data(level[curr_level].focus,&ksym,&kasc);
	    }
	    
//	    printf("2: ksym:%d kasc:%d\n",ksym,kasc);
	    if((w->ksym==ksym || w->kascii == kasc) 
	       && (level[curr_level].pressed==NULL)
	       && w->press)
	      {
//		 printf("found match! ksym:%d, w->ksym:%d  kasc:%d  w->kasc:%d\n",ksym,w->ksym,kasc,w->kascii);
		 fx=w->x1 +1;
		 fy=w->y1 +1;
		 rem_pointer(fx,fy);
//		 printf("gonna press\n");
		 (*(w->press))(w,fx,fy,1);
//		 printf("gonna depress\n");
		 if(w->depress)
		   (*(w->depress))(w,fx,fy);
//		 printf("done w bof\n");
		 draw_pointer(mx,my);
		 break;
	      }
	    w=w->next;
	 }
      
//      level[curr_level].pressed=NULL;
   }
   
   
      //  printf("out kbd loop:  %d\n", mk);  
      if(halt_on_pop == JUST_ONCE ) 
	just_popped=1;
   }  
   // Do we want this on JUST_ONCE?
//   printf("Out of the loop\n");
   just_popped=0;
}

void event_loop(int halt_on_pop)
{
   Widget* w;
   int mw;

#ifdef GFXLIB_SDL
   SDL_Event event;
#endif
   
   while(!(halt_on_pop && just_popped)) {
/*      printf("in event loop:  %d\n", mk);  */
#ifdef GFXLIB_ALLEGRO
      if(mouse_needs_poll()) {
	mw=poll_mouse();
      }
      mx=mouse_x;
      my=mouse_y;
      mk=mouse_b;
#endif
#ifdef GFXLIB_SDL
      if(SDL_PollEvent(&event)) {
	 switch (event.type) {
	  case SDL_MOUSEMOTION:
	    mx=event.motion.x;
	    my=event.motion.y;
	    break;
	  case SDL_MOUSEBUTTONUP:
	    mk=0;
	    break;
	  case SDL_MOUSEBUTTONDOWN:
	    mk=event.button.button;
	    break;
	  case SDL_QUIT:
	    exit(0);
	 }
      }
#endif
//      printf("mid event loop:  %d\n", mk); 

      if(mk) {
	 w=level[curr_level].first;
	 while(w) {
	   // if(in_box(w->x1,w->y1,w->x2,w->y2,mx,my)) 
	   //   printf("DEBUG: widget id:%s   indx:%d\n",w->id,w->indx);
	    // This will help prevent clicks on windows from clicking through
      
	    if(w->parent==NULL && in_box(w->x1,w->y1,w->x2,w->y2,mx,my)) 
	      {
	      wflag_clickprocessed=1;
//		 printf("ooh, I got this one!\n");
	      }
	    
	      
	    if(w->parent==NULL && in_box(w->x1,w->y1,w->x2,w->y2,mx,my) 
	       && (level[curr_level].pressed==NULL
		   || (level[curr_level].pressed==w && w->draggable))
	       && w->press)
	    {
	       printf("went hwer1\n");
//	       wflag_clickprocessed=1;
	       (*(w->press))(w,mx,my,mk);
	       level[curr_level].pressed=w;
	       break;
	    }
	    // for windows with parent, since coordinates are relative and
	    // can be withdrawn
	    if(w->parent!=NULL && in_box(w->x1+w->parent->x1,w->y1+w->parent->y1,w->x2+w->parent->x1,w->y2+w->parent->y1,mx,my) 
	       && (level[curr_level].pressed==NULL
		   || (level[curr_level].pressed==w && w->draggable))
	       && w->press && w->parent->active)
	    {
	       printf("went here2  %d=%d+%d\n",w->y1+w->parent->y1,w->y1,w->parent->y1);
	       wflag_clickprocessed=1;
	       printf("myclick:  %d\n",my);
	       (*(w->press))(w,mx-w->parent->x1,my-w->parent->y1,mk);
	       level[curr_level].pressed=w;
	       w->parent->refresh=1;
	       wdg_window_refresh(w->parent);
	       break;
	    }
	    w=w->next;
	 }
      } else {
//	 printf("unclick button\n");
	 wflag_clickprocessed=0;
	 w=level[curr_level].pressed;
	 if(w != NULL) {
	    if(w->depress) {
//	       rem_pointer(mx,my);
//	       scare_mouse();
	       if(w->parent!=NULL) { 
		  wdg_window_refresh(w->parent);
		  (*(w->depress))(w,mx-w->parent->x1,my-w->parent->y1);
	       } else
		  (*(w->depress))(w,mx,my);
		    
//	       draw_pointer(mx,my);
	    }

	 }
	 level[curr_level].pressed=NULL;
      }
      //  printf("out event loop:  %d\n", mk);  
      if(halt_on_pop == JUST_ONCE ) 
	just_popped=1;
   }  
   
   just_popped=0;
}
