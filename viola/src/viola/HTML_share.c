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

/* a collection of C routines to replace script version of HTML tag handlers.
 * Placed here to be shared by various HTML tag handlers accelerators.
 */

int txtDisp_HTML_txt_expose(self, result, argv)
	VObj *self;
	Packet *result;
	Packet *argv;
{
	/*************************************************************
	 * just to cut thru usual() script etc... for faster response.
	 ************************************************************/	

	Packet *arg1 = (Packet*)((Attr*)(argv[0].info.a)->
			next->val);
	Packet *arg2 = (Packet*)((Attr*)(argv[0].info.a)->
			next->next->val);
	Packet *arg3 = (Packet*)((Attr*)(argv[0].info.a)->
			next->next->next->val);
	Packet *arg4 = (Packet*)((Attr*)(argv[0].info.a)->
			next->next->next->next->val);

	GLPrepareObjColor(self);
	return tfed_expose(self, arg1->info.i, arg2->info.i, 
		 	  	 arg3->info.i, arg4->info.i);
}

int txtDisp_HTML_txt_buttonRelease(self, result, argv)
	VObj *self;
	Packet *result;
	Packet *argv;
{
	/*************************************************************
	 * case "buttonRelease":
	 *	if (charButtonMask() > 0) {
	 *		ref = HTTPDecodeURL(nextTag());
	 *		if (ref) {
	 *			if (shiftKeyP()) {
	 *				www.mesg.tf("show",
	 *					concat("Dest: ", ref));
	 *			} else {
	 *				test4(1);
	 *				send(send(parent(), "findTop"), 
	 *					"follow_href", ref);
	 *				test4(0);
	 *			}
	 *		}
	 *	}
	 *	return;
	 * break;
	 ************************************************************/	

	TFStruct *tf = updateEStrUser(self);
	char *ref;

	freePossibleDangler(result);
	if (tf && (tfed_get_charMask(tf) & MASK_BUTTON)) {
		ref = tfed_get_nextTag(tf);
		if (ref && *ref) {
			ref = decodeURL(ref);
			if (keyStat_shift) {
				static VObj *mesgObj = NULL;
				char *mesg;

				if (mesgObj) {
					mesg = (char*)malloc(sizeof(char)
						* (strlen(ref) + 10));
					strcpy(mesg, "Dest: ");
					strcat(mesg, ref);
					sendMessage1N1str(mesgObj, 
							"show", mesg);
					free(mesg);
				} else {
					mesgObj = findObject(getIdent(
						"www.mesg.tf"));
				}
			} else {
				VObj *topObj;
				Packet evalResult;

				nullPacket(&evalResult);

				GLPrepareObjColor(self);
				tfed_setReliefMaskInButtonRange(tf,1);
				drawLineOffset(tf, tf->current_row - 
						   tf->screen_row_offset, 0);
				XFlush(display);

				sendMessage1_result(GET__parent(self), 
						    "findTop", &evalResult);
				topObj = PkInfo2Obj(&evalResult);
				clearPacket(&evalResult);
				if (!topObj) {
					free(ref);
					return 0;
				}
				sendMessage1N1str(topObj, "follow_href", ref);
				
				if (!callObjStack[callObjStackIdx].destroyed) {
				  GLPrepareObjColor(self);
				  tfed_setReliefMaskInButtonRange(tf,0);
				  drawLineOffset(tf, tf->current_row - 
						 tf->screen_row_offset, 1);
				  XFlush(display);
				}
			}
			free(ref);
		}
	}
	return 1;
}

int txtDisp_HTML_txt_clone(self, result, argv)
	VObj *self;
	Packet *result;
	Packet *argv;
{
	/*************************************************************
	 * case "clone":
	 *	return clone(cloneID++);
	 * break;
	 ************************************************************/

		Packet pak;
		extern int global_cloneID;

		pak.info.i = global_cloneID++;
		pak.type = PKT_INT;
		pak.canFree = 0;

		return meth_txtDisp_clone(self, result, 1, &pak);

/*	Packet *arg1 = (Packet*)((Attr*)(argv[0].info.a)->next->val);
	int stat;
	stat = meth_txtDisp_clone(self, result, 1, arg1);
	return stat;
*/
}

