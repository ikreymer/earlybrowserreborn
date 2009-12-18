/*
 * event_x.h
 */
extern char keyStat_key;
extern int keyStat_control;
extern int keyStat_shift;
extern int mouseButtonPressedState;

extern int modalState;
extern Packet *modalResult;

extern VObj *objFDList[]; /* list of association between fd and object */
extern fd_set read_mask;

extern int flag_window_tracking;
extern FILE *user_action_tracking;

extern VObj *xselectionObj; /* object that has data for x clip buffer */
extern char *sharedSelectionBuffer;

typedef struct AcceleratorInfo {
  VObj *object;  /* the object that wants to be informed about the key */
  char key;      /* key code */
  char modifier; /* modifier code */
  struct AcceleratorKeyObjNode_struct *next;
} AcceleratorInfo;
#define MODIFIER_ACCEPT_ALL_KEYS 0
#define MODIFIER_NONE 1
#define MODIFIER_CONTROL 2
#define MODIFIER_META 3

extern int init_event();
extern int eventLoop();

extern long scheduleEvent();
extern int cancelEventByObject();
extern int cancelEvent();
extern void dumpSchedule();

void modalLoop();

