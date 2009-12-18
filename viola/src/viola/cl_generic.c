/*
 * Copyright 1991 Pei-Yuan Wei.  All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
/*
 * cl_generic.c
 *
 * class	: generic
 * superClass	: cosmic
 */
#include "utils.h"
#include <ctype.h>
#include <sys/file.h>
#include <stdlib.h>
#include "error.h"
#include "file.h"
#include "hash.h"
#include "mystrings.h"
#include "sys.h"
#include "ident.h"
#include "scanutils.h"
#include "obj.h"
#include "vlist.h"
#include "attr.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "slotaccess.h"
#include "cexec.h"
#include "classlist.h"
#include "cl_generic.h"
#include "misc.h"
#include "glib.h"
#include "tfed.h"
#include "event.h"
#include "sgml.h"
#include "html.h"
#include "html2.h"
#include "ast.h"
#include "cgen.h"

#include "stgcall.h"
#include "../libWWW/Library/Implementation/HTParse.h"
#include "../libWWW/Library/Implementation/HTTP.h"

#ifdef hpux
#include <time.h>
#else hpux
#include <sys/time.h>
#endif hpux

#include "HTML_share.h"

extern int notSecure();

extern double cos(), sin();
int flag_cliprompt = 1;

#define DEBUG FALSE
#define RADIAN_TO_DEGREE_RATIO 0.017453293

struct StrInfo init_obj_script; /* initialized in class.c */

SlotInfo cl_generic_NCSlots[] = {
	0
};
SlotInfo cl_generic_NPSlots[] = {
{
	STR_name,
	PTRS | SLOT_RW,
	(long)""
},{
	STR_parent,
	PTRS | SLOT_RW,
	(long)""
},{
	STR__parent,
	OBJP,
	NULL
},{
	STR_children,
	PTRS | SLOT_RW,
	(long)""
},{
	STR__children,
	OBJL,
	NULL
},{
	STR_security,
	LONG,
	0,
},{
	STR_script,
	STRI | SLOT_RW,
/*	(long)&init_obj_script,*/
	(long)"usual();"
},{
	STR__script,
	PCOD,
	NULL,
},{
	STR__varList,
	ATTR,
	NULL
},{
	STR__argAttr,
	ATTR,
	NULL
},{
	STR__classScriptVV,
	PTRV,
	NULL
},{
	STR__scriptVV,
	PTRV,
	NULL
},{
	STR__tempScriptVV,
	PTRV,
	NULL
},{
	STR_active,
	LONG | SLOT_RW,
	(long)0
},{
	0
}
};
SlotInfo cl_generic_CSlots[] = { /* COMMON SLOTS */
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"generic"
},{
	0
}
};
SlotInfo cl_generic_PSlots[] = { /* PRIVATE SLOTS */
{
	STR__classInfo,
	CLSI,
	(long)&class_generic
},{
	0
}
};

SlotInfo *slots_generic[] = {
	(SlotInfo*)cl_generic_NCSlots,
	(SlotInfo*)cl_generic_NPSlots,
	(SlotInfo*)cl_generic_CSlots,
	(SlotInfo*)cl_generic_PSlots
};

MethodInfo meths_generic[] = {
{
	STR_GB_copy,
	meth_generic_GB_copy
},{
	STR_GB_data,
	meth_generic_GB_data
},{
	STR_GB_char,
	meth_generic_GB_char
},{
	STR_GB_count,
	meth_generic_GB_count
},{
	STR_GB_create,
	meth_generic_GB_create
},{
	STR_GB_decChar,
	meth_generic_GB_decChar
},{
	STR_GB_decChar1,
	meth_generic_GB_decChar1
},{
	STR_GB_decLine,
	meth_generic_GB_decLine
},{
	STR_GB_free,
	meth_generic_GB_free
},{
	STR_GB_herePtr,
	meth_generic_GB_herePtr
},{
	STR_GB_incChar,
	meth_generic_GB_incChar
},{
	STR_GB_incChar1,
	meth_generic_GB_incChar1
},{
	STR_GB_incLine,
	meth_generic_GB_incLine
},{
	STR_GB_line,
	meth_generic_GB_line
},{
	STR_GB_moveToChar,
	meth_generic_GB_moveToChar
},{
	STR_GB_moveToLine,
	meth_generic_GB_moveToLine
},{
	STR_GB_moveToStart,
	meth_generic_GB_moveToStart
},{
	STR_GB_nthChar,
	meth_generic_GB_nthChar
},{
	STR_GB_nthLine,
	meth_generic_GB_nthLine
},{
	STR_GB_set,
	meth_generic_GB_set
},{
	STR_GB_setCopy,
	meth_generic_GB_setCopy
},{
	STR_HTTPCanonicalURL,
	meth_generic_HTTPCanonicalURL,
},{
	STR_HTTPCurrentDocAddr,
	meth_generic_HTTPCurrentDocAddr,
},{
	STR_HTTPCurrentDocAddrParsed,
	meth_generic_HTTPCurrentDocAddrParsed,
},{
	STR_HTTPCurrentDocAddrSet,
	meth_generic_HTTPCurrentDocAddrSet,
},{
	STR_HTTPDecodeURL,
	meth_generic_HTTPDecodeURL
},{
	STR_HTTPEncodeURL,
	meth_generic_HTTPEncodeURL
},{
	STR_HTTPGet,
	meth_generic_HTTPGet
},{
	STR_HTTPGetNParse,
	meth_generic_HTTPGetNParse
},{
	STR_HTTPPost,
	meth_generic_HTTPPost
},{
	STR_HTTPSubmit,
	meth_generic_HTTPSubmit
},{
	STR_HTTPHotListAdd,
	meth_generic_HTTPHotListAdd
},{
	STR_HTTPHotListDelete,
	meth_generic_HTTPHotListDelete
},{
	STR_HTTPHotListChange,
	meth_generic_HTTPHotListChange
},{
	STR_HTTPHotListGet,
	meth_generic_HTTPHotListGet,
},{
	STR_HTTPHotListLoad,
	meth_generic_HTTPHotListLoad,
},{
	STR_HTTPHotListSave,
	meth_generic_HTTPHotListSave,
},{
	STR_SGMLBuildDoc,
	meth_generic_SGMLBuildDoc
},{
	STR_SGMLBuildDocB,
	meth_generic_SGMLBuildDocB
},{
	STR_SGMLBuildDoc_flush,
	meth_generic_SGMLBuildDoc_flush,
},{
	STR_SGMLBuildDoc_insertObj,
	meth_generic_SGMLBuildDoc_insertObj,
},{
	STR_SGMLBuildDoc_setBuff,
	meth_generic_SGMLBuildDoc_setBuff,
},{
	STR_SGMLBuildDoc_setColors,
	meth_generic_SGMLBuildDoc_setColors,
},{
	STR_SGMLBuildDoc_span,
	meth_generic_SGMLBuildDoc_span,
},{
	STR_SGMLFindAnchorOffset,
	meth_generic_SGMLFindAnchorOffset
},{
	STR_SGMLMathFormater,
	meth_generic_SGMLMathFormater,
},{
	STR_SGMLTileDoc,
	meth_generic_SGMLTileDoc
},{
	STR_SGMLReBuildDoc,
	meth_generic_SGMLReBuildDoc
},{
	STR_SGMLGetStyle,
	meth_generic_SGMLGetStyle,
},{
	STR_SGMLSetStyle,
	meth_generic_SGMLSetStyle,
},{
	STR_SGMLTableFormater,
	meth_generic_SGMLTableFormater,
},{
	STR_STGInfo,
	meth_generic_STGInfo
},{
	STR_STG_tagPtr,
	meth_generic_STG_tagPtr
},{
	STR_STG_attr,
	meth_generic_STG_attr
},{
	STR_STG_clean,
	meth_generic_STG_clean
},{
	STR_accessible,
	meth_generic_accessible
},{
	STR_activeHelp,
	meth_generic_activeHelp
},{
	STR_addPicFromFile,
	meth_generic_addPicFromFile
},{
	STR_after,
	meth_generic_after
},{
	STR_alarm,
	meth_generic_alarm
},{
	STR_append,
	meth_generic_append
},{
	STR_argument,
	meth_generic_argument
},{
	STR_ascii,
	meth_generic_ascii
},{
	STR_asciiVal,
	meth_generic_asciiVal
},{
	STR_bell,
	meth_generic_bell
},{
	STR_bellVolume,
	meth_generic_bellVolume
},{
	STR_char,
	meth_generic_char
},{
	STR_clear,
	meth_generic_clear
},{
	STR_cli,
	meth_generic_cli
},{
	STR_clone,
	meth_generic_clone
},{
	STR_clone2,
	meth_generic_clone2
},{
	STR_compressSpaces,
	meth_generic_compressSpaces
},{
	STR_concatenate,
	meth_generic_concatenate
},{
	STR_concat,				/* alias */
	meth_generic_concatenate
},{
	STR_concatList,		
	meth_generic_concatList
},{
	STR_cos,
	meth_generic_cos
},{
	STR_countChars,				/* alias */
	meth_generic_strlen
},{
	STR_countChildren,
	meth_generic_countChildren
},{
	STR_countItems,
	meth_generic_countItems
},{
	STR_countLines,
	meth_generic_countLines
},{
	STR_countWords,
	meth_generic_countWords
},{
	STR_ctrlKeyP,
	meth_generic_ctrlKeyP
},{
	STR_cursorShape,
	meth_generic_cursorShape
},{
	STR_date,
	meth_generic_date
},{
	STR_deepObjectListSend,
	meth_generic_deepObjectListSend
},{
	STR_defineNewFont,
	meth_generic_defineNewFont
},{
	STR_delay,
	meth_generic_delay
},{
	STR_deleteFile,
	meth_generic_deleteFile
},{
	STR_deleteSubStr,
	meth_generic_deleteSubStr,
},{
	STR_deleteSubStrQ,
	meth_generic_deleteSubStrQ,
},{
	STR_depth,
	meth_generic_depth
},{
	STR_destroyVariable,
	meth_generic_destroyVariable
},{
	STR_environVar,
	meth_generic_environVar
},{
	STR_filter,
	meth_generic_filter,
},{
	STR_findPattern,
	meth_generic_findPattern,
},{
	STR_float,
	meth_generic_float
},{
	STR_format,
	meth_generic_format,
},{
	STR_freeSelf,
	meth_generic_freeSelf
},{
	STR_geta,
	meth_generic_get
},{
	STR_getResource,
	meth_generic_getResource
},{
	STR_getSelection,
	meth_generic_getSelection
},{
	STR_getVariable,
	meth_generic_getVariable
},{
	STR_gravity,
	meth_generic_gravity
},{
	STR_hash,
	meth_generic_hash
},{
	STR_height,
	meth_generic_height
},{
	STR_initialize,
	meth_generic_initialize
},{
	STR_int,
	meth_generic_int
},{
	STR_item,
	meth_generic_item
},{
	STR_isBlank,
	meth_generic_isBlank
},{
	STR_key,
	meth_generic_key
},{
	STR_listAllObjects,
	meth_generic_listAllObjects
},{
	STR_loadFile,
	meth_generic_loadFile
},{
	STR_loadSTG,
	meth_generic_loadSTG
},{
	STR_makeTempFile,
	meth_generic_makeTempFile
},{
	STR_not,
	meth_generic_not
},{
	STR_nthChar,
	meth_generic_nthChar
},{
	STR_nthChild,
	meth_generic_nthChild
},{
	STR_nthItem,
	meth_generic_nthItem
},{
	STR_nthLine,
	meth_generic_nthLine
},{
	STR_nthObjectInList,
	meth_generic_nthObjectInList,
},{
	STR_nthSibling,
	meth_generic_nthSibling
},{
	STR_nthWord,
	meth_generic_nthWord
},{
	STR_objectListAppend,
	meth_generic_objectListAppend
},{
	STR_objectListAppend_children,
	meth_generic_objectListAppend_children
},{
  STR_objectListCount,
	meth_generic_objectListCount
},{
	STR_objectListCount_children,
	meth_generic_objectListCount_children
},{
	STR_objectListDelete,
	meth_generic_objectListDelete
},{
	STR_objectListDelete_children,
	meth_generic_objectListDelete_children
},{
	STR_objectListPrepend,
	meth_generic_objectListPrepend
},{
	STR_objectListPrepend_children,
	meth_generic_objectListPrepend_children
},{
	STR_objectListSend,
	meth_generic_objectListSend
},{
	STR_objectListSend_children,
	meth_generic_objectListSend_children
},{
	STR_objectPosition,
	meth_generic_objectPosition
},{
	STR_parent,
	meth_generic_parent
},{
	STR_pipe,
	meth_generic_pipe
},{
	STR_prepend,
	meth_generic_prepend
},{
	STR_print,
	meth_generic_print
},{
	STR_printf,
	meth_generic_printf
},{
	STR_random,
	meth_generic_random
},{
	STR_replaceChar,
	meth_generic_replaceChar
},{
	STR_replaceCharQ,
	meth_generic_replaceCharQ
},{
	STR_replaceStr,
	meth_generic_replaceStr
},{
	STR_replaceStrQ,
	meth_generic_replaceStrQ
},{
	STR_saveFile,
	meth_generic_saveFile
},{
	STR_scanf,
	meth_generic_scanf,
},{
	STR_securityMode,
	meth_generic_securityMode,
},{
	STR_self,
	meth_generic_self
},{
	STR_selectionInfo,
	meth_generic_selectionInfo
},{
	STR_sendToInterface,
	meth_generic_sendToInterface
},{
	STR_seta,
	meth_generic_set
},{
	STR_setMouse,
	meth_generic_setMouse
},{
	STR_setResource,
	meth_generic_setResource
},{
	STR_setSelection,
	meth_generic_setSelection
},{
	STR_setVariable,
	meth_generic_setVariable
},{
	STR_shiftKeyP,
	meth_generic_shiftKeyP
},{
	STR_sin,
	meth_generic_sin
},{
	STR_sleep,
	meth_generic_sleep
},{
	STR_sprintf,
	meth_generic_sprintf
},{
	STR_str,
	meth_generic_str
},{
	STR_strlen,
	meth_generic_strlen
},{
	STR_system,
	meth_generic_system
},{
	STR_target,
	meth_generic_target
},{
	STR_textWidth,
	meth_generic_textWidth
},{
	STR_trimEdge,
	meth_generic_trimEdge
},{
	STR_trimEdgeQ,
	meth_generic_trimEdgeQ
},{
	STR_target,
	meth_generic_target
},{
	STR_time,
	meth_generic_time
},{
	STR_tool,
	meth_generic_tool
},{
	STR_unhash,
	meth_generic_unhash
},{
	STR_version,
	meth_generic_version
},{
	STR_violaPath,
	meth_generic_violaPath
},{
	STR_x,
	meth_generic_x
},{
	STR_watch,
	meth_generic_watch
},{
	STR_width,
	meth_generic_width
},{
	STR_write,
	meth_generic_print		/* alias */
},{
	STR_writeln,
	meth_generic_writeln
},{
	STR_y,
	meth_generic_y
},{
	STR_code_HTML_txt,
	meth_generic_code_HTML_txt,
},{
	STR_code_HTML_txtAnchor,
	meth_generic_code_HTML_txtAnchor,
},{
	STR_code_HTML_header_large,
	meth_generic_code_HTML_header_large,
},{
	STR_code_HTML_header_medium,
	meth_generic_code_HTML_header_medium,
},{
	STR_code_HTML_header_small,
	meth_generic_code_HTML_header_small,
},{
	0
}
};

