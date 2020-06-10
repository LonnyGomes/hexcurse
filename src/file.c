/******************************************************************************\
 *  Copyright (C) 2001, hexcurse is written by Jewfish and Armoth             *
 *									      *
 *  This program is free software; you can redistribute it and/or modify      *
 *  it under the terms of the GNU General Public License as published by      *
 *  the Free Software Foundation; either version 2 of the License, or	      *
 *  (at your option) any later version.					      *
 *									      *
 *  This program is distributed in the hope that it will be useful,	      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of	      *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	      *
 *  GNU General Public License for more details.			      *
 *									      *
 *  You should have received a copy of the GNU General Public License	      *
 *  along with this program; if not, write to the Free Software		      *
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *									      *
\******************************************************************************/
#include "hex.h"

/*******************************************************\
 * Description: prints out a line of text to the screen*
 *		the current address line and both the          *
 *		hex and decimal values for the current         *
 *		lines. It doesn't return anything              *
\*******************************************************/
void outline(FILE *fp, off_t linenum)
{
    int i, c, tmp[BASE];					/* holds char values  */
    hexList *tmpHead = head;					/* tmp linklist head  */

    for (i = 0; i < BASE; i++)					/* set vals to EOF    */
		tmp[i] = -1;

    while (tmpHead != NULL && tmpHead->loc < (linenum * BASE))
		tmpHead = tmpHead->next;				/* advance temp head  */

    for (i = 0; i < BASE; i++) 
	{
		while (tmpHead != NULL && (tmpHead->loc < ((linenum * BASE) + i)))
			tmpHead = tmpHead->next;
		
		if (tmpHead != NULL && (tmpHead->loc == ((linenum * BASE) + i)))
		{										/*store val from llist*/
			tmp[i] = tmpHead->val;
			tmpHead = tmpHead->next;
		}
    }

    wclrtoeol(windows->hex);					/* clear lines        */
    wclrtoeol(windows->ascii);

    /*print line's address*/
    address_color_on((intmax_t)(linenum * BASE));
    wprintw(windows->address, (printHex) ? "%0*jX ":"%0*jd ", MIN_ADDR_LENGTH, (intmax_t)(linenum * BASE));
    address_color_off((intmax_t)(linenum * BASE));

    rewind(fp);									/* reset the file ptr */
    fseeko(fp, (linenum * BASE), 0);				/* set new pos for fp */

    for (i = 0; i < BASE && (c = getc(fp)) != EOF; i++)
    {
		if (tmp[i] != -1) 						/* while not EOF      */
		{	c = tmp[i];							/* store val in c     */
			wattron(windows->ascii, A_BOLD);
			wattron(windows->hex, A_BOLD);
		}
        byte_color_on((linenum * BASE) + i, c);
		wprintw(windows->hex, "%02X ", c);		/* print out hex char */
		if (USE_EBCDIC)
			wprintw(windows->ascii, "%c", EBCDIC[c]);/* print EBCDIC char */
		else									/* print ASCII  char */
            wprintw(windows->ascii, (isprint(c)) ? "%c":".", c);
        byte_color_off((linenum * BASE) + i, c);
        if (tmp[i] != -1) {
            wattroff(windows->ascii, A_BOLD);
            wattroff(windows->hex, A_BOLD);
        }
    }
}

/*******************************************************\
 * Description: determines the maximum size of the     *
 *		file.  It returns the number of chars  *
 *		in the file                            *
\*******************************************************/
off_t maxLoc(FILE *fp)
{
    fseeko(fp, 0, SEEK_END);				/* seek to end of file*/
    return(ftello(fp));				/* return val at EOF  */
}

/******************************************************\
 * Description: prints out the command line help info *
 *		this function does not return anything*
\******************************************************/
void print_usage()
{
    char *ver = HVERSION; 

    printf("hexcurse, version %s by James Stephenson and Lonny Gomes\n",ver);
    printf("\nusage: hexcurse [-?|help] [-a] [-e] [-r rnum] [-o outputfile] ");
    printf("[[-i] infile]\n\n");
    printf("    -a\t\tOutput addresses in decimal format initially\n");
    printf("    -e\t\tOutput characters in EBCDIC format rather than ASCII\n"); 
    printf("    -t\tKeep colors defined by terminal\n");
    printf("    -r rnum\tResize the display to \"rnum\" bytes wide\n");
    printf("    -o outfile\tWrite output to outfile by default\n"); 
    printf("    -? | -help\tDisplay usage and version of hexcurse program\n");
    printf("    [-i] infile\tRead from data from infile (-i required if not last argument)\n\n");
}

/*******************************************************\
 * Description: Determines the maximum lines that will *
 *		be displayed in the hexeditor.  The            *
 *		values is returned as an integer               *
\******************************************************/
off_t maxLines(off_t len)
{   
    if (BASE == 0) return(0);						/* avoid a div by 0   */

    if ((len % BASE) !=0)							/* mod len by BASE    */
        return(len/BASE);							/* so extra line wont */
    else											/* be printed out     */
        return((BASE==1) ? 0 : (len/BASE - 1));		/* avoid pot div by 0 */
    return(len/BASE);
}

