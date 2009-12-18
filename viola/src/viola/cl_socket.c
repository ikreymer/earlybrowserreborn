/*
 * Copyright 1990 Pei-Yuan Wei.	All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
/*
 * class	: socket
 * superClass	: client
 */
/*
 * Contributors:
 * 
 * Kurt Pires (kjpires@xcf): initial socket code.
 * Tor Lillqvist (tml@tik.vtt.fi): HP-UX compatibility.
 */
#include "utils.h"
#include <ctype.h>
#include "error.h"
#include "mystrings.h"
#include "hash.h"
#include "ident.h"
#include "scanutils.h"
#include "obj.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "slotaccess.h"
#include "classlist.h"
#include "cl_socket.h"
#include "misc.h"
#include "glib.h"
#include "event.h"

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#ifdef _AIX
#include <sys/select.h>
#endif

SlotInfo cl_socket_NCSlots[] = {
	NULL
};
SlotInfo cl_socket_NPSlots[] = {
{
	STR_host,
	PTRS | SLOT_RW,
	(long)""
},{
	STR_port,
	LONG | SLOT_RW,
	0
},{
	NULL
}
};
SlotInfo cl_socket_CSlots[] = {
{
	STR_class,
	PTRS | SLOT_RW,
	(long)"socket"
},{
	STR_classScript,
	PTRS,
	(long)"\n\
		switch (arg[0]) {\n\
		case \"config\":\n\
			config(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"configSelf\":\n\
			/* icky! but necessary to give script a chance to \n\
			 * intercept and do something...*/\n\
	        	send(self(), \"config\", \n\
				x(), y(), width(), height());\n\
		break;\n\
		case \"expose\":\n\
			expose(arg[1], arg[2], arg[3], arg[4]);\n\
		break;\n\
		case \"render\":\n\
			render();\n\
		break;\n\
		case \"visible\":\n\
			set(\"visible\", arg[1]);\n\
		break;\n\
		case \"mouseMove\":\n\
		case \"enter\":\n\
		case \"leave\":\n\
		case \"buttonPress\":\n\
		case \"buttonRelease\":\n\
		case \"keyPress\":\n\
		case \"keyRelease\":\n\
		case \"shownPositionH\":\n\
		case \"shownPositionV\":\n\
		case \"shownInfoV\":\n\
		case \"shownInfoH\":\n\
		break;\n\
		case \"focus\":\n\
			mousePos = mouse();\n\
			winPos = windowPosition();\n\
			mx = mousePos[0];\n\
			my = mousePos[1];\n\
			dx = ((winPos[0] + width() / 2) - mx) / 10.0;\n\
			dy = ((winPos[1] + height() / 2) - my) / 10.0;\n\
			for (i = 0; i < 10; i = i + 1) {\n\
				mx = mx + dx;\n\
				my = my + dy;\n\
				setMouse(mx, my);\n\
			}\n\
		break;\n\
		case \"key_up\":\n\
			send(parent(), \"key_up\");\n\
			return;\n\
		break;\n\
		case \"key_down\":\n\
			send(parent(), \"key_down\");\n\
			return;\n\
		break;\n\
		case \"init\":\n\
			initialize();\n\
		break;\n\
		case \"raise\":\n\
			raise();\n\
		break;\n\
		case \"info\":\n\
			info();\n\
		break;\n\
		case \"freeSelf\":\n\
			return freeSelf();\n\
		break;\n\
		default:\n\
			print(\"unknown message, clsss = \", get(\"class\"),\n\
				\": self = \", get(\"name\"), \" args: \");\n\
			for (i = 0; i < arg[]; i++) print(arg[i], \", \");\n\
			print(\"\n\");\n\
		break;\n\
		}\n\
	",
},{
	NULL
}
};
SlotInfo cl_socket_PSlots[] = {
{
	STR__classInfo,
	CLSI,
	(long)&class_socket
},{
	NULL
}
};

SlotInfo *slots_socket[] = {
	(SlotInfo*)cl_socket_NCSlots,
	(SlotInfo*)cl_socket_NPSlots,
	(SlotInfo*)cl_socket_CSlots,
	(SlotInfo*)cl_socket_PSlots
};