ClassInfo class_generic = {
	helper_generic_get,
	helper_generic_set,
	slots_generic,		/* class slot information	*/
	meths_generic,		/* class methods		*/
	STR_generic,		/* class identifier number	*/
	&class_cosmic,		/* super class info		*/
};


/**
 ** Note: the series of GB_* procedures that operate on Global Buffers.
 ** They're here for efficiency's sake. Use them with care!
 **/
/*
 * GB_copy(GBuffID)
 *
 * Result: a copy (malloc'ed) of the global buffer
 */
int meth_generic_GB_copy(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *str;
	int bufferID;

	result->type = PKT_STR;
	if (argc != 1) {
		/* incorrect number of arguments */
		result->info.s = "";
		result->canFree = 0;
		return 0;
	}
	bufferID = PkInfo2Int(&argv[0]);
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		fprintf(stderr, "unknown Global Buffer. id=%d\n", bufferID);
		result->info.s = "";
		result->canFree = 0;
		return 0;
	}
	if (GBuff[bufferID]) {
		result->info.s = SaveString(GBuff[bufferID]);
		result->canFree = PK_CANFREE_STR;
		return 1;
	} else {
		result->info.s = "";
		result->canFree = 0;
		return 0;
	}
}

/*
 * GB_data(GBuffID)
 *
 * Result: the global buffer (no malloc occurs)
 */
int meth_generic_GB_data(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *str;
	int bufferID = PkInfo2Int(&argv[0]);

	result->type = PKT_STR;
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		fprintf(stderr, "unknown Global Buffer. id=%d\n", bufferID);
		result->info.s = "";
		result->canFree = 0;
		return 0;
	}
	if (GBuff[bufferID]) {
		result->info.s = GBuff[bufferID];
		result->canFree = 0;
		return 1;
	} else {
		result->info.s = "";
		result->canFree = 0;
		return 0;
	}
}

/*
 * GB_char(GBuffID)
 *
  * Result:  the character at the buffer's index
 */
int meth_generic_GB_char(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID;
	int idx;

	result->type = PKT_CHR;
	result->canFree = 0;
	if (argc != 1) {
		/* incorrect number of arguments */
		result->info.c = '\0';
		return 0;
	}
	bufferID = PkInfo2Int(&argv[0]);
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		fprintf(stderr, "unknown Global Buffer. id=%d\n", bufferID);
		result->info.c = '\0';
		return 0;
	}
	idx = PkInfo2Int(&argv[1]);
	if (GBuff[bufferID]) {
		result->info.c = GBuff[bufferID][GBuffIdx[bufferID]];
		return 1;
	} else {
		result->info.c = '\0';
		return 0;
	}
}

int meth_generic_GB_count(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

int meth_generic_GB_create(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

/*
 * GB_decChar(GBuffID, n)
 *
 * Decrements index by n. by 1 if n isn't given.
 *
 * Result: char at index, '\0' on error
 */
int meth_generic_GB_decChar(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID, n;

	result->type = PKT_CHR;
	result->canFree = 0;
	if (argc == 2) {
		n = PkInfo2Int(&argv[1]);
	} else {
		n = 1;
	}
	bufferID = PkInfo2Int(&argv[0]);
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		result->info.c = '\0';
		return 0;
	}
	GBuffIdx[bufferID] -= n;
	result->info.c = GBuff[bufferID][GBuffIdx[bufferID]];
	return 1;
}

/*
 * GB_decChar1(GBuffID)
 *
 * Decrements index by 1.
 *
 * Result: char at index, '\0' on error
 */
int meth_generic_GB_decChar1(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID;

	result->type = PKT_CHR;
	result->canFree = 0;
	if (argc != 1) {
		/* incorrect number of arguments */
		result->info.c = '\0';
		return 0;
	}
	bufferID = PkInfo2Int(&argv[0]);
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		result->info.c = '\0';
		return 0;
	}
	GBuffIdx[bufferID]--;
	result->info.c = GBuff[bufferID][GBuffIdx[bufferID]];
	return 1;
}

/*
 * GB_decLine(GBuffID, n)
 *
 * Decrements index back n lines. (1 means to beginning of previous line).
 * Decrements by 1 line if n isn't given.
 *
 * Result: index, -1 on error
 */
int meth_generic_GB_decLine(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID;
	int n, idx;
	char *cp;

	result->type = PKT_INT;
	result->canFree = 0;
	if (argc == 2) {
		n = PkInfo2Int(&argv[1]);
	} else {
		n = 1;
	}
	bufferID = PkInfo2Int(&argv[0]);
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		result->info.i = -1;
		result->canFree = 0;
		return 0;
	}
	idx = GBuffIdx[bufferID];
	cp = GBuff[bufferID] + idx;

	for (; idx >= 0; cp--, idx--) 
		if (*cp == '\n') {
			cp--; idx--;
			for (; idx >= 0; cp--, idx--) {
				if (*cp == '\n') {
					if (--n <= 0) {
						cp++; idx++;
						result->info.i = 
							GBuffIdx[bufferID] = 
								idx;
						result->canFree = 0;
						return 1;
					}
				}
			}
			break;
		}

	result->info.i = GBuffIdx[bufferID] = 0;
	result->canFree = 0;

	return 1;
}

/*
 * GB_free(GBuffID)
 *
 * Frees buffer memory. USE WITH CARE.
 *
 * Result: 0 on success, -1 on error
 */
int meth_generic_GB_free(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID;

	result->type = PKT_INT;
	result->canFree = 0;
	if (argc != 1) {
		/* incorrect number of arguments */
		result->info.i = -1;
		result->canFree = 0;
		return 0;
	}
	bufferID = PkInfo2Int(&argv[0]);
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		result->info.i = -1;
		result->canFree = 0;
		return 0;
	}
	if (GBuff[bufferID]) free(GBuff[bufferID]);

	result->info.i = GBuffIdx[bufferID] = -1;
	result->canFree = 0;

	return 1;
}
/*
 * GB_herePtr(GBuffID)
 *
 * Result:  string pointer at index position
 */
int meth_generic_GB_herePtr(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID;

	result->type = PKT_STR;
	if (argc != 1) {
		/* incorrect number of arguments */
		result->info.s = "";
		result->canFree = 0;
		return 0;
	}
	bufferID = PkInfo2Int(&argv[0]);
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		result->info.s = "";
		result->canFree = 0;
		return 0;
	}
	result->info.s = GBuff[bufferID] + GBuffIdx[bufferID];
	result->canFree = 0;
	return 1;
}

/*
 * GB_incChar(GBuffID, n)
 *
 * Increments index by n. by 1 if n isn't given.
 *
 * Result: char at index, '\0' on error
 */
int meth_generic_GB_incChar(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID, n;

	result->type = PKT_CHR;
	result->canFree = 0;
	if (argc == 2) {
		n = PkInfo2Int(&argv[1]);
	} else {
		n = 1;
	}
	bufferID = PkInfo2Int(&argv[0]);
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		result->info.c = '\0';
		return 0;
	}
	GBuffIdx[bufferID] += n;
	result->info.c = GBuff[bufferID][GBuffIdx[bufferID]];
	return 1;
}

/*
 * GB_incChar1(GBuffID)
 *
 * Decrements index by 1.
 *
 * Result: char at index, '\0' on error
 */
int meth_generic_GB_incChar1(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID;

	result->type = PKT_CHR;
	result->canFree = 0;
	if (argc != 1) {
		/* incorrect number of arguments */
		result->info.c = '\0';
		return 0;
	}
	bufferID = PkInfo2Int(&argv[0]);
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		result->info.c = '\0';
		return 0;
	}
	GBuffIdx[bufferID]++;
	result->info.c = GBuff[bufferID][GBuffIdx[bufferID]];
	return 1;
}

/*
 * GB_incLine(GBuffID, n)
 *
 * Increments index to beginning of n next line. by 1 if n isn't given.
 *
 * Result:  current index. -1 on error.
 */ 
int meth_generic_GB_incLine(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID;
	int n, idx;
	char *cp;

	result->type = PKT_INT;
	result->canFree = 0;
	if (argc == 2) {
		n = PkInfo2Int(&argv[1]);
	} else {
		n = 1;
	}
	bufferID = PkInfo2Int(&argv[0]);
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		result->info.i = -1;
		result->canFree = 0;
		return 0;
	}
	idx = GBuffIdx[bufferID];
	cp = GBuff[bufferID] + idx;

	for (; *cp; cp++, idx++) {
		if (*cp == '\n') {
			if (--n <= 0) {
				idx++;
				break;
			}
		}
	}
	result->info.i = idx;
	result->canFree = 0;
	GBuffIdx[bufferID] = idx;

	return 1;
}

/*
 * GB_line(GBuffID)
 *
 * Retuns current line, and advances index to next line
 *
 * Result: the line at the buffer's index (from index to next '\n')
 *         "" on error
 */
int meth_generic_GB_line(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID;
	char *cp, *ccp;
	int len, idx;

	result->type = PKT_STR;
	if (argc != 1) {
		/* incorrect number of arguments */
		result->info.s = "";
		result->canFree = 0;
		return 0;
	}

	bufferID = PkInfo2Int(&argv[0]);

	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		result->info.s = "";
		result->canFree = 0;
		return 0;
	}

	idx = GBuffIdx[bufferID];
	cp = GBuff[bufferID] + idx;

/*	for (; *cp; cp++, idx++) {
		if (*cp == '\n') {
			idx++;
			break;
		}
	}
*/
	ccp = cp;
	while (*cp) {
		if (*cp++ == '\n') {
			break;
		}
	}
	idx += cp - ccp;

	len = idx - GBuffIdx[bufferID];
	result->info.s = (char*)malloc(sizeof(char) * len + 1);
	result->canFree = PK_CANFREE_STR;
	strncpy(result->info.s, GBuff[bufferID] + GBuffIdx[bufferID], len);
	result->info.s[len] = '\0';
	GBuffIdx[bufferID] = idx;

	return 1;
}

/*
 * GB_moveToStart(GBufferID)
 *
 * Return: index (0) on success, -1 on failure
 */
int meth_generic_GB_moveToStart(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID;

	result->type = PKT_INT;
	result->canFree = 0;
	if (argc != 1) {
		/* incorrect number of arguments */
		result->info.i = -1;
		result->canFree = 0;
		return 0;
	}
	bufferID = PkInfo2Int(&argv[0]);
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		result->info.i = -1;
		result->canFree = 0;
		return 0;
	}
	GBuffIdx[bufferID] = 0;
	result->info.i = 0;
	result->canFree = 0;

	return 1;
}

/*
 * GB_moveToChar(GBufferID, idx)
 *
 * Return: index on success, -1 on failure
 */
int meth_generic_GB_moveToChar(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID;

	result->type = PKT_INT;
	result->canFree = 0;
	if (argc != 2) {
		/* incorrect number of arguments */
		result->info.i = -1;
		result->canFree = 0;
		return 0;
	}
	bufferID = PkInfo2Int(&argv[0]);
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		result->info.i = -1;
		result->canFree = 0;
		return 0;
	}
	result->info.i = GBuffIdx[bufferID] = PkInfo2Int(&argv[1]);
	result->canFree = 0;
	return 1;
}

/*
 * GB_moveToLine(GBufferID, idx)
 *
 * Return: index if successful, -1 if error occured
 */
int meth_generic_GB_moveToLine(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID;
	int n;
	char *cp;

	result->type = PKT_INT;
	result->canFree = 0;
	if (argc != 2) {
		/* incorrect number of arguments */
		result->info.i = -1;
		return 0;
	}
	bufferID = PkInfo2Int(&argv[0]);
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		result->info.i = -1;
		return 0;
	}
	result->info.i = GBuffIdx[bufferID] = PkInfo2Int(&argv[1]);

	GBuffIdx[bufferID] = 0;
	n = PkInfo2Int(&argv[1]);

	if (n > 0) {
		for (cp = GBuff[bufferID]; *cp; cp++) {
			if (*cp == '\n' || !(*cp)) {
				GBuffIdx[bufferID]++;
				if (!(*cp)) break;
				if (++n <= 0) break;
			}
		}
	}
	return 1;
}

/*
 * GB_nthChar(GBufferID, n1, [n2]) 
 *
 * Result: character range from n1 to n2, "" on error
 *
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_GB_nthChar(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID = PkInfo2Int(&argv[0]);

	result->type = PKT_CHR;
	result->canFree = 0;
	if (argc == 2) {
		int n;

		n = PkInfo2Int(&argv[1]);
		result->info.c = GBuff[bufferID][n];
		return 1;
	} else if (argc == 3) {
		int i, n1, n2;
		char *cp, *str;

		n1 = PkInfo2Int(&argv[1]);
		n2 = PkInfo2Int(&argv[2]);
		str = GBuff[bufferID];
		cp = (char*)malloc(sizeof(char) * (n2 - n1 + 2));
		if (!cp) {
			result->info.s = "";
			result->canFree = 0;
			return 0;
		}
		for (i = 0; n1 <= n2; n1++) {
			if (!str[n1]) break;
			cp[i++] = str[n1];
		}
		cp[i] = '\0';
		result->info.s = cp;
		result->canFree = PK_CANFREE_STR;
		return 1;
	}
	result->info.s = "";
	result->canFree = 0;
	return 0;
}

/*
 * GB_nthLine(bufferID, n1 [,n2])
 *
 * Extracts the specified lines (from line n1 to n2) from the specified
 * global buffer
 *
 * Line numbering start from 0.
 *
 * Result: the specified line(s), "" on error
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_GB_nthLine(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID = PkInfo2Int(&argv[0]);
	int li, hi, lines, size;
	char *cp, *str;

	result->type = PKT_STR;
	if (bufferID >= numOfGBuffs) {
		/* unknown buffer */
		result->info.s = "";
		result->canFree = 0;
		return 0;
	}
	str = GBuff[bufferID];

	if (argc == 2) {
		li = hi = PkInfo2Int(&argv[1]);
	} else if (argc == 3) {
		li = PkInfo2Int(&argv[1]);
		hi = PkInfo2Int(&argv[2]);
	} else {
		/* incorrect number of arguments */
		result->info.s = "";
		result->canFree = 0;
		return 0;
	}
	cp = getLines(li, hi, str, &size);
	if (cp) {
		result->info.s = cp;
		result->canFree = PK_CANFREE_STR;
		return 1;
	}
	result->info.s = "";
	result->canFree = 0;
	return 0;
}

/*
 * GB_set(GBuffID, data)
 *
 * Sets global buffer as string!
 *
 * Result: GBufffID. -1 on error
 */
int meth_generic_GB_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID;

	result->type = PKT_INT;
	result->canFree = 0;
	if (argc != 2) {
		/* incorrect number of arguments */
		result->info.i = -1;
		return 0;
	}
	bufferID = PkInfo2Int(&argv[0]);

	if (bufferID > NUM_OF_GBUFFS) {
		fprintf(stderr, 
	"Error: Trying to set GBuff[%d]. There's only %s Global Buffers.\n",
			NUM_OF_GBUFFS, NUM_OF_GBUFFS);
		result->info.i = -1;
		return 0;
	}
	numOfGBuffs++;
	GBuff[bufferID] = PkInfo2Str(&argv[1]);
	GBuffIdx[bufferID] = 0;
	result->info.i = bufferID;
	return 1;
}

