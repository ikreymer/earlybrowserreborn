#include "utils.h"
#include <ctype.h>
#include "error.h"
#include "mystrings.h"
#include "hash.h"
#include "ident.h"
#include "scanutils.h"
#include "obj.h"
#include "sys.h"
#include "vlist.h"
#include "attr.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "slotaccess.h"
#include "classlist.h"
#include "cl_txtDisp.h"
#include "misc.h"
#include "glib.h"
#include "tfed.h"
#include "cexec.h"
#include "sgml.h"
#include "html2.h"
#include "event_x.h"

#define DOX 1

#define VERBOSE 1

#ifdef DOX
/***************************** BEGINNING OF X DECLS ************************/
#include <X11/Xos.h>
#include <X11/Xlib.h>
/*#include <X11/Xmu/Xmu.h>*/
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#define FLUSH XFlush(display)
/*#define FLUSH ;*/

#define MAX_PTS 100

XPoint fpathPts[MAX_PTS];

/***************************** END OF X DECLS ***************************/
#endif

#define MINFO_BEGIN	1
#define MINFO_DATA	2
#define MINFO_ENTITY	3
#define MINFO_HDIV	4
#define MINFO_S_BOX 	5
#define MINFO_E_BOX 	6
#define MINFO_HBOX 	7
#define MINFO_VBOX 	8
#define MINFO_S_SUB 	9
#define MINFO_E_SUB 	10
#define MINFO_SUB 	11
#define MINFO_S_SUP 	12
#define MINFO_E_SUP 	13
#define MINFO_SUP 	14
#define MINFO_LPAREN 	15
#define MINFO_RPAREN 	16
#define MINFO_LBRACK 	17
#define MINFO_RBRACK 	18
#define MINFO_INTEGRAL	19
#define MINFO_SUM 	20

char *MInfoStr[] = {
	"?",
	"BEGIN",
	"DATA",
	"ENTITY",
	"HDIV",
	"S_BOX",
	"E_BOX",
	"HBOX",
	"VBOX",
	"S_SUB",
	"E_SUB",
	"SUB",
	"S_SUP",
	"E_SUP",
	"SUP",
	"LPAREN",
	"RPAREN",
	"LBRACK",
	"RBRACK",
	"INTEGRAL",
	"SUM",
};

typedef struct minfo {
	int type;
	char *s;
} MInfo;

typedef struct mast {
	int type;
	char *s;
	int slen;
	int x, y, rx, ry;
	int width, height;
	struct mast *parent;
	struct mast *next;
	struct mast *children;
	int fontID;
} MAST;

int drawBoxP = 0;

#define MINFO_BUFF_SIZE 200
MInfo minfoBuff[MINFO_BUFF_SIZE];

MInfo t10[] = {
	MINFO_HBOX,	"*START TheBOX",/*wrapper*/
	MINFO_DATA,	"v=",
	MINFO_S_BOX,	"*START BOX",
	MINFO_DATA,	"d1",
	MINFO_DATA,	"+",
	MINFO_DATA,	"d2",
	MINFO_HDIV,	"*OVER",
	MINFO_DATA,	"t1",
	MINFO_DATA,	"+",
	MINFO_DATA,	"t2",
	MINFO_E_BOX,	"*END BOX",
	0,0
};

MInfo t1[] = {
	MINFO_HBOX,	"*START TheBOX",/*wrapper*/
	MINFO_DATA,	"a=",
	MINFO_INTEGRAL,	"*INTEGRAL",
	MINFO_S_SUP,	"*START SUP",
	MINFO_DATA,	"1",
	MINFO_E_SUP,	"*END SUP",
	MINFO_S_SUB,	"*START SUB",
	MINFO_DATA,	"0",
	MINFO_E_SUB,	"*END SUB",
	MINFO_LPAREN,	"(",
	MINFO_S_BOX,	"*START BOX",
	MINFO_DATA,	"d1",
	MINFO_DATA,	"+",
	MINFO_LPAREN,	"(",
	MINFO_S_BOX,	"*START BOX",
	MINFO_DATA,	"d2",
	MINFO_HDIV,	"*OVER",
	MINFO_DATA,	"x",
	MINFO_E_BOX,	"*END BOX",
	MINFO_RPAREN,	")",
	MINFO_HDIV,	"*OVER",

	MINFO_S_BOX,	"*START BOX",
	MINFO_DATA,	"t",
	MINFO_HDIV,	"*OVER",
	MINFO_DATA,	"y",
	MINFO_E_BOX,	"*END BOX",

	MINFO_E_BOX,	"*END BOX",
	MINFO_RPAREN,	")",
	MINFO_LPAREN,	"(",
	MINFO_S_BOX,	"*START BOX",
	MINFO_DATA,	"y(x)",
	MINFO_HDIV,	"*OVER",
	MINFO_DATA,	"t",
	MINFO_E_BOX,	"*END BOX",
	MINFO_RPAREN,	")",
	0,0
};

