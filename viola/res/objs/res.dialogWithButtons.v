\class {vpane}
\name {res.dialogWithButtons}
\width {600}
\height {100}
\border {3}
\script {
	switch (arg[0]) {
	case "show":
		newDialogBox = clone(id);
		++id;

		windowName("Please respond to this dialog box.");

		send(newDialogBox, "show_mesg", arg[1]);
		for (argi = 2; argi < arg[]; argi += 2)
			send(newDialogBox, "adopt", arg[argi], arg[argi + 1]);
		send(newDialogBox, "visible", 1);

		return newDialogBox;
	break;
	case "showAt":
		set("x", arg[1]);
		set("y", arg[2]);
		set("width", arg[3]);
		set("height", arg[4]);

		newDialogBox = clone(id);
		++id;

		windowName("Please respond to this dialog box.");

		send(newDialogBox, "show_mesg", arg[5]);
		for (argi = 6; argi < arg[]; argi += 2)
			send(newDialogBox, "adopt", arg[argi], arg[argi + 1]);
		send(newDialogBox, "visible", 1);

		return newDialogBox;
	break;
	case "adopt":
		return send(nthChild(1), "adopt", arg[1], arg[2]);
	break;
	case "show_mesg":
		return send(nthChild(0), "show", arg[1]);
	break;
	case "done":
		set("visible", 0);
		after(1000, self(), "freeSelf");
		return;
	break;
	}
	usual();
}
\children {res.dialogWithButtons.mesg res.dialogWithButtons.buttons}
\
\class {txtDisp}
\name {res.dialogWithButtons.mesg}
\parent {res.dialogWithButtons}
\gapH {4}
\gapV {4}
\script {
	switch (arg[0]) {
	case "show":
		set("content", arg[1]);
		return;
	break;
	case "buttonRelease":
		res.dialogWithButtons.buttons("cinfo");
		return;
	break;
	}
	usual();
}
\font {normal}
\
\class {hpane}
\name {res.dialogWithButtons.buttons}
\parent {res.dialogWithButtons}
\script {
	switch (arg[0]) {
	case "show":
		set("content", arg[1]);
		return;
	break;
	case "config":
/*		print("----------------------------------GGG\n");
		for (i = 0; i < arg[]; i++) print(arg[i], ", ");
		print("\n");
*/
	break;
	case "cinfo":
		objectListSend("children", "info");
	break;
	case "done":
		send(parent(), arg[0]);
		set("children", "");
/*		after(1111, self(), "rip");*/
		return;
	break;
	case "rip":
/*		objectListSend("children", "freeSelf");*/
		set("children", "");
		return;
	break;
	case "adopt":
		/* arg[1] = <label>
		 * arg[2] = <action script>
		 */
/*
		print("----------------------------------\n");
		print("arg[1]>>>>", arg[1], "<<\n");
		print("arg[2]>>>>", arg[2], "<<\n");
*/
		cname = concat("foo", countChildren() + 1);
/*		print("cname>>>>", cname, "<<\n");
*/
		cscript = concat("if (arg[0] == \"buttonRelease\") {", 
					arg[2], 
					"\n send(parent(), \"done\");}\n",
					"usual();");
/*		print("cscript>>>>", cscript, "<<\n");
*/
		newobj = create("name", cname,
				"class", "txtButton",
				"parent", self(),
				"label", arg[1],
				"script", cscript,
				"width", "50",
				"height", "50",
				"font", "any",
				"gapH", "10");

		set("children", concat(get("children"), " ", newobj));
		return;
	break;
	}
	usual();
}
\maxHeight {25}
\gapV {4}
\gapH {4}
\
