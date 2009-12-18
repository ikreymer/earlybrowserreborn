typedef struct _MidasApplication {

    Display	         *Display;
    XrmDatabase          Database;
    char                 *Filename;
    int                  Changes;
    ListItem             *Item;
    Widget               Help;
    char                 *AppName;
    char 		 *AppClass;
    int                  Shells;
    int                  Argc;
    char                 **Argv;            	  
    Boolean              ConfirmExit;
    Boolean              AutoStore;

} MidasApplication;

typedef struct _MidasShell {

    Widget               Widget;
    MidasApplication     *Application;
    ListItem             *Item;

} MidasShell;