MethodInfo meths_socket[] = {
	/* local methods */
{
	STR__startClient,
	meth_socket__startClient
},{
	STR_freeSelf,
	meth_socket_freeSelf,
},{
	STR_geta,
	meth_socket_get,
},{
	STR_seta,
	meth_socket_set
},{
	NULL
}
};

ClassInfo class_socket = {
	helper_socket_get,
	helper_socket_set,
	slots_socket,		/* class slot information	*/
	meths_socket,		/* class methods		*/
	STR_socket,		/* class identifier number	*/
	&class_client,		/* super class info		*/
};

int meth_socket__startClient(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int fd;
	int socket_open();

	fd = socket_open("tcp", GET_host(self), GET_port(self));

	result->type = PKT_INT;
	result->canFree = 0;
	result->info.i = fd;

	return 1;
}

int helper_socket_get(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_pid:
		result->type = PKT_INT;
		result->canFree = 0;
		result->info.i = GET_pid(self);
		return 1;

	case STR_args:
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		result->info.s = SaveString(GET_args(self));
		return 1;

	case STR_path:
		result->type = PKT_STR;
		result->canFree = PK_CANFREE_STR;
		result->info.s = SaveString(GET_path(self));
		return 1;

	}
	return helper_client_get(self, result, argc, argv, labelID);
}
int meth_socket_get(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_socket_get(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

int meth_socket_freeSelf(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	int fd = GET_clientFD(self);

	if (fd != -1) {
		SET_clientFD(self, 0);
		close(fd);
		objFDList[fd] = NULL;
	}
	free(GET_host(self));
	free(GET_port(self));
	meth_client_freeSelf(self, result, argc, argv);

	return 1;
}

int helper_socket_set(self, result, argc, argv, labelID)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
	int labelID;
{
	switch (labelID) {
	case STR_host:
		result->info.s = SaveString(PkInfo2Str(&argv[1]));
		SET_host(self, result->info.s);
		result->type = PKT_STR;
		result->canFree = 0;
		return 1;

	case STR_port:
		result->info.i = PkInfo2Int(&argv[1]);
		SET_port(self, result->info.i);
		result->type = PKT_INT;
		result->canFree = 0;
		return 1;
	}
	return helper_client_set(self, result, argc, argv, labelID);
}
int meth_socket_set(self, result, argc, argv)
	VObj *self;
	Packet *result;
	int argc;
	Packet argv[];
{
	return helper_socket_set(self, result, argc, argv, 
				getIdent(PkInfo2Str(argv)));
}

/******/
/*
 * KJ's code
 */
int socket_open(proto, host, port)
	char	*proto;			/* "tcp" or "udp" */
	char	*host;			/* name or dotted quad */
	int	port;			/* service name or number */
{
	struct	sockaddr_in	addr;
	int			s;
	int			stype;

	if (strcmp("tcp", proto) == 0)
		stype = SOCK_STREAM;
	else if (strcmp("udp", proto) != 0)
		stype = SOCK_DGRAM;
	else
		return -1;

	if ((addr.sin_addr.s_addr = inet_addr(host)) != -1) {
		addr.sin_family = AF_INET;
	} else {
		struct	hostent	*hp;
		
		if ((hp = gethostbyname(host)) == NULL)
			return -2;

		bcopy(hp->h_addr, (char *)&addr.sin_addr, hp->h_length);
		addr.sin_family = hp->h_addrtype;
	}

	if (addr.sin_port = port) {
		addr.sin_port = htons(addr.sin_port);
	} else {
		struct	servent		*sp;

		if ((sp = getservbyname(port, proto)) == NULL)
			return -3;

		addr.sin_port = sp->s_port;
	}

	if ((s = socket(addr.sin_family, stype, 0)) < 0)
		return -4;

	if (connect(s, (struct sockaddr *)&addr, sizeof addr) < 0) {
		close(s);
		return -5;
	}

	return s;
}