/*
 * GB_setCopy(GBuffID, data)
 *
 * Sets global buffer as string (the malloc'ed copy)!
 *
 * Result: GBufffID. -1 on error
 */
int meth_generic_GB_setCopy(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int bufferID;

	result->type = PKT_INT;
	result->canFree = 0;
	if (argc != 2) {
		/* incorrect number of arguments */
		result->info.i = -1;
		return 0;
	}
	bufferID = PkInfo2Int(&argv[0]);

	if (bufferID > NUM_OF_GBUFFS) {
		fprintf(stderr, 
	"Error: Trying to set GBuff[%d]. There's only %s Global Buffers.\n",
			NUM_OF_GBUFFS, NUM_OF_GBUFFS);
		result->info.i = -1;
		return 0;
	}
	numOfGBuffs++;
	GBuff[bufferID] = SaveString(PkInfo2Str(&argv[1]));
	GBuffIdx[bufferID] = 0;
	result->info.i = bufferID;
	return 1;
}

int meth_generic_HTTPCanonicalURL(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return 1;
}

/*
 * HTTPCurrentDocAddr()
 */
int meth_generic_HTTPCurrentDocAddr(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	extern char *current_addr; 	/* from html.c */
	extern char* default_default;	/* from html.c */

	result->info.s = saveString(HTParse(current_addr, current_addr, 
					    PARSE_ALL));
	result->type = PKT_STR;
	result->canFree = PK_CANFREE_STR;
	return 1;
}

/*
 * HTTPCurrentDocAddrParsed([addr], [relative])
 * RENAME TO : HTTPParseDocAddr([addr], [relative])?
 *
 * Returns: 	[0] 	access scheme
 *		[1]	host
 *		[2]	path (not including file name)
 *		[3]	file
 *		[4]	anchor
 */
int meth_generic_HTTPCurrentDocAddrParsed(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	extern char *current_addr; 	/* from html.c */
	extern char* default_default;	/* from html.c */
	Attr *attrp;
	Packet *packet0 = makePacket(); /* access */
	Packet *packet1 = makePacket(); /* host */
	Packet *packet2 = makePacket(); /* path (not including file name) */
	Packet *packet3 = makePacket(); /* file */
	Packet *packet4 = makePacket(); /* anchor */
	char *addr, *relative, *path, *anchor;
	int i, length;
	
	if (argc >= 1) addr = PkInfo2Str(&argv[0]);
	else addr = current_addr;

	if (argc >= 2) relative = PkInfo2Str(&argv[1]);
	else relative = current_addr;

	if (!addr) {
		fprintf(stderr,
		     "meth_generic_HTTPCurrentDocAddrParsed: addr == NULL\n");
		clearPacket(result);
		return 0;
	}
	packet0->canFree =
	packet1->canFree =
	packet2->canFree =
	packet3->canFree =
	packet4->canFree = PK_CANFREE_STR;

	packet0->type = 
	packet1->type = 
	packet2->type = 
	packet3->type = 
	packet4->type = PKT_STR;

	packet0->info.s = saveString(HTParse(addr, relative, PARSE_ACCESS));
	packet1->info.s = saveString(HTParse(addr, relative, PARSE_HOST));
	path = HTParse(addr, relative, PARSE_PATH | PARSE_PUNCTUATION);
	
	length  = strlen(path); 
	for (i = length; i >= 0; i--) 
		if (path[i] == '/') {
			strncpy(buff, path, i + 1);
			buff[i + 1] = '\0';
			packet2->info.s = saveString(buff);
			packet3->info.s = saveString(path + i + 1);
			goto lameLoopEsc;
		}
	packet2->info.s = saveString("");
	packet3->info.s = saveString(path);
lameLoopEsc:

	anchor = HTParse(addr, relative, PARSE_ANCHOR);
	packet4->info.s = (anchor ? saveString(anchor) : saveString(""));

	result->type = PKT_ATR;
	result->info.a = attrp = makeAttr(0, packet0);
	result->canFree = 0;
	attrp->next = makeAttr(1, packet1);
	attrp->next->next = makeAttr(2, packet2);
	attrp->next->next->next = makeAttr(3, packet3);
	attrp->next->next->next->next = makeAttr(4, packet4);

	return 1;
}

/*
 * HTTPCurrentDocAddrSet()
 */
int meth_generic_HTTPCurrentDocAddrSet(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	extern char *current_addr; 	/* from html.c */

	clearPacket(result);
	if (current_addr) free(current_addr);
	current_addr = saveString(PkInfo2Str(argv));
	return 1;
}

/*
 * HTTPDecode()
 *
 */
int meth_generic_HTTPDecodeURL(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	result->info.s = decodeURL(PkInfo2Str(&argv[0]));
	result->canFree = PK_CANFREE_STR;
	result->type = PKT_STR;
	return 1;
}

/*
 * HTTPEncodeURL()
 *
 */
int meth_generic_HTTPEncodeURL(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	result->info.s = encodeURL(PkInfo2Str(&argv[0]));
	result->canFree = PK_CANFREE_STR;
	result->type = PKT_STR;
	return 1;
}

/*
 * HTTPGet()
 *
 * it's user's responsibility to remove the temporary file after usage.
 *
 * Result: name of file containing the data
 *
 */
int meth_generic_HTTPGet(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *simpleAddress, *anchorSearch;
	FILE *fp;
	char *tfn, tempFileName[200], *ext, *src;
	int len, span = 0;

	/* now... is this a leak? this method can be called by 
	 * imgNodeRefInc.. only for images, so ... probably quite safe.
	 */
	if (self) if (notSecure(self)) return 0;

	src = PkInfo2Str(&argv[0]);
	len = strlen(src);

	for (ext = src + len; ext >= src; ext--) {
		if (*ext == '.') break;
		if (*ext == '/' || span++ > 6) {
			/* assume extensions are <6 chars */
			sprintf(tempFileName, "%s%ld", 
				tempFileNamePrefix, tempFileNameIDCounter++);
			tfn = tempFileName;
			goto gogo;
		}
		if (ext == src) break;
	}
	sprintf(tempFileName, "%s%ld%s", 
		tempFileNamePrefix, tempFileNameIDCounter++, ext);
	tfn = tempFileName;
gogo:
	fp = fopen(tfn, "w");
	if (fp) {
		if (html_fetchDocument(self, src,
					&simpleAddress, &anchorSearch, fp)) {
			fclose(fp);
			result->canFree = PK_CANFREE_STR;
			result->info.s = saveString(tfn);
			result->type = PKT_STR;
			return 1;
		}
		fclose(fp);
	} else {
		fprintf(stderr, 
			"Bummers, failed to create temporary file %s!", 
			tfn);
	}
	clearPacket(result);
	return 0;
}

int helper_buildingHTML(result, obj, url, width, method, dataToPost)
	Packet *result;
	VObj *obj;
	char *url;
	int width;
	int method;
	char *dataToPost;
{
	VObj *newObj;
	char *simpleAddress, *anchorSearch;
/*	struct timeval time1, time2;*/
	clearPacket(result);

/*	gettimeofday(&time1, (struct timezone*)NULL);*/

	if (notSecure(obj)) return 0;

	newObj = html2_parseHTMLDocument(obj, url,
					 &simpleAddress, &anchorSearch,
					 width, method, dataToPost);
/*
	gettimeofday(&time2, (struct timezone*)NULL);
	printf("**** duration=%d secs to for %s\n", 
		time2.tv_sec - time1.tv_sec, url);
*/
	result->info.o = newObj;
	result->canFree = 0;
	result->type = PKT_OBJ;
	return 1;
}

/*
 * HTTPGetNParse(url, parentForBuiltObjs, width)
 *
 * it's user's responsibility to remove the temporary file after usage.
 *
 * Result: name of file containing the data
 *
 */
int meth_generic_HTTPGetNParse(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (notSecure(self)) return 0;
	return helper_buildingHTML(result, 
				PkInfo2Obj(&argv[1]), 
				PkInfo2Str(&argv[0]),
				PkInfo2Int(&argv[2]),
				HTTP_METHOD_GET,
				NULL);
}

/*
 * HTTPPost(url, parentForBuiltObjs, width, post-data)
 */
int meth_generic_HTTPPost(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (notSecure(self)) return 0;
	return helper_buildingHTML(result, 
				PkInfo2Obj(&argv[1]), 
				PkInfo2Str(&argv[0]),
				PkInfo2Int(&argv[2]),
				HTTP_METHOD_POST,
				PkInfo2Str(&argv[3]));
}

/* XXX NOT YET WORKING */
/*
 * HTTPPost(url, parentForBuiltObjs, width, post-data)
 */
int meth_generic_HTTPSubmit(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (notSecure(self)) return 0;
	return helper_buildingHTML(result, 
				PkInfo2Obj(&argv[1]), 
				PkInfo2Str(&argv[0]),
				PkInfo2Int(&argv[2]),
				HTTP_METHOD_SUBMIT,
				PkInfo2Str(&argv[3]));
}

int meth_generic_HTTPHotListAdd(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	/* url, comment, date */
	if (notSecure(self)) return 0;
	return addHotListItem(saveString(PkInfo2Str(&argv[0])),
			saveString(PkInfo2Str(&argv[1])),
			saveString(PkInfo2Str(&argv[2])));
}

int meth_generic_HTTPHotListDelete(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (notSecure(self)) return 0;
	return deleteHotListItem(PkInfo2Int(&argv[0]));
}

/* arg[0]	0	returns URL
 * arg[0]	1	returns Comment
 * arg[0]	2	returns all URLs in one big string
 * arg[0]	3	returns all Comments in one big string
 * arg[1]	n	nth item in the hotlist
 */
int meth_generic_HTTPHotListGet(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	HotListItem *hip;

	if (notSecure(self)) return 0;

	switch (PkInfo2Int(&argv[0])) {
	case 0:
		hip = getNthHotListItem(PkInfo2Int(&argv[1]));
		if (!hip) return 0;
		if (!hip->url) return 0;
		result->info.s = hip->url;
	break;
	case 1:
		hip = getNthHotListItem(PkInfo2Int(&argv[1]));
		if (!hip) return 0;
		if (!hip->comment) return 0;
		result->info.s = hip->comment;
	break;
	case 2:
		result->info.s = dumpHotList(0);
	break;
	case 3:
		result->info.s = dumpHotList(1);
	break;
	default:
		clearPacket(result);
		return 0;
	}
	result->type = PKT_STR;
	result->canFree = 0;

	return 1;
}

/* arg[0]	0	change URL
 * arg[0]	1	change Comment
 * arg[1]	n	nth item in the hotlist
 * arg[2]	str	new data
 */
int meth_generic_HTTPHotListChange(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	HotListItem *hip;

	if (notSecure(self)) return 0;

	switch (PkInfo2Int(&argv[0])) {
	case 0:/* change url */
		hip = getNthHotListItem(PkInfo2Int(&argv[1]));
		if (!hip) return 0;
		if (!hip->url) return 0;
		free(hip->url);
		hip->url = saveString(PkInfo2Str(&argv[2]));
	break;
	case 1:/* change comment */
		hip = getNthHotListItem(PkInfo2Int(&argv[1]));
		if (!hip) return 0;
		if (!hip->comment) return 0;
		free(hip->comment);
		hip->comment = saveString(PkInfo2Str(&argv[2]));
	break;
	default:
		return 0;
	}
	return 1;
}

int meth_generic_HTTPHotListLoad(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (notSecure(self)) return 0;
	clearPacket(result);
	result->info.i = initHotList();
	result->type = PKT_INT;
	return 0;
}

int meth_generic_HTTPHotListSave(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (notSecure(self)) return 0;
	return saveHotList();
}

/*
 * SGMLBuildDoc(url, parent, name, width)
 *
 * Result: document object
 */
int meth_generic_SGMLBuildDoc(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObj *newObj;

	if (notSecure(self)) return 0;
	if (argc != 6) {
		/* incorrect number of arguments */
		result->type = PKT_OBJ;
		result->canFree = 0;
		result->info.o = NULL;
		return 0;
	}
	newObj = SGMLBuildDoc(PkInfo2Str(&argv[0]),	/* source */
			      PkInfo2Str(&argv[1]),	/* URL */
			      PkInfo2Obj(&argv[2]),	/* parent */
			      PkInfo2Str(&argv[3]), 	/* name */
			      PkInfo2Int(&argv[4]),	/* width */
			      PkInfo2Int(&argv[5]));	/* anchor */
	result->info.o = newObj;
	result->type = PKT_OBJ;
	result->canFree = 0;
	return 1;
}

/*
 * SGMLBuildDocB(url, parent, name, width)
 *
 * Result: document object
 */
int meth_generic_SGMLBuildDocB(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObj *newObj;

	if (notSecure(self)) return 0;
	if (argc != 6) {
		/* incorrect number of arguments */
		result->type = PKT_OBJ;
		result->canFree = 0;
		result->info.o = NULL;
		return 0;
	}
	newObj = SGMLBuildDoc_B(PkInfo2Str(&argv[0]),	/* source */
			      PkInfo2Str(&argv[1]),	/* URL */
			      PkInfo2Obj(&argv[2]),	/* parent */
			      PkInfo2Str(&argv[3]), 	/* name */
			      PkInfo2Int(&argv[4]),	/* width */
			      PkInfo2Int(&argv[5]));	/* anchor */
	result->info.o = newObj;
	result->type = PKT_OBJ;
	result->canFree = 0;
	return 1;
}

/*
 * SGMLBuildDoc_setBuff()
 */
int meth_generic_SGMLBuildDoc_setBuff(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (notSecure(self)) return 0;
	clearPacket(result);
	result->info.i = SGMLBuildDoc_setBuff(self, PkInfo2Int(&argv[0]));
	result->type = PKT_INT;
	result->canFree = 0;
	return 1;
}

/*
 * SGMLBuildDoc_setColors()
 * equivalent to:

       color = getResource("Viola.background_doc");
       if (color) set("BGColor", color);
       color = getResource("Viola.foreground_doc");
       if (color) set("FGColor", color);
 */
int meth_generic_SGMLBuildDoc_setColors(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	static Packet largv[2];
	static char *BG_DOC;
	static char *FG_DOC;
	static int init = 0;
	int (*setFunc)() = GET__classInfo(self)->slotSetMeth;

	if (!init) {
		BG_DOC = GLGetResource("Viola.background_doc");
		if (BG_DOC) BG_DOC = saveString(BG_DOC);
		FG_DOC = GLGetResource("Viola.foreground_doc");
		if (FG_DOC) FG_DOC = saveString(FG_DOC);
		largv[1].type = PKT_STR;
		largv[1].canFree = 0;
		init = 1;
	}
	if (BG_DOC) {
		largv[1].info.s = BG_DOC;
		setFunc(self, result, 2, largv, STR_BGColor);
	}
	if (FG_DOC) {
		largv[1].info.s = FG_DOC;
		setFunc(self, result, 2, largv, STR_FGColor);
	}
	return 1;
}

int meth_generic_SGMLBuildDoc_span(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	extern int SGMLBuildDoc_span;

	clearPacket(result);
	result->info.i = SGMLBuildDoc_span;
	result->type = PKT_INT;
	result->canFree = 0;
	return 1;
}

int meth_generic_SGMLBuildDoc_flush(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
/*	return SGMLBuildDoc_flush();
*/
}

