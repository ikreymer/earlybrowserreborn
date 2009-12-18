#include "utils.h"
#include "mystrings.h"
#include "error.h"
#include "file.h"
#include "hash.h"
#include "ident.h"
#include "obj.h"
#include "slotaccess.h"
#include "glib.h"

HashTable *window2Obj;	/* used to look up object associated with a window */

int maxNumOfFonts = MAXFONTS;
FontInfo fontInfo[MAXFONTS];
int lookAndFeel = LAF_VIOLA;

/* Scott: added dpy and scrn */
int init_glib(dpy, scrn)
    Display *dpy;
    Screen *scrn;
{
	if (!GLInit(dpy, scrn)) return 0;

	window2Obj = initHashTable(2047, hash_int, cmp_int, NULL, NULL,
				   getHashEntry_int, 
				   putHashEntry_int, 
				   putHashEntry_replace_int,
				   removeHashEntry_int);
	if (!window2Obj) return 0;
	return 1;
}

Window bossWindow(self)
	VObj *self;
{
	Window w;

	while (!(w = GET_window(self))) 
		if (!(self = GET__parent(self))) return rootWindow;

	return w;
}


void unMapObject(self)
	VObj *self;
{		 
	Window w = GET_window(self);

	if (w) {
		VObjList *olist;

		/* children's windows must go as well.. */
		for (olist = GET__children(self); olist; olist = olist->next)
			unMapObject(olist->o);

/*		callMethod(self, "closeWindow");*/
	}
}

/* 
 * given a window, find the object that it belongs to 
 */

int check = 1;

VObj *findWindowObject(w)
	Window w;
{
	static Window lastFailedSearchW = NULL;
	static Window lastSuccessSearchW = NULL;
	static VObj *lastSuccessSearchObj = NULL;
	VObj *obj;
	HashEntry *entry;

	if (w == lastFailedSearchW) return NULL;
	if (w == lastSuccessSearchW) {
/*		printf("////////////w=0x%x...\n",(long)w);*/

	  if (check) {
		if (validObjectP(lastSuccessSearchObj)) 
			return lastSuccessSearchObj;
	  } else {
			return lastSuccessSearchObj;
          }

		lastSuccessSearchW = NULL;
		return NULL;
	}
/*
	printf("find window=0x%x...",(long)(eventWindow(e)));
*/
	if (entry = window2Obj->get(window2Obj, w)) {
		lastSuccessSearchW = w;
		lastSuccessSearchObj = (VObj*)entry->val;

	  if (check) {
		if (validObjectP(lastSuccessSearchObj)) 
			return lastSuccessSearchObj;
	  } else {
			return lastSuccessSearchObj;
	  }
		lastSuccessSearchW = NULL;
		return NULL;
	}

	lastFailedSearchW = w;
/*	IERROR("findWindowObject(w = 0x%x = %ld): obj not found!\n", w, w);
*/

	return NULL;
}

#ifdef aldkadgskladgslkj

/*
 * set the VCurrentObj to the mouse clicked object.
 */
VObj *findNewObject(VObj *self) {

	VObj obj = NULL, nobj;
	VObj FCardObj, BCardObj, stackObj;

	stackObj = ObjStack(self);
	FCardObj = ObjCurrentCard(stackObj);
	BCardObj = ObjBCard(FCardObj);

	if (FCardObj)
		if (nobj = findClickedObj(FCardObj, mouse.x, mouse.y)) obj = nobj;
	if (BCardObj)
		if (nobj = findClickedObj(BCardObj, mouse.x, mouse.y)) obj = nobj;
	if (stackObj)
		if (nobj = findClickedObj(stackObj, mouse.x, mouse.y)) obj = nobj;

	return obj;
}

VObj findClickedObj(self, mx, my)
		VObj self;
		int mx, my;
{
	VObj obj, matchObj = NULL;
	OLIST *olist;

	if (!self) {
		printf("findClickedObj(self==NULL): ignored.\n");
		return NULL;
	}
	if (olist = ObjFieldList(self))
		if (matchObj = findClickedObjFromList(olist, mx, my))
			if (obj = findClickedObj(matchObj, mx, my)) return(obj);

	return matchObj;
}

VObj findClickedObjFromList(olist, mx, my)
		OLIST *olist;
		int mx, my;
{
	VObj matchObj = NULL;

	if (olist)
		if (olist->o) {
			Param *param;

			do {
	if ((param = ObjParam(olist->o)) == NULL) {
		fprintf(stderr, 
			"findClickedObjFromList: no param attribute in object '%s'\n",
			ObjName(olist->o));
		return NULL;
	}
	if (mx >= ParamSx(param) && mx <= ParamDx(param))
		if (my >= ParamSy(param) && my <= ParamDy(param))
			matchObj = olist->o;
	olist = olist->next;
			} while (olist);
		}
	return matchObj;
}


void renderObjectParam(self)
		 VObj self;
{
	if (self) {
		OLIST *olist;
		Param *param = ObjParam(self);

		drawRubberFrame(self,
				ParamSx(param)+1, ParamSy(param)+1,
				ParamDx(param)-1, ParamDy(param)-1);
		
		for (olist = ObjFieldList(self); olist; olist = olist->next) {
			renderObjectParam(olist->o);
		}
	}
}
#endif