MInfo sample_integral[] = {
	MINFO_HBOX,	"*START TheBOX",/*wrapper*/
	MINFO_DATA,	"a=",
	MINFO_INTEGRAL,	"*INTEGRAL",
	MINFO_S_SUP,	"*START SUP",
	MINFO_DATA,	"1",
	MINFO_E_SUP,	"*END SUP",
	MINFO_S_SUB,	"*START SUB",
	MINFO_DATA,	"0",
	MINFO_E_SUB,	"*END SUB",
	MINFO_S_BOX,	"*START BOX",
	MINFO_DATA,	"y(x)",
	MINFO_HDIV,	"*OVER",
	MINFO_DATA,	"t",
	MINFO_E_BOX,	"*END BOX",
	0,0
};

MInfo sample_simpleOver[] = {
	MINFO_HBOX,	"*START TheBOX",/*wrapper*/
	MINFO_DATA,	"v=",
	MINFO_S_BOX,	"*START BOX",
	MINFO_DATA,	"d1",
	MINFO_DATA,	"+",
	MINFO_DATA,	"d2",
	MINFO_HDIV,	"*OVER",
	MINFO_DATA,	"t1",
	MINFO_DATA,	"+",
	MINFO_DATA,	"t2",
	MINFO_E_BOX,	"*END BOX",
	0,0
};

MInfo t3[] = {
	MINFO_HBOX,	"*START TheBOX",/*wrapper*/
	MINFO_DATA,	"v",
	MINFO_LBRACK,	"[",
	MINFO_DATA,	"n",
	MINFO_RBRACK,	"]",
/*
	MINFO_SUB,	"*START SUB",
	MINFO_DATA,	"n",
*/
	MINFO_DATA,	"=",
	MINFO_LPAREN,	"(",
	MINFO_LPAREN,	"(",
	MINFO_LPAREN,	"(",
	MINFO_LPAREN,	"(",
	MINFO_S_BOX,	"*START BOX",
	MINFO_DATA,	"d1",
	MINFO_DATA,	"+",
	MINFO_S_BOX,	"*START BOX",
	MINFO_DATA,	"d2",
	MINFO_HDIV,	"*OVER",
	MINFO_DATA,	"x",
	MINFO_E_BOX,	"*END BOX",
	MINFO_HDIV,	"*OVER",
	MINFO_DATA,	"t",
	MINFO_E_BOX,	"*END BOX",
	MINFO_RPAREN,	")",
	MINFO_RPAREN,	")",
	MINFO_RPAREN,	")",
	MINFO_RPAREN,	")",
	MINFO_DATA,	"e",
	0,0
};

/*
      dV               /   /          \  \
        out           | k | V  - V     |  |
    C ----- = I  tanh |    \  in  out /   |
       dt       b     | ----------------- |
                       \        2        /
*/
MInfo sample_fromHTMLspec_1[] = {
	MINFO_HBOX,	"*START TheBOX",/*wrapper*/
	MINFO_DATA,	"C",
	MINFO_S_BOX,	"*START BOX",
	MINFO_DATA,	"dV",
	MINFO_S_SUB,	"*START SUB",
	MINFO_DATA,	"out",
	MINFO_E_SUB,	"*END SUB",
	MINFO_HDIV,	"*OVER",
	MINFO_DATA,	"dt",
	MINFO_E_BOX,	"*END BOX",

	MINFO_DATA,	"=",

	MINFO_DATA,	"I",
	MINFO_S_SUB,	"*START SUB",
	MINFO_DATA,	"b",
	MINFO_E_SUB,	"*END SUB",

	MINFO_DATA,	"tanh",

	MINFO_LPAREN,	"(",
	MINFO_S_BOX,	"*START BOX",

	MINFO_DATA,	"K",
	MINFO_LPAREN,	"(",
	MINFO_S_BOX,	"*START BOX",
	MINFO_DATA,	"V",
	MINFO_S_SUB,	"*START SUB",
	MINFO_DATA,	"in",
	MINFO_E_SUB,	"*END SUB",
	MINFO_DATA,	"-",
	MINFO_DATA,	"V",
	MINFO_S_SUB,	"*START SUB",
	MINFO_DATA,	"out",
	MINFO_E_SUB,	"*END SUB",
	MINFO_E_BOX,	"*END BOX",
	MINFO_RPAREN,	")",

	MINFO_HDIV,	"*OVER",
	MINFO_DATA,	"2",

	MINFO_E_BOX,	"*END BOX",
	MINFO_RPAREN,	")",
	0,0
};

/*              2
	E   = MC
         out
 */
MInfo sample_subsup[] = {
	MINFO_HBOX,	"*START TheBOX",/*wrapper*/
	MINFO_DATA,	"E",
	MINFO_S_SUB,	"*START SUB",
	MINFO_DATA,	"out",
	MINFO_E_SUB,	"*END SUB",
	MINFO_DATA,	"=",
	MINFO_DATA,	"M",
	MINFO_DATA,	"C",
	MINFO_S_SUP,	"*START SUP",
	MINFO_DATA,	"2",
	MINFO_E_SUP,	"*END SUP",
	0,0
};