int txtDisp_HTML_header_D(self, result, argv)
	VObj *self;
	Packet *result;
	Packet *argv;
{
	/*************************************************************
	 * case 'D':
	 *	set("content", get("label"));
	 *	return (building_vspan() + 2);
	 * break;
	 ************************************************************/
	/* Equivalent C code:
	 */
	TFStruct *tf = GET__TFStruct(self);
	Attr *varlist = GET__varList(self);
	Packet *pk;
	char *text, *s;
	SGMLTagMappingInfo *tmi;
	int vspan;

	text = GET_label(self);

	if (*text != '\0') {
		if (GET_content(self)) 
			Vfree(GET__memoryGroup(self), GET_content(self));

		/* eliminate leading space */
		for (s = text; *s; s++) if (isprint(*s)) break;

		SET_content(self, s);
		SET_label(self, NULL);

		tf = tfed_updateTFStruct(self, s);
		if (!tf) return 0;

/*		vspan = tf->building_vspan + tf->yUL + 2;*/
		vspan = tf->building_vspan;

		freePossibleDangler(result);

		tmi = (SGMLTagMappingInfo*)GET__content2(self);
				
/*		result->info.i = tmi->top + vspan + tmi->bottom;*/
		result->info.i = vspan;
		result->type = PKT_INT;
	}
	return 1;
}

/* tag can be: H1, H2, ...
 */
int txtDisp_HTML_header_R(self, result, argv, tag)
	VObj *self;
	Packet *result;
	Packet *argv;
	char *tag;
{
	/*************************************************************
	 * case 'R':
	 * 	clearWindow();
	 * 	style = SGMLGetStyle("HTML", "H1");
	 *	vspan = style[0];
	 *	set("y", arg[1] + vspan);
	 *	set("x", style[2]);
	 *	set("width", arg[2] - x() - style[3]);
	 *	set("content", get("content"));
	 *	vspan = vspan + set("height", building_vspan()) + style[1];
	 *	render();
	 *	return vspan;
	 * break;
	 ************************************************************/

	Packet *arg1 = (Packet*)((Attr*)(argv[0].info.a)->
			next->val);
	Packet *arg2 = (Packet*)((Attr*)(argv[0].info.a)->
			next->next->val);
	TFStruct *tf = GET__TFStruct(self);
	SGMLDocMappingInfo *dmi = NULL;
	SGMLTagMappingInfo *tmi;
	XWindowChanges wc;
	int vspan;

	tmi = (SGMLTagMappingInfo*)GET__content2(self);
	if (!tmi) {
		int i;
		for (i = 0; SGMLForms[i].DTDName; i++) {
			if (!STRCMP(SGMLForms[i].DTDName, "HTML")) {
				dmi = &SGMLForms[i];
				break;
			}
		}
		tmi = findTMI(dmi->tagMap, tag);
		SET__content2(self, tmi);
	}

	vspan = tmi->top;

	SET_y(self, arg1->info.i + vspan);
	SET_x(self, tmi->left);
	SET_width(self, arg2->info.i - tmi->left - tmi->right);

	/*XXX need to track down the real problem and get rid of the
	 * wasted processing */
	if (tf->firstp) {
	  char *s = convertNodeLinesToStr(self, tf->firstp);
	  tf = tfed_updateTFStruct(self, s);
	  free(GET_content(self)); /* probably not necessary... */
	  SET_content(self, s);
	} else {
	  /*this doesn't work b/c of images stored in temp files...*/
	  tf = tfed_updateTFStruct(self, GET_content(self));
	}

	if (!tf) return 0;

	SET_height(self, tf->building_vspan + tf->yUL + 2);
	/* +2 for hot-link-indicator border around IMGs */

	if (GET_window(self)) {
		wc.x = GET_x(self);
		wc.y = GET_y(self);
		wc.width = GET_width(self);
		wc.height = GET_height(self);
		XConfigureWindow(display, GET_window(self), 
				CWX | CWY | CWWidth | CWHeight, &wc);
	}

	vspan += GET_height(self) + tmi->bottom;

	meth_txtDisp_render(self, result, 0, result);

	result->info.i = vspan;
	result->type = PKT_INT;
	result->canFree = 0;
	return 1;
}

/* 
 */
