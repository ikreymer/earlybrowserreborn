/*
 * class        : txtDisp
 * superClass	: txt
 */
#include "cl_txt.h"

extern ClassInfo class_txtDisp;
int helper_txtDisp_get();
int helper_txtDisp_set();

#define FORMAT_NONE	0
#define FORMAT_WRAP 	1
#define FORMAT_CENTER 	2
#define FORMAT_LEFT 	3
#define FORMAT_RIGHT 	4

int meth_txtDisp__getSelection();

int meth_txtDisp_append();
int meth_txtDisp_building_maxFontDescent();
int meth_txtDisp_building_maxFontHeight();
int meth_txtDisp_building_vspan();
int meth_txtDisp_charMask();
int meth_txtDisp_charButtonMask();
int meth_txtDisp_charHighLiteMask();
int meth_txtDisp_charUnderlineMask();
int meth_txtDisp_clearSelection();
int meth_txtDisp_clone();
int meth_txtDisp_clone2();
int meth_txtDisp_config();
int meth_txtDisp_currentChar();
int meth_txtDisp_currentLine();
int meth_txtDisp_currentTag();
int meth_txtDisp_currentWord();
int meth_txtDisp_cursorColumn();
int meth_txtDisp_cursorRow();
int meth_txtDisp_deHighLight();
int meth_txtDisp_drawCursor();
int meth_txtDisp_eraseCursor();
int meth_txtDisp_expose();
int meth_txtDisp_freeSelf();
int meth_txtDisp_get();
int meth_txtDisp_highLight();
int meth_txtDisp_initialize();
int meth_txtDisp_insert();
int meth_txtDisp_lineRowOffset();
int meth_txtDisp_nextTag();
int meth_txtDisp_numberOfLinesDisplayed();
int meth_txtDisp_previousTag();
int meth_txtDisp_processInput();
int meth_txtDisp_processKeyInput();
int meth_txtDisp_processMouseInput();
int meth_txtDisp_processMouseMove();
int meth_txtDisp_render();
int meth_txtDisp_selectionInfo();
int meth_txtDisp_set();
int meth_txtDisp_setSelection();
int meth_txtDisp_shownDepend();
int meth_txtDisp_totalLineCount();
int meth_txtDisp_setRevVideoFlag();

int helper_txtDisp_updateShownInfo();

int meth_txtDisp_test4();

