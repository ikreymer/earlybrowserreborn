\name {res.font}
\class {generic}
\script {
 	/*
	** Font resource object
 	*/
	switch (arg[0]) {
	case "id":
		return interpret("return " + arg[1] + ";");
	break;
	case "init":
		usual();
		/*
		 * font ID/name inventory list
		 */
		fonts[0] = "fixed";
		fonts[1] = "normal";
		fonts[2] = "normal_large";
		fonts[3] = "normal_largest";
		fonts[4] = "bold";
		fonts[5] = "bold_large";
		fonts[6] = "bold_largest";
		fonts[7] = "italic";
		fonts[8] = "italic_large";
		fonts[9] = "italic_largest";
/*		fonts[10] = "symbol";*/
		fonts[10] = "banner";
		fonts[11] = "banner_large";
		fonts[12] = "banner_largest";
		fonts[13] = "any";

	/* 
	 * font fall-back lists
	 */

	fixed[0]	= "-misc-fixed-medium-r-*-*-14-*-*-*-*-*-*-*";
	fixed[1]	= "-misc-fixed-*-*-*-*-*-*-*-*-*-*-*-*";
	fixed[3]	= "-misc-fixed-*";

	normal[0]	= "-adobe-helvetica-medium-r-*-*-14-*-*-*-p-*-*-*";
	normal[1]	= "-*-helvetica-medium-r-*-*-14-*-*-*-p-*-*-*";
	normal[2]	= "-*-*-medium-r-*-*-14-*-*-*-p-*-*-*";
	normal[3]	= "-misc-fixed-*";

	normal_large[0]	= "-adobe-helvetica-bold-r-*-*-17-*-*-*-p-*-*-*";
	normal_large[1]	= "-*-helvetica-bold-r-*-*-17-*-*-*-p-*-*-*";
	normal_large[2]	= "-*-*-medium-r-*-*-17-*-*-*-p-*-*-*";
	normal_large[3]	= "-misc-fixed-*";

	normal_largest[0] ="-adobe-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
	normal_largest[1] = "-*-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
	normal_largest[2] = "-*-*-medium-r-*-*-24-*-*-*-p-*-*-*";
	normal_largest[3] = "-misc-fixed-*";

	bold[0]		= "-adobe-helvetica-bold-r-*-*-14-*-*-*-p-*-*-*";
	bold[1]		= "-*-helvetica-bold-r-*-*-14-*-*-*-p-*-*-*";
	bold[2]		= "-*-*-bold-r-*-*-14-*-*-*-p-*-*-*";
	bold[3]		= "-misc-fixed-*";

	bold_large[1]	= "-adobe-helvetica-bold-r-*-*-17-*-*-*-p-*-*-*";
	bold_large[1]	= "-*-helvetica-bold-r-*-*-17-*-*-*-p-*-*-*";
	bold_large[2]	= "-*-*-bold-r-*-*-27-*-*-*-p-*-*-*";
	bold_large[3]	= "-misc-fixed-*";

	bold_largest[0]	= "-adobe-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
	bold_largest[1]	= "-*-helvetica-bold-r-*-*-24-*-*-*-p-*-*-*";
	bold_largest[2]	= "-*-*-bold-r-*-*-24-*-*-*-p-*-*-*";
	bold_largest[3]	= "-misc-fixed-*";

	banner[0]	= "-adobe-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
	banner[1]	= "-*-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
	banner[2]	= "-*-*-medium-r-*-*-24-*-*-*-p-*-*-*";
	banner[3]	= "-misc-fixed-*";

	banner_large[0]	= "-adobe-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
	banner_large[1]	= "-*-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
	banner_large[2]	= "-*-*-medium-r-*-*-24-*-*-*-p-*-*-*";
	banner_large[3]	= "-misc-fixed-*-";

	banner_largest[0] = "-adobe-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
	banner_largest[1] = "-*-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
	banner_largest[2] = "-*-*-medium-r-*-*-24-*-*-*-p-*-*-*";
	banner_largest[3] = "-misc-fixed-*";

	italic[0]	= "-adobe-helvetica-medium-*-*-*-14-*-*-*-p-*-*-*";
	italic[1]	= "-*-helvetica-medium-i-*-*-14-*-*-*-p-*-*-*";
	italic[2]	= "-*-*-medium-i-*-*-14-*-*-*-p-*-*-*";
	italic[3]	= "-misc-fixed-*";

	italic_large[0]	= "-adobe-helvetica-medium-i-*-*-17-*-*-*-p-*-*-*";
	italic_large[1]	= "-*-helvetica-medium-i-*-*-17-*-*-*-p-*-*-*";
	italic_large[2]	= "-*-*-medium-i-*-*-17-*-*-*-p-*-*-*";
	italic_large[3]	= "-misc-fixed-*";

	italic_largest[0] = "-adobe-helvetica-medium-i-*-*-24-*-*-*-p-*-*-*";
	italic_largest[1] = "-*-helvetica-medium-i-*-*-24-*-*-*-p-*-*-*";
	italic_largest[2] = "-*-*-medium-i-*-*-24-*-*-*-p-*-*-*";
	italic_largest[3] = "-misc-fixed-*";

	symbol[0] = "-*-symbol-medium-r-*-*-14-*-*-*-*-*-*-*";
	symbol[1] = "-misc-fixed-*";

	any[0]		= "-misc-fixed-medium-r-*-*-14-*-*-*-*-*-*-*";
	any[1]		= "-*-*-medium-*-*-*-*-*-*-*-*-*-*-*";
	any[2]		= "-misc-fixed-*";

/* for violaWWW */
fonts_HelveticaSmall[0] = "-misc-fixed-medium-r-*-*-14-*-*-*-*-*-*-*";
fonts_HelveticaSmall[1] = "-adobe-helvetica-medium-r-*-*-14-*-*-*-p-*-*-*";
fonts_HelveticaSmall[2] = "-adobe-helvetica-medium-r-*-*-17-*-*-*-p-*-*-*";
fonts_HelveticaSmall[3] = "-adobe-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_HelveticaSmall[4] = "-adobe-helvetica-bold-r-*-*-14-*-*-*-p-*-*-*";
fonts_HelveticaSmall[5] = "-adobe-helvetica-bold-r-*-*-17-*-*-*-p-*-*-*";
fonts_HelveticaSmall[6] = "-adobe-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_HelveticaSmall[7] = "-adobe-helvetica-medium-r-*-*-14-*-*-*-p-*-*-*";
fonts_HelveticaSmall[8] = "-adobe-helvetica-medium-r-*-*-17-*-*-*-p-*-*-*";
fonts_HelveticaSmall[9] = "-adobe-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_HelveticaSmall[10] = "-adobe-helvetica-medium-r-*-*-16-*-*-*-p-*-*-*";
fonts_HelveticaSmall[11] = "-adobe-helvetica-medium-r-*-*-17-*-*-*-p-*-*-*";
fonts_HelveticaSmall[12] = "-adobe-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_HelveticaSmall[13] = "-adobe-helvetica-medium-r-*-*-14-*-*-*-p-*-*-*";

fonts_HelveticaMedium[0] = "-misc-fixed-medium-r-*-*-16-*-*-*-*-*-*-*";
fonts_HelveticaMedium[1] = "-adobe-helvetica-medium-r-*-*-17-*-*-*-p-*-*-*";
fonts_HelveticaMedium[2] = "-adobe-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_HelveticaMedium[3] = "-adobe-helvetica-medium-r-*-*-34-*-*-*-p-*-*-*";
fonts_HelveticaMedium[4] = "-adobe-helvetica-bold-r-*-*-17-*-*-*-p-*-*-*";
fonts_HelveticaMedium[5] = "-adobe-helvetica-bold-r-*-*-24-*-*-*-p-*-*-*";
fonts_HelveticaMedium[6] = "-adobe-helvetica-medium-r-*-*-34-*-*-*-p-*-*-*";
fonts_HelveticaMedium[7] = "-adobe-helvetica-medium-r-*-*-17-*-*-*-p-*-*-*";
fonts_HelveticaMedium[8] = "-adobe-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_HelveticaMedium[9] = "-adobe-helvetica-medium-r-*-*-34-*-*-*-p-*-*-*";
fonts_HelveticaMedium[10] = "-adobe-helvetica-medium-i-*-*-20-*-*-*-p-*-*-*";
fonts_HelveticaMedium[11] = "-adobe-helvetica-medium-i-*-*-24-*-*-*-p-*-*-*";
fonts_HelveticaMedium[12] = "-adobe-helvetica-medium-i-*-*-34-*-*-*-p-*-*-*";
fonts_HelveticaMedium[13] = "-adobe-helvetica-medium-r-*-*-17-*-*-*-p-*-*-*";

fonts_HelveticaLarge[0] = "-misc-fixed-medium-r-*-*-20-*-*-*-*-*-*-*";
fonts_HelveticaLarge[1] = "-adobe-helvetica-medium-r-*-*-20-*-*-*-p-*-*-*";
fonts_HelveticaLarge[2] = "-adobe-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_HelveticaLarge[3] = "-adobe-helvetica-medium-r-*-*-34-*-*-*-p-*-*-*";
fonts_HelveticaLarge[4] = "-adobe-helvetica-bold-r-*-*-20-*-*-*-p-*-*-*";
fonts_HelveticaLarge[5] = "-adobe-helvetica-bold-r-*-*-24-*-*-*-p-*-*-*";
fonts_HelveticaLarge[6] = "-adobe-helvetica-medium-r-*-*-34-*-*-*-p-*-*-*";
fonts_HelveticaLarge[7] = "-adobe-helvetica-medium-r-*-*-20-*-*-*-p-*-*-*";
fonts_HelveticaLarge[8] = "-adobe-helvetica-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_HelveticaLarge[9] = "-adobe-helvetica-medium-r-*-*-34-*-*-*-p-*-*-*";
fonts_HelveticaLarge[10] = "-adobe-helvetica-medium-i-*-*-20-*-*-*-p-*-*-*";
fonts_HelveticaLarge[11] = "-adobe-helvetica-medium-i-*-*-24-*-*-*-p-*-*-*";
fonts_HelveticaLarge[12] = "-adobe-helvetica-medium-i-*-*-34-*-*-*-p-*-*-*";
fonts_HelveticaLarge[13] = "-adobe-helvetica-medium-r-*-*-20-*-*-*-p-*-*-*";

fonts_TimesSmall[0] = "-misc-fixed-medium-r-*-*-14-*-*-*-*-*-*-*";
fonts_TimesSmall[1] = "-adobe-times-medium-r-*-*-14-*-*-*-p-*-*-*";
fonts_TimesSmall[2] = "-adobe-times-medium-r-*-*-17-*-*-*-p-*-*-*";
fonts_TimesSmall[3] = "-adobe-times-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_TimesSmall[4] = "-adobe-times-bold-r-*-*-14-*-*-*-p-*-*-*";
fonts_TimesSmall[5] = "-adobe-times-bold-r-*-*-17-*-*-*-p-*-*-*";
fonts_TimesSmall[6] = "-adobe-times-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_TimesSmall[7] = "-adobe-times-medium-r-*-*-14-*-*-*-p-*-*-*";
fonts_TimesSmall[8] = "-adobe-times-medium-r-*-*-17-*-*-*-p-*-*-*";
fonts_TimesSmall[9] = "-adobe-times-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_TimesSmall[10] = "-adobe-times-medium-i-*-*-17-*-*-*-p-*-*-*";
fonts_TimesSmall[11] = "-adobe-times-medium-i-*-*-24-*-*-*-p-*-*-*";
fonts_TimesSmall[12] = "-adobe-times-medium-i-*-*-34-*-*-*-p-*-*-*";
fonts_TimesSmall[13] = "-adobe-times-medium-r-*-*-14-*-*-*-p-*-*-*";

fonts_TimesMedium[0] = "-misc-fixed-medium-r-*-*-20-*-*-*-*-*-*-*";
fonts_TimesMedium[1] = "-adobe-times-medium-r-*-*-20-*-*-*-p-*-*-*";
fonts_TimesMedium[2] = "-adobe-times-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_TimesMedium[3] = "-adobe-times-medium-r-*-*-34-*-*-*-p-*-*-*";
fonts_TimesMedium[4] = "-adobe-times-bold-r-*-*-20-*-*-*-p-*-*-*";
fonts_TimesMedium[5] = "-adobe-times-bold-r-*-*-24-*-*-*-p-*-*-*";
fonts_TimesMedium[6] = "-adobe-times-medium-r-*-*-34-*-*-*-p-*-*-*";
fonts_TimesMedium[7] = "-adobe-times-medium-r-*-*-20-*-*-*-p-*-*-*";
fonts_TimesMedium[8] = "-adobe-times-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_TimesMedium[9] = "-adobe-times-medium-r-*-*-34-*-*-*-p-*-*-*";
fonts_TimesMedium[10] = "-adobe-times-medium-i-*-*-20-*-*-*-p-*-*-*";
fonts_TimesMedium[11] = "-adobe-times-medium-i-*-*-24-*-*-*-p-*-*-*";
fonts_TimesMedium[12] = "-adobe-times-medium-i-*-*-34-*-*-*-p-*-*-*";
fonts_TimesMedium[13] = "-adobe-times-medium-r-*-*-20-*-*-*-p-*-*-*";

fonts_TimesLarge[0] = "-adobe-courier-medium-r-*-*-24-*-*-*-*-*-*-*";
fonts_TimesLarge[1] = "-adobe-times-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_TimesLarge[2] = "-adobe-times-medium-r-*-*-27-*-*-*-p-*-*-*";
fonts_TimesLarge[3] = "-adobe-times-medium-r-*-*-34-*-*-*-p-*-*-*";
fonts_TimesLarge[4] = "-adobe-times-bold-r-*-*-24-*-*-*-p-*-*-*";
fonts_TimesLarge[5] = "-adobe-times-bold-r-*-*-27-*-*-*-p-*-*-*";
fonts_TimesLarge[6] = "-adobe-times-medium-r-*-*-34-*-*-*-p-*-*-*";
fonts_TimesLarge[7] = "-adobe-times-medium-r-*-*-24-*-*-*-p-*-*-*";
fonts_TimesLarge[8] = "-adobe-times-medium-r-*-*-27-*-*-*-p-*-*-*";
fonts_TimesLarge[9] = "-adobe-times-medium-r-*-*-34-*-*-*-p-*-*-*";
fonts_TimesLarge[10] = "-adobe-times-medium-i-*-*-24-*-*-*-p-*-*-*";
fonts_TimesLarge[11] = "-adobe-times-medium-i-*-*-27-*-*-*-p-*-*-*";
fonts_TimesLarge[12] = "-adobe-times-medium-i-*-*-34-*-*-*-p-*-*-*";
fonts_TimesLarge[13] = "-adobe-times-medium-r-*-*-24-*-*-*-p-*-*-*";

	return 1;
	break;
	case "useHelveticaSmallFonts":
		for (i = fonts[] - 1; i >= 0; i--)
			defineNewFont(i, fonts[i], fonts_HelveticaSmall[i]);
		return;
	break;
	case "useHelveticaMediumFonts":
		for (i = fonts[] - 1; i >= 0; i--)
			defineNewFont(i, fonts[i], fonts_HelveticaMedium[i]);
		return;
	break;
	case "useHelveticaLargeFonts":
		for (i = fonts[] - 1; i >= 0; i--)
			defineNewFont(i, fonts[i], fonts_HelveticaLarge[i]);
		return;
	break;
	case "useTimesSmallFonts":
		for (i = fonts[] - 1; i >= 0; i--)
			defineNewFont(i, fonts[i], fonts_TimesSmall[i]);
		return;
	break;
	case "useTimesMediumFonts":
		for (i = fonts[] - 1; i >= 0; i--)
			defineNewFont(i, fonts[i], fonts_TimesMedium[i]);
		return;
	break;
	case "useTimesLargeFonts":
		for (i = fonts[] - 1; i >= 0; i--)
			defineNewFont(i, fonts[i], fonts_TimesLarge[i]);
		return;
	break;
	case "useAllFixedFonts":
		for (i = fonts[] - 1; i >= 0; i--)
			defineNewFont(i, fonts[i], "-misc-fixed-medium-*");
		return;
	break;
	case "queryFontID":
		return getVariable(arg[1]);
	break;
	}
	usual();
}
\
