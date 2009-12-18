/*
 * Copyright 1990-93 Pei-Yuan Wei.	All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
/* 
 * viola.c
 *
 * Scott: reorganized the former viola main.c to smaller modules and 
 *        put them here.
 */
/* Some extraneous includes, no doubt */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "utils.h"
#include <math.h>
#include "mystrings.h"
#include "hash.h"
#include "ident.h"
#include "obj.h"
#include "glib.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "classlist.h"
#include "loader.h"
#include "ast.h"
#include "cgen.h"
#include "attr.h"
#include "cexec.h"
#include "slotaccess.h"
#include "misc.h"
#include "file.h"
#include "event_x.h"

#define DFLT_USR_WWW_HOME "WWW" /* patched */

char *viola_version = "Viola=3.0 Alpha";
/*= <geom>		.\n\n\*/
char *viola_usage = {"Usage:\n\
-geometry <geom>	geometry\n\
-display <display>	display\n\n\
-cli			command line interface.\n\
-c			.\n\n\
-laf <mode>		(unsup	ported) look and feel mode (0 for expensive drawing).\n\n\
-obj <object name>	startup object (send \"render\" to it).\n\
-o <object name>	.\n\
-path <file path>	object file lookup path.\n\
-p <file path>		.\n\n\
-release		print release info.\n\
-r			.\n\n\
-snipet	<statement>	script snipet to interpret.\n\
-s <statement>		.\n\n\
-verbose		verbose mode.\n\
-v			.\n\n\
-V			version.\n\n\
(for debugging)\n\
-pa			print AST.\n\
-pc			print PCode.\n\
-pe			print PCode execution trace.\n\
-wt			window tracking.\n\
-at			user action tracking.\n\
-z			sync events.\n\
\n\
"};

/*
(not yet...)\n\
For more help on these arguments, do\n\
	viola -o www -arg0 show -arg1 'xcf.berkeley.edu/ht/violaOptionsHelp'\n\
viola -o foo,erwhrwehwe,erewr,wrerwe,wre

viola -o hmml -0Sshow -1I234 -2CHello

-0 <mesg>		arg[0] to -o object.\n
-1 <mesg>		arg[1] \n
-2 <mesg>		arg[2] \n
-3 <mesg>		arg[3] \n
-4 <mesg>		arg[4] \n
-5 <mesg>		arg[5] \n
-6 <mesg>		arg[6] \n
-7 <mesg>		arg[7] \n
-8 <mesg>		arg[8] \n\n
-arg0 <message>		arg[0] to send to object specified by -o.\n\
-arg1 <message>		arg[1] to send to object specified by -o.\n\
-arg2 <message>		arg[2] to send to object specified by -o.\n\
...\n\
-argN <message>		arg[N] to send to object specified by -o.\n\
-argType <type>		arg[n] type modifiers:\n\

*/

#ifndef DEFAULT_VIOLA_PATH
#define DEFAULT_VIOLA_PATH "/usr/lib/apps"
#endif

/* intefers with normal events, used only for http progress reports */
int perishableActiveHelp = 0;

Window topWindow;   /* Sub-Window that Viola is supposed to run in. */
int runInSubWindow = 0;   /* Set to 1 when Viola is run in library mode. */

void parseCommandLine();

/* Scott */
/*
 * Intended to be called by programs that use Viola as a library.
 *
 * The calling program must supply a script file name, an open Display,
 * the proper Screen pointer, and a window in which Viola is to run.
 *
 * The calling program must thereafter call violaProcessEvent() whenever
 * there is an event in Viola's window and must periodically call
 * violaIdleEvent() when there are no events to process and nothing
 * better to do otherwise.
 */
