/*
 * slotaccess.h
 */

/*******************
 * class FCard
 */
#define GET_BCard(o) ((VObj*)((o)[54]))
#define SET_BCard(o,v) ((VObj*)((o)[54]=(long)(v)))

/*******************
 * class client
 */
#define GET_inDelimStat(o) ((int)((o)[51]))
#define SET_inDelimStat(o,v) ((int)((o)[51]=(long)(v)))

#define GET_inDelimStr1(o) ((char*)((o)[52]))
#define SET_inDelimStr1(o,v) ((char*)((o)[52]=(long)(v)))

#define GET_inDelimStr2(o) ((char*)((o)[53]))
#define SET_inDelimStr2(o,v) ((char*)((o)[53]=(long)(v)))

#define GET_outDelimStr(o) ((char*)((o)[54]))
#define SET_outDelimStr(o,v) ((char*)((o)[54]=(long)(v)))

#define GET_clientFD(o) ((int)((o)[55]))
#define SET_clientFD(o,v) ((int)((o)[55]=(long)(v)))

#define GET_clientFP(o) ((int)((o)[56]))
#define SET_clientFP(o,v) ((int)((o)[56]=(long)(v)))

/*******************
 * class cosmic
 */
/* common slots accessors */
#define GET_class(o) ((char*)(((ClassInfo*)((o)[0]))->common[0]))
#define SET_class(o,v) ((char*)(((ClassInfo*)((o)[0]))->common[0]=(long)(v)))

#define SLOT_IDX_classScript 1 /* used by load_classScripts()...*/
#define GET_classScript(o) ((char*)(((ClassInfo*)((o)[0]))->common[1]))
#define SET_classScript(o,v) ((char*)(((ClassInfo*)((o)[0]))->common[1]=(long)(v)))

#define GET__classScript(o) ((union PCode*)(((ClassInfo*)((o)[0]))->common[2]))
#define SET__classScript(o,v) ((union PCode*)(((ClassInfo*)((o)[0]))->common[2]=(long)(v)))

/* private slots accessors */
#define GET__classInfo(o) ((ClassInfo*)((o)[0]))
#define SET__classInfo(o,v) ((ClassInfo*)((o)[0]=(long)(v)))

#define GET__memoryGroup(o) ((MemoryGroup*)((o)[1]))
#define SET__memoryGroup(o,v) ((MemoryGroup*)((o)[1]=(long)(v)))

/*******************
 * class field
 */
/* common slots accessors */
/* private slots accessors */
#define GET_content(o) ((char*)((o)[16]))
#define SET_content(o,v) ((char*)((o)[16]=(long)(v)))

#define GET__content(o) ((char*)((o)[17]))
#define SET__content(o,v) ((char*)((o)[17]=(long)(v)))

#define GET_content2(o) ((char*)((o)[18]))
#define SET_content2(o,v) ((char*)((o)[18]=(long)(v)))

#define GET__content2(o) ((o)[19])
#define SET__content2(o,v) ((o)[19]=(long)(v))

#define GET_label(o) ((char*)((o)[20]))
#define SET_label(o,v) ((char*)((o)[20]=(long)(v)))

#define GET__label(o) ((char*)((o)[21]))
#define SET__label(o,v) ((char*)((o)[21]=(long)(v)))

#define GET_border(o) ((long)((o)[22]))
#define SET_border(o,v) ((long)((o)[22]=(long)(v)))

#define GET_x(o) ((long)((o)[23]))
#define SET_x(o,v) ((long)((o)[23]=(long)(v)))

#define GET_y(o) ((long)((o)[24]))
#define SET_y(o,v) ((long)((o)[24]=(long)(v)))

#define GET_width(o) ((long)((o)[25]))
#define SET_width(o,v) ((long)((o)[25]=(long)(v)))
#define IDX_width(o) (25)

#define GET_height(o) ((long)((o)[26]))
#define SET_height(o,v) ((long)((o)[26]=(long)(v)))
#define IDX_height(o) (26)

#define GET_minWidth(o) ((long)((o)[27]))
#define SET_minWidth(o,v) ((long)((o)[27]=(long)(v)))
#define IDX_minWidth(o) (27)

#define GET_minHeight(o) ((long)((o)[28]))
#define SET_minHeight(o,v) ((long)((o)[28]=(long)(v)))
#define IDX_minHeight(o) (28)