int meth_generic_SGMLBuildDoc_insertObj(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (argc == 1) {
		SGMLBuildDoc_insertObj(PkInfo2Obj(&argv[0]), -1);
	} else if (argc == 2) {
		SGMLBuildDoc_insertObj(PkInfo2Obj(&argv[0]), 
					PkInfo2Int(&argv[1]));
	}
	result->info.i = 1;
	result->type = PKT_INT;
	result->canFree = 0;
	return 1;
}

/*
 * SGMLFindAnchorOffset(width)
 *
 * Result: vspan
 */
int meth_generic_SGMLFindAnchorOffset(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	if (argc != 1) {
		/* incorrect number of arguments */
		result->type = PKT_INT;
		result->canFree = 0;
		result->info.i = 0;
		return 0;
	}
	result->info.i = SGMLFindAnchorOffset(self, PkInfo2Str(&argv[0]));
	result->type = PKT_INT;
	result->canFree = 0;
	return 1;
}

int meth_generic_SGMLMathFormater(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (argc == 3) {
		return HTMLMathFormater(self, 
				&argv[0], &argv[1], PkInfo2Int(&argv[2]));
	}
}

/*
 * SGMLReBuildDoc(url, parent, name, width)
 *
 * Result: document object
 */
int meth_generic_SGMLReBuildDoc(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (notSecure(self)) return 0;
	if (argc != 6) {
		/* incorrect number of arguments */
		result->type = PKT_OBJ;
		result->canFree = 0;
		result->info.o = NULL;
		return 0;
	}
	result->info.o = SGMLBuildDoc_B(PkInfo2Str(&argv[0]),	/* source */
				      PkInfo2Str(&argv[1]),	/* URL */
				      PkInfo2Obj(&argv[2]),	/* parent */
				      PkInfo2Str(&argv[3]), 	/* name */
				      PkInfo2Int(&argv[4]),	/* width */
				      PkInfo2Int(&argv[5]));	/* anchor */
	result->type = PKT_OBJ;
	result->canFree = 0;
	return 1;
}

/*
 * SGMLTileDoc(width)
 *
 * Result: vspan
 */
int meth_generic_SGMLTileDoc(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (argc != 2) {
		/* incorrect number of arguments */
		result->type = PKT_INT;
		result->canFree = 0;
		result->info.i = 0;
		return 0;
	}
	result->info.i = SGMLTileDoc(self, 
				  PkInfo2Int(&argv[0]), PkInfo2Int(&argv[1]));
	result->type = PKT_INT;
	result->canFree = 0;
	return 1;
}

/* 
 * SGMLGetStyle(dtd, tagName, attrName)
 * 
 * Result: attribute value
 */
int meth_generic_SGMLGetStyle(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (argc == 2)
		return getSGMLStyle(PkInfo2Str(&argv[0]), 
				    PkInfo2Str(&argv[1]), 
				    NULL, result);
	else if (argc == 3)
		return getSGMLStyle(PkInfo2Str(&argv[0]), 
				    PkInfo2Str(&argv[1]), 
				    PkInfo2Str(&argv[2]), result);
	return 0;
}

int meth_generic_STGInfo(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return getSTGInfo(PkInfo2Str(&argv[0]), PkInfo2Str(&argv[1]), result);
}

int meth_generic_STG_tagPtr(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *superTagName = NULL;
	if (argc == 2) superTagName = PkInfo2Str(&argv[1]); 
	return getSTGInfo_tagPtr(result, PkInfo2Str(&argv[0]), superTagName);
}

int meth_generic_STG_attr(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return getSTGInfo_attr(PkInfo2Int(&argv[0]),
			       PkInfo2Str(&argv[1]), result);
}

int meth_generic_STG_clean(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return getSTGInfo_clean(PkInfo2Int(&argv[0]));
}

/* NOT FULLY IMPLEMENTED
 *
 * SGMLSetStyle(dtd, tagName, attrName, value)
 * 
 * Result: attribute value
 */
int meth_generic_SGMLSetStyle(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return setSGMLStyle(PkInfo2Str(&argv[0]), PkInfo2Str(&argv[1]), 
			    PkInfo2Str(&argv[2]), &argv[2]);
}

int meth_generic_SGMLTableFormater(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (argc == 2) {
		return HTMLTableFormater(self, 
				PkInfo2Obj(&argv[0]), PkInfo2Int(&argv[1]));
	} else {
		return HTMLTableFormater(self, NULL, 0);
	}
}

/*
 * accessible(filepath)
 * 
 * Determine the accessibility of a file.
 *
 * Result: full file path on success, or "" on failure
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_accessible(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int accessible;
	char *path;
	if (notSecure(self)) return 0;
	result->type = PKT_STR;
	if (path = SaveString(vl_expandPath(argv[0].info.s, buff))) {
		accessible = access(path, R_OK);
		if ((accessible & R_OK) == 0) {
			result->info.s = path;
			result->canFree = PK_CANFREE_STR;
			return 1;
		}
		free(path);
	}
	result->info.s = "";
	result->canFree = 0;
	return 0;
}

/* send message to VW motif active help field
 */
int meth_generic_activeHelp(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
#define VW
#ifdef VW
	extern void showHelpMessageInMainWindow();
	extern void blankHelpHandlerInMainWindow();
	char *str;

	if (argc == 0) {
		blankHelpHandlerInMainWindow();
	} else {
		showHelpMessageInMainWindow(PkInfo2Str(&argv[0]));
	}
#endif
	return 1;
}

/*
 * after(milliseconds, object, [message list])
 *
 * Schedule to send messages to an object after a period of time.
 *
 * Result/Return: 1 if successful, 0 if error occured
 */
int meth_generic_after(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int msec;
	VObj *obj;
	Packet *messages;
	int i, j, size;

	result->type = PKT_INT;
	result->canFree = 0;

	msec = PkInfo2Int(&argv[0]);
	obj = PkInfo2Obj(&argv[1]);

	if (!msec || !obj) {
		result->info.i = 0;
		return 0;
	}

	size = sizeof(struct Packet) * (argc - 2);
	messages = (Packet*)malloc(size);
	if (!messages) return 0;

	for (i = 0, j = 2; j < argc; j++, i++) {
		nullPacket(&messages[i]);
		copyPacket(&messages[i], &argv[j]);
	}

	/* it's event loop's responsibility to free the message list */

	if (scheduleEvent(msec, sendMessagePackets, obj, argc - 2, messages)) {
		result->info.i = 1;
		return 1;
	}
	result->info.i = 1;
	return 0;
}

/*
 * alarm(?time, object, message)
 *
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_alarm(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	if (argv[0].type == PKT_INT) {
		sys_alarm((unsigned)(PkInfo2Int(&argv[0])));
		return 1;
	}
	return 0;
}

/*
 * append(<list>, id, value)
 *
 * Append item to list
 *
 * Result:
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_append(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
#ifdef NOTYET
			attrp = NULL;
			for (attrp2 = packetp->info.a; attrp2; 
			     attrp2 = attrp2->next) {
				if (attrp2->id == i) {
					attrp = attrp2;
					break;
				}
			}
			if (!attrp) {
				/* make the new list node */
				attrp = makeAttr(i, (long)makePacket());
				prependAttr(&(packetp->info.a), attrp);
			}
			packetp = (Packet*)attrp->val;
			copyPacket(&reg1, &execStack[stackExecIdx]);
			clearPacket(&execStack[stackExecIdx]);
			--stackExecIdx;
			copyPacket(packetp, &reg1);
#endif
	return 0;
}

/*
 * argument();
 *
 * Return the string following the -ar option.
 *
 * Result: version string
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_argument(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	extern char *passthru_argument;	/* defined in cexec.c */
	result->info.s = passthru_argument;
	result->type = PKT_STR;
	result->canFree = 0;
	return 1;
}

/*
 * ascii(ASCII code)
 *
 * Result: corresponding ASCII character
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_ascii(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_CHR;
	result->canFree = 0;
	result->info.c = (char)argv[0].info.i;
	return 1;
}

/*
 * asciiVal(ASCII character)
 *
 * Result: corresponding ASCII code
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_asciiVal(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_INT;
	result->canFree = 0;
	result->info.i = (int)argv[0].info.c;
	return 1;
}

/*
 * bell();
 *
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_bell(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	SLBell();
	return 1;
}

/*
 * bellVolume(0-100);
 *
 * Result: volume value
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_bellVolume(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_INT;
	result->canFree = 0;
	result->info.i = SLBellVolume(PkInfo2Int(argv));
	return 1;
}

/*
 * char(<ASCII value>)
 */
int meth_generic_char(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_CHR;
	result->info.c = (char)PkInfos2Str(argc, argv)[0];
	result->canFree = 0;
	return 1;
}

/*
 * XXX
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_clear(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

/*
 * cli();
 *
 * Invoke Command Line Interface to Viola language interpreter
 * 
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_cli(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
#define LINE_LENGTH 300
	char c, cmdLine[LINE_LENGTH];
	int i, balance, finish = 0;

	if (notSecure(self)) return 0;

	do {

		if (flag_cliprompt) printf("<%s>: ", GET_name(VCurrentObj));
		i = 0;
		balance = 0;

		for (;;) {
			c = getchar();
			if (i < LINE_LENGTH - 1) {
			}
			cmdLine[i++] = c;
			if (c == ';' && (balance == 0)) {
				cmdLine[i] = '\0';
				break;
			} else if (c == '{' || c == '(') {
				balance++;
			} else if (c == '}' || c == ')') {
				balance--;
			} else if (c == EOF) {
				cmdLine[--i] = '\0';
				finish = 1;
				break;
			}
		}
/*		printf(">>>%s<<<\n", cmdLine);*/

		if (!AllBlank(cmdLine)) {
			result = execScript(VCurrentObj, result, cmdLine);

			if (flag_cliprompt && result) {
				dumpPacket(result);
				printf("\n");
			}
		}
	} while (!finish);

	return 1;
}

/*
 * clone(clone name suffix)
 * 
 * Make a clone self
 *
 * Result: clone object, and optinally name it
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_clone(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObj *cloneObj;

	if (!meth_generic_clone2(self, result, argc, argv)) return 0;
	cloneObj = result->info.o;
	if (cloneObj) {
		sendInitToChildren(cloneObj);
		result->type = PKT_OBJ;
		result->canFree = 0;
		result->info.o = cloneObj;
		return 1;
	}
	result->type = PKT_OBJ;
	result->canFree = 0;
	result->info.o = NULL;
	return 0;
}

int meth_generic_clone2(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObj *cloneObj, *obj;
	VObjList *olist, *newolist;
	union PCode *pcode;
	int size;

	if (!meth_cosmic_clone2(self, result, argc, argv)) return 0;
	cloneObj = result->info.o;

	newolist = NULL;
	olist = GET__children(self);
	if (olist) {
		Packet lresult;

		nullPacket(&lresult);
		for (; olist; olist = olist->next) {
			if (!callMeth(olist->o, &lresult, argc, argv, 
				      STR_clone2)) {
				fprintf(stderr, 
				    "clone (at children cloning) failed\n");

				clearPacket(&lresult);
				return 0;
			}
			newolist = appendObjToList(newolist, lresult.info.o);
			SET__parent(lresult.info.o, cloneObj);
			SET_parent(lresult.info.o, GET_name(cloneObj));
		}
		clearPacket(&lresult);
		SET_children(cloneObj, 
			     VSaveString(GET__memoryGroup(cloneObj),
					 OListToStr(newolist)));
	} else {
		SET_children(cloneObj, 
			     VSaveString(GET__memoryGroup(cloneObj), ""));
	}
	SET__children(cloneObj, newolist);

	SET__varList(cloneObj, NULL);

	GET_script(self)->refc++;
	SET_script(cloneObj, GET_script(self));

	pcode = GET__script(self);
	if (pcode) {
		pcode[PCODE_IDX_REFC].i++;
		size = sizeof(union PCode) * 
			(pcode[PCODE_IDX_INSTR].i + pcode[PCODE_IDX_SIZE].i);
		SET__script(cloneObj, pcode);
	}

	SET__argAttr(cloneObj, NULL);
	if (!makeArgAttr(cloneObj)) {
	  fprintf(stderr, "makeArgAttr() failed\n");
	}

	SET__scriptVV(cloneObj, NULL);
	SET__classScriptVV(cloneObj, NULL);
	SET__tempScriptVV(cloneObj, NULL);

	result->type = PKT_OBJ;
	result->canFree = 0;
	result->info.o = cloneObj;

	return 1;
}

/* change to: collapseSpaces 
 */
int meth_generic_compressSpaces(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char c, *src, *e, *p1, *p2, *p3;

	freePossibleDangler(result);
	src = PkInfo2Str(&argv[0]);
	e = p1 = src;
	for (;;) {
		if (*p1 == '\0') {
			goto done;

		} else if (ISSPACE(*p1)) {
			p2 = p1;
			while (c = *p2, ISSPACE(c) && (c != '\0')) p2++;
			if (*p2 == '\0') goto done;
			
			for (p3 = p2;;p3++) {
				if (*p3 == '\0') break;
				if (c = *p3, ISSPACE(c)) {
					if (ISSPACE(*(p3 + 1)) &&
					    *(p3 + 1) != '\0') {
						p3 = p3 + 1;
						break;
					}
				}
			}
			while (p2 < p3) *e++ = *p2++;
			if (!*p3) goto done;
			p1 = p3;

		} else {
			for (;;) {
				if (e == p1) {
					while (c = *p1, !ISSPACE(c) &&
					       (c != '\0')) *p1++;
					e = p1;
				} else {
					while (c = *p1, !ISSPACE(c) && 
					       (*p1 != '\0')) *e++ = *p1++;
				}
				if (*p1 == '\0') goto done;
				if (*(p1+1) == '\0') goto done;
				*e++ = *p1++;
				if (ISSPACE(*p1)) break;
			}
		}
	}
  done:
	*e = '\0';

	result->type = PKT_STR;
	result->info.s = saveString(src);
	result->canFree = PK_CANFREE_STR;
	return 1;
}

/*
 * concatenate(arglist)
 *
 * Result: concatenation of arguments as strings
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_concatenate(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp;

	clearPacket(result);

	result->type = PKT_STR;
	cp = PkInfos2Str(argc, argv);
	result->canFree = PK_CANFREE_STR;
	if (result->info.s = SaveString(cp)) return 1;
	return 0;
}

/*
 * concatList(arglist)
 *
 * Result: concatenation of list items to one string
 * Return: 1 if successful, 0 if error occured
 */
#define REVERSERBUFF 1000
char *reverserBuff[REVERSERBUFF];
int meth_generic_concatList(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp;
	Attr *attrp;
	Attr *hattrp;
	int totalLength = 0;
	int li = 0;

	hattrp = (Attr*)argv[0].info.a;

	result->type = PKT_STR;
	if (!hattrp) {
		clearPacket(result);
		return 0;
	}
	for (attrp = hattrp; attrp; attrp = attrp->next) {
		cp = ((Packet*)(attrp->val))->info.s;
		totalLength += strlen(cp);
		if (li < REVERSERBUFF) reverserBuff[li++] = cp;
	}
	if (li >= REVERSERBUFF) {
		fprintf(stderr, 
			"ERROR: a feeble fixed sized buffer has been exceeded.\n. Error not recovered."); /*XXX*/
		/* do something to expand buffer... */
		clearPacket(result);
		return 0;
	}
	cp = (char*)malloc(sizeof(char) * (totalLength + 1));
	cp[0] = '\0';
	while (--li >= 0) strcat(cp, reverserBuff[li]);
	result->info.s = cp;
	result->canFree = PK_CANFREE_STR;
	return 1;
}

