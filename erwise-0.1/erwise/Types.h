/* $Id: Types.h,v 1.1 1992/05/18 21:43:03 tvr Exp $ */


typedef struct _page {
    char *Address;
    HText_t *HText;
    struct _page *ParentPage;
    struct _page *Parents;
    struct _page *Children;	/* only two levels */
    struct _page *Next;
} Page_t;


typedef struct _connection {
    char *Address;
    int Status;
    int FD;
    int InputId;
    int TimeOutId;
    ClConnection_t *ClConnection;
    Page_t *TopPage;
    Page_t *ParentPage;
    struct _connection *Next;
} Connection_t;


typedef enum {
    EOC,			/* eof */
    EMPTY,			/* "" */
    UNKNOWN,
    RESOURCE,			/* resource definition, id = value */
    BLOCK,			/* a block containing resources */
    DYNAMIC,			/* dynamic BLOCK */
    DYNAMIC_PTR,		/* ptr to dynamic data (void *ptr[]) */
    COMMENT,			/* "# ..." */
    START_OF_BLOCK,		/* '{' */
    END_OF_BLOCK		/* '}' */
} ConfigType_t;


typedef struct _config {
    ConfigType_t type;
    char *id;
    void *value;
} Config_t;