#define GET_maxWidth(o) ((long)((o)[29]))
#define SET_maxWidth(o,v) ((long)((o)[29]=(long)(v)))
#define IDX_maxWidth(o) (29)

#define GET_maxHeight(o) ((long)((o)[30]))
#define SET_maxHeight(o,v) ((long)((o)[30]=(long)(v)))
#define IDX_maxHeight(o) (30)

#define GET_gapH(o) ((long)((o)[31]))
#define SET_gapH(o,v) ((long)((o)[31]=(long)(v)))
#define IDX_gapH(o) (31)

#define GET_gapV(o) ((long)((o)[32]))
#define SET_gapV(o,v) ((long)((o)[32]=(long)(v)))
#define IDX_gapV(o) (32)

#define GET_window(o) ((int)((o)[33]))
#define SET_window(o,v) ((int)((o)[33]=(long)(v)))

#define GET_shownPositionH(o) ((int)((o)[34]))
#define SET_shownPositionH(o,v) ((int)((o)[34]=(long)(v)))

#define GET_shownPositionV(o) ((int)((o)[35]))
#define SET_shownPositionV(o,v) ((int)((o)[35]=(long)(v)))

#define GET_shownSizeH(o) ((int)((o)[36]))
#define SET_shownSizeH(o,v) ((int)((o)[36]=(long)(v)))

#define GET_shownSizeV(o) ((int)((o)[37]))
#define SET_shownSizeV(o,v) ((int)((o)[37]=(long)(v)))

#define GET_shownDepend(o) ((char*)((o)[38]))
#define SET_shownDepend(o,v) ((char*)((o)[38]=(long)(v)))

#define GET__shownDepend(o) ((VObjList*)((o)[39]))
#define SET__shownDepend(o,v) ((VObjList*)((o)[39]=(long)(v)))

#define GET_shownNotify(o) ((char*)((o)[40]))
#define SET_shownNotify(o,v) ((char*)((o)[40]=(long)(v)))

#define GET__shownNotify(o) ((VObjList*)((o)[41]))
#define SET__shownNotify(o,v) ((VObjList*)((o)[41]=(long)(v)))

#define GET_lock(o) ((int)((o)[42]))
#define SET_lock(o,v) ((int)((o)[42]=(long)(v)))

#define GET_visible(o) ((int)((o)[43]))
#define SET_visible(o,v) ((int)((o)[43]=(long)(v)))

#define GET__colorInfo(o) ((ColorInfo*)((o)[44]))
#define SET__colorInfo(o,v) ((ColorInfo*)((o)[44]=(long)(v)))

#define GET_BDColor(o) ((char*)((o)[45]))
#define SET_BDColor(o,v) ((char*)((o)[45]=(long)(v)))

#define GET_BGColor(o) ((char*)((o)[46]))
#define SET_BGColor(o,v) ((char*)((o)[46]=(long)(v)))

#define GET_CRColor(o) ((char*)((o)[47]))
#define SET_CRColor(o,v) ((char*)((o)[47]=(long)(v)))

#define GET_FGColor(o) ((char*)((o)[48]))
#define SET_FGColor(o,v) ((char*)((o)[48]=(long)(v)))

#define GET__eventMask(o) ((int)((o)[49]))
#define SET__eventMask(o,v) ((int)((o)[49]=(long)(v)))

#define GET_eventMask(o) ((char*)((o)[50]))
#define SET_eventMask(o,v) ((char*)((o)[50]=(long)(v)))

/*******************
 * class GIF
 */
/* common slots accessors */
/* private slots accessors */
#define GET__baseImage(o) ((XImage*)((o)[51]))
#define SET__baseImage(o,v) ((XImage*)((o)[51]=(long)(v)))

#define GET__expImage(o) ((XImage*)((o)[52]))
#define SET__expImage(o,v) ((XImage*)((o)[52]=(long)(v)))

#define GET_baseImageWidth(o) ((XImage*)((o)[53]))
#define SET_baseImageWidth(o,v) ((XImage*)((o)[53]=(long)(v)))

#define GET_baseImageHeight(o) ((XImage*)((o)[54]))
#define SET_baseImageHeight(o,v) ((XImage*)((o)[54]=(long)(v)))

/*******************
 * class generic
 */
