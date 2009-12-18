/*
 * Copyright 1991 Pei-Yuan Wei.	All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
/*
 * Packet Routines
 */
#include "utils.h"
#include <ctype.h>
#include "mystrings.h"
#include "hash.h"
#include "obj.h"
#include "packet.h"
#include "slotaccess.h"

#include "membership.h"
#include "class.h"

char *packetTypeStr[] = {
	"INT",
	"FLT",
	"STR", 
	"CHR",
	"OBJ",
	"PKT",
	"ATR",
	"VLT",
	"ARY",
/*	"STRI",*/
};

#define PACKET_STACK_SIZE	500

Packet *scrap_packet;
Packet *packetStack;
int 	packetStackIdx = -1;
int	packetStackSize = PACKET_STACK_SIZE;

int init_packet()
{
	packetStack = (Packet*)malloc(sizeof(struct Packet) * packetStackSize);
	return 1;
}

Packet *makePacket()
{
	Packet *new;

	new = (Packet*)malloc(sizeof(struct Packet));
	new->type = 0;
	new->info.i = 0;
	new->canFree = 0;

	return new;
}

Packet *borrowPacket()
{
	Packet *packetp;

	if (++packetStackIdx > packetStackSize) {
		/* try to make a larger packet stack... */
		fprintf(stderr,
			"internal error: Packet stack overflow.\n");
		fprintf(stderr,
			"use ``-pstack'' option to allocate larger stack.\n");
		fprintf(stderr,
			"Whoops, -pstack doesn't yet exist. Tell Pei that this is needed.\n");
		/* XXX unimplemented */
		exit(1);
	}
	packetp = &packetStack[packetStackIdx];
	packetp->info.i = NULL;
	packetp->type = 0;
	packetp->canFree = 0;
	return packetp;
}

void returnPacket()
{
	Packet *packetp;

	if (packetStackIdx < 0) {
		fprintf(stderr, 
			"internal error: returnPacket() stack underflow...\n");
		return;
	}
	packetp = &packetStack[packetStackIdx--];

	if (packetp->canFree & PK_CANFREE_STR) free(packetp->info.s); /* suspect */
}

void returnPackets(n)
     int n;
{
	while (n-- > 0) {
		if (packetStackIdx < 0) {
			fprintf(stderr, 
				"internal error: returnPackets() stack underflow...\n");
			return;
		}
		scrap_packet = &packetStack[packetStackIdx--];
/*
		if (PkFreeable(scrap_packet) && PkContent(scrap_packet))
			free(PkContent(scrap_packet));
*/
	}
}

void dumpPacket(packet) 
	Packet *packet;
{

	printf("(%d %s) ", packet->type, packetTypeStr[packet->type]);
	switch (packet->type) {

	case PKT_INT:
		printf("%d", packet->info.i);
	break;

	case PKT_FLT:
		printf("%f", packet->info.f);
	break;

	case PKT_STR:
		if (packet->info.s)
			printf("\"%s\"", packet->info.s);
		else
			printf("(NULL)");
	break;

	case PKT_CHR:
		printf("'%c'", packet->info.c);
	break;

	case PKT_OBJ:
		if (packet->info.o)
			printf("%s", GET_name(packet->info.o));
		else
			printf("(NULL)");
	break;

	case PKT_PKT:
		printf("%x", packet->info.p);
	break;

	case PKT_ATR:
		printf("%x", packet->info.a);
	break;

	case PKT_VLT:
		printf("%x", packet->info.l);
	break;

	case PKT_ARY:
		if (packet->info.y) {
			int i;
			printf("size=%d: ", packet->info.y->size);
			for (i = 0; i < packet->info.y->size; i++) {
				printf("%d ", packet->info.y->info[i]);
			}
		}
	break;
/*
	case PKT_STRI:
		if (packet->info.si) {
			if (packet->info.si.s)
				printf("\"%s\"", packet->info.si.s);
			else
				printf("(NULL)");
		} else {
			printf("(NULL)");
		}
	break;
*/
	default:
		printf("(%d)", packet->info.i);
	break;
	}
}

/* MACRONIZED
void nullPacket(packet)
	Packet *packet;
{
	packet->info.i = 0;
	packet->type = 0;
	packet->canFree = 0;
}
*/

void freePossibleDangler(packet)
	Packet *packet;
{
	if ((packet)->canFree & PK_CANFREE_STR) {
		free((packet)->info.s);
		(packet)->info.s = NULL;
		(packet)->canFree = 0;
	}
}

#ifdef DONT_USE_MACRO_OF_CLEARPACKET
Packet *clearPacket(packet)
	Packet *packet;
{
	if (packet->canFree & PK_CANFREE_STR) {
		free(packet->info.s);/*suspect*/
		packet->info.s = NULL; /* just in case */
		packet->canFree = 0;
	}
	packet->info.i = 0;
	packet->type = 0;
	return packet;
}
#endif

int checkleak = 0;

void _copyPacket(packetTo, packetFrom)
	Packet *packetTo;
	Packet *packetFrom;
{
/*	char *savep;
	int leaks;
*/
	if ((packetTo->type == PKT_STR) &&
	    (packetTo->canFree & PK_CANFREE_STR)) {
		free(packetTo->info.s);
	}

	if ((packetTo->type = packetFrom->type) == PKT_STR) {

/*
		savep = packetTo->info.s;
if (checkleak && savep)
  if (leaks = purify_newleaks()) {
    print("BEFORE S>>>>>>> leaks = %d\n", leaks);
  }
*/
		packetTo->info.s = SaveString(packetFrom->info.s);
/*if (checkleak && savep)
  if (leaks = purify_newleaks()) {
    print("AFTER S>>>>>>> leaks = %d  savep=(%s)\n", leaks, savep);
  }
*/
		packetTo->canFree = PK_CANFREE_STR;
	} else {
/*
		savep = packetTo->info.s;
print(">>>>>>>POSSIBLE LEAK??:");
dumpPacket(packetTo);
print("}\n");
*/

/*if (checkleak && savep)
  if (leaks = purify_newleaks()) {
    print("BEFORE I>>>>>>> leaks = %d\n", leaks);
  }
*/
		packetTo->info.i = packetFrom->info.i; /* very bunk */
/*if (checkleak && savep)
  if (leaks = purify_newleaks()) {
    print("AFTER I>>>>>>> leaks = %d  savep=%x\n", leaks, savep);
  }
*/
		packetTo->canFree = 0;
	}
}

