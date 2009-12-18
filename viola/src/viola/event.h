/*
 * event.h
 */
#include "event_x.h"

#define IS_WITHIN_PARAM(x, y, x1, y1, x2, y2) \
(Boolean)((x >= x1 && x <= x2) ? ((y >= y1 && y <= y2) ? TRUE : FALSE) : FALSE)

#define EXPOSE_SELF 1
#define EXPOSE_BCARD 2
#define EXPOSE_FCARD 4
#define EXPOSE_STACK 8

#define SET_UPDATE 0
#define SET_NO_UPDATE 1

#define NUMBER_OF_TOOLS		5
#define ACTION_TOOL 		0
#define TARGET_TOOL 		1
#define MOVE_TOOL   		2
#define RESIZE_TOOL 		3
#define REPARENT_TOOL 		4

extern int currentTool;
extern char *toolID2Str[];

#define RC_UPPER 		1
#define RC_LOWER 		2
#define RC_LEFT 		3
#define RC_RIGHT 		4
#define RC_UPPER_LEFT 		5
#define RC_UPPER_RIGHT 		6
#define RC_LOWER_LEFT 		7
#define RC_LOWER_RIGHT 		8

#define MOUSE_BUTTON_1		(1<<1)
#define MOUSE_BUTTON_2		(1<<2)
#define MOUSE_BUTTON_3		(1<<3)
#define MOUSE_BUTTON_4		(1<<4)
#define MOUSE_BUTTON_5		(1<<5)

