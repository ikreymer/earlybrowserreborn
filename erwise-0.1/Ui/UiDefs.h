/* $Id: UiDefs.h,v 1.2 1992/04/05 11:44:09 kss Exp kny $ */


#define UI_INVALID (-1)

#define UI_SEARCH_WDG_OFFSET       10
#define UI_PAGESETTINGS_WDG_OFFSET 10
#define UI_LIST_WDG_OFFSET         10
#define UI_INFO_WDG_OFFSET         10
#define UI_CP_WDG_OFFSET           10
#define UI_DEFAULTS_WDG_OFFSET     10
#define UI_PRINT_WDG_OFFSET        10
#define UI_CONNECTIONS_WDG_OFFSET        10

#define UI_LEFT  0
#define UI_RIGHT 1
#define UI_UP    2
#define UI_DOWN  3

#define UI_HSCROLLBAR "HScrollBar"
#define UI_VSCROLLBAR "VScrollBar"

#define UI_ERWISE_TITLE "Erwise"
#define UI_SEARCH_TITLE "Text search"
#define UI_LIST_TITLE "List of tags"
#define UI_PRINT_TITLE "Print"
#define UI_SETTINGS_TITLE "Page settings"
#define UI_RECALL_TITLE "Browsed documents"
#define UI_CONTROL_TITLE "Control panel"
#define UI_CONNECTIONS_TITLE "Open connections"
#define UI_DEFAULTS_TITLE "Defaults"

#define ui_VALID(x) ((x) != (-1))
#define ui_INVALID(x) ((x) == (-1))

#define ui_HELPMENU(x) (!strcmp(x, "Help"))
#define ui_TOGGLE(x) ((x)[-1] == '*')