char *initViola(argc, argv, vObjFile, display, screen, parentWindow)
	int argc;
	char *argv[];
	char *vObjFile;
	Display *display;
	Screen *screen;
	Window parentWindow;
{
	extern char **environ;
	Packet *result;
	HashEntry *entry;
	int i = 0;
	int startWithCommandLineInterpreter = 0;
	char *defaultViolaPath = DEFAULT_VIOLA_PATH;
	char *violaPath;
	char *scriptSnipet = NULL;
	char *startObj[32];
	int startObjCount = 0;
	char topVObjName[256];
	VObj *obj;
	void scanTopObjName();
	extern char *sgmlPath;

	/* Scott */
	if (parentWindow) {
		runInSubWindow = 1;
		topWindow = parentWindow;
	}

	if (vObjFile)
		startObj[startObjCount++] = vObjFile;
	
	if (violaPath = getEnvironVars(environ, "VIOLA_PATH", buff))
	violaPath = saveString(violaPath);

	if (sgmlPath = getEnvironVars(environ, "VIOLA_SGML", buff))
	sgmlPath = saveString(sgmlPath);

	parseCommandLine(argc, argv,
			 startObj,
			 &startObjCount,
			 &violaPath,
			 &startWithCommandLineInterpreter,
			 &scriptSnipet);
	
	/* initUserWWW(DFLT_USR_WWW_HOME); */ /*PATCHED*/
	
	if (!init_sys()) exit(2);
	if (!init_ident()) exit(4);
	if (!init_class()) exit(5);
	if (!init_obj()) exit(6);
	if (!init_scanutils()) exit(7);
	if (!init_ast()) exit(8);
	if (!init_packet()) exit(9);
	if (!init_glib(display, screen)) exit(10);
	if (!init_tfed()) exit(1);
	if (!init_event()) exit(11);
	if (!init_cexec()) exit(12);
	if (!init_html()) exit(13);
	if (!SGMLInit()) exit(14);
	
	if (violaPath) {
		if (init_loader(violaPath)) {
			if (verbose)
			fprintf(stderr, "Using VIOLA_PATH=``%s''\n", 
				violaPath);
		}
	} else if (init_loader(defaultViolaPath)) {
	if (verbose)
		fprintf(stderr, "Using default violaPath=``%s''\n", 
			defaultViolaPath);
	} else {
		fprintf(stderr, 
			"Can't find application files. Check VIOLA_PATH.\n");
		exit(20);
	}
	
/*	if (!setHTMLStyle()) exit(14);*/
	
	load_object("res.font.v", NULL);
	entry = objID2Obj->get(objID2Obj, storeIdent("res.font"));
	if (!entry) {
		fprintf(stderr, 
			"can't find: font resource file (\"res.font.v\").\n");
	exit(21);
	}
	if (verbose) 
		fprintf(stderr, 
			"loaded: font resource file (\"res.font.v\").\n");
	
	VFontResourceObj = (VObj*)entry->val;

	if (!init_fonts()) {
		fprintf(stderr, "aborting due to font related problem.\n");
		exit(23);
	}
	
	if (verbose) fprintf(stderr, "initialized font resources.\n");
	
	/* always instantiate object "res" */
	
	load_object("res.v", NULL);
	entry = objID2Obj->get(objID2Obj, storeIdent("res"));
	if (!entry) {
		fprintf(stderr, 
			"can't find: resource file (\"res.v\").\n");
		exit(24);
	}
	if (!(entry->val)) {
		fprintf(stderr, 
			"can't find: resource file (\"res.v\").\n");
		exit(25);
	}
	obj = (VObj*)entry->val;
	if (obj) {
		VPrevObj = VTargetObj = VResourceObj = VCurrentObj 
		= obj;
		/*			sendMessage1(obj, "init");*/
	} else {
		fprintf(stderr, "Failed to instantiate res! Bye.\n");
		exit(99);
	}
	
	for (i = 0; i < startObjCount; i++) {
		char objFileName[100];
	
		sprintf(objFileName, "%s.v", startObj[i]);
		load_object(objFileName, NULL);
		entry = objID2Obj->get(objID2Obj, storeIdent(startObj[i]));
		if (!entry) {
			fprintf(stderr, 
				"can't find: start up file (\"%s\").\n",
				objFileName);
			exit(22);
		}
		obj = (VObj*)entry->val;
		if (obj) {
			VPrevObj = VTargetObj = VCurrentObj = obj;
			/*sendMessage1(obj, "init");*/
			sendMessage1(obj, "render");
		} else {
			fprintf(stderr, "No start up object! Bye.\n");
			exit(27);
		}
	}
	
	result = borrowPacket();
	
	if (startWithCommandLineInterpreter)
	meth_generic_cli(VCurrentObj, result, 0, 0);
	
	sendMessage1N1int(VCurrentObj, "visible", 1);
/*	sendMessage1(VCurrentObj, "render");*/
	
	if (scriptSnipet) {
		execScript(VCurrentObj, result, scriptSnipet);
	}
	
	if (verbose) fprintf(stderr, "entering event loop.\n");

	/* Scott: don't enter viola's eventLoop if motif's running */
	if (!runInSubWindow) eventLoop();
	
	returnPacket();
	
	if (user_action_tracking)
	if (user_action_tracking != stderr) 
		close(user_action_tracking);

	if (!runInSubWindow) exit(0);

	/* Scott */
	if (vObjFile) {
		char *name;
	
		scanTopObjName(vObjFile, topVObjName);
		name = malloc(strlen(topVObjName)+1);
		strcpy(name, topVObjName);
		return(name);
	} else {
		return(NULL);
	}
	XtToolkitInitialize();	/* just to make sure Xt is linked??*/
}

