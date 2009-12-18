extern int cmd_history;
extern int verbose;

int transferNumList2Array();
int argNumsToInt();
VObjList *strOListToOList();
char *OListToStr();
char *OListToStrPlusSuffix();

/*
void setNotifyShownInfo();
void setDependShownInfo();
*/
int makeArgv();

char *PkInfos2Str();

char *PkInfo2Str();
char PkInfo2Chr();
int PkInfo2Int();
float PkInfo2Flt();
VObj *PkInfo2Obj();
VObj *getObject();
