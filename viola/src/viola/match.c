int matchString(inStr, patStr)
	char *inStr;
	char *patStr;
{
	char *cp;
	int ii = 0;
	int pi = 0;
	int inLength = strlen(inStr);
	int patLength = strlen(patStr);

	for (cp = inStr; *cp; cp++) {
		if (*cp == patStr[pi]) {
			if (++pi >= patLength) {
				return ii;
			}
			if (ii >= inLength) {
				return -1;
			}
		} else {
			pi = 0;
		}
		ii++;
	}
	return -1;
}
/*
			result->info.i = pi;
	result->type = PKT_INT;

	result->info.i = -1;
	return 0;
*/

