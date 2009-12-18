#  Makefile to construct the midaswww library
#  Tony Johnson, Oct, 1992. Some changes for UCX by B.S. Nilsson, Dec. 1992.
#  For UCX: MMS/MACRO=(UCX=1)


.First
	@ delete == "delete/noconf"
.IFDEF UCX
CFLAGSX = /DEFINE=UCX
OPTION_FILE = MIDASWWW_UCX.OPT
.ELSE
	@ define rpc     multinet_root:[multinet.include.rpc]
	@ define netinet multinet_root:[multinet.include.netinet]
	@ define sys     multinet_root:[multinet.include.sys]
CFLAGSX = /include=multinet_root:[multinet.include]
OPTION_FILE = MIDASWWW_MULTINET.OPT
.ENDIF

CFLAGS = $(CFLAGSX)/DEFINE=(VERSION="""2.0""")
NAME=midaswww
LINK_OPTIONS = /NoTrace


LIBRARY = $(NAME).olb

LIBMEMS = $(LIBRARY)(-
		SGMLAnchorText.obj , -
		SGMLCompositeText.obj , -
		SGMLFormattedText.obj , -
		SGMLHyper.obj , -
		SGMLContainerText.obj , -
		SGMLListText.obj , -
		SGMLMarkerText.obj , -
		SGMLRuleText.obj , -
		SGMLPlainText.obj , -
		SGMLText.obj , -
		Tree.obj , -
		Ghostview.obj , -
		GhostviewPlus.obj , -
		ps.obj , -
		custom.obj , -
		midas.obj , -
		midasclass.obj , -
		midasconvert.obj , -
		midasdeclare.obj , -
		midasevaluate.obj , -
		midasingot.obj , -
		midaslist.obj , -
		midasloadimage.obj , -
               midaswww.obj , -
		midasutil.obj , -
               mwhttp.obj , -
               mwhttp2.obj , -
               mwgopher.obj , -
               mwftp.obj , -
		xgifload.obj , -
               strcasecmp.obj )          
 
# Targets

all   : exe midas.uid midaswww.uid
	! done

midas.uid	: midas.uil
	UIL/MOTIF $<

midaswww.uid	: midaswww.uil
	UIL/MOTIF $<

$(NAME).exe : mwmain.obj $(NAME).olb
	link$(LINK_OPTIONS)/exe=$(NAME) mwmain,$(OPTION_FILE)/opt
	purge $(NAME).exe

exe : 
	@ $(MMS) $(MMSQUALIFIERS)/skip $(NAME).exe

test	:	all
	midaswww := $sys$disk:[]midaswww.exe
	midaswww

clean :
	delete/noconfirm *.obj;*

help :
	@echo ' lib:     makes $(NAME).olb'
	@echo ' debug:   makes $(NAME)_debug.olb'
	@echo ' clean:   removes all .obj files'
	@echo ' install: installs libraries and include files'

$(LIBRARY) : $(LIBMEMS)

