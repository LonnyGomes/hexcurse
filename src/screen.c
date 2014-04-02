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

/******************************************************\
 * Description: Inits the menu and the user interface *
 *		that the hex editor uses.             *
\******************************************************/
void init_menu(WINS *windows)
{   
    int x;
    WINDOW *win;

    /*signal(SIGCHLD,  SIG_DFL);                                              */
    signal(SIGSEGV,  catchSegfault);			/* catch segfault sig */
    /* the following is still under development, please report any bugs found */
    signal(SIGWINCH, checkScreenSize);		 	/*catch resize screen */

							/* draw the windows   */
    windows->hex_outline = drawbox(0, 0, LINES, hex_outline_width);
    windows->ascii_outline = drawbox(0, hex_outline_width + 1, LINES, 
	    			     ascii_outline_width);
    windows->scrollbar = newwin(LINES, 1, 0, hex_outline_width);

    windows->hex = newwin(LINES - 2, hex_win_width, 1, MIN_ADDR_LENGTH + 2);
    windows->ascii = newwin(LINES - 2, ascii_win_width, 1, 
	                    hex_outline_width + 2);
    windows->address = newwin(LINES - 2, MIN_ADDR_LENGTH + 1, 1, 1);

    windows->cur_address = newwin(1, MIN_ADDR_LENGTH, 0, 1);

    waddch(windows->scrollbar, ACS_UARROW);
    wattron(windows->scrollbar, A_REVERSE);
    waddch(windows->scrollbar, ACS_DIAMOND);
    wattroff(windows->scrollbar, A_REVERSE);
    for (x = 0; x < LINES - 2; x++)
        waddch(windows->scrollbar, ACS_CKBOARD);
    waddch(windows->scrollbar, ACS_DARROW);
    wrefresh(windows->scrollbar);
    
    cbreak();						/* allow cbreak()     */
    noecho();						/* disable echoing    */
    raw();

    win = windows->hex;
    for (x = 0; x <= 2; x++)				
    {
	idlok(win, TRUE);				/* enable scrolling   */

        keypad(win, TRUE);				/* enable the keypad  */

	win = (x == 0) ? windows->ascii:windows->address;
    }

}

/******************************************************\
 * Description: Deallocates the windows               *
 *		that the hex editor uses.             *
\******************************************************/
void free_windows(WINS *windows)
{
      delwin(windows->cur_address);
      windows->cur_address = NULL;
      
      delwin(windows->address);
      windows->address = NULL;
      
      delwin(windows->ascii);
      windows->ascii = NULL;
      
      delwin(windows->hex);
      windows->hex = NULL;
      
      delwin(windows->scrollbar);
      windows->scrollbar = NULL;
      
      delwin(windows->ascii_outline);
      windows->ascii_outline = NULL;
      
      delwin(windows->hex_outline);
      windows->hex_outline = NULL;
}

/******************************************************\
 * Description:	exit the program printing out an error*
 *		from the errno value.	 	      *
\******************************************************/
void exit_err(char *err_str)
{
    endwin(); 						/* end curses screen  */
    perror(err_str);					/* print errno        */
    printf("\n\n");
    exit(EXIT_FAILURE);					/* return with exitval*/
}

/******************************************************\
 * Description: Initialize the curses graphical screen*
 *		If the screen does not support curses *
 *		exit gracefully.		      *
\******************************************************/ 
void init_screen(void)
{
    if ((initscr()) == NULL)				/*couldn't init screen*/
    {
        perror("initscr");				/* print out error    */
        printf("exiting\n");
	exit(EXIT_FAILURE);
    }

}

/******************************************************\
 * Description:	Terminate the curses screen and exit  *
 *		out of the program.		      *
\******************************************************/
void screen_exit(int exit_val)
{
    endwin();						/* end curses screeen */
    printf("\n");
    exit(exit_val);
}

/******************************************************\
 * Description: Initialize function key menu interface*
\******************************************************/ 
void init_fkeys()
{
    slk_set(1, "Help",		1);			/* init menu tabs     */
    slk_set(2, "Save",		1);
    slk_set(3, "Open",	 	1);
    slk_set(4, "Goto",		1);
    slk_set(5, "Find", 		1);

    slk_set(7, "Hex Edit",	1);
    slk_set(8, "Quit",		1);
    slk_set(9, "Quit",		1);
    slk_noutrefresh();					/* refresh the screen */
}

