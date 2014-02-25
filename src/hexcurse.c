/******************************************************************************\
 *  Copyright (C) 2001 writen by Jewfish and Armoth                           *
 *									      *
 *  Description: this codes allows a user to view and edit the hexadecimal and*
 *		 and ascii values of a file.  The curses library is used to   *
 *		 display and manipulate the output.  See the README file      *
 *		 included for more information.				      *
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

#include "hex.h"					/* custom header      */

/*#define DEBUG_LLIST*/
/*#define DEBUG_GOTO*/

int     BASE, MAXY, resize = 0;
int     MIN_ADDR_LENGTH;
hexList *head;						/* linked list struct */
WINS    *windows;					/* window structure   */
char    EBCDIC[256],
	*fpINfilename = NULL,
        *fpOUTfilename = NULL;
bool 	printHex;					/* address format     */
bool    USE_EBCDIC;
bool    IN_HELP;					/* if help displayed  */
int     hex_win_width,
        ascii_win_width,
        hex_outline_width,
        ascii_outline_width;


    /* partial EBCDIC table contributed by Ted (ted@php.net) */
    char EBCDIC[] = {
    /* 0   1   2   3   4   5   6   7   8   9   A   B   C   D    E   F */
      '.','.','.','.','.','.','.','.','.','.','.','.','.','.' ,'.','.', /* 0 */
      '.','.','.','.','.','.','.','.','.','.','.','.','.','.' ,'.','.', /* 1 */
      '.','.','.','.','.','.','.','.','.','.','.','.','.','.' ,'.','.', /* 2 */
      '.','.','.','.','.','.','.','.','.','.','.','.','.','.' ,'.','.', /* 3 */
      ' ','.','.','.','.','.','.','.','.','.','.','.','<','(' ,'+','|', /* 4 */
      '&','.','.','.','.','.','.','.','.','.','!','$','*',')' ,';','.', /* 5 */
      '-','/','.','.','.','.','.','.','.','.','.',',','%','_' ,'>','?', /* 6 */
      '.','.','.','.','.','.','.','.','.','.',':','#','@','\'','=','"', /* 7 */
      '.','a','b','c','d','e','f','g','h','i','.','.','.','.' ,'.','.', /* 8 */
      '.','.','j','k','l','m','n','o','p','q','.','.','.','.' ,'.','.', /* 9 */
      '.','r','s','t','u','v','w','x','y','z','.','.','.','.' ,'.','.', /* A */
      '.','.','.','.','.','.','.','.','.','`','.','.','.','.' ,'.','.', /* B */
      '.','A','B','C','D','E','F','G','H','I','.','.','.','.' ,'.','.', /* C */
      '.','.','J','K','L','M','N','O','P','Q','.','.','.','.' ,'.','.', /* D */
      '.','R','S','T','U','V','W','X','Y','Z','.','.','.','.' ,'.','.', /* E */
      '0','1','2','3','4','5','6','7','8','9','.','.','.','.' ,'.','.'};/* F */

int main(int argc, char *argv[])			/* main program       */
{

    int   x, retval = 1, val;			/* counters, etc.     */
    off_t len;					/* len need to be off_t*/

    windows = (WINS *) calloc(1, sizeof(WINS));	/* malloc windows     */
    head = llalloc();							/* malloc list space  */
    fpINfilename = (char *) malloc(FN_LEN+1);	/* allocate in and    */
    fpOUTfilename = (char *) malloc(FN_LEN+1);	/* out file name ptrs */
    printHex = TRUE;							/* address format     */
    USE_EBCDIC = FALSE;							/*use ascii by default*/

							/* get cmd line args  */
    len = parseArgs(argc, argv, fpINfilename, fpOUTfilename);
    MIN_ADDR_LENGTH = getMinimumAddressLength(len);

    use_env(TRUE);					/* use env values     */
    slk_init(0);					/* init menu bar      */
    init_screen();					/* init visuals       */

    if ((COLS < MIN_COLS) || (LINES < MIN_LINES))	/* screen's too small */
    {
	endwin();
	fprintf(stderr,"\n\nThe screen size too small.\nThe minimum allowable");
	fprintf(stderr," screen size is %dx%d\n\n", MIN_COLS, MIN_LINES + 1);
	exit(-1);
    }
    
    slk_set(6, (printHex) ? "Hex Addr":"Dec Addr", 1);
    init_fkeys();					/* define menu bar    */
    

    while (retval)
    {
	free_windows(windows);
        
	/* calculate screen   */
	BASE                = (resize > 0 && resize < COLS) ? resize:((COLS-6-MIN_ADDR_LENGTH)/4);
	MAXY                = (LINES) - 3;
	hex_win_width       = BASE * 3;
	ascii_win_width     = BASE;
	hex_outline_width   = (BASE * 3) + 3 + MIN_ADDR_LENGTH;
	ascii_outline_width = BASE + 2;
            
	init_menu(windows);				/* init windows       */
	head = freeList(head);				/* free & init head   */
							/* print origin loc   */
	mvwprintw(windows->hex_outline, 0, 1, "%0*d", MIN_ADDR_LENGTH, 0);
    
	if (fpIN != NULL)				/* if no infile...    */
	{
            len = maxLoc(fpIN);				/* get last file loc  */
	    val = maxLines(len); 			/* max file lines     */
            for (x = 0; x <= MAXY && x<=val; x++)       /* output lines       */
		outline(fpIN, x);
	}

	wmove(windows->hex, 0, 0);			/* cursor to origin   */
    
	refreshall(windows);				/* refresh all wins   */
	doupdate();					/* update screen      */
        
	mvwaddch(windows->scrollbar, 1, 0, ACS_CKBOARD);/* clear scroller     */
							/* get user input     */
	retval = wacceptch(windows, len, fpINfilename, fpOUTfilename); 
    }
    
    screen_exit(0);					/* end visualizations */
    return retval;					/* return             */
}

