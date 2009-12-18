/*
** pcx.c - load a ZSoft PC Paintbrush (PCX) file for use inside xloadimage
**
**	Tim Northrup <tim@BRS.Com>
**	Adapted from code by Jef Poskanzer (see Copyright below).
**
**	Version 0.1 --  4/25/91 -- Initial cut
**
**  Copyright (c) 1991 Tim Northrup
**	(see file "tgncpyrght.h" for complete copyright information)
*/
/*
** Copyright (C) 1988 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** This program (pcxtopbm) is based on the pcx2rf program by:
**   Mike Macgirvin
**   Stanford Relativity Gyro Program GP-B
**   Stanford, Calif. 94503
**   ARPA: mike@relgyro.stanford.edu
*/

#include <stdio.h>
#include "tgncpyrght.h"
#include "image.h"

#define PCX_MAGIC 0x0a			/* first byte in a PCX image file */

static void PCX_LoadImage();		/* Routine to load a PCX file */


/*
**  pcxIdent
**
**	Identify passed file as a PC Paintbrush image or not
**
**	Returns 1 if file is a PCX file, 0 otherwise
*/

unsigned int pcxIdent(fullname, name)

char *fullname, *name;
{
	ZFILE *zf;
	unsigned int  ret;
	int xmin;
	int xmax;
	int ymin;
	int ymax;
	unsigned char pcxhd[128];

	ret = 0;

	if (! (zf = zopen(fullname)))
		return(0);

	/* Read .pcx header. */
	if (zread(zf,pcxhd,128) == 128) {
		if (pcxhd[0] == PCX_MAGIC) {
			/* Calculate raster header and swap bytes. */
			xmin = pcxhd[4] + ( 256 * pcxhd[5] );
			ymin = pcxhd[6] + ( 256 * pcxhd[7] );
			xmax = pcxhd[8] + ( 256 * pcxhd[9] );
			ymax = pcxhd[10] + ( 256 * pcxhd[11] );
			xmax = xmax - xmin + 1;
			ymax = ymax - ymin + 1;

			printf("%s is a %dx%d PC Paintbrush image\n",
				name,xmax,ymax);
			ret = 1;
			}
		}

	zclose(zf);

	return(ret);
}


/*
**  pcxLoad
**
**	Load PCX Paintbrush file into an Image structure.
**
**	Returns pointer to allocated struct if successful, NULL otherwise
*/

Image *pcxLoad (fullname,name,verbose)

char *fullname, *name;
unsigned int verbose;
{
	ZFILE *zf;
	unsigned char pcxhd[128];
	int cnt, b;
	int xmin;
	int xmax;
	int ymin;
	int ymax;
	int bytes_per_row;
	register Image *image;

	/* Open input file. */
	if ( ! (zf = zopen(fullname)))
		return((Image *)NULL);

	/* Read .pcx header. */
	if (zread(zf,pcxhd,128) != 128) {
		zclose(zf);
		return((Image *)NULL);
		}

	if ((pcxhd[0] != PCX_MAGIC) || (pcxhd[1] > 5)) {
		zclose(zf);
		return((Image *)NULL);
		}

	if (pcxhd[65] > 1) {
		printf("Unable to handle Color PCX image\n");
		return((Image *)NULL);
		}

	/* Calculate raster header and swap bytes. */
	xmin = pcxhd[4] + ( 256 * pcxhd[5] );
	ymin = pcxhd[6] + ( 256 * pcxhd[7] );
	xmax = pcxhd[8] + ( 256 * pcxhd[9] );
	ymax = pcxhd[10] + ( 256 * pcxhd[11] );
	xmax = xmax - xmin + 1;
	ymax = ymax - ymin + 1;
	bytes_per_row = pcxhd[66] + ( 256 * pcxhd[67] );

	if (verbose)
		printf("%s is a %dx%d PC Paintbrush image\n",name,xmax,ymax);
	znocache(zf);

	/* Allocate pbm array. */
	image = newBitImage(xmax,ymax);

	/* Read compressed bitmap. */
	PCX_LoadImage( zf, bytes_per_row, image, ymax );
	zclose( zf );

	image->title = dupString(name);

	return(image);
}


/*
**  PCX_LoadImage
**
**	Load PC Paintbrush file into the passed Image structure.
**
**	Returns no value (void function)
*/

static void PCX_LoadImage (zf,bytes_per_row,image,rows)

    ZFILE *zf;
    int bytes_per_row;
    Image *image;
    int rows;
{
	/* Goes like this: Read a byte.  If the two high bits are set,
	** then the low 6 bits contain a repeat count, and the byte to
	** repeat is the next byte in the file.  If the two high bits are
	** not set, then this is the byte to write.
	*/

	register unsigned char *ptr;
	int row = 0;
	int bytes_this_row = 0;
	int b, i, cnt;

	ptr = &(image->data[0]);

	while ((b = zgetc(zf)) != EOF) {

		if ((b & 0xC0) == 0xC0) {
			/* have a repetition count -- mask flag bits */
			cnt = b & 0x3F;
			b = zgetc(zf);
			if (b == EOF)
				printf("Error in PCX file: unexpected EOF\n");
			}
		else {
			cnt = 1;		/* no repeating this one */
			}

		for ( i = 0; i < cnt; i++ ) {
			if ( row >= rows ) {
				printf("Warning: junk after bitmap data ignored\n");
				return;
				}
			*ptr++ = (unsigned char) (255 - b);
			if (++bytes_this_row == bytes_per_row) {
				/* start of a new line */
				row++;
				bytes_this_row = 0;
				}
			}
		}

	return;
}
