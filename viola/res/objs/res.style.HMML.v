\class {generic}
\name {res.style.HMML}
\script {
	switch (arg[0]) {
	case "init":
		/*
		 * _top info current ignored
		 */
		SECTION_top = 3;
		SECTION_bottom = 3;
		SECTION_class = "field";
		SECTION_BDColor = "lemonChiffon2";
		SECTION_BGColor = "lemonChiffon1";
		SECTION_FGColor = "lemonChiffon1";
	
		TITLE_top = 6;
		TITLE_bottom = 6;
		TITLE_class = "txtDisp";
		TITLE_BDColor = "lemonChiffon1";
		TITLE_BGColor = "lemonChiffon1";
		TITLE_FGColor = "black";

		H1_top = 10;
		H1_bottom = 5;

		H2_top = 10;
		H2_bottom = 5;

		H3_top = 5;
		H3_bottom = 3;

		P_top = 4;
		P_left = 6;
		P_right = 6;
		P_bottom = 4;

		PIC_left = 10;
		PIC_right = 10;
		PIC_bottom = 6;

		LABEL_left = 10;
		LABEL_right = 10;
		LABEL_bottom = 6;

		LISTING_top = 6;
		LISTING_left = 10;
		LISTING_right = 10;
		LISTING_bottom = 6;

		EXAMPLE_top = 6;
		EXAMPLE_left = 15;
		EXAMPLE_right = 15;
		EXAMPLE_bottom = 6;

		VOBJF_top = 10;
		VOBJF_left = 10;
		VOBJF_right = 10;
		VOBJF_bottom = 10;

		XBM_top = 10;
		XBM_left = 10;
		XBM_right = 10;
		XBM_bottom = 10;

		XPM_top = 10;
		XPM_left = 10;
		XPM_right = 10;
		XPM_bottom = 10;

		ITITLE_left = 10;
		ITITLE_right = 10;
		ITITLE_bottom = 10;

		QUOTE_left = 10;
		QUOTE_right = 10;
		QUOTE_bottom = 10;

		AUTHOR_top = 6;
		AUTHOR_left = 50;
		AUTHOR_right = 6;
		AUTHOR_bottom = 6;
	break;
	default:
/*		print("**** ", arg[0], " = ", getVariable(arg[0]), "\n");*/
		return getVariable(arg[0]);
	break;
	}
	usual();
}
\