/******************************************************\
 * Description:	Determines the screen size of the the *
 *		terminal and prompts for the user to  *
 *		change the screen.		      *
\******************************************************/
RETSIGTYPE checkScreenSize(int sig)
{   
    int count;
    
    /* Avoid unused variable warning */
    UNUSED(sig);

    clearScreen(windows);
    endwin();
    init_screen();                                      /* init visuals       */

    slk_clear();                                        /* init menu bar      */
    slk_restore();                                      /* restore bottom menu*/
    slk_noutrefresh();
    doupdate();
							/* recacl these values*/
    BASE                =   (COLS - 6 - MIN_ADDR_LENGTH) / 4;            /*base for the number */
    hex_outline_width   =   (BASE * 3) + 3 + MIN_ADDR_LENGTH;
    MAXY                =   LINES - 3;
    hex_win_width       =   BASE * 3;
    ascii_outline_width =   BASE + 2;
    ascii_win_width     =   BASE;
    maxlines		=   maxLines((fpIN != NULL) ? maxLoc(fpIN) : 0);
    currentLine		=   0;
    SIZE_CH		=   TRUE;
                                                        /* check for term size*/
    if ((COLS < MIN_COLS) || (LINES < MIN_LINES))	
    {    
	/*
        endwin();
        printf("\n\n\n\nscreen size too small\n");
        exit(0);
	*/

	init_fkeys();                                   /* define menu bar    */
	init_menu(windows);                             /* init windows       */
	clearScreen(windows);
	slk_clear();
	mvwprintw(windows->hex, 0, 0, "Your screen is too small");
	/*mvwprintw(windows->hex, 1, 2, "Resize it to continue");*/
	refreshall(windows);
	doupdate();
    }
    else
    {
	init_fkeys();                                   /* define menu bar    */
	init_menu(windows);                             /* init windows       */
	wmove(windows->hex,0,0);
	if (fpIN)					/* if a file is open  */
	{
	    for (count = 0; count <= MAXY && count <= maxLines(maxLoc(fpIN)); 
		 count++)
		outline(fpIN, count);

	    mvwprintw(windows->cur_address, 0, 0, "%0*d", MIN_ADDR_LENGTH, 0);
	    wmove(windows->hex,0,0);
	}

	refreshall(windows);
        wnoutrefresh(windows->cur_address);
	/* this next refresh is to put the cursor in the correct window */
        wnoutrefresh(windows->hex);
	doupdate();
    }
}

/******************************************************\
 * Description:	refresh all of the curses windows     *
\******************************************************/
void refreshall(WINS *win)
{
    slk_noutrefresh();					/* refresh fkeys menu */
    wnoutrefresh(win->hex_outline);			/* refresh all windows*/
    wnoutrefresh(win->ascii_outline);
    wnoutrefresh(win->scrollbar);
    wnoutrefresh(win->ascii);
    wnoutrefresh(win->address);
    wnoutrefresh(win->hex);
}

/******************************************************\
 * Description:	draws a box with the given parameters *
 *		these boxes are used to create the    *
 *		user interface.  The function returns *
 *		the newly created window	      *
\******************************************************/
WINDOW *drawbox(int y, int x, int height, int width)
{
    WINDOW *win;
    win = newwin(height, width, y, x);			/* create a new window*/
    
    box(win, 0, 0);					/* draw the outline   */

    return win;						/*return created win  */
}

/******************************************************\
 * Description:	construct a scroll bar that is used   *
 *		for scrolling through the hex editor  *
\******************************************************/ 
void scrollbar(WINS *windows,  int cl, long maxLines)
{
    float x, percent;

    /* Avoid unused variable warning */
    UNUSED(maxLines);
    
    /* cl really should be a long, it wasn't changed because cl is used
     * in other functions as an int.  Eventually everything should be using
     * long rather than int */

    if (fpIN != NULL)
	percent = (float)cl/(float)maxLoc(fpIN);	/* protect against x/0*/
    else
	percent = 0.0;

    x = (int)(percent * (float)(LINES));
    x = (x < 1) ? 1 : x;
    x = (x >= (LINES - 1)) ? LINES - 2 : x;

    wattron(windows->scrollbar, A_REVERSE);		/* set attribs on bar */
    wattron(windows->scrollbar, A_REVERSE);		/* set attribs on bar */
    mvwaddch(windows->scrollbar, (int)x, 0, ACS_DIAMOND);
    wattroff(windows->scrollbar, A_REVERSE);
    wnoutrefresh(windows->scrollbar);			/* refresh the bar    */
    mvwaddch(windows->scrollbar, (int)x, 0, ACS_CKBOARD);
}

