#include "membership.h"

int isMember(mhp, val)
	MHInfo *mhp;
	long val;
{
	int byte = mhp->bits[val % mhp->bitsSize];
	int nthBits = val % sizeof(long);
	int result;

/*	mhp->refc++;*/
/*
	printf("isMember=%d : place=%d, bit=%d\n", 
			byte & (1 << nthBits), byte, nthBits);
*/
	result = byte & (1 << nthBits);

/*	if (result) mhp->hits++;
	else mhp->misses++;
*/

/*
	printf("isMember: mhp=0x%x  \t\t refc=%5ld  hits=%5ld  misses=%5ld\n", 
	       mhp, mhp->refc, mhp->hits, mhp->misses);
*/
	return result;

/*	return byte & (1 << nthBits);*/
}

void setMember(mhp, val)
	MHInfo *mhp;
	long val;
{
	mhp->refc++;
/*
	printf("setMember: place=%d, bit=%d\n", 
			val % mhp->bitsSize, (1 << val % sizeof(long)));
*/

	if (mhp->bits[val % mhp->bitsSize] & (1 << val % sizeof(long)))
	  mhp->collisions++;

	mhp->bits[val % mhp->bitsSize] |= (1 << val % sizeof(long));
/*
	printf("setMember: mhp=%x  \t\t refc=%ld\t hits=%ld\t misses=%ld collision=%ld\n", 
	       mhp, mhp->refc, mhp->hits, mhp->misses, mhp->collisions);
*/

}
