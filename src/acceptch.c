/******************************************************************************\
 *  Copyright (C) 2001, hexcurse is  writen by Jewfish and Armoth             *
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

/********************************************************\
 * Description: accepts a character from the user and	*
 *		interprets it.				*
 * Returns:	retval (if quit program or no)		*
\********************************************************/
off_t maxlines;						/*extern val for lines*/
off_t currentLine;					/*extern val for lines*/
off_t LastLoc;                                          /*last cursor location*/
bool editHex;						/* flag to edit h or a*/
int SIZE_CH;                                            /* global winch flag  */

int wacceptch(WINS *win, off_t len)
{
    intmax_t tmp_max;
    
    off_t count;
    int  col = 0, val, tmpval, 	    			/* counters, etc.     */   
         ch[17],					/* holds search string*/
	 eol = (BASE * 3) - 1,				/* end of line pos    */
	 lastRow = 0, lastCol = 0,			/* last row/col coords*/
	 curVal = 0,	        			/* vals @ cursor locs */
	 tmp = 0;

    off_t cl,						/* current loc in file*/
	  gotoLoc = 0,					/* goto location      */
	  lastLine = 0,					/* line b4 LastLine   */
	  currentLine = 0,				/* current line value */
	  row = 0;

    char *gotoLocStr,					/* convert to gotoLoc */
         *temp,
    	 *tmpstr,					/* tmp str 4 inputLine*/
	 SearchStr[13];

    short int key;					/* key capture        */
    WINDOW *Winds;					/* window pointer     */
    hexStack *stack;					/* used for stack     */
    hexStack *tmpStack;					/* used for stack     */
    editHex = TRUE;					/* val for editing    */

    SIZE_CH	= FALSE;				/* set winch to false */
    Winds       = win->hex;				/* curr edit window   */
    maxlines    = maxLines(len);			/* lines in file      */
    
    bool shouldExit = false;

    /*createStack(stack);*/				/* init the stack     */
    stack = NULL;
    temp = (char *)calloc(81, sizeof(char));

    if (fpIN)						/* if file opened then*/
    {							/* highlight 0,0 loc  */
        curVal = getLocVal(0);
	wattron(win->ascii, A_UNDERLINE);
	if (USE_EBCDIC)
		mvwprintw(windows->ascii, 0, 0, "%c", EBCDIC[curVal]);/* print EBCDIC char */
	else	/* print ASCII  char */
		mvwprintw(windows->ascii, 0, 0, (isprint(curVal)) ? "%c":".", (char)curVal);
	wmove(win->hex, 0, 0);
	wattrset(win->ascii, A_NORMAL);
    	wnoutrefresh(win->ascii);
	doupdate();
    }
        
							/* get keys til exit  */
    while(!shouldExit)
    //while (!(save=quitProgram(isEmptyStack(stack),(key = wgetch(Winds)))))
    {
        key = wgetch(Winds);
	lastRow = row;
	lastCol = col;
	getyx(Winds, row, col);				/* curent cursor loc  */

	if (SIZE_CH)					/* if win size changed*/
	{
	    cl=row=col  = 0;
	    eol         = (BASE * 3) - 1;
	    editHex     = TRUE;
	    Winds       = win->hex;
	    /*cl = LastLoc;*/
	    currentLine = 0;
	    /*maxlines = maxLines(len);*/
	    SIZE_CH = FALSE;
	}

	if ((COLS < MIN_COLS) || (LINES < MIN_LINES))	/* screen too small so*/
	    continue;					/* ignore user input  */

	/* if file isn't opened only allow user input from the following keys */
	if (!fpIN && (key != CTRL_AND('o')) && (key != KEY_F(3)) &&
	   (key != CTRL_AND('h')) && (key != KEY_F(1)))
	    continue;
	

	switch (key) {					/* check keypress     */
    
							/* if exit key...     */
	case CTRL_AND('q'):
	case CTRL_AND('x'):
	case KEY_F(8):
		if (isEmptyStack(stack))
		{
			/* No pending changes */
			shouldExit = true;
		}
		else
		{
			short int str = questionWin("Do you want to save changes? (y/n/c)");
			if (str == 'Y' || str == 'y')
			{
				if (savefile(win) == 0)
					shouldExit = true;
			}
			else if (str == 'N' || str == 'n')
			{
				shouldExit = true;
			}
			else if (str == 'C' || str == 'c')
			{
				continue;
			}
		}
		break;

	case KEY_UP:					/* if UP...           */
		if (currentLine > 0) 			/* move up...         */
		{   
		    currentLine--;
		    wmove(Winds, --row, col);
		}

		if ((currentLine >= 0) && (row < 0))	/* scroll up...       */
		    winscroll(win, Winds, -1, currentLine);
		break;

	case KEY_DOWN:					/* if DOWN...         */
		if (cursorLoc(currentLine + 1, col, editHex,BASE) < len) 
		{   
		    if (currentLine < maxlines)		/* move down...       */
		    {
		        wmove(Winds, ++row, col);
		        currentLine++;
		    }
							/* scroll down...     */
		    if((row > MAXY) && (currentLine <= maxlines))
		        winscroll(win, Winds, 1, currentLine);
		}
		break;

	case KEY_BACKSPACE:
	case KEY_LEFT:					/* if LEFT or BACK... */
		if ((col == 0) && (currentLine != 0))   /* move up...         */
		{   
	  	    currentLine--;
		    if (row == 0)			/* scroll up...       */
		    {
			winscroll(win, Winds, -1, currentLine);
			row++;
		    }
		    wmove(Winds, --row, eol-1);
		}

		wmove(Winds, row, --col);		/* move left...       */
		if (editHex)
		{
		    if ((col + 1) % 3 == 0)
		        wmove(Winds, row, --col);
		}
		break;

	default:					/* if other key...    */
							/* if key we want...  */
		if (isprint(key) && ((editHex && isxdigit(key)) || !editHex))
		{
		    if ((cl=cursorLoc(currentLine, col, editHex,BASE))< len) 
		    {   
		
							/* if not in ll...    */
			if ((val = searchList(head, cl)) == -1)
		  	{
			    fseeko(fpIN, cl, SEEK_SET);	/* get val from file  */
			    val = fgetc(fpIN);
			}

			wattron(win->hex, A_BOLD);
			wattron(win->ascii, A_BOLD);
							/* output it          */
			wprintw(Winds, "%c", editHex ? toupper(key): key);

			tmpval = val;			/* val b4 key press   */

			if (editHex)			/* if in hex win...   */
			{
			    if (key >= 65 && key <= 70)	/* get correct val    */
			    	key -= 7;
			    else if (key >= 97 && key <= 102)
			    	key -= 39;
			    key -= 48;
			
			    if ((col % 3) == 0)		/* compute byte val   */
                            val = (key * 16) + (val % 16);
			    else if ((col % 3) == 1)
			    	val = (val - ((val + 16) % 16) + key);
			}
			else				/* else...            */
			    val = key;			/* val is key pressed */

			if (editHex)			/* update ascii win   */
			{
			    wmove(win->ascii, row, (col/3));	
			    wprintw(win->ascii, "%c", (USE_EBCDIC) ?EBCDIC[val]:
			           (isprint(val) ? val : 46));
			    wmove(win->hex, row, col);
			    wrefresh(win->ascii);
			}
			else				/* update hex win     */
			{
			    wmove(win->hex, row, (col*3));	
			    wprintw(win->hex, "%02X", val);
			    wmove(win->ascii, row, col);
			    wrefresh(win->hex);
			}

			wattrset(win->hex, A_NORMAL);
			wattrset(win->ascii, A_NORMAL);
			
							/* edit list          */
							/* first add the original value for save+undo */
			if (searchList(head, cl) == -1) head = insertItem(head, cl, tmpval);
							/* then add the current value */
			head = insertItem(head, cl, val);
		/* calloc() is used because it NULLS out all returned memory  */
    			tmpStack = (hexStack *) calloc(1, sizeof(hexStack));
			tmpStack->currentLoc = cl;
			tmpStack->llist	     = head;
			tmpStack->savedVal   = tmpval;
			tmpStack->prev       = NULL;
			pushStack(&stack, tmpStack);
		    }					/* continue to next
							    case              */

	case KEY_RIGHT:					/* if RIGHT...        */
		    if (cursorLoc(currentLine, col, editHex, BASE) < len) 
		    {   
			wmove(Winds, row, ++col);	/* move right         */
		        if (cursorLoc(currentLine, col, editHex,BASE) == len)
			    wmove(Winds, row, --col);
							/* move down          */
		        if ((col == eol) && (currentLine < maxlines))
		        {
		            currentLine++;
		            if (row < MAXY) {
                                col = 0;
	  	                wmove(Winds, ++row, col);
                            }
	                    else			/* scroll down        */
	                    {
			        winscroll(win, Winds, 1, currentLine);
	 		        wmove(Winds, row, 0);
	                    }
		        }
		        if (editHex)			/* adjust for hex win */
		            if ((col + 1) % 3 == 0)
		                wmove(Winds, row, ++col);
		    }

							/* if end of file...  */
							/* adjust properly    */
		    if (cursorLoc(currentLine, col, editHex,BASE) == len)
			wmove(Winds, row, (col = col - 2));
		}
		break;

	case CTRL_AND('u'):
	case KEY_PGUP:					/* if KEY_PGUP...     */
		if (currentLine == row) {		/* if first page      */
		    currentLine = 0;			/* just move to top   */
		    wmove(Winds,0,0);
		    break;
		}
		else
		    currentLine -= (2*MAXY);
                /* fall through */

	case CTRL_AND('d'):
	case KEY_PGDN:					/* if KEY_PGDN...     */
		getyx(Winds, row, col);		/* current location   */
                                                        /* if EOF < page away */
		if ((maxlines - currentLine) <= MAXY)
                {                                       /* inc line til end   */
		    while (cursorLoc(currentLine + 1, col, editHex,BASE) < len)
                    {
		        if (currentLine < maxlines)	/* move down...       */
                        {
		            row++;
		            currentLine++;
		        }
							/* scroll down...     */
	                if ((row > MAXY) && (currentLine <= maxlines))
                        {
	                    winscroll(win, Winds, 1, currentLine);
			    row = MAXY;			/* last row/col       */
		            col = (editHex) ? 3*((len % BASE)-1):(len % BASE)-1;
                            if (col < 0)
		                col = (editHex) ? 3 * (BASE - 1):BASE - 1;
			}
		    }
							/* move to EOF        */
		    if ((len - cursorLoc(currentLine, col,editHex,BASE) <= BASE)
                        && (currentLine < maxlines) && (row >= MAXY))
		        winscroll(win, Winds, 1, ++currentLine);

		    row += (maxlines - currentLine);	/* adj to last line   */
		    currentLine = maxlines;
							/* last column        */
		    col = (editHex) ? 3 * ((len % BASE) - 1):(len % BASE) - 1;
                    if (col < 0)
		        col = (editHex) ? 3 * (BASE - 1) : BASE - 1;

		    wmove(Winds,row,col);		/* move cursor        */

 		    break;
		}
		
		currentLine += MAXY;

		if (currentLine > maxlines)		/* adjust currentLine */
		    currentLine = maxlines;
		else if (currentLine < 0)
		    currentLine = 0;

		val = currentLine - row;		/* get val            */
		if (val < 0) 
		{
		    row = val = 0;
                    if (cursorLoc(currentLine, col, editHex,BASE) != 
			cursorLoc(row, col, editHex,BASE))
                        row = currentLine;
                }

		wmove(win->hex, 0, 0);			/* position cursors   */
		wmove(win->ascii, 0, 0);
		wmove(win->address, 0, 0);
							/* output lines       */
		for(count = 0; count <= MAXY && (count + val) <= maxlines;
		    count++)
		    outline(fpIN, (count + val));

            	if (count < MAXY)
                    for (; count <= MAXY; count++) {
                        wmove(win->address, count, 0);
                        wclrtoeol(win->address);
                        wmove(win->hex, count, 0);
                        wclrtoeol(win->hex);
                        wmove(win->ascii, count, 0);
                        wclrtoeol(win->ascii);
                    }
		
		wmove(Winds, row, col);			/* restore cursor     */
    		wnoutrefresh(win->ascii);
    		wnoutrefresh(win->address);
    		wnoutrefresh(win->hex);
		break;

	case CTRL_AND('t'):
	case KEY_HOME:					/* goto head of file  */
		currentLine = cl = row = col = 0;	/*set vals to 1st line*/
		wmove(win->hex, 0, 0);			/* position cursors   */
		wmove(win->ascii, 0, 0);
		wmove(win->address, 0, 0);

		for (count = 0; count <= MAXY && count <= 
		     maxLines(maxLoc(fpIN)); count++)
		    outline(fpIN, count);

		mvwprintw(windows->cur_address, 0, 0, "%0*d", MIN_ADDR_LENGTH, 0); 
		wmove((editHex) ? win->hex : win->ascii, 0, 0);
    		wnoutrefresh(win->ascii);
    		wnoutrefresh(win->address);
    		wnoutrefresh(win->hex);
		break;
		
	case CTRL_AND('b'):
	case KEY_END:					/* goto end of file   */
		if (cursorLoc(currentLine, col, editHex, BASE)==maxLoc(fpIN)-1)
		    break;				/* alread at oef      */
		
		/* if there's more than 1 screen, move to the last screenfull */
		if ((maxlines - currentLine) >= MAXY)	/*if more than 1 scrn */
		    currentLine = gotoLine(fpIN, 
			  	  cursorLoc(currentLine, col, editHex, BASE),
                                  maxLoc(fpIN)-(BASE*((len%BASE)?MAXY:MAXY+1)),
				  maxlines, Winds);

		getyx(Winds, row, col);			/* move to EOF        */
		currentLine = gotoLine(fpIN, 
				cursorLoc(currentLine, col, editHex, BASE),
				maxLoc(fpIN)-1, maxlines, Winds);

    		wnoutrefresh(win->ascii);
    		wnoutrefresh(win->address);
    		wnoutrefresh(win->hex);
	   	break;

	case CTRL_AND('o'):
	case KEY_F(3):					/* if F3 or ^o...     */


		if (openfile(win))			/* open file          */
		{
		    if (fpIN)
                    {
			MIN_ADDR_LENGTH = getMinimumAddressLength(maxLoc(fpIN));
			curVal = getLocVal(0);
                    }                
	 	    return TRUE;			/* TRUE if worked     */
		}
		break;

	case CTRL_AND('s'):				/* if F2 or ^s...     */
	case KEY_F(2):					/* save the file      */
		savefile(win);
		break;

	case CTRL_AND('f'):
	case KEY_F(5):
 		/* SeachStr stores the last searched string into the format *\
	 	\* "(XXXXXXX...)" with 10 being the max chars shown         */
                if (!fpINfilename || !strcmp(fpINfilename, "")) 
		{	 				/* output prompt      */
		    wmove(win->hex_outline, LINES-1, 1);
                    wclrtoeol(win->hex_outline);
		    popupWin("No file loaded!", -1);
                    restoreBorder(win);			/* restore border     */
                    wrefresh(win->hex_outline);
                    break;
                }
 

		if (temp != NULL)
		{
		    bzero(SearchStr, 13);
		    strcat(SearchStr, "(");
		    if (strlen(temp) <= 10)
			strncat(SearchStr, temp, strlen(temp));
		    else
		    {
			strncat(SearchStr, temp, 7);
			strcat(SearchStr, "...");
		    }
		    strcat(SearchStr, ")");
		}

		wmove(win->hex_outline, LINES - 1, 19); /* output prompt      */
    		wclrtoeol(win->hex_outline);
    		mvwprintw(win->hex_outline, LINES - 1, 1, 
		          "Enter %s value %s: ", 
		          (editHex) ? "hex" : "ascii", SearchStr);

		echo();					/* echo chars         */
		/*wscanw(win->hex_outline, "%s", temp);*/
		/* the third parameter positions the cursor in the correct loc*/
		tmpstr = inputLine(win->hex_outline, LINES - 1, 
			 ((editHex) ? 21 : 23) + 
			 ((strlen(temp) > 10) ? 10 : strlen(temp)));
		noecho();

		wmove(win->hex_outline, LINES - 1, 1);
		wclrtoeol(win->hex_outline);
    		mvwprintw(win->hex_outline, LINES - 1, 1, "Searching ...");
		wrefresh(win->hex_outline);
		doupdate();

		if (tmpstr[0] == 27)			/* escape was hit     */
		{					/* restore & return   */
		    restoreBorder(win);
		    wrefresh(win->hex_outline);
		    break;
		}

		if (tmpstr[0] != '\0' )			/* enter was hit so   */
		{					/* don't change temp  */
		    bzero(temp, 81);
		    strncpy(temp, tmpstr, (strlen(tmpstr) > 80) 
			    ? 80 : strlen(tmpstr));
		}

		val = 0;
							/* parse out input    */
	        for (count = 0; temp[count] != 0 && count < 80; count++)
		    if (!editHex)
			ch[count] = temp[count];
		    else
							/* if hex digit       */
	 		if (isxdigit(key = temp[count])) 
			{
			    if (key >= 65 && key <= 70)	/* get correct val    */
			    	key -= 7;
			    else if (key >= 97 && key <= 102)
			    	key -= 39;
			    key -= 48;
			
			    if ((count % 2) == 0)	/* compute byte val   */
                            	tmp = (key * 16);
			    else 
			    	ch[(count - 1) / 2] = tmp + key;
			}
			else				/* if not hex, bad!   */
			    val = -1;

		if ((count % 2 > 0) && (editHex))	/* add last byte on   */
			    ch[(count + 1) / 2] = tmp;

		if (val != -1)				/* if val checks out  */
							/* search for it      */
		    val = hexSearch(fpIN, ch, cursorLoc(currentLine, col,
			  editHex, BASE), (editHex) ? ((count+1)/2) : count);

		if (val == -1) 				/* if nothing came up */
		{
		    popupWin("Value not found!", -1);
                    restoreBorder(win);			/* restore border     */
		    wrefresh(win->hex_outline);
		}
		else 
		{
                    getyx(Winds, row, col);
							/* goto found loc     */
                    currentLine = gotoLine(fpIN,
                                        cursorLoc(currentLine,col,editHex,BASE),
                                           val, maxlines, Winds);

		}
		break;


	case CTRL_AND('a'):
	case KEY_F(6):					/* if F6, ^a, ^d...   */
		printHex = (!printHex);			/* reverse printHex   */

		getyx(Winds, row, col);			/* current location   */
		row = currentLine - row;		/* compute top line   */

		wmove(win->address, 0, 0);		/* move to origin     */
							/* write out values   */
		/* check to see if screen is filled     */
		if ((maxlines - row) < MAXY)
		    for(count = 0; count <= (maxlines - row); count++)
			wprintw(win->address, (printHex) ? "%0*jX ":"%0*jd ",
			       MIN_ADDR_LENGTH, (intmax_t)((count + row) * BASE));
		else
		    for(count = 0; count <= MAXY && count <= maxlines ; count++)
			wprintw(win->address, (printHex) ? "%0*jX ":"%0*jd ",
			       MIN_ADDR_LENGTH, (intmax_t)((count + row) * BASE));

							/* update menu button */
		slk_set(6, (printHex) ? "Hex Addr":"Dec Addr", 1);
		slk_noutrefresh();			/* refresh            */
		wnoutrefresh(win->address);
		break;

 	case CTRL_AND('g'):
	case KEY_F(4):					/* if F4 or ^g...     */
		wmove(win->hex_outline, LINES-1, 21);   /* output prompt      */
    		wclrtoeol(win->hex_outline);
    		mvwprintw(win->hex_outline, LINES - 1, 1, 
		"Enter %s location: ", (printHex) ? "HEX":"decimal");

		echo();					/* echo chars         */
		gotoLocStr = inputLine(win->hex_outline, LINES - 1, 
			              (printHex) ? 21 : 25);
		if (gotoLocStr[0] == 27)		/* escape was hit     */
		{					/* restore & return   */
		    restoreBorder(win);
		    wrefresh(win->hex_outline);
		    break;
		}
                
                /* convert str to number */
                if (sscanf(gotoLocStr, printHex ? "%jX" : "%jd", &tmp_max) != 1)
                    tmp_max = 0;
                    
                gotoLoc = (off_t)tmp_max;
		/*wscanw(win->hex_outline, (printHex) ? "%X":"%d",&gotoLoc);  */
		noecho();				/* disable echoing    */

		if ((gotoLoc < 0) || (gotoLoc > len-1))
		{   
		    popupWin("Invalid location!", -1);
                    restoreBorder(win);			/* restore border     */
		    wrefresh(win->hex_outline);
		}
		else 
		{
                    getyx(Winds, row, col);
							/* goto found loc     */
                    currentLine = gotoLine(fpIN,
                                           cursorLoc(currentLine, col, editHex,
					   BASE), gotoLoc, maxlines, Winds);
	 	}
		break;

	case KEY_TAB:
	case KEY_F(7):					/* if F7, TAB, ^i...  */
							/*switch the underline*/
		getyx(Winds, row, col);			/* current location   */
		wattrset((editHex) ? win->ascii : win->hex, A_NORMAL);
		mvwprintw((editHex) ? win->ascii : win->hex, row,
			  (editHex) ? col/3 : col*3, 
			  (editHex) ? "%c": "%02X", 
			  (editHex) ? ((isprint(curVal))?curVal : '.'):curVal);
		wnoutrefresh((editHex) ? win->ascii : win->hex);
		if (editHex)				/* already in hex win */
		{
		    Winds = win->ascii;			/* change to ascii    */
		    eol = BASE;
	 	    wmove(Winds, row, (col/3));
		    slk_set(7, "Asc Edit", 1);
		}
		else					/* already in ascii   */
		{
		    Winds = win->hex;			/* change to hex win  */
		    eol = (BASE * 3) - 1;
	 	    wmove(Winds, row, (col*3));	
		    slk_set(7, "Hex Edit", 1);
		}

		editHex = (!editHex);			/* change test val    */
		slk_noutrefresh();			/* refresh menu       */

		getyx(Winds, row, col);			/* current location   */
		/* re-bold char if the value over the cursor is modified      */
		if (inHexList(cursorLoc(currentLine, col, editHex, BASE))) 
		{
		    wattron((editHex) ? win->hex : win->ascii, A_BOLD);
		    mvwprintw((editHex) ? win->hex : win->ascii , row, col, 
			      (editHex) ? "%02X" : "%c", 
			      (editHex) ?curVal:((isprint(curVal))?curVal:'.'));
		    wattrset((editHex) ? win->hex : win->ascii, A_NORMAL);
	 	    wmove(Winds, row, col);	
		    wnoutrefresh((editHex) ? win->ascii : win->hex);
		}
		break;

	case CTRL_AND('?'):
	case CTRL_AND('h'):
	case CTRL_AND('p'):
	case KEY_F(1):					/* if F1, ^?, ^h...   */
		getyx(Winds, row, col);			/* current location   */

		printHelp(win);				/* display the help   */

	 	wmove(Winds, row, col);			/* restore cursor     */
		break;

	case CTRL_AND('z'):				/* ^z undo last mod   */
		getyx(Winds, row, col);

							/* set previous loc   */
		cl = (stack == NULL) ? cl : stack->currentLoc;
		if (stack != NULL)
		{

		    /*if (stack != NULL) val = stack->savedVal;               */
		    val = stack->savedVal;
		    popStack(&stack);
		    head = deleteNode(head,cl);


                    currentLine = gotoLine(fpIN, 
			  	    cursorLoc(currentLine, col, editHex,BASE),
                                    cl, maxlines, Winds);
		/*
                    if (stack != NULL)
		    { 
		        mvwprintw(win->hex_outline,0,20,"                    ");
		        mvwprintw(win->hex_outline,0,10,
			      "value:%02X    location:%d  ",
		    	      stack->currentLoc, 
			      currentLine);
		    }
		*/
		
		    getyx(Winds, row, col);

   		    if ((searchList(head, cl)) != -1)
		    {	
			wattron(win->hex, A_BOLD);
			wattron(win->ascii, A_BOLD);
		    }
                    if (editHex)
		    {
                        wmove(win->ascii, row, (col/3));    
			wprintw(win->ascii, "%c", (USE_EBCDIC) ? EBCDIC[val] :
			       (isprint(val) ? val : 46));
                        wmove(win->hex, row, col);
                        wrefresh(win->ascii);

                        wprintw(win->hex, "%02X", val);
                        wmove(win->hex, row, col);      
                        wrefresh(win->hex);
		    }
		    else
		    {
                        wmove(win->hex, row, (col*3));      
                        wprintw(win->hex, "%02X", val);
                        wmove(win->ascii, row, col);
                        wrefresh(win->hex);

			wprintw(win->ascii, "%c", (USE_EBCDIC) ? EBCDIC[val] :
			       (isprint(val) ? val : 46));
                        wmove(win->ascii, row, col);    
                        wrefresh(win->ascii);
		    }
		    wattrset(win->hex, A_NORMAL);
		    wattrset(win->ascii, A_NORMAL);


		}
		break;

#ifdef DEBUG_LLIST
	case CTRL_AND('x'):
	    printDebug(head, -1);
	    break;
#endif
	}

	getyx(Winds, row, col);				/* get cur row/col    */
	if (fpIN)
	{
							/* store current loc  */
	    /* remove underline from previous character */
	    if ((lastRow == MAXY) && (key == KEY_DOWN) && maxlines!=currentLine)
		lastRow--;
	    else if ((lastRow == 0) && (key == KEY_UP) && currentLine != 0)
		lastRow++;
	    if ((lastRow == MAXY) && (lastCol == eol-1) && (key == KEY_RIGHT) &&
		(maxlines != currentLine))
		lastRow--;
	    else if ((lastRow == 0) && (lastCol == 0) && (key == KEY_LEFT) &&
		    (currentLine != 0))
		lastRow++;
	    

	    if (key != KEY_TAB)				/* if tab, don't do it*/
	    {   /* unhighlight the previous character if tab was not pressed  */
	        curVal = getLocVal(cursorLoc(lastLine, lastCol, editHex, BASE));
		wattrset((editHex) ? win->ascii : win->hex, A_NORMAL);
		wattron((editHex) ? win->ascii : win->hex,
		  (inHexList(cursorLoc(lastLine, lastCol, editHex, BASE))) ?
		  A_BOLD : A_NORMAL);
		mvwprintw((editHex) ? win->ascii : win->hex, lastRow, 
			  (editHex) ? lastCol/3 : lastCol*3, 
			  (editHex) ? "%c" : "%02X", 
			  (editHex) ? (isprint(curVal)) ? curVal : '.': curVal);
		wmove(Winds, row, col);
		wattrset((editHex) ? win->ascii : win->hex, A_NORMAL);
		wnoutrefresh((editHex) ? win->ascii : win->hex);
	    }
							/* highlight new char */
	    wattron((editHex) ? win->ascii : win->hex, A_UNDERLINE);
	    if (inHexList(cursorLoc(currentLine, col, editHex, BASE)))
	        wattron((editHex) ? win->ascii : win->hex, A_BOLD);

	    curVal = getLocVal(cursorLoc(currentLine, col, editHex, BASE));

	    mvwprintw((editHex) ? win->ascii : win->hex, row, 
		      (editHex) ? col/3 : col*3,  
		      (editHex) ? "%c" : "%02X", 
		      (editHex) ? (isprint(curVal)) ? curVal : '.' : curVal); 
	    wattrset((editHex) ? win->ascii : win->hex, A_NORMAL);
	    wnoutrefresh((editHex) ? win->ascii : win->hex);
	}
							/* save last location */
	LastLoc = cursorLoc(currentLine, col, editHex, BASE);
	lastLine = currentLine;
							/* print cur location */
	mvwprintw(win->cur_address, 0, 0, (printHex) ? "%0*jX":"%0*jd",
	   	  MIN_ADDR_LENGTH, (intmax_t)cursorLoc(currentLine, col, editHex,BASE));
	
	wnoutrefresh(win->cur_address);			/* refresh outline    */
	
	scrollbar(win, cursorLoc(currentLine, col, editHex,BASE), maxlines);
	
	wmove(Winds, row, col);				/* restore cursor     */
	doupdate();					/* update visual      */
    }

    free(temp);
    while (stack != NULL)
	popStack(&stack);
    
    return 0;						/* return             */
}