/*
 * cos(degree)
 *
 * Cosine at the given degree.
 * 
 * Result: cosine value at given degree
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_cos(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_FLT;
	result->info.f = cos((double)(PkInfo2Flt(&argv[0]) * 
					RADIAN_TO_DEGREE_RATIO));
	return 1;
}

/*
 * countChildren();
 *
 * Equivalent to objectListCount("children")
 *
 * Result: number of children
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_countChildren(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist = GET__children(self);
	int i;

	result->type = PKT_INT;
	result->canFree = 0;
	if (olist) {
		for (i = 0; olist; olist = olist->next, i++);
		result->info.i = i;
		return 1;
	}
	result->info.i = 0;
	return 0;
}

/*
 * countItems(str)
 *
 * Comma seperate items.
 *
 * Result: number of items
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_countItems(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp = PkInfo2Str(&argv[0]);

	result->type = PKT_INT;
	result->canFree = 0;
	result->info.i = numOfChar(cp, ',') + 1;

	return 1;
}

/*
 * countLines(str)
 *
 * Counts number of '\n' charcters.
 *
 * Result: number of lines
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_countLines(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp = PkInfo2Str(&argv[0]);

	result->type = PKT_INT;
	result->canFree = 0;
	result->info.i = numOfChar(cp, '\n');

	return 1;
}

/*
 * countWords(str)
 *
 *
 *
 * Result: number of words
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_countWords(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}


/*
 * ctrlKeyP()
 *
 *
 * Result: control key state
 * Return: 1
 */
int meth_generic_ctrlKeyP(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_INT;
	result->info.i = keyStat_control;
	result->canFree = 0;
	return 1;
}

/*
 * cursorShape([busy|idle])
 *
 * Change cursor shape.
 *
 * Result: current cursor shape
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_cursorShape(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *shape = PkInfo2Str(argv);

	result->type = PKT_STR;

	if (argc == 0 || !GET_window(self)) {
		result->info.s = "?";
		result->canFree = 0;
		return 0;
	}
	
	result->canFree = 0;
	if (!STRCMP(shape, "busy")) {
		result->info.s = "busy";
		GLChangeToBusyMouseCursor(GET_window(self));
	} else {
		result->info.s = "idle";
		GLChangeToNormalMouseCursor(GET_window(self));
	}
	return 1;
}

/*
 * date() 
 *
 * Result: date string, like ``Sat Feb 16 19:59:04 1991''
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_date(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	time_t theTime;
	time(&theTime);
	result->info.s = SaveString(ctime(&theTime));
	result->type = PKT_STR;
	result->canFree = PK_CANFREE_STR;
	return 1;
}

/* deepObjectListSend(objectListName, arg1, arg2, ..., argn)
 *
 * Recursively send the arguments to each objects in the object list.
 *
 * Result: cleared
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_deepObjectListSend(self, result, argc, argv)
      VObj *self;
      Packet *result;
      int argc;
      Packet argv[];
{
      VObjList *olist = NULL;
      char *listName;

      listName = PkInfo2Str(&argv[0]);
      if (!STRCMP(listName, "children")) {
              olist = GET__children(self);
      }
      while (olist) {
              if (olist->o) {
                      sendMessagePackets(olist->o, &argv[1], argc - 1);
                      callMeth(olist->o, result, argc, argv, 
                               STR_deepObjectListSend);
              } else {
                      /* error */
              }
              olist = olist->next;
      }
      return 1;
}


/*
 * was test 2
 * defineNewFont(font#, fontName, XFontName)
 */
int meth_generic_defineNewFont(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	result->info.i = GLDefineNewFont(fontInfo, 
				PkInfo2Int(&argv[0]),
				PkInfo2Str(&argv[1]),
				PkInfo2Str(&argv[2]));
	result->type = PKT_INT;
	return 1;
}

/*
 * XXX
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_delay(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	result->type = PKT_INT;
	result->info.i = sleep((unsigned)PkInfo2Int(argv[0]));
	return 1;
}

int meth_generic_deleteFile(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (notSecure(self)) return 0;
	clearPacket(result);
	result->type = PKT_INT;
	result->info.i = unlink(PkInfo2Str(argv[0]));
	return 1;
}

int meth_generic_deleteSubStr(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_STR;
	result->info.s = "NOT IMPLEMENTED";
	result->canFree = 0;
	return 1;
}

int meth_generic_deleteSubStrQ(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_STR;
	result->info.s = "NOT IMPLEMENTED";
	result->canFree = 0;
	return 1;
}

/*
 * XXX
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_depth(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

/*
 * destroy(object name)
 * 
 * Result:
 * Return:
 */
int meth_generic_destroy(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	Vfree(GET__memoryGroup(self), GET_name(self));
	return 0;
}

/*
 * destroyVariable(attribute)
 *
 *
 * Result: the gotten attribute
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_destroyVariable(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

/*
 * environVar(attribute)
 *
 * Get environment variable.
 *
 * Result: the gotten attribute
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_environVar(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	extern char **environ;

	if (notSecure(self)) return 0;

	result->type = PKT_STR;
	if (getEnvironVars(environ, PkInfo2Str(&argv[0]), buff)) {
		result->info.s = SaveString(buff);
		result->canFree = PK_CANFREE_STR;
		return 1;
	}
	result->info.s = "";
	result->canFree = 0;
	return 0;
}

/*
 * XXX
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_field(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

/*
 * XXX
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_fieldList(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

/*
 * filter(text)
 *
 * Result: replaces escape code into escape char
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_filter(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *inStr = PkInfo2Str(&argv[0]);
	char *outStr;
	char c, *cp;

	outStr = SaveString(inStr);
	cp = outStr;

	result->info.s = outStr;
	result->type = PKT_STR;
	result->canFree = PK_CANFREE_STR;

	while (c = *inStr++) {
		if (c == '\\') {
			switch (c = *inStr++) {
			case 'n': 
				*cp++ = '\n'; 	/* newline */
				continue;
			case 't':
				*cp++ = '\t'; 	/* horizontal tab */
				continue;
			case 'b': 
				*cp++ = '\b'; 	/* backspace */
				continue;
			case 'r': 
				*cp++ = '\r';	/* carriage */
				continue;
			case 'f': 
				*cp++ = '\f'; 	/* form feed */
				continue;
			case '\\': 
				*cp++ = '\\'; 	/* backslash */
				continue;
			case '\'': 
				*cp++ = '\''; 	/* single quote */
				continue;
			case '\"': 
				*cp++ = '\"';
				continue;
			case EOF:
				continue;
			default:
				*cp++ = '\\';
				*cp++ = c;
			}
		} else {
			*cp++ = c;
		}
	}
	*cp = '\0';
	return 1;
}

/*
 * findPattern(text, pattern)
 *
 * Result: position where the matching string pattern ends, or -1 if not found
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_findPattern(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp;
	char *inStr, *patStr;
	int ii = 0, pi = 0;
	int inLength, patLength;

	inStr = PkInfo2Str(&argv[0]);
	patStr = PkInfo2Str(&argv[1]);

	result->type = PKT_INT;
	result->canFree = 0;
	if (!inStr || !patStr) {
		result->info.i = -1;
		return 0;
	}
	inLength = strlen(inStr);
	patLength = strlen(patStr);

	for (cp = inStr; *cp; cp++) {
		if (*cp == patStr[pi]) {
			if (++pi >= patLength) {
				result->info.i = ii;
				return 1;
			}
			if (ii >= inLength) {
				result->info.i = -1;
				return 0;
			}
		} else {
			pi = 0;
		}
		ii++;
	}
	result->info.i = -1;
	return 0;
}

/*
 * float(argument)
 *
 * Convert argument to floating point type.
 *
 * Result: arguement in float
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_float(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_FLT;
	result->info.f = PkInfo2Flt(argv);
	return 1;
}

/* 
 * format(text, style [,style info])
 *
 * Simple text formatting kludge
 *
 * Result: arguement in float
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_format(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (!STRCMP(PkInfo2Str(&argv[1]), "split")) {
	}
/*
	result->type = PKT_STR;
	result->info.s = PkInfo2Flt(argv);
	return 1;
*/
	clearPacket(result);
	return 0;
}

/*
 * freeSelf()
 *
 * Free object attributes and.
 * 
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_freeSelf(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	HashEntry *entry;
	union PCode *pcode;
	VObjList *olist;
	Attr *varlist;

	if (!exitingViola) {
		objID2Obj->remove(objID2Obj, getIdent(GET_name(self)));
		symStr2ID->remove(symStr2ID, GET_name(self));
		objObj2ExistP->remove(objObj2ExistP, self);
/*
	if (entry = symStr2ID->get(symStr2ID, GET_name(self))) {
		print("FREESELF: in symStr2ID >>>%s<<\n", entry->label);
	}
	if (entry = objID2Obj->get(objID2Obj, getIdent(GET_name(self)))) {
		print("FREESELF: in objID2Obj >>>%s<<\n", entry->label);
	}
	if (entry = objObj2ExistP->get(objObj2ExistP, self)) {
		print("FREESELF: in objObj2ExistP val=%d\n", entry->val);
	}
*/
		Vfree(GET__memoryGroup(self), GET_name(self));

		GET_script(self)->refc--;
		if (GET_script(self)->refc <= 0) {
			Vfree(GET__memoryGroup(self), GET_script(self)->s);
			Vfree(GET__memoryGroup(self), GET_script(self));
		}


		pcode = GET__script(self);
		if (pcode)
			if (--pcode[PCODE_IDX_REFC].i <= 0)
				Vfree(GET__memoryGroup(self),
					GET__script(self));

		/* free variables */
		varlist = GET__varList(self);
		freeVarList(varlist);
	}

	/* free children */
	for (olist = GET__children(self); olist; olist = olist->next) {
		if (olist->o) {
/*
		  	ASSERT(validObjectP(olist->o));
*/
			if (validObjectP(olist->o)) {
				sendMessage1(olist->o, "freeSelf");
				/*XXX make faster*/
			} else {
/*	fprintf(stderr, "ERROR: trying to free (again?) obj %x\n", 
			       olist->o);
*/
			}
		}
	}

	meth_cosmic_freeSelf(self, result, argc, argv);
	return 1;
}

/*
 * XXX
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_gravity(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

int meth_generic_hash(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	result->info.t = storeIdent(PkInfo2Str(&argv[0]));
	result->type = PKT_TOK;
	return 1;
}

/*
 * get(attribute)
 *
 * Get an attribute: active, children, class, message, name, parent, script.
 *
 * Result: the gotten attribute
 * Return: 1 if successful, 0 if error occured
 */