/*       /     d1 + d2       \
        | ------------------  | 
    v = |        / a1 + a2 \  | e
        |  x1 + |  -------  | |
	 \       \ b1 + b2 / /
*/
MInfo sample_over[] = {
	MINFO_HBOX,	"*START TheBOX",/*wrapper*/
	MINFO_DATA,	"v",
	MINFO_DATA,	"=",
	MINFO_LPAREN,	"(",
	MINFO_S_BOX,	"*START BOX",

	MINFO_DATA,	"d1",
	MINFO_DATA,	"+",
	MINFO_DATA,	"d2",
	MINFO_HDIV,	"*OVER",
	MINFO_DATA,	"x1",
	MINFO_DATA,	"+",

	MINFO_LPAREN,	"(",
	MINFO_S_BOX,	"*START BOX",
	MINFO_DATA,	"a1",
	MINFO_DATA,	"+",
	MINFO_DATA,	"a2",
	MINFO_HDIV,	"*OVER",
	MINFO_DATA,	"b1",
	MINFO_DATA,	"+",
	MINFO_DATA,	"b2",
	MINFO_E_BOX,	"*END BOX",
	MINFO_RPAREN,	")",

	MINFO_E_BOX,	"*END BOX",
	MINFO_RPAREN,	")",
	MINFO_DATA,	"e",
	0,0
};

#define INTEGRAL_WIDTH 17
#define GAP_AFTER_INTEGRAL 5
#define GAP_INTEGRAL 20
#define PAREN_WIDTH 5
#define BRACK_WIDTH 5


typedef struct fpair {
	float x,y;
} FPair;

#define FPATH_COUNT_INTEGRAL 46
FPair fpath_integral[] = {
        0.869565,       0.892193,
        0.794466,       0.914498,
        0.770751,       0.936803,
        0.798419,       0.962825,
        0.837945,       0.981413,
        0.743083,       0.977695,
        0.640316,       0.944238,
        0.577075,       0.832714,
        0.553360,       0.706320,
        0.561265,       0.598513,
        0.584980,       0.468401,
        0.584980,       0.312268,
        0.561265,       0.204461,
        0.505929,       0.107807,
        0.450593,       0.050186,
        0.367589,       0.018587,
        0.276680,       0.000000,
        0.142292,       0.009294,
        0.047431,       0.024164,
        0.007905,       0.052045,
        0.000000,       0.074349,
        0.023715,       0.104089,
        0.086957,       0.115242,
        0.150198,       0.115242,
        0.205534,       0.098513,
        0.225296,       0.070632,
        0.189723,       0.037175,
        0.126482,       0.035316,
        0.150198,       0.022305,
        0.284585,       0.018587,
        0.371542,       0.050186,
        0.422925,       0.115242,
        0.430830,       0.189591,
        0.450593,       0.323420,
        0.442688,       0.427509,
        0.426877,       0.609665,
        0.442688,       0.750929,
        0.498024,       0.869888,
        0.612648,       0.966543,
        0.750988,       0.996283,
        0.877470,       1.000000,
        0.972332,       0.983271,
        1.000000,       0.957249,
        0.996047,       0.925651,
        0.948617,       0.899628,
        0.873518,       0.892193,
};

MAST *handle;

MAST *makeMAST()
{
	MAST *m = (MAST*)malloc(sizeof(struct mast));
	m->type = 0;
	m->s = NULL;
	m->y = m->x = m->rx = m->ry = 0;
	m->width = m->height = 0;
	m->parent = NULL;
	m->next = NULL;
	m->children = NULL;
	return m;
}

void appendSibling(self, sibling)
	MAST *self, *sibling;
{
	MAST *mast;
	if (self->next) {
		for (mast = self->next; mast->next; mast = mast->next);
		mast->next = sibling;
	} else {
		self->next = sibling;
	}
}

void appendChild(parent, child)
	MAST *parent, *child;
{
	MAST *mast;
	if (parent->children) {
		for (mast = parent->children; mast->next; mast = mast->next);
		mast->next = child;
	} else {
		parent->children = child;
	}
}

void removeChild(self, child)
	MAST *self, *child;
{
	MAST *mast;
	if (self->children) {
		if (self->children == child) {
			self->children = NULL;
		} else {
			for (mast = self->children; mast->next; 
		 	     mast = mast->next)
				if (mast->next == child) {
					mast->next = child->next;
					break;
				}
		}
	}
}
void dumpMAST(self, level)
	MAST *self;
	int level;
{
	MAST *mast;
	int i;

	for (mast = self; mast; mast = mast->next) {
		if (!mast->type) return;
		for (i = 0; i < level; i++) printf("  ");
		printf("[%14s]\t%9s\t%3d(%d) %3d(%d) %3d %3d\n",
			mast->s, MInfoStr[mast->type], 
			mast->x, mast->rx, mast->y, mast->ry,
			mast->width, mast->height);
		if (mast->children) {
			dumpMAST(mast->children, level+1);
		} else if (mast->type == MINFO_E_BOX ||
			   mast->type == MINFO_RPAREN ||
			   mast->type == MINFO_RBRACK) {
			return;
		}
	}
}