int txtDisp_HTML_header_A(self, result, argv)
	VObj *self;
	Packet *result;
	Packet *argv;
{
	/******************************************************************
	 * case 'A':
	 *	if (text) {
	 *		text = concat(text, send(arg[1], 'i'), '\n');
	 *	} else {
	 *		text = send(arg[1], 'i');
	 *	}
	 * 		return 0;
	 * break;
	 ****************************************************************/

	Attr *varlist;
	char *inset;
	Packet *arg1 = (Packet*)((Attr*)(argv[0].info.a)->next->val);
	VObj *obj;
	Packet *pk;
	Packet evalResult;

	obj = PkInfo2Obj(arg1);
	if (!obj) return 0;

	nullPacket(&evalResult);

	sendMessage1chr_result(obj, 'i', result);
	if (evalResult.canFree & PK_CANFREE_STR) {
		inset = evalResult.info.s;
	} else {
		inset = saveString(evalResult.info.s);
	}

	varlist = GET__varList(self);
	pk = (Packet*)getVariable_id(varlist, STR_text);
	if (pk) {
		if (pk->info.s) {
			strcpy(buff, pk->info.s);
			if (inset) strcat(buff, inset);
			SET__varList(self, 
				setVariable_id_STR(varlist, STR_text,
					     saveString(buff), 1));
			free(inset);
		} else {
			SET__varList(self, 
				setVariable_id_STR(varlist, STR_text,
					     inset, 1));
		}
	} else {
		SET__varList(self, setVariable_id_STR(varlist, STR_text, 
						inset, 1));
	}
	result->info.i = 0;
	result->type = PKT_INT;
	result->canFree = 0;
	return 1;
} 


/******************************************************************
 * table stuff
 */
typedef struct TableMatrixCell {
	int type;
	int x, y, width, height;
	float colSpan, rowSpan;
	VObj *obj;
} TableMatrixCell;

#define TABLE_CELL_HEIGHT_TB 5

#define TABLE_CELL_TYPE_PADDING 1
#define TABLE_CELL_TYPE_TR 2
#define TABLE_CELL_TYPE_TB 4
#define TABLE_CELL_TYPE_TH 8
#define TABLE_CELL_TYPE_TD 16
#define TABLE_CELL_TYPE_THTD 24
#define TABLE_CELL_TYPE_TCAP 32

int cellType(self)
	VObj *self;
{
	Packet *pk;
	pk = getVariable_id(GET__varList(self), STR_cellType);
	if (pk) return pk->info.i;
	return 0;
}

float getRowSpan(self)
	VObj *self;
{	
	Packet *pk;
	float i;

	pk = (Packet*)getVariable_id(GET__varList(self), STR_rowSpan);
	if (pk) {
		i = PkInfo2Flt(pk);
		if (i > 0) return i;
	}
	return 1.0;
}

float getColSpan(self)
	VObj *self;
{	
	Packet *pk;
	float i;

	pk = (Packet*)getVariable_id(GET__varList(self), STR_colSpan);
	if (pk) {
		i = PkInfo2Flt(pk);
		if (i > 0) return i;
	}
	return 1.0;
}

void dumpMatrix(matrix, matCols, matRows, mode)
	TableMatrixCell *matrix;
	int matCols, matRows;
	int mode;
{
	int i, j;
	TableMatrixCell *mat;

	for (j = 0; j < matRows; j++) {
		printf("%d: ", j);
		for (i = 0; i < matCols; i++) {
			mat = &matrix[j * matCols + i];
			if (mat->type & TABLE_CELL_TYPE_THTD ||
			    mat->type & TABLE_CELL_TYPE_TB) {
				if (mode == 1) {
					printf("[%d](%d: \"%s\" %d,%d,%d,%d) ", 
					i, mat->type, 
					GET_name(mat->obj),
					mat->x, mat->y, 
					mat->width, mat->height);
				} else {
					printf("[%d](%d: %d,%d,%d,%d) ", 
					i, mat->type, 
					mat->x, mat->y, 
					mat->width, mat->height);
				}
			}
		}
		printf("\n");
	}
}

#define MAX_TABLE_ROWS 500