/********************************************************\
 * Description: opens file specified at input returning *
 *		non-zero if the file opened correctly   *
\********************************************************/
int openfile(WINS *win)
{
    char *ch;
    FILE *tmpfp;                                        /* temp pointer       */

    wmove(win->hex_outline, LINES-1, 21);               /* output prompt      */
    wclrtoeol(win->hex_outline);
    mvwaddstr(win->hex_outline, LINES - 1, 1, "Enter file to open: ");

    freeList(head);										/* free linked list   */
	head = NULL;
	if (!fpINfilename)
        fpINfilename = (char *) malloc(81);             /* allocate if NULL   */

    ch = inputLine(win->hex_outline, LINES - 1, 21);	/* get filename       */

    if (ch[0] == 27)									/* if escape was hit  */
    {
		restoreBorder(win);								/* restore border     */
		wrefresh(win->hex_outline);                    	/* refresh window     */
		free(ch);
		return FALSE; 
    }

    if (ch[0] != '\0') {
	free(fpINfilename);
	fpINfilename = ch;
    }

    if (!(tmpfp = fopen(fpINfilename, "r")))            /* if cannot open...  */
    {
        wmove(win->hex_outline, LINES-1, 1);
        wclrtoeol(win->hex_outline);
														/* output message    */
        /*mvwaddstr(win->hex_outline, LINES - 1, 1, "Could not open file");*/
        popupWin("Could not open file!", -1);
        restoreBorder(win);
        wnoutrefresh(win->hex_outline);					/* refresh window     */

        return FALSE;                                   /* return bad         */
    }

    restoreBorder(win);									/* restore border     */

    wrefresh(win->hex_outline);                         /* refresh window     */
    fclose(tmpfp);                                      /* close temp pointer */

    fpIN = fopen(fpINfilename, "r");                    /* open file: read    */

    wclear(win->hex);                                   /* clear windows      */
    wclear(win->ascii);
    wclear(win->address);

    refreshall(win);                                    /* refresh everything */
    doupdate();                                         /* update screen      */
    return TRUE;                                        /* return good        */
}

/********************************************************\
 * Description: saves file specified from input, return *
 *		non-zero if the file is saved correctly *
\********************************************************/
int savefile(WINS *win)
{
    char *ch;                                           /* temp string        */
    int exitCode = 0;

    wmove(win->hex_outline, LINES-1, 20);               /* clear line and     */
    wclrtoeol(win->hex_outline);                        /* output prompt      */
    mvwprintw(win->hex_outline, LINES-1, 1, "Enter file to save: %s",
             (fpOUTfilename != NULL && strcmp(fpOUTfilename, "")) ? fpOUTfilename:fpINfilename);

    wrefresh(win->hex_outline);                         /* refresh window     */

    ch = inputLine(win->hex_outline, LINES - 1, 21);	/* get filename       */

    if (ch[0] != 27)						/*if escape wasn't hit*/
    {
	if (ch[0] != '\0')
	{
		free(fpOUTfilename);
		fpOUTfilename = ch;		/* copy into fileout  */
	}
	else
	{
		free(ch);
	}
	
	/* if infile is equal to outfile...   */
	if (fpOUTfilename && strcmp(fpOUTfilename, fpINfilename) == 0)
	{
		free(fpOUTfilename);
		fpOUTfilename = NULL;
	}
	
	/*write to file       */
	if (!writeChanges())
	    popupWin("The file has been saved.", -1);
	else
	    exitCode = 1;
    }
    else
    {
	free(ch);
	exitCode = 2;
    }

    restoreBorder(win);
    wnoutrefresh(win->hex_outline);
    return exitCode;
}

/********************************************************\
 * Description: searches for a series of either hex or  *
 *		ascii values of upto 16 bytes long.  The*
 *		function returns the next location of   *
 *		the specified string or -1 if not found *
\********************************************************/
off_t hexSearch(FILE *fp, int ch[], off_t startfp, int length)
{
    int loop, tmp, c, flag=0;
    off_t currLoc, startLoc, foundLoc=-1;		/* init vars          */

    fseeko(fp, startfp, SEEK_SET);			/* begin from loc     */

    c = getc(fp);					/* get char from file */
    currLoc = ftello(fp);				/* get location       */
    if ((tmp = searchList(head, currLoc-1)) != -1)	/* check for val in ll*/
	c = tmp;

    while (currLoc != startfp) 				/* while not back to  */
    {							/* beginning...       */
	if (c == ch[0]) 				/* if char we want... */
	{						
	    startLoc = ftello(fp);			/* get location       */
							/* loop to find rest  */
	    for (loop = 1; (loop < length) && (c != EOF); loop++)
	    {
		c = fgetc(fp); 
    		currLoc = ftello(fp);
    		if ((tmp = searchList(head, currLoc-1)) != -1)
		    c = tmp;
		if (c != ch[loop])
		    break;
		    
	    }
		fseeko(fp, startLoc, SEEK_SET);

	    if ((loop == length) && (flag))		/* if found it        */
		return startLoc - 1;			/* return location    */
	    else if (loop == length)
		foundLoc = startLoc -1;			/* return if no others*/
		    	
	}
	flag = 1;    /* if cursor is on the search string, continue searching */
		

	if (c == EOF)					/* if EOF rewind      */
	    rewind(fp);

        currLoc = ftello(fp);				/* current location   */

	if (currLoc != startfp) 			/* if not at start... */
	{
	    c = fgetc(fp);				/* get another char   */
    	    currLoc = ftello(fp);
    	    if ((tmp = searchList(head, currLoc-1)) != -1)
	        c = tmp;
	}
    } 

    return foundLoc;					/* return not found   */
}