void frameCoords(self, rx, ry)
	MAST *self;
	int rx, ry;
{
	MAST *mast;

	self->rx = rx + self->x;
	self->ry = ry + self->y;

	for (mast = self; mast; mast = mast->next) {
		if (!mast->type) return;
		mast->rx = rx + mast->x;
		mast->ry = ry + mast->y;
		if (mast->children)
			frameCoords(mast->children, mast->rx, mast->ry);
	}
}

void doOver(self)
	MAST *self;
{
	MAST *mast, *new, *objsToPush, *hbox;

	hbox = makeMAST();
	hbox->parent = self->parent;/*XXX*/
	hbox->type = MINFO_HBOX;
	hbox->s = "*HBox";
	hbox->next = self;

	objsToPush = self->parent->children;
	hbox->children = objsToPush;
/*
	printf("-hbox--1-----------\n");
	dumpMAST(hbox, 0);
	printf("--------------\n");
*/
	hbox->next = self;
	self->parent->children = hbox;
/*
	printf("-objsToPush-------------\n");
	dumpMAST(objsToPush, 0);
	printf("--------------\n");
*/

	for (mast = objsToPush; mast; mast = mast->next) {
		mast->parent = hbox;
		if (mast->next == self) {
			mast->next = NULL;
			break;
		}
	}
/*
	printf("-objsToPush 2-------------\n");
	dumpMAST(objsToPush, 0);
	printf("--------------\n");

	printf("-hbox-------------\n");
	dumpMAST(hbox, 0);
	printf("--------------\n");

	printf("-handle-------------\n");
	dumpMAST(handle, 0);
	printf("--------------\n");
*/
}

void tile(self, level)
	MAST *self;
	int level;
{
	MAST *mast, *cmast;
	int i, x, y;
	int vspan = 0, maxWidth = 0;
	int hspan = 0, maxHeight = 0;

	mast = self; 

	if (!mast->type) return;

#ifdef VERBOSE
	for (i = 0; i < level; i++) printf(" ");
	printf("T(%s)\t%s\t%d %d %d %d\n",
		mast->s, MInfoStr[mast->type], 
		mast->x, mast->y, mast->width, mast->height);
#endif

	switch (mast->type) {
	case MINFO_VBOX:
	case MINFO_S_BOX:
		vspan = 0;
		maxWidth = 0;
		if (mast->children) {
			for (cmast = mast->children; cmast;
			     cmast = cmast->next) {
				cmast->y = vspan;
				tile(cmast, level+1);
				vspan += cmast->height;
				if (maxWidth < cmast->width) 
					maxWidth = cmast->width;
			}
		}
		mast->width = maxWidth;
		mast->height = vspan;
	break;
	case MINFO_HBOX:
		hspan = 0;
		maxHeight = 0;
		if (mast->children) {
			for (cmast = mast->children; cmast;
			     cmast = cmast->next) {
				cmast->x = hspan;
				tile(cmast, level+1);
				hspan += cmast->width;
				if (maxHeight < cmast->height) 
					maxHeight = cmast->height;
			}
		}
		mast->width = hspan;
		mast->height = maxHeight;
	break;
	case MINFO_S_SUB:
	case MINFO_S_SUP:
		vspan = 0;
		maxWidth = 0;
		if (mast->children) {
			for (cmast = mast->children; cmast;
			     cmast = cmast->next) {
				cmast->y = vspan;
				vspan += cmast->height;
				if (maxWidth < cmast->width) 
					maxWidth = cmast->width;
			}
		}
		mast->width = maxWidth;

		if (mast->parent->type == MINFO_INTEGRAL)
			mast->height = vspan;
		else
			mast->height = (float)vspan * 1.7;
	break;
	case MINFO_LPAREN:
	case MINFO_LBRACK:
		hspan = PAREN_WIDTH;/*==BRACK_WIDTH*/
		maxHeight = 0;
		if (mast->children) {
			for (cmast = mast->children; cmast;
			     cmast = cmast->next) {
				cmast->x = hspan;
				tile(cmast, level+1);
				hspan += cmast->width;
				if (maxHeight < cmast->height) 
					maxHeight = cmast->height;
			}
		}
		mast->width = hspan + PAREN_WIDTH;/*==BRACK_WIDTH*/
		mast->height = maxHeight + 2;
	break;
	case MINFO_INTEGRAL:
		vspan = 0;
		maxWidth = 0;
		cmast = mast->children;
		if (cmast && cmast->type == MINFO_S_SUP) {
			cmast->y = vspan;
			tile(cmast, level+1);
			vspan += cmast->height;
			if (maxWidth < cmast->width) maxWidth = cmast->width;
			cmast = cmast->next;
		}
		vspan += GAP_INTEGRAL;
		if (cmast && cmast->type == MINFO_S_SUB) {
			cmast->y = vspan;
			tile(cmast, level+1);
			vspan += cmast->height;
			if (maxWidth < cmast->width) maxWidth = cmast->width;
		}
		mast->width = maxWidth + INTEGRAL_WIDTH + GAP_AFTER_INTEGRAL;
		mast->height = vspan;
	break;
	case MINFO_BEGIN:
		tile(mast->next, level);
	break;
	case MINFO_ENTITY:
	case MINFO_DATA:
	case MINFO_HDIV:
	break;
	case MINFO_E_BOX:
	case MINFO_RPAREN:
	case MINFO_RBRACK:
	default:
		return;
	}
}