/********************************************************\
 * Description: outputs key command help                *
 * Returns:     none                                    *
\********************************************************/
void printHelp(WINS *win)
{
    WINDOW *ctrl, *help, *small;

    ctrl = newwin(LINES - 2, 9, 1, 1),
    help = newwin(LINES - 2, hex_win_width, 1, 10),
    small = newwin(LINES - 2, ascii_win_width, 1, hex_outline_width + 2);
    
    if ((LINES < 18) || (COLS < 78))			/* min size to display*/
    {
	mvwprintw(help, 0, 0, "Screen too small to display help");
	wmove(help,0,0);
    }
    else
    {
	wmove(ctrl,0,0);
	mvwprintw(ctrl, 0, 1, "Ctrl Key");		/* print in address   */
	mvwprintw(ctrl, 2, 1, "CTRL+?");
	mvwprintw(ctrl, 3, 1, "CTRL+S");
	mvwprintw(ctrl, 4, 1, "CTRL+O");
	mvwprintw(ctrl, 5, 1, "CTRL+G");
	mvwprintw(ctrl, 6, 1, "CTRL+F");
	mvwprintw(ctrl, 7, 1, "CTRL+A");
	mvwprintw(ctrl, 8, 1, "TAB");
	mvwprintw(ctrl, 9, 1, "CTRL+Q");
	mvwprintw(ctrl, 11, 1, "CTRL+U");
	mvwprintw(ctrl, 12, 1, "CTRL+D");
	mvwprintw(ctrl, 13, 1, "CTRL+Z");
	mvwprintw(ctrl, 14, 1, "CTRL+T");
	mvwprintw(ctrl, 15, 1, "CTRL+B");

	mvwprintw(small, 0, 1, "Function Keys");	/* print in ascii     */
	mvwprintw(small, 2, 1, "Help     = F1");
	mvwprintw(small, 3, 1, "Save     = F2");
	mvwprintw(small, 4, 1, "Open     = F3");
	mvwprintw(small, 5, 1, "Goto     = F4");
	mvwprintw(small, 6, 1, "Find     = F5");
	mvwprintw(small, 7, 1, "HexAdres = F6");
	mvwprintw(small, 8, 1, "Hex Edit = F7");
	mvwprintw(small, 9, 1, "Quit     = F8");
	mvwprintw(small, 11, 1, "Page up  = PGUP");
	mvwprintw(small, 12, 1, "Page down= PGDN");

	mvwprintw(help, 0, 10, "HexCurse Keyboard Commands");
	mvwprintw(help, 2, 2, "Help     - you are reading it now");
	mvwprintw(help, 3, 2, "Save     - saves the current file open");
	mvwprintw(help, 4, 2, "Open     - opens a new file");
	mvwprintw(help, 5, 2, "Goto     - goto a specified address");
	mvwprintw(help, 6, 2, "Find     - search for a hex/ascii value");
	mvwprintw(help, 7, 2, "HexAdres - toggle between hex/decimal address");
	mvwprintw(help, 8, 2, "Hex Edit - toggle between hex/ASCII windows");
	mvwprintw(help, 9, 2, "Quit     - exit out of the program");
	mvwprintw(help, 11, 2, "Page up  - scrolls one screen up");
	mvwprintw(help, 12, 2, "Page down- scrolls one screen down");
	mvwprintw(help, 13, 2, "Undo     - reverts last modification");
	mvwprintw(help, 14, 2, "Home     - returns to the top of the file");
	mvwprintw(help, 15, 2, "End      - jumps to the bottom of the file");
	mvwprintw(help, 17, 12, "Press enter to continue");
    }

    wnoutrefresh(ctrl);					/* refresh new wins   */
    wnoutrefresh(help);
    wnoutrefresh(small);
    doupdate();						/* update screen      */

    wgetch(ctrl);					/* wait for a char    */

    delwin(ctrl);					/* delete help wins   */
    delwin(help);
    delwin(small);

    redrawwin(win->hex);				/* redraw previous    */
    redrawwin(win->ascii);
    redrawwin(win->address);

    wnoutrefresh(win->hex);				/* refresh            */
    wnoutrefresh(win->ascii);
    wnoutrefresh(win->address);
    doupdate();						/* update screen      */

}

/********************************************************\
 * Description: scrolls window one line up or down      *
 * Returns:     none                                    *
\********************************************************/
void winscroll(WINS *windows, WINDOW *win, int n, int currentLine)
{
    int row, col;                                       /* row and col        */

    getyx(win, row, col);                               /* get cur row/col    */

    scrollok(windows->hex, TRUE);                       /* allow scrolling    */
    scrollok(windows->ascii, TRUE);
    scrollok(windows->address, TRUE);

    wscrl(windows->hex, n);                             /* scroll each win    */
    wscrl(windows->ascii, n);
    wscrl(windows->address, n);

    scrollok(windows->hex, FALSE);                      /*disable scrolling   */
    scrollok(windows->ascii, FALSE);
    scrollok(windows->address, FALSE);

    wmove(windows->hex, (n == 1) ? MAXY:0, 0);		/* place cursor       */
    wmove(windows->ascii, (n == 1) ? MAXY:0, 0);
    wmove(windows->address, (n == 1) ? MAXY:0, 0);

    outline(fpIN, currentLine);				/* output line        */

    wnoutrefresh(windows->hex);                         /* set win refreshes  */
    wnoutrefresh(windows->ascii);
    wnoutrefresh(windows->address);

    wmove(win, row, col);                               /* restore cursor     */
}

