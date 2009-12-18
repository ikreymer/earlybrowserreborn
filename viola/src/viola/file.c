/*
 * file.c
 */
/*
 * Copyright 1991 Pei-Yuan Wei.  All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
#include <stdio.h>
#include <pwd.h>
#include "utils.h"
#include "mystrings.h"
#include "file.h"
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

int io_stat;

char *vl_expandPath(path, buffer)
	char *path;
	char *buffer;
{
	if (!path) return NULL;
	if (path[0] == '~') {
/*		extern char *cuserid();*/ /* Bede McCall <bede@mitre.org> */
		struct passwd *info;
		char userName[256];
		int i = 0;

		if (path[1] == '\0' || path[1] == '/') {
                        char *cp = getlogin();

			/* assume path looks like: "~" "~/viola" */
			if (cp) {
				strcpy(userName, cp);
			} else {
				struct passwd *pwds;

				pwds = getpwuid(getuid());
				strcpy(userName, pwds->pw_name);

				while (userName[i++] != ':');
				userName[i] = '\0';
			}
		} else {
			char c;
			/* ie: "~wei/viola" */

			/* get user's name */
			for (; c = path[i + 1]; i++) {
				if (isalpha(c)) userName[i] = c;
				else break;
			}
			userName[i] = '\0';
		}
		if (info = getpwnam(userName)) {
			strcpy(buffer, info->pw_dir);
			strcat(buffer, &path[i + 1]);
			return buffer;
		}
	} else {
		strcpy(buffer, path);
		return buffer;
	}
	fprintf(stderr, "failed to expand ~ for \"%s\"\n", path);
	return NULL;
}

/*
 * enter environment variables into the resource's variable list.
 * 
 */
char *getEnvironVars(argv, name, content)
	char *argv[];
	char *name;
	char *content;
{
	if (argv) {
		int ai = 0, i;
		char label[100];

		while (argv[ai]) {
/*			fprintf(stderr, "argv[%d] = [%s]\n",ai,argv[ai]);*/
			for (i = 0; (label[i] = argv[ai][i]) != '='; i++);
			label[i] = '\0';
			if (!STRCMP(name, label)) {
				strcpy(content, &argv[ai][++i]);
/*				fprintf(stderr, "name=[%s] content= [%s]\n", name, content);*/
				return content;
			}
			++ai;
		}
	}
	return NULL;
}

/*
 * loads a file from disk, and puts it in str.
 *
 * return: -1 if unable to open file.
 */
int loadFile(fileName, strp)
	char *fileName;
	char **strp;
{
	int fd;
	FILE *fp;
	int i = 0, c;
	long size;

	/* printf("loading file '%s'\n",fileName); */

	fd = open(fileName, O_RDONLY);
	if (!fd) {
		io_stat = -1;
		return -1;
	}
	fp = fdopen(fd, "r");
	if (!fp) {
		io_stat = -1;
		return -1;
	}

	/* determine size of file */
	size = lseek(fd, 0, SEEK_END);

	*strp = (char*)malloc(sizeof(char) * (int)size + 1);

	rewind(fp);
	fread(*strp, size, 1, fp);

	(*strp)[size] = '\0';

	fclose(fp);
	close(fd);

	io_stat = i;

	return i;
}

/*
 * saves a str to a file
 *
 * return: -1 if unable to open file.
 */
int saveFile(fileName, str)
	char *fileName;
	char *str;
{
	FILE *fp;

	/*printf("save: name='%s'	 content='%s'\n",fileName,str);*/
	fp = fopen(fileName, "w");

	if (!fp) {
		fprintf(stderr, 
			"Unable to open file '%s'. aborted.\n", fileName);
		return -1;
	}
	fputs(str, fp);

	fclose(fp);

	return 0;
}