/********************************************************\
 * Description: prints out debug info to a file         *
 * Returns:     nothing                                 *
\********************************************************/
/*
void printDebug(hexList *head, long int loc)
{
    FILE *tmpofp;
    hexList *tmpHead = head;

    tmpofp = fopen("debug_llist", "a+");
    tmpHead = head;

    fprintf(tmpofp, "location undone: %08X\n", loc);
    while (tmpHead != NULL)
    {
	fprintf(tmpofp, "head->loc: %08X   head->val: %02X (%c)\n", tmpHead->loc, tmpHead->val, tmpHead->val);

	tmpHead = tmpHead->next;
    }
    fprintf(tmpofp, "\n");

    fclose(tmpofp);
}
*/

/********************************************************\
 * Description: parses command line arguments and 	*
 *		processes them.				*
 * Returns:	length of file				*
\********************************************************/
off_t parseArgs(int argc, char *argv[], char *fpINfilename, char *fpOUTfilename)
{
    extern char *optarg;				/* extern vars for    */
    extern int optind, /*opterr,*/ optopt;		/* getopt()	      */

    int val;						/* counters, etc.     */

							/* get args           */
    while ((val = hgetopt(argc, argv, "a:i:o:r:e")) != -1) 
    {
	switch (val)					/* test args          */
        {
            case 'a':	printHex = FALSE;		/* decimal addresses  */
                        break;
							/* infile             */
	    case 'i':	strncpy(fpINfilename, optarg, FN_LEN);
			break;
							/* outfile            */
	    case 'o':   strncpy(fpOUTfilename, optarg, FN_LEN);
			break;

            case 'r':   resize = atoi(optarg);		/* don't resize screen*/
                        break;

            case 'e':   USE_EBCDIC=TRUE;		/*use instead of ascii*/
                        break;
							/* help/invalid args  */
							/* help/invalid args  */
	    case '?':	print_usage();			/* output help        */
                        if ((optopt == 'h') || (optopt == '?'))
			    exit(0);			/* exit               */
			else				/* illegal option     */
			    exit(-1);
        }
    }
    argc -= optind;
    argv += optind;

    if (argv[0])
        strncpy(fpINfilename, argv[0], FN_LEN);

    if (strcmp(fpINfilename, ""))
        if ((fpIN = fopen(fpINfilename, "r")) == NULL)
            exit_err("Could not open file");

    return ((fpIN != NULL) ? maxLoc(fpIN):0);		/* return file length */
}

/********************************************************\
 * Description: Get the minimum address length for the  *
 *              address column                          *
 * Returns:	minimum length for addresses		*
\********************************************************/
int getMinimumAddressLength(off_t len)
{
        char buffer[1];
        int min_address_length;
        
        min_address_length = snprintf(buffer, 1, "%jd", (intmax_t)len);
        
        /* At least 8 characters wide */
        return min_address_length > 8 ? min_address_length : 8;
}

/********************************************************\
 * Description: in the event of a segmentation fault    *
 * 		this catches the signal and prints out  *
 *		instructions on where to send a bug     *
 * 		report.					*
 * Returns:	length of file				*
\********************************************************/
void catchSegfault(int sig)
{
    /* Avoid unused variable warning */
    UNUSED(sig);
    
    endwin();
    printf("\n\nHexcurse has encountered a segmentation fault!\n");
    printf("\tPlease submit a full bug report to devel@jewfish.net.\n");
    printf("\tInclude what you did to cause the segfault, and if possible\n");
    printf("\tinclude the core dump.  And for your troubles, we'll add you \n");
    printf("\tto the Changelog. Then you can brag to your friends about it!\n");

    exit(-1);
}
