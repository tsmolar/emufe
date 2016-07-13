#ifndef BUTTON_H
#define BUTTON_H
#include "widget.h"

/* add_* - create a new widget */

Widget* add_button(int,int,int,int,char*,int(*handler)(Widget*,int,int,int));
Widget* add_arrow_button(int,int,int,int(*handler)(Widget*,int,int,int));
Widget* add_invisible_button
	(int x1,int y1,int x2,int y2,int(*handler)(Widget*,int,int,int));
Widget* add_bmp_button(int,int,int,int,bmpbtn_t,int(*handler)(Widget*,int,int,int));
#endif /* button.h */
