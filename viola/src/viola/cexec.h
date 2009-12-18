#define PCODE_IDX_SIZE 	0
#define PCODE_IDX_REFC 	1
#define PCODE_IDX_INSTR	2

typedef struct CallObjStack {
	int destroyed;
	VObj *obj;
} CallObjStack;

extern CallObjStack callObjStack[];
extern int callObjStackIdx;

/*notes:
 * pcode[0] stores size of pcode array
 * pcode[1] stores reference count to this pcode
 * pcode[2] start of instructions
 * ...
 */

extern Packet *codeExec();
extern Packet *execObjScript();
extern Packet *execObjClassScript();
extern Packet *execScript();
extern void freeVarList();
extern void dumpVarList();
extern Attr *makeArgAttr();
extern int makeArgList();
extern void freeArgList();

extern int sendMessagePackets();
extern int sendMessageAndInts();
extern int sendMessage1_result();
extern int sendMessage1N1str();
extern int sendMessage1N1str_result();
extern int sendMessage1N2str();
extern int sendMessage1N1int();
extern int sendMessage1N1int_result();
extern int sendMessage1N2int();
extern int sendMessage1N4int();
extern int sendMessage1chr();
extern int sendMessage1chr_result();
extern int sendTokenMessageAndInts();
extern int sendTokenMessage_result();
extern int sendTokenMessage();
extern int sendTokenMessageN1int();

extern int flag_printExec;
extern char *passthru_argument;
extern int getVariable();
extern int setVariable();
extern Attr *setVariable_STR();
extern Attr *setVariable_id_STR();
extern Attr *setVariable_INT();

extern int destroyVariable();

/*extern int ASSERT();*/
#define ASSERT(t,mesg) NULL
