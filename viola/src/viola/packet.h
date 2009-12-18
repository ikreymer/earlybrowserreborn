/*
 * packet.h
 */
/***************************************************************************
 *
 * Info Packet Structure
 *
 * biop.c depends (for convenient direct mapping) on:
 *	PKT_INT = 0, PKT_CHR = 1, PKT_FLT = 2, PKT_STR = 3
 */
enum {
	PKT_INT, /* 0 */
	PKT_CHR, /* 1 */
	PKT_FLT, /* 2 */
	PKT_STR, /* 3 */
	PKT_OBJ, /* 4 */
	PKT_PKT, /* 5 */
	PKT_ATR, /* 6 */
	PKT_VLT, /* 7 */
	PKT_ARY, /* 8 */
	PKT_TOK, /* 9 */
/*	PKT_STRI,*/ /* 9 */
};

#define PK_CANFREE_STR 1	/* can free info.s */
#define PK_CANFREE_NOT 2	/* do not clear&free this packet */

typedef struct Packet {
	union {
		VObj	*o;
		int	i;
		float	f;
		char	c;
		char	*s;
		FILE	*fp;
		int	t;
		struct Packet *p;
		struct Attr *a;
		struct Array *y;
		struct VList *l;
/*		struct StrInfo *si;*/
	} info;
	char	type;
	char	canFree;	/* regarding .info.s */
	char	sticky;		/* if on, do not free packet structure */
} Packet;

typedef struct Array {
	int size;
	int *info;
} Array;

typedef struct StrInfo {
	char *s;
	int refc;
} StrInfo;

extern int init_packet();
extern Packet *makePacket();
extern Packet *borrowPacket();
extern Packet *clearPacket();

/*extern void nullPacket();*/
#define nullPacket(packet) {\
	(packet)->info.i = 0;\
	(packet)->type = 0;\
	(packet)->canFree = 0;}

#define DONT_USE_MACRO_OF_CLEARPACKET
#ifndef DONT_USE_MACRO_OF_CLEARPACKET
#define clearPacket(packet)\
{\
	if (packet->canFree) {\
		free(packet->info.s);\
		packet->info.s = NULL;\
		packet->canFree = 0;\
	}\
	packet->info.i = 0;\
	packet->type = 0;\
}
#endif

/*
#define freePossibleDangler(packet) {\
	if ((packet)->canFree) {\
		free((packet)->info.s);\
		(packet)->canFree = 0;\
	}\
}
*/
extern void freePossibleDangler();

#define copyPacket(to,from) if (to!=from) _copyPacket(to,from)
extern void _copyPacket();

extern void returnPacket();
extern void returnPackets();
extern void dumpPacket();



