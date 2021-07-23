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
    printf("\nusage: hexcurse [-?|help] [-a] [-r rnum] [-o outputfile] "); 
    printf("[[-i] infile]\n\n");
    printf("    -a\t\tOutput addresses in decimal format initially\n");
    printf("    -e\t\tOutput characters in EBCDIC format rather than ASCII\n"); 
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

void make_delta1(int *delta1, int *pat, size_t patlen) {
    size_t i;
    for (i=0; i < ALPHABET_LEN; i++) {
        delta1[i] = patlen;
    }
    for (i=0; i < patlen-1; i++) {
        delta1[pat[i]] = patlen-1 - i;
    }
}

// true if the suffix of word starting from word[pos] is a prefix
// of word
int is_prefix(int *word, size_t wordlen, size_t pos) {
    size_t i;
    size_t suffixlen = wordlen - pos;

    for (i = 0; i < suffixlen; i++) {
        if (word[i] != word[pos+i]) {
            return 0;
        }
    }
    return 1;
}

// length of the longest suffix of word ending on word[pos].
// suffix_length("dddbcabc", 8, 4) = 2
size_t suffix_length(int *word, size_t wordlen, size_t pos) {
    size_t i;
    // increment suffix length i to the first mismatch or beginning
    // of the word
    for (i = 0; (word[pos-i] == word[wordlen-1-i]) && (i < pos); i++);
    return i;
}

void make_delta2(int *delta2, int *pat, size_t patlen) {
    size_t  p;
    size_t  last_prefix_index = patlen-1;

    // first loop
    for (p=patlen-1; ;p--) {
        if (is_prefix(pat, patlen, p+1)) {
            last_prefix_index = p+1;
        }
        delta2[p] = (int) (last_prefix_index + (patlen-1 - p));
        if (p == 0) break;
    }

    // second loop
    for (p=0; p < patlen-1; p++) {
        int slen = suffix_length(pat, patlen, p);
        if (pat[p - slen] != pat[patlen-1 - slen]) {
            delta2[patlen-1 - slen] = (int) (patlen-1 - p + slen);
        }
    }
}

/********************************************************\
 * Description: searches for a series of either hex or  *
 *		ascii values of upto 16 bytes long.  The*
 *		function returns the next location of   *
 *		the specified string or -1 if not found *
 *      algorithm based on the wikipedia example *
 * http://en.wikipedia.org/wiki/Boyer%E2%80%93Moore_string_search_algorithm *
\********************************************************/
off_t hexSearchBM(WINDOW *w, FILE *fp, int pat[], off_t startfp, int patlen)
{
    if (! (pat && patlen > 0)) return -2;

    int         i, m = 1;
    int         j = patlen - 1;
    int         delta1 [ ALPHABET_LEN ];
    int         *delta2 = (int *)malloc(patlen * sizeof(int));

    char        *buf;           // circular buffer
    char        *patt = (char *) malloc(patlen);

    int         n;              // number of bytes read by fread()
    int         rem_bytes = 0;  // remaining bytes in the buffer
    int         bytes_to_read = BUF_L;
    int         full_length;    // full length of the current buffer
    int         cur_percent, last_percent = -1;

    off_t       pos1, pos2;     // lower and upper limit of the buffer
    off_t       pos_max = -1;   // EOF position
    off_t       rv = -1;        // return value: default = -1

    if (posix_memalign((void **)&buf, getpagesize(), BUF_L) != 0)
        return -1;

    if (! (delta2 && patt)) return -1;

    make_delta1(delta1, pat, patlen);
    make_delta2(delta2, pat, patlen);
    // converting int to (unsigned char) -> (unsigned char) is faster
    for (i = 0; i < patlen; i++) patt[i] = (unsigned char) pat[i];

    // get the pos_max
    if (fseeko(fp, 0, SEEK_END) == 0) pos_max = ftello(fp);

    // we ignore the current byte
    startfp++;
    if (fseeko(fp, startfp, SEEK_SET) != 0) {
        goto end;
    }

    // set to non-blocking
    wtimeout(w, 0);
    pos1 = pos2 = startfp;

    while (1) {
        n = (int) fread(&buf[rem_bytes], 1, bytes_to_read, fp);
        full_length = n + rem_bytes;
        if (n == 0 || full_length < patlen) break;
        pos2 = pos1 + (off_t) full_length;

        // apply changes by user, if any
        updateBuf(head, buf, pos1, pos2);

        i = patlen - 1;
        while (i < full_length) {
            j = patlen - 1;
            while (j >= 0 && (buf[i] == patt[j])) {
                --i;
                --j;
            }
            if (j < 0) {
                // success
                rv = pos1 + i + 1;
                goto end;
            }

            m = max(delta1[(unsigned char) buf[i]], delta2[j]);
            i += m;
        }

        rem_bytes = full_length + m - i - 1;
        if (rem_bytes > full_length) rem_bytes = full_length;

        bytes_to_read = BUF_L - rem_bytes;
        memmove(buf, &buf[full_length - rem_bytes], rem_bytes);
        pos1 = pos2 - rem_bytes;

        if (wgetch(w) == 27) { // escape
            rv = -2;
            goto end;
        }

        if (pos_max > 0) {
            cur_percent = (int) ((pos2 * 100) / pos_max);
            if (cur_percent != last_percent) {
                mvwprintw(w, LINES - 1, 1, "Searching (hit Esc to cancel) ...%d%%", cur_percent);
                wrefresh(w);
                last_percent = cur_percent;
            }
        }
    }

    end:
        // set back to blocking
        wtimeout(w, -1);
        free(buf);
        free(patt);
        free(delta2);

    return rv;
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