/* common slots accessors */
/* private slots accessors */
#define GET_name(o) ((char*)((o)[2]))
#define SET_name(o,v) ((char*)((o)[2]=(long)(v)))

#define GET_parent(o) ((char*)((o)[3]))
#define SET_parent(o,v) ((char*)((o)[3]=(long)(v)))

#define GET__parent(o) ((VObj*)((o)[4]))
#define SET__parent(o,v) ((VObj*)((o)[4]=(long)(v)))

#define GET_children(o) ((char*)((o)[5]))
#define SET_children(o,v) ((char*)((o)[5]=(long)(v)))

#define GET__children(o) ((VObjList*)((o)[6]))
#define SET__children(o,v) ((VObjList*)((o)[6]=(long)(v)))

#define GET_security(o) ((int)((o)[7]))
#define SET_security(o,v) ((int)((o)[7]=(long)(v)))

#define GET_script(o) ((StrInfo*)((o)[8]))
#define SET_script(o,v) ((StrInfo*)((o)[8]=(long)(v)))

#define GET__script(o) ((union PCode*)((o)[9]))
#define SET__script(o,v) ((union PCode*)((o)[9]=(long)(v)))

#define GET__varList(o) ((Attr*)((o)[10]))
#define SET__varList(o,v) ((Attr*)((o)[10]=(long)(v)))

#define GET__argAttr(o) ((Attr*)((o)[11]))
#define SET__argAttr(o,v) ((Attr*)((o)[11]=(long)(v)))

#define GET__classScriptVV(o) ((int*)((o)[12]))
#define SET__classScriptVV(o,v) ((int*)((o)[12]=(long)(v)))
#define PTR__classScriptVV(o) &o[12]

#define GET__scriptVV(o) ((int*)((o)[13]))
#define SET__scriptVV(o,v) ((int*)((o)[13]=(long)(v)))
#define PTR__scriptVV(o) &o[13]

#define GET__tempScriptVV(o) ((int*)((o)[14]))
#define SET__tempScriptVV(o,v) ((int*)((o)[14]=(long)(v)))
#define PTR__tempScriptVV(o) &o[14]

#define GET_active(o) ((long)((o)[15]))
#define SET_active(o,v) ((long)((o)[15]=(long)(v)))

/*******************
 * class HTML
 */
/* common slots accessors */
/* private slots accessors */
#define GET_HTMLAddress(o) ((char*)((o)[60]))
#define SET_HTMLAddress(o,v) ((char*)((o)[60]=(long)(v)))

#define GET_HTMLAnchor(o) ((char*)((o)[61]))
#define SET_HTMLAnchor(o,v) ((char*)((o)[61]=(long)(v)))

#define GET_HTMLIsIndex(o) ((long)((o)[62]))
#define SET_HTMLIsIndex(o,v) ((long)((o)[62]=(long)(v)))

#define GET_HTMLSource(o) ((char*)((o)[63]))
#define SET_HTMLSource(o,v) ((char*)((o)[63]=(long)(v)))

#define GET_HTMLStyle(o) ((char*)((o)[64]))
#define SET_HTMLStyle(o,v) ((char*)((o)[64]=(long)(v)))

#define GET_HTMLTitle(o) ((char*)((o)[64]))
#define SET_HTMLTitle(o,v) ((char)((o)[64]=(long)(v)))

#define GET_HTMLStruct(o) ((o)[65])
#define SET_HTMLStruct(o,v) ((o)[65]=(long)(v))


/*******************
 * class menu
 */
#define GET_menuConfig(o) ((char*)((o)[55]))
#define SET_menuConfig(o,v) ((char*)((o)[55]=(long)(v)))

#define GET__menu(o) ((o)[56])
#define SET__menu(o,v) (o)[56]=(long)(v)

#define GET__menuEntries(o) ((o)[57])
#define SET__menuEntries(o,v) (o)[57]=(long)(v)

/*******************
 * class pane
 */
/* common slots accessors */
/* private slots accessors */
#define GET_paneConfig(o) ((char*)((o)[51]))
#define SET_paneConfig(o,v) ((char*)((o)[51]=(long)(v)))

#define GET__paneConfig(o) ((long)((o)[52]))
#define SET__paneConfig(o,v) ((long)((o)[52]=(long)(v)))

/*******************
 * class PS
 */