void freeViolaResources()
{
	tfed_FreeTmpFileToFree(0);
	free_fonts();
	freeAllObjects();
	freeGLibResources();
}

/* Scott: moved this code from the former main.c into this file
 */ 
void parseCommandLine(argc, argv,
			  startObj,
			  startObjCount,
			  violaPath,
			  startWithCommandLineInterpreter,
			  scriptSnipet)
	int argc;
	char *argv[];
	char *startObj[];
	int *startObjCount;
	char **violaPath;
	int *startWithCommandLineInterpreter;
	char **scriptSnipet;
{
	int i=0;
	
	if (argc)
	while (argv[++i]) {
		/*fprintf(stderr, "arg[%d]=%s\n", i, argv[i]);*/

		if (noCaseStrCmp(argv[i], "-ar")) {

			passthru_argument = argv[++i];
		
		} else if (noCaseStrCmp(argv[i], "-cli") ||
			noCaseStrCmp(argv[i], "-c")) {
		
			if (!runInSubWindow)
				*startWithCommandLineInterpreter = 1;
		
		} else if (noCaseStrCmp(argv[i], "-cmd_history")) {
		
			cmd_history = 1;
		
		} else if (noCaseStrCmp(argv[i], "-mono")) {
			extern int noShading;
			noShading = 1;
		
		} else if (noCaseStrCmp(argv[i], "-geometry") ||
			   noCaseStrCmp(argv[i], "=")) {
		
		/* Scott */
		if (!runInSubWindow)
			XParseGeometry(argv[++i], 
				   &startUpGeom_x, &startUpGeom_y,
				   &startUpGeom_width, &startUpGeom_height);
		
		} else if (noCaseStrCmp(argv[i], "-display")) {
		
			startUpDisplay = argv[++i];
		
		} else if (noCaseStrCmp(argv[i], "-nocliprompt")) {
			extern int flag_cliprompt;
	
			if (!runInSubWindow)	/* Scott */
				flag_cliprompt = 0;

		} else if (noCaseStrCmp(argv[i], "-obj") ||
			   noCaseStrCmp(argv[i], "-o")) {

			startObj[*startObjCount] = argv[++i];
			*startObjCount += 1;
		
		} else if (noCaseStrCmp(argv[i], "-pa")) {
		
			flag_printAST = 1;
		
		} else if (noCaseStrCmp(argv[i], "-pc")) {
		
			flag_printPCode = 1;
		
		} else if (noCaseStrCmp(argv[i], "-pe")) {
		
			flag_printExec = 1;
		
		} else if (noCaseStrCmp(argv[i], "-z") ||
			   noCaseStrCmp(argv[i], "-xsync")) {
		
			sync_event = 1;
		
		} else if (noCaseStrCmp(argv[i], "-LAF") || 
			   noCaseStrCmp(argv[i], "-laf") || 
			   noCaseStrCmp(argv[i], "-l")) {
		
		if (!argv[++i]) {
			/* assume simple mode (1) */
			lookAndFeel = 1;
		} else {
			lookAndFeel = atoi(argv[i]);
		}
		
		} else if (noCaseStrCmp(argv[i], "-noarg")) {
		
		} else if (noCaseStrCmp(argv[i], "-path") ||
			   noCaseStrCmp(argv[i], "-p")) {

			/* Scott */
			*violaPath = argv[++i];
		
		} else if (noCaseStrCmp(argv[i], "-release") || 
			   noCaseStrCmp(argv[i], "-r")) {
		
		fprintf(stderr, "%s\n", viola_version);
		
		} else if (noCaseStrCmp(argv[i], "-snipet") ||
			   noCaseStrCmp(argv[i], "-s")) {

			/* Scott */
			*scriptSnipet = argv[++i];
		
		} else if (noCaseStrCmp(argv[i], "-verbose") ||
			   noCaseStrCmp(argv[i], "-v")) {
		
			verbose = 1;

		} else if (noCaseStrCmp(argv[i], "-tagd")) {

			extern int printTagDelims;

			printTagDelims = 1;
		
		} else if (noCaseStrCmp(argv[i], "-wt")) {
		
			flag_window_tracking = 1;
		
		} else if (noCaseStrCmp(argv[i], "-at")) {
		
			if (argv[i+1][0] != '-') {
				user_action_tracking = fopen(argv[++i], "w");
				if (!user_action_tracking) {
				fprintf(stderr, 
	"Failed to open file \"%s\" for user action tracking. Using stderr.\n", 
					argv[i]);
				user_action_tracking = stderr;
				}
			} else {
				user_action_tracking = stderr;
			}
		
		} else if (noCaseStrCmp(argv[i], "-lcs")) {
		
		    loadClassScriptsP = 1;

		} else if (noCaseStrCmp(argv[i], "-debug")) {
		    
		    /* Flag allows spider to print out diagnostic stuff. */;
		
		} else {
			passthru_argument = argv[i];
		}
/*
 * Scott
 * 
 * When viola sees arguments that are for Spider, it barfs.  Until Spider
 * removes it's arguments, we'll just comment out this code that complains
 * about unknown flags.
 *
                else {
			fputs(viola_usage, stderr);
			exit(0);
		}
 */
		if (i >= argc) break;
	}

	/* for convenience, automatically start up www if no 
	 * application is specified 
	 */
	if (*startObjCount == 0) {
		startObj[0] = "www";
		*startObjCount = 1;
	}
}


