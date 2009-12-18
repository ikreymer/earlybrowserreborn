#include "hotlist.h"

extern int printTagDelims;

typedef struct SGMLBuildInfoState {
	SGMLTagMappingInfo *tmi;
	int dataBuffIdx_localStart;
	int *localizingTagIDMap;
	char *tag;
	VObj *parent;
	VObj *obj;
	int y;
	int width;
	int height;
	int sub_y;
	int sub_width;
	int sub_height;
	VObj *insert;
} SGMLBuildInfoState;

/* SGMLBuildInfo
 */
typedef struct SGMLBuildInfo {
	SGMLDocMappingInfo *dmi;
	struct SGMLBuildInfoState stack[100]; /* limit on nesting depth */
	int stacki;
	int endP;
	VObj *caller;
} SGMLBuildInfo;

SGMLTagMappingInfo *findTMI();

int init_html2();

VObj *HTMLBuildObj();

VObj *html2_parseHTMLDocument();
void CB_HTML_new();
void CB_HTML_data();
void CB_HTML_stag();
void CB_HTML_etag();
void CB_HTML_end();

char *encodeURL();
char *decodeURL();

/* for libwww to call */

int http_progress_reporter_level;
int http_progress_expected_total_bytes;
int http_progress_total_bytes;
int http_progress_subtotal_bytes;

int http_progress_notify();
void user_alert();
void user_message();
int user_message_confirm();
char *user_prompt_default();
char *user_prompt_password();
void user_prompt_username_and_password();

void nullifyCallerDataBuff();

Bool initHotList();
Bool addHotListItem();
HotListItem *getNthHotListItem();
Bool deleteHotListItem();
char *dumpHotList();
char *HotList_nextLine();
