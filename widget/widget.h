#ifndef WIDGET_H
#define WIDGET_H

#define MAX_LEVELS 20 /* maximum levels of windows (depth) */

#define UP 0
#define DOWN 1
#define RIGHT 2
#define LEFT 3
/* These are used to retrieve data from level[] 
   Remove if level[] is made global */
#define W_LAST 0
#define W_FIRST 1
#define W_PRESSED 2
#define W_FOCUS 3
#define W_HILIGHTED 4

/* These three are used when calling event_loop: */
#define FOREVER 0    /* Perpetual event loop */
#define HALT_ON_POP 1 /* Exit when window is closed */
#define JUST_ONCE 2 /* one iteration (useful for extending the event loop into your program */

typedef struct bmpbtn_t {  // Should this go in button.h?
   void* btnupbmp; /* Bitmap for buttons */ 
   void* btndnbmp; /* Bitmap for buttons */ 
   void* btnhlbmp; /* HL Bitmap */
   int up_x1, up_x2, up_y1, up_y2;
   int dn_x1, dn_x2, dn_y1, dn_y2;
   int hl_x1, hl_x2, hl_y1, hl_y2;
} bmpbtn_t;

typedef struct wstyle_t {
   int bg; // Background
   int hl; // Hilight
   int sh; // Shadow
   int tx; // Text Color
   int al; // Alpha for modes that don't support it
} wstyle_t;

typedef struct style_t {
   wstyle_t window;
   wstyle_t button;
   wstyle_t input;
} style_t;

typedef struct widget{
	int x1,y1,x2,y2;
        int active,refresh;
        int indx;   /* numeric index */
        char id[12];  /* id of widget type 4 chars with padding */ 
        int ksym;  /* keysym of bound keypress or -1 */
	char kascii; /* ascii code of bound keypress or -1 */
	char draggable; /* if set to TRUE, the 'press' function will be
			   called every time the mouse is moved, while
			   holding down the mouse button. - Otherwise
			   it is only called once when the mouse button
			   is pressed down. */
        char hilight; /* if T, then this widget can be hilighted */
	int (*press)(struct widget*,int,int,int);
	int (*depress)(struct widget*,int,int);
	void (*destroy)(struct widget*);
	
	struct widget *parent,*prev,*next; /* for a double-linked list */

	char* text; /* maybe this should be part of 'extra' */
	int (*handler)(struct widget*,int,int,int);
	
	void* under; /* for saving what's underneeth */
	void* extra; /* widget specific data */
        bmpbtn_t btbmp;
} Widget;

void widget_init(); /* Initialize the widget library */

/* event handling routines: */
void event_loop(int);

/* manipulate levels of widgets */
int push_level(int,int,int,int);
int pop_level();

/* allocationg memory for widgets */
Widget* new_widget();

/* Save and restore the background under a widget */
void restore_under(Widget *w);
void save_under(Widget *w);
void wdg_destroy(Widget *w);

/* Interogating level[] (which should be global btw) */
Widget* wdg_getlevel(int);
void wdg_setlevel(int,Widget *w);
extern BITMAP* getwdgbmp(Widget *b);
extern void unfocus_input(Widget *b,int x, int y, int m);
//extern Widget* wdg_input_add (Widget *parent,int x1,int y1,int wc,char* text);
//extern Widget* add_input (int x1,int y1,int wc,char* text)

/* Globals */
extern int just_popped;
extern style_t activestyle;
extern int wflag_clickprocessed;
extern BITMAP *drawbmp;
#endif /* widget.h */
