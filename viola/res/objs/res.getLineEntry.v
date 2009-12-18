\class {vpane}
\name {res.getLineEntry}
\parent {}
\width {400}
\height {85}
\script {
	switch (arg[0]) {
	case "show":
		/* arg[1]	display message
		 * arg[2]	default entry
		 * arg[3]	caller, the obj to return result to
		 * arg[4]	returning message token
		 * arg[5]	optional x position
		 * arg[6]	optional y position
		 */
		newObj = clone(id);
		id++;
		if (arg[] > 5)
		  return send(newObj, "_show", arg[1], arg[2], arg[3], arg[4],
				arg[5], arg[6]);
		else 
		  return send(newObj, "_show", arg[1], arg[2], arg[3], arg[4]);
	break;
	case "_show":
		if (arg[] > 5) {
			set("x", arg[5]);
			set("y", arg[6]);
		}
		send(nthChild(0), "show", arg[1]);
		send(nthChild(1), "show", arg[2]);
		caller = arg[3];
		returnMessage = arg[4];

		windowName("Attention, please...");
		send(newObj, "visible", 1);
		set("visible", 1);
		return self();
	break;
	case "_ok":
		set("visible", 0);
		send(caller, returnMessage, send(nthChild(1), "contentP"));
		return;
	break;
	case "_cancel":
		set("visible", 0);
		return;
	break;
	}
	usual();
}
\children {res.getLineEntry.mesg res.getLineEntry.entry res.getLineEntry.buttons}
\border {6}
\
\class {txtDisp}
\name {res.getLineEntry.mesg}
\parent {res.getLineEntry}
\script {
	switch (arg[0]) {
	case "show":
		set("content", arg[1]);
		return;
	break;
	}
	usual();
}
\gapH {2}
\gapV {2}
\font {normal}
\
\class {txtEdit}
\name {res.getLineEntry.entry}
\parent {res.getLineEntry}
\script {
	switch (arg[0]) {
	case "keyPress":
		c = key();
		if (c == '\r' || c == '\n') {
			send(parent(), "_ok");
			return;
		} else {
			insert(c);
			return;
		}
	break;
	case "show":
		set("content", arg[1]);
		return;
	break;
	case "contentP":
		return get("content");
	break;
	}
	usual();
}
\gapH {4}
\gapV {4}
\font {normal}
\
\class {hpane}
\name {res.getLineEntry.buttons}
\parent {res.getLineEntry}
\children {res.getLineEntry.buttons.ok res.getLineEntry.buttons.cancel}
\script {
	switch (arg[0]) {
	case "_cancel":
	case "_ok":
		send(parent(), arg[0]);
		return;
	break;
	}
	usual();
}
\maxHeight {22}
\gapV {3}
\
\class {txtButton}
\label {OK}
\name {res.getLineEntry.buttons.ok}
\parent {res.getLineEntry.buttons}
\script {
	switch (arg[0]) {
	case "buttonRelease":
		send(parent(), "_ok");
		return;
	break;
	}
	usual();
}
\gapH {10}
\
\class {txtButton}
\label {Cancel}
\name {res.getLineEntry.buttons.cancel}
\parent {res.getLineEntry.buttons}
\script {
	switch (arg[0]) {
	case "buttonRelease":
		send(parent(), "_cancel");
		return;
	break;
	}
	usual();
}
\gapH {10}
\
