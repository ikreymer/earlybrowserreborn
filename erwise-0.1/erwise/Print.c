/*
 * Print.c --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi> Copyright (c) 1992 Teemu Rantanen All
 * rights reserved
 *
 * Created: Sun Apr 26 21:30:47 1992 tvr Last modified: Mon May 11 22:56:41 1992
 * tvr
 *
 */

#include "Includes.h"

#include <fcntl.h>

int PrintTopMargin = 0;
int PrintBottomMargin = 0;
int PrintLeftMargin = 0;
int PrintWidth = 80;
int PrintToFile = 0;

char PrintCommand[1024] = "lpr";
char PrintFileName[1024] = "foobar.www";


/*
 * One object on a line
 */
typedef struct PrintObject_s {

    HTextObject_t *HtObject;

    struct PrintObject_s *Next;
} PrintObject_t;


/*
 * All line objects and info on line
 */
typedef struct PrintLine_s {

    int LineY;

    PrintObject_t *Objects;

    struct PrintLine_s *Next;

} PrintLine_t;



/*
 * Allocate new PrintObject.
 */
PrintObject_t *
 new_print_object()
{
    PrintObject_t *p;

    p = (PrintObject_t *) malloc(sizeof(*p));

    if (!p)
	return p;

    memset(p, 0, sizeof(*p));

    return p;
}


/*
 * Allocate new PrintLine.
 */
PrintLine_t *
 new_print_line()
{
    PrintLine_t *p;

    p = (PrintLine_t *) malloc(sizeof(*p));

    if (!p)
	return p;

    memset(p, 0, sizeof(*p));

    return p;
}


/*
 * Append object to a line. Sort objects according to x position
 */
int printobject_append(line, htobject)
PrintLine_t *line;
HTextObject_t *htobject;
{
    PrintObject_t *object = line->Objects;
    PrintObject_t *prev = 0;

    /*
     * First object on a line
     */
    if (!object) {
	if (!(line->Objects = new_print_object()))
	    return 0;

	line->Objects->HtObject = htobject;

	return 1;
    }
    /*
     * Check on which place do we put this ?
     */
    while (object && (htobject->x > object->HtObject->x)) {

	prev = object;

	object = object->Next;
    }

    /*
     * Set object to line-list
     */

    {
	PrintObject_t *new_object = new_print_object();

	if (!new_object)
	    return 0;

	new_object->Next = object;

	new_object->HtObject = htobject;

	if (prev) {

	    prev->Next = new_object;

	} else {

	    line->Objects = new_object;

	}
    }
}


/*
 * Check on which line object should be appended
 */
PrintLine_t *
 print_check_line_append(first, object)
PrintLine_t *first;
HTextObject_t *object;
{
    PrintLine_t *p;
    PrintLine_t *new_line;
    PrintLine_t *prev;

    /*
     * If this is first line, no problemo ...
     */
    if (!first) {

	p = new_print_line();

	if (!p)
	    return 0;

	p->LineY = object->y;

	if (!printobject_append(p, object))
	    return 0;

	return p;
    }
    /*
     * Find a line
     */
    p = first;
    prev = 0;

    while (p && (p->LineY < object->y)) {

	prev = p;

	p = p->Next;
    }

    /*
     * Line already exists ?
     */
    if (p && (p->LineY == object->y)) {

	if (!printobject_append(p, object))
	    return 0;

	return first;
    }
    /*
     * Line does not exist ... make one
     */

    new_line = new_print_line();

    if (!new_line)
	return 0;

    new_line->LineY = object->y;

    if (prev) {
	/*
         * Append to middle of the list
         */
	prev->Next = new_line;

	new_line->Next = p;

	if (!printobject_append(new_line, object))
	    return 0;

	return first;
    } else {
	/*
         * Append to first
         */
	new_line->Next = p;

	if (!printobject_append(new_line, object))
	    return 0;

	return new_line;
    }

    /*
     * Should not get here
     */
    return 0;
}


/*
 * Print using this command
 */
int erwise_popen(command)
char *command;
{
    int fd[2];

    int ppid;

    if (pipe(fd)) {

	printf("erwise_popen: cannot make pipe\n");

	return -1;
    }
    switch (ppid = fork()) {
    case 0:

	dup2(fd[0], 0);

	close(fd[1]);
	close(fd[0]);

	system(command);

	close(0);

	exit(0);

	break;

    case -1:

	printf("cannot fork\n");

	return -1;

	break;

    default:

	close(fd[0]);

	return (fd[1]);
    }

    /*
     * should not get here ...
     */
    return -1;
}