/* common slots accessors */
/* private slots accessors */
#define GET_PSMag(o) ((char*)((o)[51]))
#define SET_PSMag(o,v) ((char*)((o)[51]=(long)(v)))

/*******************
 * class project
 */
/* common slots accessors */
/* private slots accessors */
#define GET_filePath(o) ((char*)((o)[51]))
#define SET_filePath(o,v) ((char*)((o)[51]=(long)(v)))

#define GET_projectIcon(o) ((char*)((o)[52]))
#define SET_projectIcon(o,v) ((char*)((o)[52]=(long)(v)))

#define GET__projectIcon(o) ((o)[53])
#define SET__projectIcon(o,v) ((o)[53]=(long)(v))

/*******************
 * class stack
 */
#define GET_BCardList(o) ((char*)((o)[53]))
#define SET_BCardList(o,v) ((char*)((o)[53]=(long)(v)))

#define GET_FCardList(o) ((char*)((o)[54]))
#define SET_FCardList(o,v) ((char*)((o)[54]=(long)(v)))

#define GET__BCardList(o) ((VObjList*)((o)[55]))
#define SET__BCardList(o,v) ((VObjList*)((o)[55]=(long)(v)))

#define GET__FCardList(o) ((VObjList*)((o)[56]))
#define SET__FCardList(o,v) ((VObjList*)((o)[56]=(long)(v)))

#define GET__currentCard(o) ((VObj*)((o)[57]))
#define SET__currentCard(o,v) ((VObj*)((o)[57]=(long)(v)))

/*******************
 * class txt
 */
/* common slots accessors */
/* private slots accessors */
#define GET_font(o) ((char*)((o)[53]))
#define SET_font(o,v) ((char*)((o)[53]=(long)(v)))

#define GET__font(o) ((long)((o)[54]))
#define SET__font(o,v) ((long)((o)[54]=(long)(v)))

/*******************
 * class txtDisp
 */
/* common slots accessors */
/* private slots accessors */
#define GET_cursor(o) ((long)((o)[55]))
#define SET_cursor(o,v) ((long)((o)[55]=(long)(v)))

#define GET_wrap(o) ((long)((o)[56]))
#define SET_wrap(o,v) ((long)((o)[56]=(long)(v)))

#define GET_verbatim(o) ((long)((o)[57]))
#define SET_verbatim(o,v) ((long)((o)[57]=(long)(v)))

#define GET_format(o) ((long)((o)[58]))
#define SET_format(o,v) ((long)((o)[58]=(long)(v)))

#define GET__TFStruct(o) ((TFStruct*)((o)[59]))
#define SET__TFStruct(o,v) ((TFStruct*)((o)[59]=(long)(v)))

/*******************
 * class txtEdit
 */
/* common slots accessors */
/* private slots accessors */

/*******************
 * class txtLabel
 */
/* common slots accessors */
/* private slots accessors */

/*******************
 * class slider
 */
/* common slots accessors */
/* private slots accessors */
#define GET_direction(o) ((char*)((o)[51]))
#define SET_direction(o,v) ((char*)((o)[51]=(long)(v)))

/*******************
 * class socket
 */
/* common slots accessors */
/* private slots accessors */
#define GET_host(o) ((char*)((o)[57]))
#define SET_host(o,v) ((char*)((o)[57]=(long)(v)))

#define GET_port(o) ((char*)((o)[58]))
#define SET_port(o,v) ((char*)((o)[58]=(long)(v)))

/*******************
 * class toggle
 */
/* common slots accessors */
/* private slots accessors */
#define GET_toggleState(o) ((int)((o)[51]))
#define SET_toggleState(o,v) ((int)((o)[51]=(long)(v)))

#define GET_toggleStyle(o) ((char*)((o)[52]))
#define SET_toggleStyle(o,v) ((char*)((o)[52]=(long)(v)))

/*******************
 * class TTY
 */
/* common slots accessors */
/* private slots accessors */
#define GET_path(o) ((char*)((o)[57]))
#define SET_path(o,v) ((char*)((o)[57]=(long)(v)))

#define GET_args(o) ((char*)((o)[58]))
#define SET_args(o,v) ((char*)((o)[58]=(long)(v)))

#define GET_pid(o) ((long)((o)[59]))
#define SET_pid(o,v) ((long)((o)[59]=(long)(v)))