void center(self, level)
	MAST *self;
	int level;
{
	MAST *mast, *cmast;
	int i, vspan, hspan;

#ifdef VERBOSE
	for (i = 0; i < level; i++) printf(" ");
	printf("C(%s)\t%s\t%d %d %d %d\n",
		self->s, MInfoStr[self->type], 
		self->x, self->y, self->width, self->height);
#endif
	for (mast = self; mast; mast = mast->next) {
		if (!mast->type) return;

		hspan = mast->width;
		vspan = mast->height;

		if (mast->children) {
			switch (mast->type) {
			case MINFO_ENTITY:
			case MINFO_DATA:
			case MINFO_BEGIN:
			case MINFO_HDIV:
			case MINFO_INTEGRAL:
			break;
			case MINFO_S_BOX:
			case MINFO_VBOX:
				center(mast->children, level+1);
				for (cmast = mast->children; cmast;
				     cmast = cmast->next) {
					cmast->x = (hspan - cmast->width) / 2;
				}
			break;
			case MINFO_LPAREN:
			case MINFO_LBRACK:
			case MINFO_HBOX:
				center(mast->children, level+1);
				for (cmast = mast->children; cmast;
				     cmast = cmast->next) {
					cmast->y = (vspan - cmast->height) / 2;
				}
			break;
			case MINFO_S_SUB:
				vspan = mast->height / 2;
				center(mast->children, level+1);
				for (cmast = mast->children; cmast;
				     cmast = cmast->next) {
					cmast->y = (vspan - cmast->height) / 2
							+ vspan;
				}
			break;
			case MINFO_S_SUP:
				vspan = mast->height / 2;
				center(mast->children, level+1);
				for (cmast = mast->children; cmast;
				     cmast = cmast->next) {
					cmast->y = (vspan - cmast->height) / 2;
				}
			break;
			case MINFO_E_BOX:
			case MINFO_E_SUB:
			case MINFO_E_SUP:
			case MINFO_RPAREN:
			case MINFO_RBRACK:
			default:
				return;
			}
		}
	}
}

void expandables(self)
	MAST *self;
{
	MAST *mast, *cmast;

	for (mast = self; mast; mast = mast->next) {
		if (!mast->type) return;
#ifdef VERBOSE
		printf("E [%s]\t%s\n",
			MInfoStr[mast->type], mast->s);
#endif
		switch (mast->type) {
		case MINFO_LPAREN:
		case MINFO_LBRACK:
		case MINFO_S_BOX:
		case MINFO_VBOX:
		case MINFO_HBOX:
		case MINFO_S_SUB:
		case MINFO_S_SUP:
			expandables(mast->children);
		break;
		case MINFO_HDIV:
			mast->width = mast->parent->width - 2;
		break;
		case MINFO_INTEGRAL:
			mast->height = mast->parent->height;
			cmast = mast->children;
			if (cmast && cmast->type == MINFO_S_SUP) {
				cmast->x = INTEGRAL_WIDTH;
				cmast->y = 0;
				expandables(cmast);
				cmast = cmast->next;
			}
			if (cmast && cmast->type == MINFO_S_SUB) {
				cmast->x = INTEGRAL_WIDTH;
				cmast->y = mast->height - cmast->height;
				expandables(cmast);
			}
		break;
		case MINFO_ENTITY:
		case MINFO_DATA:
		case MINFO_BEGIN:
		case MINFO_E_SUB:
		case MINFO_E_SUP:
		break;
		case MINFO_RPAREN:/*not used?*/
		case MINFO_RBRACK:
			mast->height = mast->parent->height;
			return;
		break;
		case MINFO_E_BOX:
		default:
			return;
		}
	}
}

int fold(self)
	MAST *self;
{
	MAST *mast, *mast2, *next;

	for (mast = self; mast; mast = mast->next) {
		if (!mast->type) return 0;
#ifdef VERBOSE
		printf("F [%s]\t%s\n",
			MInfoStr[mast->type], mast->s);
#endif
		switch (mast->type) {
		case MINFO_INTEGRAL: {
			MAST *sup = NULL, *sub = NULL;
			for (mast2 = mast->next; mast2;) {
				next = mast2->next;
				if (sup == NULL && 
				    mast2->type == MINFO_S_SUP) {
					sup = mast2;
					removeChild(mast->parent, sup);
					sup->next = NULL;
					sup->parent = mast;
					appendChild(mast, sup);
					mast2 = next;
					continue;
				}
				if (sub == NULL && 
				    mast2->type == MINFO_S_SUB) {
					sub = mast2;
					removeChild(mast->parent, sub);
					sub->next = NULL;
					sub->parent = mast;
					appendChild(mast, sub);
					mast2 = next;
					continue;
				}
				if (sup || sub) return 1;
				if (mast2->type != MINFO_S_SUP ||
				    mast2->type != MINFO_S_SUB) break;
				mast2 = next;
			}
		} break;
		case MINFO_LPAREN:
		case MINFO_LBRACK:
		case MINFO_S_BOX:
		case MINFO_VBOX:
		case MINFO_HBOX:
			if (fold(mast->children)) return 1;
		break;
		case MINFO_ENTITY:
		case MINFO_DATA:
		case MINFO_BEGIN:
		case MINFO_S_SUB:
		case MINFO_S_SUP:
		case MINFO_E_SUB:
		case MINFO_E_SUP:
		case MINFO_HDIV:
		break;
		default:
			return 0;
		}
	}
	return 0;
}