/*
 * Open files, handle commands, call DoPrint
 */
int Print(htext)
HText_t *htext;
{
    int fd;

    if ((PrintWidth <= 0) || (PrintTopMargin < 0) || (PrintBottomMargin < 0) ||
	(PrintLeftMargin < 0)) {

	printf("Print: parameters insane\n");

	return 1;
    }
    if (PrintToFile) {

	fd = open(PrintFileName, O_WRONLY | O_CREAT, 0666);

	if (fd < 0) {

	    printf("Print: cannot create file %s\n", PrintFileName);

	    return 1;
	}
	DoPrint(htext, fd, PrintWidth, PrintLeftMargin, PrintTopMargin,
		PrintBottomMargin);

	close(fd);

    } else {

	fd = erwise_popen(PrintCommand);

	if (fd < 0) {

	    printf("Print: failed printing to command %s\n", PrintCommand);

	    return 1;
	}
	DoPrint(htext, fd, PrintWidth, PrintLeftMargin, PrintTopMargin,
		PrintBottomMargin);

	close(fd);
    }

    return 0;
}


/*
 * Handle printing. Format text (ascii) and put it to wanted fd
 */
int DoPrint(old_htext, fd, width, lmargin, top, bottom)
HText_t *old_htext;
int fd;
int width;
int lmargin;
int top;
int bottom;
{
    PrintLine_t *first_line = 0;

    HText_t *htext = 0;

    htext = (HText_t *) HtDuplicate(old_htext);

    if (!htext)
	return 0;

    /*
     * Format text using fixed width. After this call every object is
     * positioned correctly.
     */
    XlFormatTextForPrinting(htext, lmargin, lmargin + width);

    /*
     * Because some objects may not be in x,y order, contruct list of lines
     * so that lines are in order 0->j and for every line objects are in
     * order 0->i.
     *
     * Because of allmost all objects are in order by default, contruct lists
     * from last to first. Now most objects can be inserted as the first
     * object of the first line.
     */
    {
	HTextObject_t *p;

	p = htext->last;

	while (p) {

	    /*
             * XXXXX free allocated on error
             */
	    first_line = print_check_line_append(first_line, p);

	    p = p->prev;
	}
    }

    /*
     * Now, print line by line
     */

    {
	PrintLine_t *line = first_line;

	char newline[1];

	int y = 0;
	int max_x;

	*newline = 10;

	/*
         * Write top margin
         */
	for (; top > 0; top--)
	    write(fd, newline, 1);

	while (line) {
	    /*
             * Print empty lines
             */

	    while (y < line->LineY) {
		y++;

		write(fd, newline, 1);
	    }

	    /*
             * Get maximum x on a line
             */
	    {
		PrintObject_t *p;

		p = line->Objects;

		max_x = 0;

		while (p) {

		    if (max_x < (p->HtObject->x + p->HtObject->width))
			max_x = p->HtObject->x + p->HtObject->width;

		    p = p->Next;
		}
	    }

	    /*
             * Collect data on a line and print it
             */
	    {
		PrintObject_t *p;

		char *d = malloc(max_x + 1);

		p = line->Objects;

		/*
	         * XXXXXX free
	         */
		if (!d)
		    return;

		memset(d, ' ', max_x + 1);

		while (p) {
		    if (p->HtObject->data) {

			memcpy(d + p->HtObject->x, p->HtObject->data,
			       p->HtObject->length);
		    }
		    p = p->Next;
		}

		write(fd, d, max_x + 1);

		free(d);
	    }

	    line = line->Next;

	}

	write(fd, newline, 1);

	/*
         * Write bottom margin
         */
	for (; bottom > 0; bottom--)
	    write(fd, newline, 1);

    }

    /*
     * Free objects allocated here
     */

    {
	PrintLine_t *l;
	PrintObject_t *p;

	l = first_line;

	while (l) {

	    p = l->Objects;

	    while (p) {

		free(p);

		p = p->Next;
	    }

	    free(l);

	    l = l->Next;
	}
    }

    /*
     * Delete extra page
     */

    HText_free(htext);
}
