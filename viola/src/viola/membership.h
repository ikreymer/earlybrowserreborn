#define MHBITS_SIZE 64

typedef struct MHInfo {
	long *bits;
	long bitsSize;
	long hits;
	long misses;
	long collisions;
	long refc;
} MHInfo;

