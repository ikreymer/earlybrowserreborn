typedef struct SlotStruct {
  char *slotName;
  char *slotValue;
} SlotStruct;

typedef struct ObjsNamesStruct {
  char *name;
  SlotStruct *slots;
} ObjsNamesStruct;

extern ObjsNamesStruct objsNames[];
