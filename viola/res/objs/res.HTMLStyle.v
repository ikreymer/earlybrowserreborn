\name	{res.HTMLStyle}
\class	{generic}
\parent	{}
\script	{
send("SGML_vobjf", "init");
/*
send("SGML_vobjf", 'D');
print("1000 % 7=", 1000 % 7, "...............\n");

print("=Hi there!=========================================\n");
x = 199;
y = 41;
Lucifer[666] = "evil";
print("x=", x, "<<<\n");
print("y=", y, "<<<\n");
print("Luci=", Lucifer[666], "<<<\n");
print("Arg[0]=", arg[0], "<<<\n");
print("=goodbye!=========================================\n");
*/
	/**
 	*** A makeshift HTML Style sheet resource object
 	**/
	switch (arg[0]) {
	case "id":
/*
		print("=BEGIN============================================\n");
		foo = get("name");
		print("=END==============================================\n");
		return interpret("return " + arg[1] + ";");
*/
	break;
	case "init":
	/*      tag                             wrap first   right  after
	 *               font           align     free   left    before
	 */

		S       = "normal        left   1 1  0   0   0   0  0";
		P       = "normal        left   1 0  24  24  24  0  1";
		OL      = "normal        left   1 0  24  24  8   0  0";
		UL      = "normal        left   1 0  24  24  8   0  0";
		ULC     = "normal        left   1 0  24  24  24  0  0";
		DL      = "normal        left   1 0  24  150 24  0  1";
		DLC     = "normal        left   1 0  24  150 8   0  1";
		LI      = "normal        left   1 0  24  24  24  0  1";
		DIR     = "normal        left   1 0  24  24  24  1  0";
		MENU    = "normal        left   1 0  24  24  24  1  0";
		HP      = "normal        left   1 0  0   0   0   1  0";
		XMP     = "fixed         left   0 1  0   0   1   1  0";
		XMPA    = "fixed         left   0 1  0   0   1   1  0";
		PRE     = "fixed         left   0 1  0   0   1   1  1";
		LISTING = "normal        left   0 1  0   0   1   1  0";
		ADDRESS = "bold_largest  right  0 0  0   0   24  1  1";
		H1      = "bold_large    left   0 0  0   0   10   0  0";
		H2      = "bold          left   1 0  0   0   8   0  0";
		H3      = "bold          left   1 0  24  24  0   0  0";
		H4      = "bold          left   1 0  24  24  0   0  0";
		H5      = "bold          left   1 0  64  64  0   0  0";
		H6      = "bold          left   1 0  128 128 0   0  0";
		H7      = "bold          left   1 0  160 170 0   0  0";
	return 1;
	break;
	case "queryFontID":
		return getVariable(arg[1]);
	break;
	}
	usual();
}
\