int helper_generic_get(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_name:
		result->info.s = SaveString(GET_name(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;

	case STR_class:
		result->info.s = SaveString(GET_class(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;

	case STR_classScript:
		result->info.s = SaveString(GET_classScript(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;

	case STR_parent:
		result->info.s = SaveString(GET_parent(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;

	case STR_children:
		result->info.s = SaveString(GET_children(self));
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;

	case STR_security:
		result->info.i = GET_security(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_script:
		result->info.s = SaveString(GET_script(self)->s);
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;

	case STR_active:
		result->info.i = GET_active(self);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;
	}
	return 0;
}
int meth_generic_get(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_generic_get(self, result, argc, argv, 
					getIdent(PkInfo2Str(argv)));
}

int meth_generic_getResource(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	result->info.s = GLGetResource(PkInfo2Str(&argv[0]));
	result->canFree = 0;
	result->type = PKT_STR;
	return 1;
}

int meth_generic_getSelection(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return GLGetSelection(result);
}

/*
 * getVariable(variabel name)
 *
 * Get a named variable in the object.
 *
 * Result: the gotten attribute
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_getVariable(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp = PkInfo2Str(&argv[0]);

/*	result->type = PKT_STR;*/
	if (!cp) {
		result->info.s = "";
		result->canFree = 0;
		return 0;
	}
/*	if (getVariable(GET__varList(self), cp, &(result->info.s))) {
		return 1;
	}
*/
	if (getVariable(GET__varList(self), cp, result)) {
		return 1;
	}
	clearPacket(result);
	return 0;
}

/*
 * height()
 * 
 * Result: self's height
 */
int meth_generic_height(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->info.i = GET_height(self);
	result->type = PKT_INT;
	result->canFree = 0;
	return 1;
}

/*
 * initialize()
 *
 * Initializes object.
 *
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_initialize(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp;

	if (!makeArgAttr(self)) {
	  fprintf(stderr, "makeArgAttr() failed\n");
	}

	/* make internal pointer link to parent object */
	if (!GET__parent(self)) {
		if (cp = GET_parent(self)) 
		  if (*cp) SET__parent(self, findObject(getIdent(cp)));
	}

	/* make internal pointer link to children objects */
	if (!GET__children(self)) {
		cp = GET_children(self);
		if (cp) if (*cp) SET__children(self, strOListToOList(cp));
	}
	clearPacket(result);

	return 1;
}

/*
 * int(float|str)
 *
 * Convert single argument to integer.
 *
 * Result: integer type of the argument
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_int(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_INT;
	result->canFree = 0;
	result->info.i = PkInfo2Int(&argv[0]);
	return 1;
}

/*
 * isBlank(str)
 *
 *
 * Result: 1 or 0
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_isBlank(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_INT;
	result->canFree = 0;
	result->info.i = AllBlank(PkInfo2Str(&argv[0])) ? 1 : 0;
	return 1;
}

/* XXX
 * item(str, n1 [,n2])
 *
 * Extract the item(s) ranged by n1 and n2. Comma (,) is the seperating 
 * character.
 *
 * Result: string containing the items
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_item(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

/*
 *
 * Result: key
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_key(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->info.c = keyStat_key;
	result->type = PKT_CHR;
	result->canFree = 0;
	return 1;
}

/*
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_listAllObjects(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 1;
}

/*
 * objectListAppend(objList, obj)
 *
 * Append an object to a named object list.
 *
 * Result: object count, -1 if the list is not found
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_objectListAppend(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *listName;
	VObjList *olist = NULL;

	result->type = PKT_OBJ;
	result->canFree = 0;
	listName = PkInfo2Str(&argv[0]);
	if (!STRCMP(listName, "children")) {
		VObj *obj = PkInfo2Obj(&argv[1]);
		olist = GET__children(self);
		if (obj) {
			SET__children(self, appendObjToList(olist, obj));
			result->info.o = obj;
			return 1;
		}
	}
	result->info.o = NULL;
	return 0;
}
/*
 * objectListAppend_children(obj)
 *
 * Append an object to children list
 *
 * Result: object count, -1 if the list is not found
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_objectListAppend_children(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist = GET__children(self);
	VObj *obj = PkInfo2Obj(&argv[0]);

	result->type = PKT_OBJ;
	result->canFree = 0;
	if (obj) {
		SET__children(self, appendObjToList(olist, obj));
		result->info.o = obj;
		return 1;
	}
	result->info.o = NULL;
	return 0;
}

/*
 * objectListCount(objList)
 *
 * Count the number of objects in a named object list.
 *
 * Result: object count, -1 if the list is not found
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_objectListCount(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int i;
	char *listName;
	VObjList *olist = NULL;

	listName = PkInfo2Str(&argv[0]);
	if (!STRCMP(listName, "children")) {
		olist = GET__children(self);
	}
	result->type = PKT_INT;
	result->canFree = 0;
	if (olist) {
		for (i = 0; olist; olist = olist->next, i++);
		result->info.i = i;
		return 1;
	}
	result->info.i = -1;
	return 0;
}
/*
 * objectListCount_children()
 *
 * Count the number of children list
 *
 * Result: object count, -1 if the list is not found
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_objectListCount_children(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int i;
	VObjList *olist = GET__children(self);

	result->type = PKT_INT;
	result->canFree = 0;
	if (olist) {
		for (i = 0; olist; olist = olist->next, i++);
		result->info.i = i;
		return 1;
	}
	result->info.i = -1;
	return 0;
}

/*
 * objectListDelete(objList, obj)
 *
 * Delte an object from a named object list.
 *
 * Result: object count, -1 if the list is not found
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_objectListDelete(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist = NULL;
	char *listName = PkInfo2Str(argv);

	clearPacket(result);
	if (!STRCMP(listName, "children")) {
		VObj *obj = PkInfo2Obj(&argv[1]);
		olist = GET__children(self);
		if (olist && obj) {
			olist = removeVObjListNode(&olist, obj);
			SET__children(self, olist);
			return 1;
		}
	}
	return 0;
}

/*
 * objectListDelete_children(obj)
 *
 * Delte an object from children list
 *
 * Result: object count, -1 if the list is not found
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_objectListDelete_children(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist = GET__children(self);
	VObj *obj = PkInfo2Obj(&argv[0]);

	clearPacket(result);
	if (olist && obj) {
		olist = removeVObjListNode(&olist, obj);
		SET__children(self, olist);
		return 1;
	}
	return 0;
}

/*
 * objectListPrepend(objList, obj)
 *
 * Prepend an object to a named object list.
 *
 * Result: object count, -1 if the list is not found
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_objectListPrepend(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *listName;
	VObjList *olist = NULL;

	result->type = PKT_OBJ;
	result->canFree = 0;
	listName = PkInfo2Str(&argv[0]);
	if (!STRCMP(listName, "children")) {
		VObj *obj = PkInfo2Obj(&argv[1]);
		olist = GET__children(self);
		if (obj) {
			VObjList *node;
			node = (VObjList*)malloc(sizeof(VObjList));
			node->o = obj;
			if (olist) node->next = olist;
			else node->next = NULL;
			SET__children(self, node);
			result->info.o = obj;
			return 1;
		}
	}
	result->info.o = NULL;
	return 0;
}

/*
 * objectListPrepend_children(obj)
 *
 * Prepend an object to children list
 *
 * Result: object count, -1 if the list is not found
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_objectListPrepend_children(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist = GET__children(self);
	VObj *obj = PkInfo2Obj(&argv[0]);

	result->type = PKT_OBJ;
	result->canFree = 0;
	if (obj) {
		VObjList *node;
		node = (VObjList*)malloc(sizeof(VObjList));
		node->o = obj;
		if (olist) node->next = olist;
		else node->next = NULL;
		SET__children(self, node);
		result->info.o = obj;
		return 1;
	}
	result->info.o = NULL;
	return 0;
}

/*
 * objectListSend(objectListName, arg1, arg2, ..., argn)
 *
 * Send the arguments to each objects in the object list.
 *
 * Result: cleared
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_objectListSend(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist = NULL;
	char *listName;

	listName = PkInfo2Str(&argv[0]);
	if (!STRCMP(listName, "children")) {
		olist = GET__children(self);
	}
	while (olist) {
		sendMessagePackets(olist->o, &argv[1], argc - 1);
		olist = olist->next;
	}
	clearPacket(result);
	return 1;
}

/*
 * objectListSend_children(arg1, arg2, ..., argn)
 *
 * Send the arguments to each objects in the children list.
 *
 * Result: cleared
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_objectListSend_children(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist = GET__children(self);

	while (olist) {
		sendMessagePackets(olist->o, &argv[0], argc);
		olist = olist->next;
	}
	clearPacket(result);
	return 1;
}

/*
 * objectPosition(obj, objList)
 *
 * Return the position an object in the named object list.
 * 
 * Result: position (starting from 0), -1 if object is not found in list
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_objectPosition(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObjList *olist = NULL;
	VObj *obj;
	char *listName;
	int i;

	obj = PkInfo2Obj(&argv[0]);
	listName = PkInfo2Str(&argv[1]);

	if (!STRCMP(listName, "children")) {
		olist = GET__children(self);
	}

	result->type = PKT_INT;
	result->canFree = 0;
	for (i = 0; olist; olist = olist->next, i++) {
		if (olist->o == obj) {
			result->info.i = i;
			return 1;
		}
	}
	result->info.i = -1;
	return 0;
}

/*
 * loadFile(fileName)
 *
 * Loads a file and returns its content.
 *
 * Result: loaded file
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_loadFile(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp, *retStrp, *path;

	if (notSecure(self)) return 0;

	result->type = PKT_STR;
	cp = PkInfo2Str(&argv[0]);
	if (!cp) {
		result->info.s = "";
		result->canFree = 0;
	}
	if (cp = SaveString(vl_expandPath(cp, buff))) {
		if (loadFile(cp, &retStrp) != -1) {
			if (retStrp) {
				result->info.s = retStrp;
				result->canFree = PK_CANFREE_STR;
				free(cp);
				return 1;
			} else {
				sprintf(buff, "loadFile \"%s\" failed.\n", cp);
				messageToUser(self, MESSAGE_ERROR, buff);
			}
		} else {
			sprintf(buff,
				"Unable to open file '%s'. aborted.\n", cp);
			messageToUser(self, MESSAGE_ERROR, buff);
		}
		free(cp);
	} else {
		sprintf(buff, "no such file\n");
		messageToUser(self, MESSAGE_ERROR, buff);
	}
	result->info.s = "";
	result->canFree = 0;
	return 0;
}

int meth_generic_loadSTG(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (notSecure(self)) return 0;
	return loadSTG(PkInfo2Str(&argv[0]));
}
/*
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_makeTempFile(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char tfn[200];
	sprintf(tfn, "%s%ld", tempFileNamePrefix, tempFileNameIDCounter++);
	result->info.s = saveString(tfn);
	result->type = PKT_STR;
	result->canFree = PK_CANFREE_STR;
	return result->info.s ? 1 : 0;
}

/*
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_not(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

/*
 * nthChar(string, n1, [n2]) 
 *
 * Result: character range from n1 to n2
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_nthChar(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (argc == 2) {
		int n;
		char *s;

		result->type = PKT_CHR;
		result->canFree = 0;
		n = PkInfo2Int(&argv[1]);
		s = PkInfo2Str(&argv[0]);
		if (s) result->info.c = s[n];
		else result->info.c = '\0';

	} else if (argc == 3) {
		int i, n1, n2;
		char *cp, *str;

		result->type = PKT_STR;
		str = PkInfo2Str(&argv[0]);
		n1 = PkInfo2Int(&argv[1]);
		n2 = PkInfo2Int(&argv[2]);
		cp = (char*)malloc(sizeof(char) * (n2 - n1 + 2));
		if (!cp) {
			result->info.s = "";
			result->canFree = 0;
			return 0;
		}
		if (str)
			for (i = 0; n1 <= n2; n1++) {
				if (!str[n1]) break;
				cp[i++] = str[n1];
			}
		cp[i] = '\0';
		result->info.s = cp;
		result->canFree = PK_CANFREE_STR;
	}
	return 1;
}

/*
 * nthChild(n)
 * Result:  the n'th object in self's children list.
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_nthChild(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int i = 0, n;
	char *listName;
	VObjList *olist;

	if (argc != 1) {
		clearPacket(result);
		return 0;
	}
	n = PkInfo2Int(&argv[0]);

	olist = GET__children(self);

	result->type = PKT_OBJ;
	result->canFree = 0;
	while (olist) {
		if (i++ >= n) {
			result->info.o = olist->o;
			return 1;
		}
		olist = olist->next;
	}
	result->info.o = NULL;
	return 0;
}

/*
 * nthItem(string, n)
 *
 * 
 *
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_nthItem(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

/*
 * nthLine(str, n1 [,n2])
 *
 * Extracts the specified lines (from line n1 to n2) from the string.
 * Line numbering start from 0.
 *
 * Result: the specified line(s)
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_nthLine(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int li, hi, lines, size;
	char *str;

	result->type = PKT_STR;
	str = PkInfo2Str(&argv[0]);
	if (str) {
		if (argc == 2) {
			li = hi = PkInfo2Int(&argv[1]);
		} else if (argc == 3) {
			li = PkInfo2Int(&argv[1]);
			hi = PkInfo2Int(&argv[2]);
		} else {
			/* incorrect number of arguments */
			result->info.s = "";
			result->canFree = 0;
			return 0;
		}
		result->canFree = PK_CANFREE_STR;
		if (result->info.s = getLines(li, hi, str, &size)) return 1;
	}
	result->info.s = "";
	result->canFree = 0;
	return 0;
}

/*
 * nthObjectInList(n, objectList)
 *
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_nthObjectInList(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int i = 0, n;
	char *listName;
	VObjList *olist;

	if (argc != 2) {
		clearPacket(result);
		return 0;
	}
	n = PkInfo2Int(&argv[0]);
	listName = PkInfo2Str(&argv[1]);

	if (!STRCMP(listName, "children")) {
		olist = GET__children(self);
	}

	result->type = PKT_OBJ;
	result->canFree = 0;
	while (olist) {
		if (i++ >= n) {
			result->info.o = olist->o;
			return 1;
		}
		olist = olist->next;
	}
	result->info.o = NULL;
	return 0;
}


/*
 * nthSibling(n)
 *
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_nthSibling(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int i = 0, n;
	char *listName;
	VObjList *olist;
	VObj *parent;

	if (argc != 1) {
		clearPacket(result);
		return 0;
	}
	n = PkInfo2Int(&argv[0]);

	parent = GET__parent(self);
	if (parent) {
		olist = GET__children(parent);
		result->type = PKT_OBJ;
		result->canFree = 0;
		while (olist) {
			if (i++ >= n) {
				result->info.o = olist->o;
				return 1;
			}
			olist = olist->next;
		}
		result->info.o = NULL;
	}
	return 0;
}

/*
 * nthWord(str, n1, n2)
 *
 * Result: the word string if successful, or "" if failed.n
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_nthWord(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *str;
	int n1, n2;

	str = PkInfo2Str(&argv[0]);
	if (!str) {
		result->type = PKT_STR;
		result->info.s = "";
		result->canFree = 0;
		return 0;
	} else {
		str = SaveString(str);
		result->type = PKT_STR;
		if (argc == 2) {
			n1 = n2 = PkInfo2Int(&argv[1]);
		} else if (argc == 3) {
			n1 = PkInfo2Int(&argv[1]);
			n2 = PkInfo2Int(&argv[2]);
		} else {
			result->info.s = "";
			result->canFree = 0;
			return 0;
		}
		extractWord(str, n1, n2, buff);
		result->info.s = SaveString(buff);
		result->canFree = PK_CANFREE_STR;
		free(str); /* argh! */
	}
	return 1;
}

/*
 * parent()
 * 
 * Result: parent object
 */
int meth_generic_parent(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->info.o = GET__parent(self);
	result->type = PKT_OBJ;
	result->canFree = 0;
	return 1;
}

int meth_generic_pipe(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int c;
	FILE *fp;
	char *buffp;

	clearPacket(result);

	fp = popen(PkInfo2Str(&argv[0]), PkInfo2Str(&argv[1]));
	if (!fp) {
		return 0;
	} else {
		buffp = buff;
		while ((c = fgetc(fp)) != EOF) {
			*buffp = c;
			buffp++;
		}
		*buffp = '\0';
		pclose(fp);
		result->info.s = SaveString(buff);
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		return 1;
	}
}

/*
 * prepend(<list>, id, value)
 *
 * Prepend item to list
 *
 * Result:
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_prepend(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return 1;
}

/*
 * print(argument> [, ...])
 *
 * Print arguments to standard output.
 *
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_print(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int i;

	clearPacket(result);
	for (i = 0; i < argc; i++) {
		switch (argv[i].type) {
		case PKT_OBJ:
			if (argv[i].info.o)
				printf("%s", GET_name(argv[i].info.o));
			else 
				printf("(NULL)");
		continue;
		case PKT_INT:
			printf("%d", argv[i].info.i);
		continue;
		case PKT_FLT:
			printf("%f", argv[i].info.f);
		continue;
		case PKT_CHR:
			printf("%c", argv[i].info.c);
		continue;
		case PKT_STR:
			if (argv[i].info.s)
				printf("%s", argv[i].info.s);
			else 
				printf("(NULL)");
		continue;
		case PKT_ARY:
			if (argv[i].info.y) {
				int n;
				Array *array = argv[i].info.y;
				for (n = 0; n < array->size; n++)
					printf("%d ", array->info[n]);
			}
		continue;
		case PKT_ATR: {
			Attr *attrp;
			attrp = argv[i].info.a;
			for (; attrp; attrp = attrp->next) {
				printf("id=%d val=%d:", 
					attrp->id, attrp->val);
				dumpPacket((Packet*)attrp->val);
				printf("\n");
			}
		}
		continue;
		default:
			printf("??");
			dumpPacket(&argv[i]);
			return 0;
		}
	}
	return 1;
}

/*
 * printf(argument [, ...])
 *
 * Print arguments to standard output.
 *
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_printf(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int i;

	clearPacket(result);
	for (i = 0; i < argc; i++) {
		switch (argv[i].type) {
		case PKT_OBJ:
			printf("%s", GET_name(argv[i].info.o));
		continue;
		case PKT_INT:
			printf("%d", argv[i].info.i);
		continue;
		case PKT_FLT:
			printf("%f", argv[i].info.f);
		continue;
		case PKT_CHR:
			printf("%c", argv[i].info.c);
		continue;
		case PKT_STR:
			printf("%s", argv[i].info.s);
		continue;
		case PKT_ARY:
			if (argv[i].info.y) {
				int n;
				Array *array = argv[i].info.y;
				for (n = 0; n < array->size; n++)
					printf("%d ", array->info[n]);
			}
		continue;
		default:
			printf("??");
			dumpPacket(&argv[i]);
			return 0;
		}
	}
	return 1;
}

int meth_generic_random(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_INT;
	result->canFree = 0;

#if defined(SVR4)
	result->info.i = random((unsigned)PkInfo2Int(&argv[0]));
#else
#ifdef i386
	result->info.i = rand();
#else 
	result->info.i = rand((unsigned)PkInfo2Int(&argv[0]));
#endif
#endif
	return 1;
}

/* replaceChar(originalStr, charToReplace, newChar)
 */
int meth_generic_replaceChar(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp;
	char *inStr, patChar, repChar;

	inStr = SaveString(PkInfo2Str(&argv[0]));
	patChar = PkInfo2Char(&argv[1]);
	repChar = PkInfo2Char(&argv[2]);

	for (cp = inStr; *cp; cp++) 
		if (*cp == patChar) *cp = repChar;

	result->type = PKT_STR;
	result->info.s = inStr;
	result->canFree = PK_CANFREE_STR;
	return 1;
}


/* replaceCharQ(originalStr, charToReplace, newChar)
 */
int meth_generic_replaceCharQ(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp;
	char *inStr, patChar, repChar;

	inStr = PkInfo2Str(&argv[0]);
	patChar = PkInfo2Char(&argv[1]);
	repChar = PkInfo2Char(&argv[2]);

	for (cp = inStr; *cp; cp++)
		if (*cp == patChar) *cp = repChar;

	result->type = PKT_STR;
	result->info.s = inStr;
	result->canFree = 0;
	return 1;
}

/* replaceStr(originalStr, pattern, patternReplaceStr)
 */
int meth_generic_replaceStr(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp, *lp;
	char *inStr, *patStr, *repStr;
	int ii = 0, pi = 0;
	int inLength, patLength, repLength;

	inStr = PkInfo2Str(&argv[0]);
	patStr = PkInfo2Str(&argv[1]);
	repStr = PkInfo2Str(&argv[3]);
	inLength = strlen(inStr);
	patLength = strlen(patStr);
	repLength = strlen(repStr);

	result->type = PKT_STR;
	lp = inStr;
	for (cp = inStr; *cp; cp++) {
		if (*cp == patStr[pi]) {
			if (++pi >= patLength) {
				strncat(buff, lp, cp - lp);
				lp = cp;
				strcat(buff, repStr);
			}
			if (ii >= inLength) {
				break;
			}
		} else {
			pi = 0;
		}
		ii++;
	}
	strncat(buff, lp, cp - lp);
	result->info.s = SaveString(buff);
	result->canFree = PK_CANFREE_STR;
	return 1;
}

int meth_generic_replaceStrQ(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_STR;
	result->info.s = "NOT IMPLEMENTED";
	result->canFree = 0;
	return 1;
}

/*
 * saveFile(fileName, str)
 *
 * Save a file and returns its content.
 *
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_saveFile(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp;

	if (notSecure(self)) return 0;

	result->type = PKT_INT;
	result->canFree = 0;
	if (cp = SaveString(vl_expandPath(PkInfo2Str(&argv[0]), buff))) {
		if (saveFile(cp, PkInfo2Str(&argv[1])) != -1) {
			free(cp);
			result->info.i = 1;
			return 1;
		}
	}
	sprintf(buff, "saveFile(\"%s\", data) failed.\n", cp);
	messageToUser(self, MESSAGE_ERROR, buff);
	result->info.i = 0;
	return 0;
}

/*
 * scan(argument [, ...])
 *
 * Print arguments to standard output.
 *
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_scan(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int i;
	clearPacket(result);
	return 0;
}

/*
 * scanf(argument [, ...])
 *
 * Print arguments to standard output.
 *
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_scanf(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int i;

	clearPacket(result);
	for (i = 0; i < argc; i++) {
		switch (argv[i].type) {
		case PKT_OBJ:
			printf("%s", GET_name(argv[i].info.o));
		continue;
		case PKT_INT:
			printf("%d", argv[i].info.i);
		continue;
		case PKT_FLT:
			printf("%f", argv[i].info.f);
		continue;
		case PKT_CHR:
			printf("%c", argv[i].info.c);
		continue;
		case PKT_STR:
			printf("%s", argv[i].info.s);
		continue;
		case PKT_ARY:
			if (argv[i].info.y) {
				int n;
				Array *array = argv[i].info.y;
				for (n = 0; n < array->size; n++)
					printf("%d ", array->info[n]);
			}
		continue;
		default:
			printf("??");
			dumpPacket(&argv[i]);
			return 0;
		}
	}
	return 1;
}

/*
 * securityMode()
 *
 * forces security mode for newly created objects. all objects created
 * during the time when mode > 0 will have the security value set to 
 * the mode value. Can alter securityMode value only if mode == 0;
 *
 * Result: mode
 */
int meth_generic_securityMode(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (securityMode == 0) securityMode = PkInfo2Int(&argv[1]);

	clearPacket(result);
	result->type = PKT_INT;
	result->info.i = securityMode;
	return 1;
}

/*
 * self()
 * 
 * Result: self object
 */
int meth_generic_self(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->info.o = self;
	result->type = PKT_OBJ;
	result->canFree = 0;
	return 1;
}

/*
 * selectionInfo()
 *
 * Result: [0] object
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_selectionInfo(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Packet *packet0 = makePacket();
	Attr *attrp;
	result->type = PKT_ATR;
	result->info.a = attrp = makeAttr(0, packet0);
	result->canFree = PK_CANFREE_STR;

	if (xselectionObj) {
		packet0->info.o = xselectionObj;
		packet0->type = PKT_OBJ;
		packet0->canFree = 0;
		return 1;
	} else {
		packet0->info.o = NULL;
		packet0->type = PKT_OBJ;
		packet0->canFree = 0;
		return 0;
	}
}

helper_setSecurity(self, level)
	VObj *self;
	int level;
{
	VObjList *olist;

	SET_security(self, level);
	for (olist = GET__children(self); olist; olist = olist->next)
		helper_setSecurity(olist->o, level);
}

/*
 * returns non-zero if set operation succeded, zero otherwise.
 *
 *
 * Result: ...
 * Return: 1 if successful, 0 if error occured
 */
int helper_generic_set(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	char *str;

	switch (labelID) {
	case STR_name:

		objID2Obj->remove(objID2Obj, getIdent(GET_name(self)));

		/* Until object names have their own name space, 
		 * don't remove them... lest risk deleting keywords 
		 */
/*		symStr2ID->remove(symStr2ID, GET_name(self));*/

		if (GET_name(self)) Vfree(GET__memoryGroup(self), 
						GET_name(self));
		result->info.s = VSaveString(GET__memoryGroup(self), 
					     PkInfo2Str(&argv[1]));
		SET_name(self, result->info.s);
		result->type = PKT_STR;
		result->canFree = 0;
		objID2Obj->put_replace(objID2Obj, 
			storeIdent(saveString(GET_name(self))), (int)self);
		return 1;

	case STR_parent: {
		HashEntry *entry;
		VObj *obj;

		result->info.s = VSaveString(GET__memoryGroup(self), 
					     PkInfo2Str(&argv[1]));
		SET_parent(self, result->info.s);
		result->type = PKT_STR;
		result->canFree = 0;
		if (entry = symStr2ID->get(symStr2ID, (int)result->info.s))
			if (obj = findObject(entry->val)) {
				SET__parent(self, obj);
				return 1;
			}
		SET__parent(self, NULL);
		return 1;
		}

	case STR_children: {
		char *cp;
		VObjList *objl;

		result->type = PKT_INT;
		result->canFree = 0;
		cp = VSaveString(GET__memoryGroup(self),
				 PkInfo2Str(&argv[1]));
		if (cp) {
			objl = strOListToOList(cp);
		}
		if (objl) {
			/* free olist GET__children(); */
			SET_children(self, cp);
			SET__children(self, objl);
			result->info.i = 1;
			return 1;
		} else {
			SET_children(self, "");
			SET__children(self, NULL);
			result->info.i = 0;
			return 0;
		}
	}

	case STR_security:
		result->info.i = PkInfo2Int(&argv[1]);
		helper_setSecurity(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	case STR_script: {
		union PCode *pcode = GET__script(self);
		StrInfo *si;

		GET_script(self)->refc--;
		if (GET_script(self)->refc <= 0) {
			Vfree(GET__memoryGroup(self), GET_script(self)->s);
			Vfree(GET__memoryGroup(self), GET_script(self));
		}
		result->info.s = VSaveString(GET__memoryGroup(self), 
					     PkInfo2Str(&argv[1]));
		si = (StrInfo*)malloc(sizeof(struct StrInfo));
		si->s = result->info.s;
		si->refc = 1;
		SET_script(self, si);
		result->type = PKT_STR;
		result->canFree = 0;
		if (pcode) {
			Vfree(GET__memoryGroup(self), pcode);
			SET__script(self, NULL);
		}
		return 1;
	}

	case STR_active:
		result->info.i = PkInfo2Int(&argv[1]);
		SET_active(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;
	}
	clearPacket(result);
	return 0;
}
int meth_generic_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_generic_set(self, result, argc, argv, 
					getIdent(PkInfo2Str(argv)));
}

/* Scott */
/*
 * This routine forwards a Viola script message to the program
 * interface via the MessageHandler mechanism.
 */
int meth_generic_sendToInterface(self, result, argc, argv)
    	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
    int i;
    char **arg = (char **) malloc(argc * sizeof(char *));

    for (i=0; i<argc; i++) arg[i] = saveString(PkInfo2Str(&argv[i]));

    ViolaInvokeMessageHandler(arg, argc);

    for (i=0; i<argc; i++) free(arg[i]);
    free(arg);

    return 1;
}

int meth_generic_setResource(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (notSecure(self)) return 0;
	return 0; /* no can't do */
}

/*
 * setSelection(value)
 *
 * Result: 
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_setSelection(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	GLSetSelection(self, SaveString(PkInfo2Str(&argv[0])));
	return 1;
}

/*
 * setVariable(attribute, value)
 *
 * Result: 
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_setVariable(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	return 0;
}

/*
 * setMouse(x, y)
 *
 * Result: the root offset
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_setMouse(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
/*	Window w = PkInfo2Int(argv);*/
	int x = PkInfo2Int(&argv[0]);
	int y = PkInfo2Int(&argv[1]);
/*
	if (!w) {
		w = rootWindow;
	}
*/
	/* relative to root */
	clearPacket(result);
	XWarpPointer(display, None, rootWindow, 0,0,0,0, x, y);

	return 1;
}

