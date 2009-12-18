typedef struct MemoryGroupHeap {
	char *mem;
	int size;
	struct MemoryGroupHeap *next;
} MemoryGroupHeap;

typedef struct MemoryGroup {
	struct MemoryGroupHeap *heap;
	int heapUsedOffset;
	int defaultSize;
} MemoryGroup;

/*MemoryGroup *newMemoryGroup();*/

char *_Vmalloc();
void _Vfree();

/*#define Vmalloc(gp, size) _Vmalloc(size)*/
/*#define Vfree(gp, size) _Vfree(size)*/

#define Vmalloc(gp, size) malloc(size)
#define Vfree(gp, x) free(x)

/*
void freeMemoryGroup();
void freeMemoryGroupHeaps();
*/

#define Rmalloc(x) malloc(x)
#define Rfree(x) free(x)

extern long tempFileNameIDCounter;
extern char *tempFileNamePrefix;

int sys_alarm();
int sys_date();
int sys_time();
