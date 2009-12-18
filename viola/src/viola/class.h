/*
 *    class.h
 */

/*****************************************************************************
 * slot info
 */
#define	SLOT_MASK_TYPE	0x000f

#define	LONG 		1	/* long */
#define	OBJP	 	2	/* object pointer */
#define	OBJL		3	/* object list */
#define	PTRV		4	/* generic pointer */
#define	PTRS		5 	/* string */
#define	PTRA		6	/* string, append to */
#define	ATTR		7 	/* attributes */
#define	FUNC		8	/* set value to function */
#define	PROC		9	/* procedure, don't set slot value */
#define CLSI		10	/* classInfo */
#define PCOD		11	/* pcode */
#define ARRY		12	/* array */
#define RGBV		13	/* RGB structure */
#define TFLD		14	/* TFStruct (text field structure) */
#define STRI		15	/* StrInfo */

#define	SLOT_MASK_RW	0x00f0

#define	SLOT_R		0x0010
#define	SLOT_W		0x0020
#define	SLOT_RW		0x0030

/* to be converted to later... */

typedef struct SlotInfo {
	long	id;		/* str ID 	*/
	int	flags;		/* slot type, save-or-not */
	long	val;		/* slot value	*/
	int	offset;		/* slot offset  */
	int	section;	/* section to look in */
	long	tmp;		/* temporary	*/
} SlotInfo;

/*****************************************************************************
 * method info
 */
typedef struct MethodInfo {
	int	id;
	int	(*method)();
} MethodInfo;

/*****************************************************************************
 * class info
 */
typedef struct ClassInfo {
	int 		(*slotGetMeth)();
	int 		(*slotSetMeth)();

	SlotInfo	**slots;
	MethodInfo	*methods;
	int		id;
	struct ClassInfo *superClass;

	MHInfo		mhp;
	HashTable	*mht;

	/* used to build private slots */
	SlotInfo	**slookup;
	int 		newcount;	/* count of new slots */
	int 		totalcount;	/* count of all slots in this type */

	/* used to build common slots */
	VObj		*common; 	/* object containing common slots */
	SlotInfo	**common_slookup;
	int		common_newcount;   /* used for building common obj */
	int		common_totalcount;
} ClassInfo;

extern int init_class();
extern ClassInfo *getClassInfoByID();
extern ClassInfo *getClassInfoByName();
extern VObj *buildObjWithLoadedSlots();
extern VObj *instantiateObj();
extern long *searchSlot();
extern int saveSelfAndChildren();
extern int dumpObj();
extern VObj *clone();
extern int loadClassScriptsP;