/*
 * shiftKeyP()
 *
 *
 * Result: shift key state
 * Return: 1
 */
int meth_generic_shiftKeyP(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_INT;
	result->info.i = keyStat_shift;
	result->canFree = 0;
	return 1;
}

/*
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_sin(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_FLT;
	result->info.f = sin((double)(PkInfo2Flt(&argv[0]) * 
					RADIAN_TO_DEGREE_RATIO));
	return 1;
}

/*
 * sleep(sec)
 *
 * Result: seconds slept
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_sleep(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_INT;
	result->canFree = 0;
	result->info.i = sleep(PkInfo2Int(&argv[0]));
	return 1;
}

/*
 * sprintf(format, argument [, ...])
 *
 * format print
 *
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_sprintf(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int i;
/*XXX NOT IMPLEMENTED*/

/*
	clearPacket(result);
	for (i = 0; i < argc; i++) {
		switch (argv[i].type) {
		case PKT_OBJ:
			printf("%s", GET_name(argv[i].info.o));
		continue;
		case PKT_INT:
			printf("%d", argv[i].info.i);
		continue;
		case PKT_FLT:
			printf("%f", argv[i].info.f);
		continue;
		case PKT_CHR:
			printf("%c", argv[i].info.c);
		continue;
		case PKT_STR:
			printf("%s", argv[i].info.s);
		continue;
		case PKT_ARY:
			if (argv[i].info.y) {
				int n;
				Array *array = argv[i].info.y;
				for (n = 0; n < array->size; n++)
					printf("%d ", array->info[n]);
			}
		continue;
		default:
			printf("??");
			dumpPacket(&argv[i]);
			return 0;
		}
	}
*/
	return 1;
}

/*
 * str(argument [, ...])
 *
 * Result: arguments in string type
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_str(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->type = PKT_STR;
	result->info.s = PkInfos2Str(argc, argv);
	result->canFree = 0;
	return 1;
}

/*
 * strlen(str)
 */
/*
 * Result: strlen
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_strlen(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	clearPacket(result);
	result->type = PKT_INT;
	result->info.i = strlen(PkInfos2Str(argc, argv));
	return 1;
}

/*
 * system(cmdline1 [, cmdline2 , ...])
 *
 * Call to the system(3) call.
 *
 * Result: return value from system()
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_system(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int i;

	if (notSecure(self)) return 0;

	clearPacket(result);
	result->type = PKT_INT;
	for (i = 0; i < argc; i++) 
		result->info.i = system(PkInfo2Str(&argv[i]));

	return 1;
}

/*
 * target() 
 *
 * Return the object that was mark()'ed.
 *
 * Result: target object
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_target(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	char *cp;

	result->info.o = VTargetObj;
	result->type = PKT_OBJ;
	result->canFree = 0;
	if (argc == 0) {
		return 1;
	}
	cp = PkInfo2Str(&argv[0]);
	if (cp) 
		if (VTargetObj = findObject(getIdent(cp))) {
			sendMessage1(VResourceObj, "targetSet");
			result->info.o = VTargetObj;
			result->type = PKT_OBJ;
			result->canFree = 0;
			return 1;
		}
	return 0;
}

int meth_generic_textWidth(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->info.i = GLTextWidth(PkInfo2Int(&argv[0]), 
				     PkInfo2Str(&argv[1]));
	result->type = PKT_INT;
	result->canFree = 0;
	return 1;
}

int meth_generic_trimEdge(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->info.s = trimEdgeSpaces(SaveString(PkInfo2Str(&argv[0])));
	result->type = PKT_STR;
	result->canFree = PK_CANFREE_STR;
	return 1;
}

int meth_generic_trimEdgeQ(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	/* NOTE: since the returned data isn't copied, argv can no longer
	 * think this data may be canFree. Because, if that were so,
	 * upon return to the interpreter, argv->info.s will be freed,
	 * and that leaves result->info.s pointing to freed memory 
	 */
/*	if (argv[0].canFree) {
		printf("Potential leak!! in meth_generic_trimEdgeQ()\n");
	}
*/
	result->canFree = 0;
	argv[0].canFree = 0;

	result->info.s = trimEdgeSpaces(PkInfo2Str(&argv[0]));
	result->type = PKT_STR;
	
	return 1;
}

/*
 * time() 
 *
 * Result: elapsed seconds since 00:00:00 GMT, January 1, 1970
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_time(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	time_t theTime;
	result->info.i = time(&theTime);
	result->type = PKT_INT;
	result->canFree = 0;
	return 1;
}

/*
 * tool([tool name]);
 *
 * Tool name: "action", "move", "reparent", "resize", "target"
 * if no argument is given, the current tool is returned.
 *
 * Result: name of the selected/current tool
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_tool(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int i;
	char *cp;

	result->type = PKT_STR;
	if (argc == 0) {
		result->info.s = toolID2Str[currentTool];
		result->canFree = 0;
		return 1;
	}
	if (cp = PkInfos2Str(argc, argv)) {
		for (i = 0; toolID2Str[i]; i++) {
			if (!STRCMP(cp, toolID2Str[i])) {
				currentTool = i;
				result->info.s = toolID2Str[currentTool];
				result->canFree = 0;
				return 1;
			}
		}
	}
	result->info.s = "";
	result->canFree = 0;
	return 0;
}

int meth_generic_unhash(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (notSecure(self)) return 0;
	clearPacket(result);
	result->info.s = (char*)symID2Str->get(symID2Str, 
						(PkInfo2Int(&argv[0])))->val;
	result->type = PKT_STR;
	result->canFree = 0;
	return 1;
}

/*
 * version();
 *
 * Return the Viola version identifier string.
 *
 * Result: version string
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_version(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	extern char *viola_version;	/* defined in main.c */
	result->info.s = viola_version;
	result->type = PKT_STR;
	result->canFree = 0;
	return 1;
}

/*
 * violaPath(str)
 *
 * Set a new path for searching viola object files.
 *
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_violaPath(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (argc == 0) {
		int i = 0;
		extern char *viola_path[];

		result->type = PKT_STR;
		buff[0] = '\0';
		while (viola_path[i]) {
			strcat(buff, viola_path[i]);
			if (viola_path[++i]) strcat(buff, " ");
		}
		result->info.s = SaveString(buff);
		result->canFree = PK_CANFREE_STR;
	} else if (argc == 1) {
		result->type = PKT_INT;
		result->canFree = 0;
		if ((result->info.i = setViolaPath(PkInfo2Str(&argv[0]))) > 0)
			return 1;
	}
	return 0;
}

/*
 * x()
 * 
 * Result: self's x position
 */
