/*		The FOSI to rtf object converter			FOSI.h
**		--------------------------------
*/

#ifndef FOSI_H
#define FOSI_H

#include "HTUtils.h"
#include "HTAnchor.h"
#include "FOSIDTD.h"


extern CONST HTStructuredClass FOSIPresentation;

/*	HTConverter to present FOSI
*/
PUBLIC HTStream* FOSIToPlain PARAMS((
	HTPresentation *	pres,
	HTParentAnchor *	anchor,	
	HTStream *		sink));

PUBLIC HTStream* FOSIToC PARAMS((
	HTPresentation *	pres,
	HTParentAnchor *	anchor,	
	HTStream *		sink));

PUBLIC HTStream* FOSIPresent PARAMS((
	HTPresentation *	pres,
	HTParentAnchor *	anchor,	
	HTStream *		sink));

extern HTStructured* FOSI_new PARAMS((
	HTParentAnchor * anchor,
	HTStream *	target));

/*	Names for selected internal representations:
*/
typedef enum _FOSICharacterSet {
	FOSI_ISO_LATIN1,
	FOSI_NEXT_CHARS,
	FOSI_PC_CP950
} FOSICharacterSet;

extern void FOSIUseCharacterSet PARAMS((FOSICharacterSet i));

/*	Record error message as a hypertext object
**	------------------------------------------
**
**	The error message should be marked as an error so that
**	it can be reloaded later.
**	This implementation just throws up an error message
**	and leaves the document unloaded.
**
** On entry,
**	sink 	is a stream to the output device if any
**	number	is the HTTP error number
**	message	is the human readable message.
** On exit,
**	a retrun code like HT_LOADED if object exists else < 0
*/

PUBLIC int HTLoadError PARAMS((
	HTStream * 	sink,
	int		number,
	CONST char *	message));

#endif
