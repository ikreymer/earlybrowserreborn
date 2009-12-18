#ifndef MIDASLIST
#define MIDASLIST
#include "midasoperand.h"

struct ListItem  {
    struct ListItem     *Next;
    struct ListItem     *Prev;
    char                *Entry;
    XrmQuark		Name;	
    void                *Pointer;
};

typedef struct ListItem  ListItem ;

struct List {
    struct ListItem     *First;
    struct ListItem     *Last;
    Widget               W;
    struct ListItem     *Selected;
    char                *Name;
};

typedef struct List List;

static List NullList={0,0,0,0,0}; 

typedef XtPointer (*MidasCopyListRtn)();

List *MidasCreateEmptyList();
Boolean MidasConvertStringList();
void MidasEmptyList();
void MidasDestroyList();
ListItem *MidasFindItemInList();
int MidasFindItemPosInList();
ListItem *MidasFindItemInListPos();
ListItem *MidasAddItemToListPos();
ListItem *MidasAddItemToList();
void MidasCopyList();
void MidasRealizeList();
void MidasUnrealizeList();
void MidasSelectItemPosInList();

#endif 