/*******************************************************\
 * Description:  restores the border outline           *
 * Returns:      nothing                               *
\*******************************************************/
void restoreBorder(WINS *win)
{
    int count;

    for (count = 1; count < hex_outline_width - 1; count++)
        mvwaddch(win->hex_outline, LINES-1, count, ACS_HLINE);
    mvwaddch(win->hex_outline, LINES-1, hex_outline_width - 1, ACS_LRCORNER);

}

/********************************************************\
 * Description:  gets a line of input from user		*
 * Returns:	 received string			*
\********************************************************/
char *inputLine(WINDOW *win, int line, int col)
{
    int x;
    unsigned long int c;
    char *ch;
    int allocated = 81;

    noecho();

    ch = (char *)malloc(allocated);				/* allocate space     */

    wmove(win, line, col);

    for (x = 0; (c = wgetch(win)) != 10; x++) 
    {
	if (x > 0 && x >= allocated)
	{
		ch = (char*)realloc(ch, x + 1);
		allocated = x + 1;
	}
	
        wclrtoeol(win);					/* clear line         */
        if (c == '\b' || c == 127) 			/* get backspace      */
	{
            mvwaddch(win, line, col + (x-1), 32);
            wmove(win, line, col + (x-1));
            ch[x] = '\0';
            x -= 2;					/* modify ptr         */
        }
        else if (c > 32 && c < 127) 			/* if printable char  */
	{
            ch[x] = c;
            waddch(win, ch[x]);
        }
	else if (c == 27)				/* if the escape key  */
	{   						/* is pressed, return */
	    ch[0] = 27;					/* setting ch to 0xff */
	    return ch;					
	}
        else 						/* if anything else   */
	{
            ch[x] = '\0';
            x--;
        }

        if (x < 0) 
	{
            wmove(win, line, col);			/* move cursor        */
            x = -1;
        }
    }

    ch[x] = '\0';					/* terminate          */

    return ch;
}