/* Bede McCall <bede@mitre.org> MITRE Corp 
 */
#include <errno.h>
#include <pwd.h>

extern int errno;
extern char *sys_errlist[];
extern int sys_nerr;

initUserWWW ( www )
	 char *www;
{
	int Errno;
	char *cp;
	static char path[128];
	struct passwd *pw;

	Errno = 0;
	bzero(path, 128);

#ifdef i386
	pw = getpwuid(getuid());
#else
	{
	  char *cuserid();
	  struct passwd *getpwnam();

	  cp = cuserid((char *)NULL);
	  if (!cp) return;
	  pw = getpwnam(cp);
	}
#endif
	(void) strcat(path, pw->pw_dir);
	(void) strcat(path, "/");
	(void) strcat(path, www);
	
	if ( mkdir(path, 0x755) ) {
	  Errno = errno;
	  if ( verbose ) {
	    if ( Errno < sys_nerr )
	      (void) printf("mkdir(%s), error %d: %s\n", 
			    path, Errno, sys_errlist[Errno]);
	    else
	      (void) printf("mkdir(%s), error %d\n", path, Errno);
	  }
	} else if ( verbose ) {
	  (void) printf("created %s\n", path);
	}
	(void) fflush(stdout);
}


/*
 * To get a Viola object name from a file path name.
 */
void scanTopObjName(fileName, returnName)
	char *fileName, *returnName;
{
	char *cp;
	int len;

	if (!fileName) {
	*returnName = '\0';
	return;
	}
	len = strlen(fileName);
	
	if (!len) {
 	*returnName = '\0';
	return;
	}
   
	cp = fileName + len - 1;

	while(cp != fileName && *cp != '/')
	cp--;
	if (*cp == '/') cp++;

	while(*cp && *cp != '.')
	*returnName++ = *cp++;
	*returnName = '\0';
}