void setMASTSize(mast)
	MAST *mast;
{
	switch (mast->type) {
	case MINFO_ENTITY:
	break;
	case MINFO_DATA:
#ifdef DOX
		mast->width = XTextWidth(mfinfo,
					mast->s, strlen(mast->s)) + 1;
		mast->height = mfinfo->ascent + mfinfo->descent;
#else
		/*dummy values*/
		mast->width = 20;
		mast->height = 10;
#endif
	break;
	case MINFO_HDIV:
		/* width to be expanded later*/
		mast->height = 3;
	break;
	case MINFO_LPAREN:
	case MINFO_RPAREN:
	case MINFO_LBRACK:
	case MINFO_RBRACK:
		/* height to be expanded later */
		mast->width = 5; /* should be proportional to height, but...*/
	break;
	case MINFO_S_BOX:
	case MINFO_E_BOX:
	break;
	case MINFO_BEGIN:
	default:
	break;
	}
}

MAST *build(self, parent, minfo, minfoIdx, level)
	MAST *self;
	MAST *parent;
	MInfo *minfo;
	int *minfoIdx;
	int level;
{
	MInfo *m;
	MAST *new, *insert, *big_brother = NULL, *child, *ret;
	int i;

	for (;;) {
		m = &minfo[*minfoIdx];
		if (m->type == MINFO_E_BOX) {
/*			(*minfoIdx)++;
*/
#ifdef VERBOSE
			for (i = 0; i < level; i++) printf(" ");
			printf("[%s]\t%s\n",
				MInfoStr[m->type], m->s);
#endif
			return big_brother;
		} else if (m->s) {
			(*minfoIdx)++;
#ifdef VERBOSE
			for (i = 0; i < level; i++) printf(" ");
			printf("[%s]\t%s\n",
				MInfoStr[m->type], m->s);
#endif
		} else {
			return big_brother;
		}
		new = makeMAST();
		new->parent = parent;
		new->type = m->type;
		new->s = m->s;

		setMASTSize(new);

		if (self) appendSibling(self, new);
		else {
			big_brother = new;
			parent->children = big_brother;
		}
		self = new;

		switch (m->type) {
		case MINFO_S_BOX:
			insert = makeMAST();
			insert->parent = self;
			insert->type = MINFO_HBOX;
			insert->s = "*HBOX i1";
			setMASTSize(insert);
			appendChild(self, insert);

			ret = build(NULL, insert, minfo, minfoIdx, level+1);

			if (!ret) return big_brother; /*error*/
			if (ret->type == MINFO_HDIV) {
				removeChild(ret->parent, ret);
				appendSibling(insert, ret);
				ret->parent = self;
			}
			self = ret;

			insert = makeMAST();
			insert->parent = self;
			insert->type = MINFO_HBOX;
			insert->s = "*HBOX i2";
			setMASTSize(insert);
			appendSibling(self, insert);

			ret = build(NULL, insert, minfo, minfoIdx, level+1);

			m = &minfo[*minfoIdx];

			if (m->type == MINFO_E_BOX) {
				(*minfoIdx)++;
				self = new;
			} else if (m->type == MINFO_RPAREN ||
				   m->type == MINFO_RBRACK) {
				return big_brother;
			}
		break;
		case MINFO_VBOX:
		case MINFO_HBOX:
			child = build(NULL, new, minfo, minfoIdx, level+1);
			m = &minfo[*minfoIdx];
			if (m->type == MINFO_RPAREN ||
			    m->type == MINFO_RBRACK)
				return big_brother;
		break;
		case MINFO_S_SUB:
		case MINFO_S_SUP:
			child = build(NULL, new, minfo, minfoIdx, level+1);
			m = &minfo[*minfoIdx];
			if (m->type == MINFO_RPAREN ||
			    m->type == MINFO_RBRACK)
				return big_brother;
		break;
		case MINFO_LPAREN:
		case MINFO_LBRACK:
			child = build(NULL, new, minfo, minfoIdx, level+1);
			m = &minfo[*minfoIdx];
			if (m->type == MINFO_RPAREN ||
			    m->type == MINFO_RBRACK)
				(*minfoIdx)++;
		break;
		case MINFO_HDIV:
			return new;
		break;
		case MINFO_E_BOX:
		case MINFO_E_SUP:
		case MINFO_E_SUB:
		case MINFO_RPAREN:
		case MINFO_RBRACK:
			return big_brother;
		break;
		}
/*		printf("(type=%s\t[%s])\n", MInfoStr[self->type], self->s);
*/
	}
}

