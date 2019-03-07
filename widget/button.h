#ifndef BUTTON_H
#define BUTTON_H
#include "widget.h"

extern Widget* wdg_button_add(Widget *parent,int x1, int y1, int x2, int y2,
			      int(*handler)(Widget*,int,int,int));
extern Widget* wdg_button_txt_add(Widget *parent,int x1, int y1, int x2, int y2,
				  char* text,int(*handler)(Widget*,int,int,int));
extern Widget* wdg_button_bmp_add(Widget *parent,int x1, int y1, int x2, int y2,
		                  bmpbtn_t bbmp,int(*handler)(Widget*,int,int,int));
extern Widget* wdg_button_arrow_add(Widget *parent,int x,int y,int dir,
			     int(*handler)(Widget*,int,int,int));
extern Widget* wdg_checkbox_add
   (Widget *parent,int x,int y,int checked,int(*handler)(Widget*,int,int,int));
extern Widget* add_checkbox (int x,int y,int checked,int(*handler)(Widget*,int,int,int));
/* add_* - create a new widget */
extern Widget* add_button(int,int,int,int,char*,int(*handler)(Widget*,int,int,int));
extern Widget* add_arrow_button(int,int,int,int(*handler)(Widget*,int,int,int));
extern Widget* add_invisible_button
	(int x1,int y1,int x2,int y2,int(*handler)(Widget*,int,int,int));
extern Widget* add_bmp_button(int,int,int,int,bmpbtn_t,int(*handler)(Widget*,int,int,int));
#endif /* button.h */
