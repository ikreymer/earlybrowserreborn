#include "utils.h"
#include "error.h"
#include "mystrings.h"
#include "hash.h"
#include "ident.h"
#include "obj.h"
#include "packet.h"
#include "membership.h"
#include "class.h"

#define ERRORLOG_SIZE 32
typedef struct ErrorLog {
	int	errcode;
	VObj	*obj;
	char	*messg;
} ErrorLog;
ErrorLog errorLog[ERRORLOG_SIZE];
int errc = 0;
int posting = 0;

strNIntPair errIDList[] = {
	ERR_NONE,	"ERR_NONE",
	ERR_MALLOC,	"ERR_MALLOC",
	ERR_DIVZERO,	"ERR_DIVZERO",
	ERR_FLOW,	"ERR_FLOW",
	ERR_INDEX,	"ERR_INDEX",
	ERR_FOPEN,	"ERR_FOPEN",
	ERR_FWRITE,	"ERR_FWRITE",
	ERR_SYNTAX,	"ERR_SYNTAX",
	ERR_UNKFUNC,	"ERR_UNKFUNC",
	ERR_UNKOBJ,	"ERR_UNKOBJ",
/*
	ERR_,		"ERR_",
*/
	NULL,		NULL
};

/*
 * error messages associated with error codes
 *
 * used to produce, ie: "ERR_MALLOC: not enough memory\n"
 */
strNIntPair errMessgList[] = {
	ERR_NONE,		"no error",
	ERR,			"error",
	ERR_MALLOC,		"not enough memory",
	ERR_DIVZERO,		"division by zero",
	ERR_FLOW,		"over/underflow",
	ERR_INDEX,		"over indexing",
	ERR_FOPEN,		"file open failed",
	ERR_FWRITE,		"file write failed",
	ERR_SYNTAX,		"bad syntax",
	ERR_UNKFUNC,		"unknown function",
	ERR_UNKOBJ,		"unknown object",
	ERR_PRGUNKNOWN,		"program not found",
	ERR_FILENENGH,		"not enough disk memory",
	ERR_NOTDIR,		"not a directory",
	ERR_FILEEXIST,		"file already exists",
	ERR_OBJNOEXIST,		"no such object",
	ERR_FILERO,		"read-only file",
	ERR_ARGCOVER,		"too many arguments",
	ERR_ARGCUNDER,		"too few arguments",
	ERR_CONNCLOSE,		"connection closed",
	ERR_CONNREFUSE,		"connection refused",
	ERR_CONNTOUT,		"connection time out",
	ERR_NETUNREACH,		"network unreachable",
	ERR_FNTOOLONG,		"filename too long",
	ERR_DIRNOTEMPTY,	"directory not empty",
/*
	ERR_,		"",
*/
	NULL,		NULL
};


void clearErrors()
{
	errc = 0;
}

char *messageToUserWithObj(self, messg)
	VObj *self;
	char *messg;
{
	return 0;
}

char *messageToUser(self, type, messg)
	VObj *self;
	int type;
	char *messg;
{
	if (posting) {

		VObj *handlerObj;
		char *handler;
		char *dumpBuff;

		switch (type) {
		case MESSAGE_DEBUG:
			handler = "res.stat.debug";
		break;

		case MESSAGE_ERROR:
			handler = "res.stat.error";
		break;

		case MESSAGE_WARNING:
			handler = "res.stat.warning";
		break;

		case MESSAGE_HISTORY:
			handler = "res.stat.history";
		break;

		case MESSAGE_PROMPT:
		default:
			handler = "res.stat";
		}

		if (self == NULL) self = VResourceObj;
		if (messg == NULL) messg = "";
		
		dumpBuff = (char*)malloc(sizeof(char) * (strlen(messg) + 20));
		strcpy(dumpBuff, "post ");
		strcat(dumpBuff, messg);
		
/*
		if (self) {
			if (handlerObj = retrieveObj(handler, NULL)) {
				if (ObjActive(handlerObj)) {
					Packet *result = borrowPacket();

					method_generic_send(self, result, 
							handlerObj, dumpBuff);

					free(dumpBuff);
					returnPacket(result);
					return 0;
				}
			}
		}
*/
		fprintf(stderr, "stat(%s): %s", handler, dumpBuff);
		free(dumpBuff);
	}
	return 0; /* return error code...*/
}