make(minfo)
	MInfo *minfo;
{
	int i;
	int minfoIdx = 0;

	handle = makeMAST();
	handle->s = "BEGIN";
	handle->type = MINFO_BEGIN;

#ifdef VERBOSE
	for (i = 0; minfo[i].type; i++) {
		printf("%d:\ttype=%s\t[%s]\n", 
			i, MInfoStr[minfo[i].type], minfo[i].s);
	}
#endif
	minfoIdx = 0;
	build(handle, NULL, minfo, &minfoIdx, 0);
/*
	printf("-Built-------------\n");
	dumpMAST(handle, 0);
	printf("--------------\n");
*/
	fold(handle);
/*
	printf("-Folded-------------\n");
	dumpMAST(handle, 0);
	printf("--------------\n");
*/
	setMASTSize(handle);
/*
	printf("-Individual sizes-------------\n");
	dumpMAST(handle, 0);
	printf("--------------\n");
*/
	tile(handle, 0);
/*
	printf("-Tiled-------------\n");
	dumpMAST(handle, 0);
	printf("--------------\n");
*/
	expandables(handle);
/*
	printf("-Expanded-------------\n");
	dumpMAST(handle, 0);
	printf("--------------\n");
*/
	center(handle, 0);
/*
	printf("-Centered-------------\n");
	dumpMAST(handle, 0);
	printf("--------------\n");
*/
	frameCoords(handle, 0, 0);
/*
	printf("-Framed-------------\n");
	dumpMAST(handle, 0);
	printf("--------------\n");
*/
}

/*
box
	"v"
	"="
	box
		"d"
		over
		"t"
*/

#ifdef DOX
/******* X STUFF *******/

void drawMAST(self, level, w)
	MAST *self;
	int level;
	Window w;
{
	MAST *mast;
	int i;
	XTextItem item;
	XFontStruct *theFont = mfinfo;/*XXX*/

	for (mast = self; mast; mast = mast->next) {
		if (!mast->type) return;

		for (i = 0; i < level; i++) printf("  ");
		printf("XX [%14s]\t%9s\t%3d(%d) %3d(%d) %3d %3d\n",
			mast->s, MInfoStr[mast->type], 
			mast->x, mast->rx, mast->y, mast->ry,
			mast->width, mast->height);

		switch (mast->type) {
		case MINFO_S_BOX:
		case MINFO_HBOX:
		case MINFO_VBOX:
		case MINFO_S_SUB:
		case MINFO_S_SUP:
			if (drawBoxP) {
				XDrawRectangle(display, w, gc_dash,
						mast->rx, mast->ry, 
						mast->width, mast->height);
				FLUSH;
			}
			drawMAST(mast->children, level+1, w);
		break;
		case MINFO_E_BOX:
			return;
		break;
		case MINFO_HDIV:
			XDrawLine(display, w, gc_fg, 
				  mast->rx, mast->ry + 1, 
				  mast->rx + mast->width - 1, mast->ry + 1);
			FLUSH;
		break;
		case MINFO_DATA:
			item.delta = 0;
			item.font = theFont->fid;
			item.chars = mast->s;
			item.nchars = strlen(mast->s);
			XDrawText(display, w, gc_fg, 
				  mast->rx + 1, mast->ry + mast->height - 2,
				  &item, 1);
			FLUSH;
		break;
		case MINFO_LPAREN: {
			int y0, y1, y2, y3, y4, y5, x0, x1, x2;
			float ylen = (float)(mast->height-2) / 9.0;
			float ylen3 = (float)(mast->height-2) / 3.0;
			float xlen = (float)PAREN_WIDTH / 3.0;

			y0 = mast->ry+1;
			y5 = y0 + mast->height-2;
			y1 = y0 + ylen;
			y2 = y0 + ylen3;
			y3 = y5 - ylen3;
			y4 = y5 - ylen;

			x0 = mast->rx + 1;
			x2 = x0 + PAREN_WIDTH;
			x1 = x2 - xlen;

			XDrawLine(display, w, gc_fg, x2, y0, x1, y1);
			XDrawLine(display, w, gc_fg, x1, y1, x0, y2);
			XDrawLine(display, w, gc_fg, x0, y2, x0, y3);
			XDrawLine(display, w, gc_fg, x0, y3, x1, y4);
			XDrawLine(display, w, gc_fg, x1, y4, x2, y5);

			FLUSH;

			drawMAST(mast->children, level+1, w);

/*			x2 = mast->rx + mast->width;*/
			x2 = mast->rx + mast->width - PAREN_WIDTH;
			x1 = x2 - xlen;
			x0 = x2 - PAREN_WIDTH;

			XDrawLine(display, w, gc_fg, x0, y0, x1, y1);
			XDrawLine(display, w, gc_fg, x1, y1, x2, y2);
			XDrawLine(display, w, gc_fg, x2, y2, x2, y3);
			XDrawLine(display, w, gc_fg, x2, y3, x1, y4);
			XDrawLine(display, w, gc_fg, x1, y4, x0, y5);

			FLUSH;
		} break;
		case MINFO_LBRACK: {
			int yt, yb, xl, xr;
			yt = mast->ry + 1;
			yb = mast->ry + mast->height - 2;
			xl = mast->rx;
			xr = mast->rx + BRACK_WIDTH - 1;
			XDrawLine(display, w, gc_fg, xr, yt, xl, yt);
			XDrawLine(display, w, gc_fg, xl, yt, xl, yb);
			XDrawLine(display, w, gc_fg, xl, yb, xr, yb);
			FLUSH;
			drawMAST(mast->children, level+1, w);
		} break;
		case MINFO_RBRACK: {
			int yt, yb, xl, xr;
			yt = mast->ry + 1;
			yb = mast->ry + mast->height - 2;
			xl = mast->rx;
			xr = mast->rx + BRACK_WIDTH - 1;
			XDrawLine(display, w, gc_fg, xl, yt, xr, yt);
			XDrawLine(display, w, gc_fg, xr, yt, xr, yb);
			XDrawLine(display, w, gc_fg, xr, yb, xl, yb);
			FLUSH;
		} break;
		case MINFO_INTEGRAL: {
			FPair *fpath;
			float fx, fy;
			int i, width, height, x, y, lx, ly, sx, sy;
/*
			int y0, y1, y2, y3, x0, x1, x2, x3, x4;
			float ylen = (float)(mast->height-2) / 9.0;
			float xlen = (float)INTEGRAL_WIDTH / 4.0;

			y0 = mast->ry+1;
			y3 = y0 + mast->height-2;
			y1 = y0 + ylen;
			y2 = y3 - ylen;

			x0 = mast->rx + 1;
			x1 = x0 + xlen;
			x2 = x0 + xlen * 2;
			x3 = x0 + xlen * 3;
			x4 = x0 + xlen * 4;

			XDrawLine(display, w, gc_fg, x4, y1, x3, y0);
			XDrawLine(display, w, gc_fg, x3, y0, x2, y0);
			XDrawLine(display, w, gc_fg, x2, y0, x1, y1);
			XDrawLine(display, w, gc_fg, x1, y1, x3, y2);
			XDrawLine(display, w, gc_fg, x3, y2, x2, y3);
			XDrawLine(display, w, gc_fg, x2, y3, x1, y3);
			XDrawLine(display, w, gc_fg, x1, y3, x0, y2);
*/
			sx = mast->rx + 1;
			sy = mast->ry + 1;
			fpath = fpath_integral;
			width = INTEGRAL_WIDTH - 2;
			height = mast->height - 2;
			lx = width - (fpath->x * width) + sx;
			ly = fpath->y * height + sy;
			fpathPts[0].x = lx;
			fpathPts[0].y = ly;
			for (i = 1; i < FPATH_COUNT_INTEGRAL-1; i++) {
				fpath++;
				x = width - (fpath->x * width) + sx;
				y = fpath->y * height + sy;
				fpathPts[i].x = x;
				fpathPts[i].y = y;
				/*XDrawLine(display, w, gc_fg, lx, ly, x, y);*/
				lx = x;
				ly = y;
			}
			XFillPolygon(display, w, gc_fg, fpathPts, i, 
					Complex, CoordModeOrigin);
			
			FLUSH;
			drawMAST(mast->children, level+1, w);
		} break;
		}
	}
}
#endif