int meth_generic_x(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->info.i = GET_x(self);
	result->type = PKT_INT;
	result->canFree = 0;
	return 1;
}

/*
 * watch(flag)
 *
 * Specifies variable tracking flags. NOT YET IMPLEMENTED.
 *
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_watch(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	extern int flag_vwatch; /* defined in cexec.c */

	clearPacket(result);
	flag_vwatch = PkInfo2Int(&argv[0]);

	return 1;
}

/*
 * width()
 * 
 * Result: self's width
 */
int meth_generic_width(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->info.i = GET_width(self);
	result->type = PKT_INT;
	result->canFree = 0;
	return 1;
}

/*
 * 
 * 
 * Result: unaffected
 * Return: 1 if successful, 0 if error occured
 */
int meth_generic_writeln(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	if (meth_generic_print(self, result, argc, argv)) {
		printf("\n");
		return 1;
	} else {
		return 0;
	}
}

/*
 * y()
 * 
 * Result: self's y position
 */
int meth_generic_y(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	result->info.i = GET_y(self);
	result->type = PKT_INT;
	result->canFree = 0;
	return 1;
}

/* USE WITH EXTREME CAUTION. kludge for text-inlined images
 * 
 * (obj)arg[0] 's __content us used to store pic list.
 * (char*)arg[1] is filepath of picture
 */
int meth_generic_addPicFromFile(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	VObj *obj = PkInfo2Obj(&argv[0]);
	int picID;
	TFPic *pics, *pic;
	char *fname = PkInfo2Str(&argv[1]);

	pics = (TFPic*)GET__content(obj);
	pic = tfed_addPicFromFile(&pics, fname/*should be URL*/, fname);
	SET__content(obj, pics);

	picID = pic->id;

	result->type = PKT_INT;
	result->canFree = 0;
	result->info.i = picID;

	return 1;
}



/** 
 ** Major ViolaWWW kludge .. er, optimization methods
 **/
/*
 * HTML_txt object script equivalent method
 */
int meth_generic_code_HTML_txt(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Packet *arg0 = (Packet*)((Attr*)(argv[0].info.a)->val);
	char *mesg;
	int vspan;
	SGMLTagMappingInfo *tmi;
	char *text, *s;

if (arg0->type == PKT_INT/*XXX TOK*/) {

	if (arg0->info.i == STR_D) {

	/* Viola script:
	 */
	/*************************************************************
	 * switch (arg[0]) {
	 * case 'D':
	 *	txt = send(parent(), "data");
	 *	if (isBlank(txt)) {
	 *		style = SGMLGetStyle("HTML", "P");
	 *		return (style[0] + style[1]);
	 *	} else {
	 *		set("content", txt);
	 *		style = SGMLGetStyle("HTML", "P");
NOT TRUE ANYMORE. 
	 *		return (style[0] + building_vspan() + style[1]);
IS NOW: need to clean up.
	 *		return building_vspan();
	 *	}
	 * break;
	 ************************************************************/

	/* Equivalent C code:
	 */
		TFStruct *tf = GET__TFStruct(self);
		Packet *pk, evalResult;

		nullPacket(&evalResult);

		if (GET__parent(self)) {
			sendMessage1_result(GET__parent(self), "data", 
					    &evalResult);
			text = evalResult.info.s;

			if (evalResult.type == PKT_INT) {
			  /* if it's a number, then no data, but offset-y
			   * to make insertion. kludge allert!XXX
			   */
				SET_y(self, GET_y(self) + evalResult.info.i);

				/* return some reasonable height for this
				 * empty paragraph... probably inserted (in
				 * HTML) to get vertical space. ...
				 */
				freePossibleDangler(result);
				result->info.i = 10;
				result->type = PKT_INT;
				return 1;
			}
			if (!text) {
				return 0;
			}
			if (evalResult.canFree & PK_CANFREE_STR == 0) {
				text = saveString(evalResult.info.s);
			}

			if (*text == '\0') {

				tmi = (SGMLTagMappingInfo*)GET__content2(self);
				freePossibleDangler(&evalResult);
				result->info.i = tmi->top + tmi->bottom;
				result->type = PKT_INT;

			} else {
				if (GET_content(self)) 
					Vfree(GET__memoryGroup(self), 
						GET_content(self));

				/* eliminate leading space */
				for (s = text; *s; s++) if (isprint(*s)) break;
				
				SET_content(self, s);
				tf = tfed_updateTFStruct(self, s);
				if (!tf) return 0;

				vspan = tf->building_vspan + tf->yUL + 2;
	
				clearPacket(&evalResult);

				tmi = (SGMLTagMappingInfo*)GET__content2(self);
				
				result->info.i = tmi->top + vspan;

				result->type = PKT_INT;
			}
			return 1;
		} else {
			return 0;
		}
	}

	/*************************************************************
	 * usual();
	 ************************************************************/	
	return meth_cosmic_usual(self, result, 0, argv);

}
	mesg = PkInfo2Str(arg0);

	if (*mesg == 'R') {

	/*************************************************************
	 * case 'R':
	 * 	clearWindow();
	 * 	style = SGMLGetStyle("HTML", "P");
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
		XWindowChanges wc;

		tmi = (SGMLTagMappingInfo*)GET__content2(self);
		if (!tmi) {
			int i;
			for (i = 0; SGMLForms[i].DTDName; i++) {
				if (!STRCMP(SGMLForms[i].DTDName, "HTML")) {
					dmi = &SGMLForms[i];
					break;
				}
			}
			tmi = findTMI(dmi->tagMap, "P");
			SET__content2(self, tmi);
		}

		vspan = tmi->top;

		SET_y(self, arg1->info.i + vspan);
		SET_x(self, tmi->left);
		SET_width(self, arg2->info.i - tmi->left - tmi->right);

		tf = tfed_updateTFStruct(self, GET_content(self));
		if (!tf) return 0;

		SET_height(self, tf->building_vspan + tf->yUL);

		if (GET_window(self)) {
			wc.x = GET_x(self);
			wc.y = GET_y(self);
			wc.width = GET_width(self);
			wc.height = GET_height(self);
			XConfigureWindow(display, GET_window(self), 
					CWX | CWY | CWWidth | CWHeight, 
					&wc);
		}
		vspan += GET_height(self) + tmi->bottom;

		meth_txtDisp_render(self, result, 0, result);

		freePossibleDangler(result);
		result->info.i = vspan;
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;

	} else if (!STRCMP(mesg, "setRef")) {
	
	/*************************************************************
	 * case "setRef":
	 *	return;
	 * break;
	 ************************************************************/	
		return 1;

	} else if (!STRCMP(mesg, "make")) {

	/*************************************************************
	 * case "make":
	 *	 * arg[1]	parent
	 *	 * arg[2]	data
	 *	 * arg[3]	x
	 *	 * arg[4]	width
	 *	 * arg[5]	y
	 *	 * arg[6]	anchorName
	 *	 *
	 *	set("parent", arg[1]);
	 *	set("x", arg[3]);
	 *	set("y", arg[5]);
	 *	set("width", arg[4]);
	 *	set("content", arg[2]);
	 *	h = building_vspan();
	 *	set("height", h);
	 *	set("maxHeight", h);
	 *	style = SGMLGetStyle("HTML", "P");
	 *	h = style[0] + h + style[1];
	 *	if (arg[6]) set("content2", arg[6]);
	 *	return h;
	 * break;
	 ************************************************************/	

		Packet *arg1, *arg2, *arg3, *arg4, *arg5, *arg6;
		TFStruct *tf = GET__TFStruct(self);
		SGMLDocMappingInfo *dmi = NULL;
		int vspan;
		Attr *attr, *varlist = GET__varList(self);
		Packet *pk, evalResult;
		int inPreP = 0;
		VObj *parent;
		int mask;
		
		nullPacket(&evalResult);

		attr = argv[0].info.a;
		arg1 = (Packet*)(attr->next->val);
		arg2 = (Packet*)(attr->next->next->val);
		arg3 = (Packet*)(attr->next->next->next->val);
		arg4 = (Packet*)(attr->next->next->next->next->val);
		arg5 = (Packet*)(attr->next->next->next->next->next->val);
		arg6 = (Packet*)(attr->next->next->next->next->next->next->val);

		parent = (VObj *)arg1->info.i;

		pk = (Packet*)getVariable_id(varlist, STR_inPreP);
		if (pk) {
			inPreP = PkInfo2Int(pk);
		} else {
			/* find out and set inPreP */
			if (parent) {
				sendMessage1_result(parent, "inPreP", 
						    &evalResult);
				inPreP = PkInfo2Int(&evalResult);
			} else {
				inPreP = 0;
			}
		}

		SET__parent(self, parent);

		SET_x(self, arg3->info.i);

		SET_y(self, arg5->info.i);

		SET_width(self, arg4->info.i);

		if (GET_content(self)) 
			Vfree(GET__memoryGroup(self), GET_content(self));

		if (arg2->canFree & PK_CANFREE_STR) {
			text = arg2->info.s;
			if (!inPreP) {
				/* eliminate leading space */
				for (text; *text; text++) 
					if (isprint(*text)) break;
			}
			arg2->canFree = 0;
			arg2->info.s = NULL;
		} else {
			/* eliminate leading space */
			s = arg2->info.s;
			if (!inPreP) {
				/* eliminate leading space */
				for (; *s; s++) if (isprint(*s)) break;
			}
			text = saveString(s);
		}
		SET_content(self, text);
		tf = tfed_updateTFStruct(self, text);
		if (!tf) return 0;

		vspan = tf->building_vspan + tf->yUL + 2;

		SET_height(self, vspan);
		SET_maxHeight(self, vspan);

		tmi = (SGMLTagMappingInfo*)GET__content2(self);
		if (!tmi) {
			int i;
			for (i = 0; SGMLForms[i].DTDName; i++) {
				if (!STRCMP(SGMLForms[i].DTDName, "HTML")) {
					dmi = &SGMLForms[i];
					break;
				}
			}
			tmi = findTMI(dmi->tagMap, "P");
			SET__content2(self, tmi);
		}

		mask = GET__eventMask(self) | PointerMotionMask;
		SET__eventMask(self, mask);

		vspan += tmi->top + tmi->bottom;

		if (arg6->info.s) SET_content2(self, arg6->info.s);

		result->info.i = vspan;
		result->type = PKT_INT;
		return 1;

	} else if (!STRCMP(mesg, "config")) {

	/*************************************************************
	 * case "config":
	 *	return;
	 * break;
	 ************************************************************/	
		return 1;

	} else if (!STRCMP(mesg, "clone")) {

		return txtDisp_HTML_txt_clone(self, result, argv);

#ifdef NOT_CORRECT

	} else if (!STRCMP(mesg, "gotoAnchor")) {

	/*************************************************************
XXXX INVALID
	 * case "gotoAnchor":
	 *	if (arg[1] == get("content2")) return y();
	 * 	n = countChildren();
	 *	if (n > 0) {
	 *		for (i = 0; i < n; i = i + 1) {
	 *			offset = send(nthChild(i), 
	 *					"gotoAnchor", arg[1]);
	 *			if (offset > 0) return offset;
	 *		}
	 *	}
	 *	return 0;
	 * break;
	 ************************************************************/	

		Packet *arg1 = (Packet*)((Attr*)(argv[0].info.a)->
				next->val);
		VObjList *olist;
		int offset = 0;
		char *anchor = arg1->info.s;
		Packet evalResult;

		nullPacket(&evalResult);

		if (!STRCMP(anchor, GET_content2(self))) {
			result->info.i = GET_y(self);
			result->type = PKT_INT;
			result->canFree = 0;
			return 1;
		}

		for (olist = GET__children(self); olist; olist = olist->next) {
			sendMessage1N1str(olist->o, "gotoAnchor", anchor);
			if (offset = PkInfo2Int(&evalResult)) break;
		}
		clearPacket(&evalResult);
		result->info.i = offset;
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;
#endif
	} else if (!STRCMP(mesg, "follow_href")) {
		/* return send(parent(), "show_request_by_child", 
		 * 		arg[1], docURL);
		 */
		Packet *arg1 = (Packet*)((Attr*)(argv[0].info.a)->next->val);
		Attr *varlist = GET__varList(self);
		Packet *pk;	

		pk = (Packet*)getVariable_id(varlist, STR_docURL);
		if (pk) {
			if (pk->info.s && arg1->info.s) {
				sendMessage1N2str(GET__parent(self),
					   "show_request_by_child",
					   arg1->info.s, pk->info.s);
			}
		}
		return 1;

	} else if (!STRCMP(mesg, "buttonRelease")) {

		return txtDisp_HTML_txt_buttonRelease(self, result, argv);

	} else if (!STRCMP(mesg, "expose")) {
	/*************************************************************
	 * just to cut thru usual() script etc... for faster response.
	 ************************************************************/	
		return txtDisp_HTML_txt_expose(self, result, argv);
	}

	/*************************************************************
	 * usual();
	 ************************************************************/	
	return meth_cosmic_usual(self, result, 0, argv);
}

int meth_generic_code_HTML_txtAnchor(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return 1;
}

/*
 * HTML_header_large object script equivalent method
 */
int meth_generic_code_HTML_header_large(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	Packet *arg0 = (Packet*)((Attr*)(argv[0].info.a)->val);
	char *mesg;
	int vspan;
	SGMLTagMappingInfo *tmi;
	char *text;

	if (arg0->type == PKT_INT) {
		if (arg0->info.i == STR_D) {
			return txtDisp_HTML_header_D(self, result, argv);
		} else if (arg0->info.i == STR_F) {
			freePossibleDangler(result);
			result->info.i = -1;
			result->type = PKT_INT;
			result->canFree = 0;
			return 1;
		}
		return meth_cosmic_usual(self, result, 0, argv);
	}

	mesg = PkInfo2Str(arg0);

	if (*mesg == 'R') {
		return txtDisp_HTML_header_R(self, result, argv, "H1");
	} else if (*mesg == 'A') {
		return txtDisp_HTML_header_A(self, result, argv);
	} else if (!STRCMP(mesg, "clone")) {
		return txtDisp_HTML_txt_clone(self, result, argv);
	} else if (!STRCMP(mesg, "setRef")) {
		return 1;
	} else if (*mesg == 'I') {
		return 1;
	} else if (!STRCMP(mesg, "config")) {
		return 1;
	} else if (!STRCMP(mesg, "gotoAnchor")) {
		return 0;
	} else if (!STRCMP(mesg, "buttonRelease")) {
		return txtDisp_HTML_txt_buttonRelease(self, result, argv);
	} else if (!STRCMP(mesg, "expose")) {
		/* just to cut thru usual() script etc for faster response */
		return txtDisp_HTML_txt_expose(self, result, argv);
	}
	return meth_cosmic_usual(self, result, 0, argv);
}

/*
 * HTML_header_medium object script equivalent method
 */
int meth_generic_code_HTML_header_medium(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_generic_code_HTML_header_large(self, result, argc, argv);
}

/*
 * HTML_header_small object script equivalent method
 */
int meth_generic_code_HTML_header_small(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return meth_generic_code_HTML_header_large(self, result, argc, argv);
}



