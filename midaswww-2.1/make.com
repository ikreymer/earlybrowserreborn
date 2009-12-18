$ !
$ ! DCL file to build MidasWWW on an ALPHA or VAX using Multinet or UCX
$ !
$ ! Tony Johnson    17 November 1993
$ !
$ set noverify
$
$ if f$trnlnm("MULTINET").nes."" 
$ Then 
$   TCPIP = "MULTINET"
$ Else
$   TCPIP = "UCX"
$ Endif
$
$ if f$getsyi("HW_MODEL").ge.1024
$ Then 
$   System = "an ALPHA"
$ Else
$   System = "a VAX"
$ Endif
$
$ Write sys$output "Building MidasWWW for ''System' using ''TCPIP'" 
$
$ delete == "delete/noconf"
$ if TCPIP.eqs."UCX"
$ Then
$   Extra = "UCX,"
$   OPTION_FILE = "MIDASWWW_UCX.OPT"
$   if system.eqs."an ALPHA" Then CFLAGSX = "''CFLAGSX'/Stand=VAXC"
$ Else
$   extra ="" 
$   define rpc     multinet_root:[multinet.include.rpc]
$   define netinet multinet_root:[multinet.include.netinet]
$   define sys     multinet_root:[multinet.include.sys]
$   CFLAGSX = "/include=multinet_root:[multinet.include]"
$   OPTION_FILE = "MIDASWWW_MULTINET.OPT"
$   if system.eqs."an ALPHA" Then CFLAGSX = "''CFLAGSX'/Stand=VAXC/PREFIX=ANSI"
$ EndIf
$
$ cc := cc'CFLAGSX'/DEFINE=('extra'VERSION="""""""2.1""""""")
$ LINK_OPTIONS = "/NoTrace"
$!
$! For ill understood reasons this routine fails if compiled with OPT
$! We have traced this as far as the call to XtGetValues in check_tag where
$! the Frame Pointer does not get correctly restored after the call. 
$!
$ cc		SGMLHyper/noopt
$
$ cc		SGMLAnchorText
$ cc		SGMLCompositeText
$ cc		SGMLFormattedText
$ cc		SGMLContainerText
$ cc		SGMLListText
$ cc		SGMLMarkerText
$ cc		SGMLRuleText
$ cc		SGMLPlainText
$ cc		SGMLText
$ cc		SGMLFormText
$ cc		SGMLInputText
$ cc		SGMLTableText
$ cc		SGMLCaptionText
$ cc		Tree
$ cc		Ghostview
$ cc		GhostviewPlus
$ cc		ps
$ cc		custom
$ cc		midas
$ cc		midasclass
$ cc		midasconvert
$ cc		midasdeclare
$ cc		midasevaluate
$ cc		midasingot
$ cc		midaslist
$ cc		midasloadimage
$ cc            midaswww
$ cc		midasutil
$ cc            mwhttp
$ cc            mwhttp2
$ cc            mwgopher
$ cc            mwftp
$ cc            mwfont16
$ cc		xgifload
$ cc            strcasecmp           
$
$ lib/create midaswww *.obj
$ cc            mwmain
$ link'LINK_OPTIONS'/exe=midaswww mwmain,'OPTION_FILE'/opt
$ purge midaswww.exe
$ delete *.obj;* 
$
$ uil/motif midas
$ uil/motif midaswww
$
$ Write sys$output "Done"