int HTMLMathFormater(self, tokenListPk, dataListPk, listSize)
	VObj *self;
	Packet *tokenListPk;
	Packet *dataListPk;
	int listSize;
{
	int i, token;
	char *dataStr;
	Attr *tokAttrp, *dataAttrp, *attrp2;
	Packet *tokPk, *dataPk;
/*
drawMAST(handle, 0, w);
*/

	tokAttrp = tokenListPk->info.a;
	dataAttrp = dataListPk->info.a;

	printf("tokList:-------\n");
	dumpVarList(tokAttrp);
	printf("---------------\n");

	/* dependent on retro list entry order */
	for (i = listSize-1; i >= 0; i--) {
		tokPk = (Packet*)(tokAttrp->val);
		dataPk = (Packet*)(dataAttrp->val);

		printf("tokPk: ");
		dumpPacket(tokPk);
		printf("\n");
		printf("dataPk: ");
		dumpPacket(dataPk);
		printf("\n");

		token = PkInfo2Int(tokPk);
		dataStr = PkInfo2Str(dataPk);
		minfoBuff[i+1].type = token;
		minfoBuff[i+1].s = saveString(dataStr);
		tokAttrp = tokAttrp->next;
		dataAttrp = dataAttrp->next;
	}
	minfoBuff[0].type = MINFO_HBOX;
	minfoBuff[0].s = saveString("*START TheBOX"),/*wrapper*/

	minfoBuff[listSize+1].type = 0;
	minfoBuff[listSize+1].s = NULL;
		
/* must deref */
	
	make(minfoBuff);

	SET_width(self, handle->next->width+3);
	SET_height(self, handle->next->height+3);
	SET__content(self, handle);
	return 1;
}


int HTMLMathDraw(self)
	VObj *self;
{
	MAST *mast = (MAST *)(GET__content(self));
	Window w = GET_window(self);

	GLPrepareObjColor(self);
	drawMAST(mast, 0, w);
	return 1;
}