int HTMLTableFormater(self, titleObj, titleTopP)
	VObj *self;
	VObj *titleObj;
	int titleTopP;
{
	int i, j, r, c, row, col;
	int reg[MAX_TABLE_ROWS];/* limit on number of rows */
	int rowIdx = 0, rowCount = 0, elemCount = 0;
	VObjList *olist;
	TableMatrixCell *mat, *matrix;
	int matCols, matRows;
	int maxCols;
	int x, y, width, height, xspan, yspan;
	float rowSpan, colSpan;
	int type, lastCellType = 0;
	Packet evalResult;

	nullPacket(&evalResult);

	width = GET_width(self);

	/* scan table to find max matrix boundary; create matrix; init matrix.
	 */
	olist = GET__children(self);
	if (!olist) return 0;

	bzero(reg, sizeof(int) * MAX_TABLE_ROWS);

	for (; olist; olist = olist->next) {

		type = cellType(olist->o);
		if (type & TABLE_CELL_TYPE_TR) {
			rowIdx++;
			if (rowCount <= rowIdx) rowCount = rowIdx;
		} else if (type & TABLE_CELL_TYPE_TB) {
			/* Questionable: here TB has effect of TR,bar,TR
			 */
			rowIdx++;
			if (rowCount <= rowIdx) rowCount = rowIdx;
			reg[rowIdx++] = 1;
			if (rowCount <= rowIdx) rowCount = rowIdx;
		} else if (type & TABLE_CELL_TYPE_THTD) {
			rowSpan = getRowSpan(olist->o);
			colSpan = getColSpan(olist->o);

			if (rowSpan == 1.0 && colSpan == 1.0) {
				reg[rowIdx]++;
			} else {
				for (r = rowIdx; r < rowIdx + rowSpan; r++) {
					reg[r] += colSpan;
				}
				if (rowCount < rowIdx + rowSpan) 
					rowCount = rowIdx + rowSpan;
			}
		} else if (type & TABLE_CELL_TYPE_TCAP) {
			SET_width(olist->o, width);
			if (GET_window(olist->o))
				GLUpdateSize(!GET__parent(olist->o),
					GET_window(olist->o),
					GET_width(olist->o) - 1,
					GET_height(olist->o));
		}
		lastCellType = type;
	}
	/* to handle case where final row isn't terminated with TR */
	if (lastCellType != TABLE_CELL_TYPE_TR) {
		rowIdx++;
		if (rowCount <= rowIdx) rowCount = rowIdx;
	}

	matRows = rowCount;
	maxCols = 0;
	for (i = 0; i < rowCount; i++) {
		/*printf("row:%d  col <= %d\n", i, reg[i]);*/
		if (maxCols < reg[i]) maxCols = reg[i];
	}

	matrix = (TableMatrixCell*)malloc(sizeof(struct TableMatrixCell) *
						matRows * maxCols);
	for (i = matRows * maxCols - 1; i >= 0; i--) {
		mat = &matrix[i];
		mat->obj = NULL;
		mat->type = NULL;
		mat->x = 0;
		mat->y = 0;
		mat->width = 0;
		mat->height = 0;
	}

	/* place objects onto appropriate matrix cell
	 * and figure out actual columns (with colspan accounted)
	 */
	matCols = 0;
	olist = GET__children(self);
	for (row = 0; row < matRows;) {
		for (col = 0; col < maxCols; col++) {
			if (!olist) goto done;
			mat = &matrix[row * maxCols + col];
repeat:	
			type = cellType(olist->o);
			if (type & TABLE_CELL_TYPE_THTD) {

			    if (mat->type == 0) {

				rowSpan = mat->rowSpan = getRowSpan(olist->o);
				colSpan = mat->colSpan = getColSpan(olist->o);

				mat->obj = olist->o;

				if (rowSpan == 1.0 && colSpan == 1.0) {
					mat->type = TABLE_CELL_TYPE_THTD;
				} else {
					TableMatrixCell *matp;

			for (r = row; r < row + rowSpan; r++) {
				for (c = col; c < col + colSpan; c++) {
					matp = &matrix[r * maxCols + c];
					matp->type = TABLE_CELL_TYPE_PADDING;
/*printf("%%%%% padding row=%d col=%d \n", r, c);*/
				}
/*				if (matCols < c) matCols = c;*/
			}
					mat->type = TABLE_CELL_TYPE_THTD;
				  }
				  olist = olist->next;
			    }
			    if (matCols < col + colSpan) 
			      matCols = col + colSpan;
/*printf("%%%%% matCols=%d\n", matCols);*/

			} else if (type & TABLE_CELL_TYPE_TR) {
				olist = olist->next;
				goto nextrow;
			} else if (type & TABLE_CELL_TYPE_TB) {
				mat->obj = olist->o;
				mat->type = TABLE_CELL_TYPE_TB;
				olist = olist->next;
				goto nextrow;
			} else if (type & TABLE_CELL_TYPE_PADDING) {
				if (matCols < col + colSpan) 
					matCols = col + colSpan;
			} else {
				/* unknown object */
				olist = olist->next;
				if (olist) goto repeat;
			}
		}

		if (olist && (cellType(olist->o) & TABLE_CELL_TYPE_TR))
			olist = olist->next;

		nextrow:
		row++;
	}
	done:
/*
printf("XXX matRows=%d\n", matRows);
printf("XXX matCols=%d\n", matCols);
	printf("1===================== cols=%d rows=%d\n", maxCols, matRows);
	dumpMatrix(matrix, maxCols, matRows, 1);
	printf("=============\n");
*/
	/* set x, width
	 * and **let objects format, to get height
	 */
	xspan = width / maxCols;

	for (row = 0; row < matRows; row++) {
		reg[row] = 0; /* use this to record max height */
		x = 0;
		for (col = 0; col < maxCols; col++) {
			mat = &matrix[row * maxCols + col];	/*XXX*/
			if (mat->type & TABLE_CELL_TYPE_THTD) {
				mat->x = x;
				mat->width = (float)xspan * mat->colSpan - 1;
				SET_x(mat->obj, x);
				SET_width(mat->obj, mat->width);

				/* format to get height */
				sendMessage1chr_result(mat->obj, 'r',
						       &evalResult);
				i = PkInfo2Int(&evalResult);
				clearPacket(&evalResult);

/*print("@@@ col=%d row=%d maxHeight=%d height=%d\n", col, row, reg[row], i);*/

				if (i > reg[row]) reg[row] = i;
			} else if (mat->type & TABLE_CELL_TYPE_TB) {
				i = TABLE_CELL_HEIGHT_TB;
				if (i > reg[row]) reg[row] = i;
			}
			x += xspan;
		}
	}

	/* set y, and finalize height
	 */
	if (titleObj && titleTopP) y = GET_height(titleObj);
	else y = 0;

	for (row = 0; row < matRows; row++) {
		for (col = 0; col < maxCols; col++) {
			mat = &matrix[row * maxCols + col];	/*XXX*/
			if (mat->type & TABLE_CELL_TYPE_THTD) {
				mat->y = y;
				if (mat->rowSpan > 1) {
					for (i = 0; i < mat->rowSpan; i++) {
						mat->height += reg[i + row];
					}
				} else {
					mat->height = reg[row];
				}

				SET_y(mat->obj, y);
				SET_height(mat->obj, mat->height);

/*print("/// col=%d row=%d maxHeight=%d\n", col, row, reg[row]);*/

				if (GET_window(mat->obj)) {
				  GLUpdateGeometry(!GET__parent(mat->obj),
						   GET_window(mat->obj),
						   GET_x(mat->obj), 
						   GET_y(mat->obj),
						   GET_width(mat->obj),
						   GET_height(mat->obj));
				}
			} else if (mat->type & TABLE_CELL_TYPE_TB) {
/*				mat->height -= 2;
				mat->y = y;
				mat->height = TABLE_CELL_HEIGHT_TB;
*/
/*
				SET_y(mat->obj, y);
				SET_height(mat->obj, mat->height);
				if (GET_window(mat->obj)) {
				  GLUpdatePosition(!GET__parent(mat->obj),
						   GET_window(mat->obj),
						   GET_x(mat->obj), 
						   GET_y(mat->obj));
				}
*/
			}
		}
		y += reg[row];
	}

	if (titleObj && !titleTopP) {
		SET_y(titleObj, y);
		y += GET_height(titleObj) + 2;
	}
/*
	printf("2==================== cols=%d rows=%d\n", maxCols, matRows);
	dumpMatrix(matrix, maxCols, matRows, 1);
	printf("=============\n");
	printf("y=%d \n", y);
*/
	SET_height(self, y);

	free(matrix);

	return 1;
}