/********************************************************\
 * Description: goes to a certain location in the file  *
 * Returns:     currentLine (in file)                   *
\********************************************************/
off_t gotoLine(FILE *fp, off_t currLoc, off_t gotoLoc, off_t maxlines, WINDOW *win)
{
    off_t count, currentLine, tmp, linediff;
    int row, col;

    /* col is not used, but needed for the getyx macro, this line is to avoid compiler warnings */
    getyx(win, row, col);
    UNUSED(col);

    restoreBorder(windows);				/* restore border     */

    wrefresh(windows->hex_outline);
    currentLine = maxLines(gotoLoc);

    if ((gotoLoc % BASE) == 0)
        currentLine++;

    /* we must calculate the lines between the currloc and gotoLoc.       *\
     * we do this by calculating the line number for the value of currloc *
     * if it was on the first row and substract it from the line number   *
     * of gotoLoc.  If it's less then MAXY then it's on the the that      *
     * means the destination is visible on the screen and that the cursor *
    \* should move to the spot rather then scroll to the position         */

    if (gotoLoc >= currLoc) /* the destination is below the current loc   */
    {
        tmp = currLoc - (row * BASE);
        linediff=currentLine-((tmp%BASE) ? maxLines(tmp) : maxLines(++tmp));
    }
    else /*the dest is above the current location, must handle differently*/
    {
        tmp = currLoc + ((MAXY - row) * BASE);
        linediff = currentLine
                 - ((tmp%BASE) ? maxLines(tmp) : maxLines(++tmp));
        if (linediff >= (MAXY*-1) && linediff < 1) linediff += MAXY;
    }

    if (linediff < 1) linediff *= -1;                   /* take abs value     */

    if (linediff <= MAXY) /* if the dest on the same screen as currloc   */
    {
        /* now move position w/o scrolling dest line to the top of screen */
        wmove((editHex) ? windows->hex : windows->ascii, linediff , (editHex) ?
             ((gotoLoc % BASE) * 3) : (gotoLoc % BASE));
    }
    else
    {

        wclear(windows->hex);                           /* clear windows      */
        wclear(windows->ascii);
        wclear(windows->address);

        wmove(windows->hex, 0, 0);                      /*position cursors    */
        wmove(windows->ascii, 0, 0);
        wmove(windows->address, 0, 0);
                                                        /* output lines       */
        for (count = 0; count <= MAXY && (count + currentLine)<=maxlines;
            count++)
            outline(fp, count+currentLine);

        wmove((editHex) ? windows->hex : windows->ascii, 0, (editHex) ?
             ((gotoLoc % BASE) * 3) : (gotoLoc % BASE));
                                                        /* restore cursor     */
    }
#ifdef DEBUG_GOTO
        mvwprintw(windows->hex_outline,0,20,"                      ");
        mvwprintw(windows->hex_outline,0,10,"value:%d    location:%d %d %d %d",
        maxLines(tmp), maxLines(gotoLoc), linediff, gotoLoc, MAXY);
#endif

    wnoutrefresh(windows->scrollbar);                   /* refresh screen     */
    wnoutrefresh(windows->ascii);
    wnoutrefresh(windows->address);
    wnoutrefresh(windows->hex);
    doupdate();                                         /* update screen      */

    return currentLine;
}

/********************************************************\
 * Description: gets values at given location in file	*
 * Returns:     the value at the given location	 	*
\********************************************************/
int getLocVal(off_t loc)
{
    hexList *tmpHead = head;				/* tmp linklist head  */

    while (tmpHead != NULL && (tmpHead->loc < loc))
	tmpHead = tmpHead->next;
    if (tmpHead != NULL && (tmpHead->loc == loc))
	return(tmpHead->val);

    fseeko(fpIN, loc, SEEK_SET);				/* goto location      */
    return(fgetc(fpIN));
}

/********************************************************\
 * Description: checks if a loc is in the linked list   *
 * Returns:     returns TRUE if loc is in the llist     *
\********************************************************/
bool inHexList(off_t loc)
{
    hexList *tmpHead = head;				/* tmp linklist head  */

    while (tmpHead != NULL && (tmpHead->loc < loc))
	tmpHead = tmpHead->next;
    if (tmpHead != NULL && (tmpHead->loc == loc))
	return(TRUE);
    else
	return(FALSE);
}
