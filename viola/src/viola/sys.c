/*
 * system dependent stuff 
 */
#include <stdio.h>
#include <sys/file.h>
#include <sys/time.h>
#include "sys.h"

/*#define MEM_VERBOSE */
/*#define MEM_USE*/

long tempFileNameIDCounter = 0;
char *tempFileNamePrefix = "/tmp/violaTmp";

int init_sys()
{
	char *tmp = tmpnam(NULL);
	if (!tmp) {
perror("Failed to create temporary file name. Using /tmp/violaTmp\n");
	}
	tempFileNamePrefix = saveString(tmp);

	return 1;
}

/* return 1 on success */
int sys_alarm(sec)
     int sec;
{
/*
	alarm((unsigned)sec);
*/
	return 0; 
}

int sys_date(year, month, day)
	int *year, *month, *day;
{
	return 0;
}

int sys_time(hour, minute, sec)
	int *hour, *minute, *sec;
{
	return 0;
}


/*
do mem hack on:

tfed.c:
	add bucket-list info in tfstruct, if it's read-only.
	
	splitLine(bl, ...)
	deleleLineNodes(bl, ...)

cexec.c:
	makeArgList()	?

ident.c:

hash.c:
	use heap for HashEntry mallocs. they're rarely deleted.

	except for obj names (which are changed for clonned objects),
	in this hashTable, use malloc and trade-off speed.

MemoryGroup *ml;
ml = newMemoryGroup(10);
Vmalloc(ml, 2);
*/
/*
MemoryGroup *newMemoryGroup(defaultSize)
	int defaultSize;
{
	MemoryGroup *group =
		(MemoryGroup*)malloc(sizeof(struct MemoryGroup));
	if (!group) {
		perror("malloc failed in newMemoryGroup()");
		return NULL;
	}
	group->heap = NULL;
	group->heapUsedOffset = 0;
	group->defaultSize = defaultSize;

	return group;
}
*/
#ifdef MEM_USE
char *_Vmalloc(group, size)
	MemoryGroup *group;
	int size;
{
	if (!group->heap) {
		group->heap =
		    (MemoryGroupHeap*)malloc(sizeof(struct MemoryGroupHeap));
		if (!group->heap) {
			perror("malloc failed in Vmalloc()");
			return NULL;
		}
		if (size > group->defaultSize) group->heap->size = size;
		else group->heap->size = group->defaultSize;
		group->heap->mem = (char*)malloc(group->heap->size);
		if (!group->heap->mem) {
			perror("malloc failed in Vmalloc()");
			free(group->heap);
			group->heap = NULL;
			return NULL;
		}
		group->heap->next = NULL;
		group->heapUsedOffset = 0;
#ifdef MEM_VERBOSE
printf("(0x%x) MAKE group=0x%x heap=0x%x mem=0x%x\n", group, group->heap, group->heap->mem);
#endif
	}
	/* the correct way to get around "signal BUS (alignment error)" ? */
	group->heapUsedOffset += 
		4 - ((int)(group->heap->mem + group->heapUsedOffset) % 4);
	if (group->heapUsedOffset + size <= group->heap->size) {
		char *ptr; 
		ptr = group->heap->mem + group->heapUsedOffset;
#ifdef MEM_VERBOSE
printf("(0x%x) OLD group=0x%x heap=0x%x mem=0x%x memSZ=%d: iB=%d size=%d iA=%d\n", 
ptr, group, group->heap, group->heap->mem, group->heap->size, group->heapUsedOffset, size,
group->heapUsedOffset + size);
#endif

		group->heapUsedOffset += size;
		return ptr;
	} else {
		MemoryGroupHeap *newHeap =
		    (MemoryGroupHeap*)malloc(sizeof(struct MemoryGroupHeap));
		if (!newHeap) {
			perror("malloc failed in Vmalloc()");
			return NULL;
		}
		if (size > group->defaultSize) newHeap->size = size;
		else newHeap->size = group->defaultSize;
		newHeap->mem = (char*)malloc(newHeap->size);
		if (!newHeap->mem) {
			perror("malloc failed in Vmalloc()");
			free(newHeap);
			return NULL;
		}
		newHeap->next = group->heap;
		group->heap = newHeap;
#ifdef MEM_VERBOSE
printf("(0x%x) NEW group=0x%x heap=0x%x mem=0x%x memSZ=%d: iB=%d size=%d iA=%d\n", 
newHeap->mem, group, group->heap, newHeap->mem, group->heap->size, group->heapUsedOffset, size,
size);
#endif
		group->heapUsedOffset = size;
		return newHeap->mem;
	}
}

void _Vfree(group, ptr)
	MemoryGroup *group;
	char *ptr;
{
/*
FOR THE FUTURE:
for determining when to nuke a piece of heap, 
get rid heapUsedOffset, replace with heap_begin & heap_end.
add refc to store number of references into a heap.
decrement the heap which ptr is within. When refc == 0, remove heap 
(or put on removal list, and do at idle time).
to be safer, but still not guarrantee detection of faults, use the
first byte of a chunk for ref-validity, and let Vmalloc return ptr+1.
(thus, in Vfree()'s *(ptr-1) is the "free'edP" flag 
*/
#ifdef MEM_VERBOSE
printf("(0x%x) FREE group=0x%x\n", ptr, group);
#endif
}
#endif

#ifdef MEM_USE
void freeMemoryGroup(group)
	MemoryGroup *group;
{
	if (group) {
		if (group->heap) freeMemoryGroupHeaps(group->heap);
		free(group);
	}
}
#endif

#ifdef MEM_USE
void freeMemoryGroupHeaps(group)
	MemoryGroup *group;
{
	MemoryGroupHeap *heap, *heap_next;
	for (heap = group->heap; heap; heap = heap_next) {
#ifdef MEM_VERBOSE
printf("FREE group=0x%x heap=0x%x heapSize=%d\n", group, heap, heap->size);
		free(heap->mem);
		heap_next = heap->next;
		free(heap);
#endif
	}
}
#endif

#ifdef MEM_USE
Rfree(ptr)
	char *ptr;
{
#ifdef MEM_VERBOSE
printf("(0x%x) R_FREE\n", ptr);
#endif
}
#endif