/******************************************************\
 * Description:	Clears the entire screen              *
\******************************************************/
void clearScreen(WINS *win)
{
    wclear(win->hex);
    wclear(win->ascii);
    wclear(win->address);
    wclear(win->scrollbar);
    wclear(win->hex_outline);
    wclear(win->ascii_outline);
    wclear(win->cur_address);
    refreshall(win);
    doupdate();
}

/******************************************************\
 * Description:	Checks if the exit key has been       *
 *		pressed.  If so, it checks if the     *
 *		file has been saved since the last    *
 *		modification.  If it hasn't it prompts*
 *		the user asking if they would like to *
 *		save.				      *
 * Returns:     2 == save, 1 == no save, 0 == cancel  *
\******************************************************/
int quitProgram(int notChanged, short int ch)
{   
    short int str;
							/* if exit key...     */
    if ((ch == KEY_F(8)) || (ch == CTRL_AND('q')) || (ch == CTRL_AND('x')))
    {
        if (!notChanged)				/* if changes made    */
        {
							/* prompt for exit    */
            str = questionWin("Do you want to save changes? (y/n)");

            if (str == 'Y' || str == 'y')		/* if yes, save       */
                return 2;
            else if ((str != 'N') && (str != 'n'))	/* don't quit, cancel */
                return 0;
        }

        return 1;					/* quit, don't save   */
    }
    else 
        return 0;					/* don't quit         */
}

/******************************************************\
 * Description:	Creates a "window popup" where given  *
 * 		the parameters, and message, it       *
 * 		creates a window and displays the     *
 * 		message.  Some current restrictions   *
 * 		are that it must only be one line, and*
 * 		it is assumed that the text is padded *
 * 		with one space left for the border and*
 * 		another space left for aesthetics     *
\******************************************************/
void popupWin(char *msg, int time)
{
    WINDOW *tmpwin;

    int y = (LINES / 2) - 3,				/* calc location      */
        len = strlen(msg),
        x = (COLS - len)/2;

    y = (y < 2) ? 2 : y;				/* minimum height     */
    time = (!time) ? 2 : time;

    tmpwin = drawbox(y, x, 5, len + 6);			/* create window      */

    keypad(tmpwin, TRUE);

    mvwprintw(tmpwin,2,3, msg);				/* output mesg        */
    wmove(tmpwin,2,len+4);
    wrefresh(tmpwin);

    if (time == -1)
        wgetch(tmpwin);
    else
        sleep(time);					/* wait               */

    delwin(tmpwin);

    wtouchln(windows->hex, y - 1, 5, 1);		/* touch windows      */
    wtouchln(windows->ascii, y - 1, 5, 1);
    wtouchln(windows->address, y - 1, 5, 1);
    wtouchln(windows->hex_outline, y, 5, 1);
    wtouchln(windows->ascii_outline, y, 5, 1);
    wtouchln(windows->scrollbar, y, 5, 1);

    refreshall(windows);				/* refresh all wins   */

    doupdate();
}

/********************************************************\
 * Description:	Similar to popupWin, only it prmpts the	*
 *		user for a character, such as (y/n)	*
 * Returns:	Returns character pressed		*
\********************************************************/
short int questionWin(char *msg)
{
    WINDOW *tmpwin;
    short int ch;

    int y = (LINES / 2) - 3,				/* calc location      */
        len = strlen(msg),
        x = (COLS - len)/2;

    y = (y < 2) ? 2 : y;				/* minimum height     */

    tmpwin = drawbox(y, x, 5, len + 6);			/* create window      */

    mvwprintw(tmpwin,2,3, msg);
    wmove(tmpwin,2,len+4);
    wrefresh(tmpwin);

    ch = wgetch(tmpwin);

    delwin(tmpwin);

    wtouchln(windows->hex, y - 1, 5, 1);		/* touch the lines    */
    wtouchln(windows->ascii, y - 1, 5, 1);		/* covered by win     */
    wtouchln(windows->address, y - 1, 5, 1);
    wtouchln(windows->hex_outline, y, 5, 1);
    wtouchln(windows->ascii_outline, y, 5, 1);
    wtouchln(windows->scrollbar, y, 5, 1);

    refreshall(windows);				/* refresh all wins   */

    doupdate();

    return ch;
}
