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
#include <config.h>
#include <ctype.h>				/* char types                 */
#include <errno.h>				/* errors                     */
#include <limits.h>
#include <stdio.h>				/* general includes           */
#include <stdlib.h>				/* standard library           */
#include <signal.h>				/* unix signals               */
#include <string.h>				/* string processing          */
#include <strings.h>				/* string processing          */
#include <unistd.h>				/* unix std routines          */
#if defined(HAVE_STDINT_H)
    #include <stdint.h>
#endif
#if defined(HAVE_INTTYPES_H)
    #include <inttypes.h>
#endif
#include <sys/types.h>				/* types                      */
#include "hgetopt.h"

#ifdef HAS_NCURSES
    #include <ncurses.h>
#else
    #include <curses.h> 
#endif

#if !defined(TRUE)
    #define TRUE        1
#endif
#if !defined(FALSE)
    #define FALSE       0
#endif

#if !defined(HAVE_FSEEKO)
    #define fseeko fseek
    #define fteelo ftell
#endif

/* datatypes */
typedef struct {                                /* struct that holds windows  */
    WINDOW  *hex,
            *ascii,
            *address,
            *scrollbar,
            *hex_outline,
            *ascii_outline,
            *cur_address;
} WINS;

struct LinkedList {				/* linked list structure      */
    off_t loc;
    int val;
    struct LinkedList *next;
};

struct Stack {					/* struct to be used for stack*/
    int savedVal;
    off_t currentLoc;
    struct Stack *prev;
    struct LinkedList *llist;
};

/* typedefs */
typedef struct LinkedList hexList;		/* alias name to hexList      */
typedef struct Stack      hexStack;		/* alias name to hexStack     */

extern char *fpINfilename, *fpOUTfilename;      /* global file ptrs           */
extern int  MAXY;				/* external globals           */
extern WINS *windows;                           /* struct that stores windows */
extern hexList *head;				/* head for linked list       */
extern int  BASE;                               /* the base for the number    */
extern int  MIN_ADDR_LENGTH;
extern int  hex_outline_width;
extern int  hex_win_width;
extern int  ascii_outline_width;
extern int  ascii_win_width;
extern off_t maxlines;
extern off_t currentLine;
extern bool editHex;
extern bool printHex;
extern off_t LastLoc;
extern int  SIZE_CH;
extern bool USE_EBCDIC;
extern char EBCDIC[256];

/* macros */
/*#define currentLoc(line, col) ((line) * BASE +((col)/3)) */
#define MAXY() ((CUR_LINES) - 3)		/* macro for maxy lines       */
#define CTRL_AND(c) ((c) & 037)			/* macro to use the ctrl key  */
						/* cursor location in the file*/
#define cursorLoc(line,col,editHex,b) (((line)*(b)) + ((col)/((editHex)?3:1)))
#define llalloc() (struct LinkedList *) calloc(1, sizeof(struct LinkedList))
#define isEmptyStack(stack) (((stack) == NULL) ? TRUE : FALSE)

#define UNUSED(x) (void)(x)

/* constants */
#define KEY_PGDN        338
#define KEY_PGUP        339
#define HVERSION	VERSION			/* the version of the source  */
#define MIN_COLS        70                      /* screen has to be 70< cols  */
#define MIN_LINES       7     /* 8 - 1 */       /* the slk crap minuses 1 line*/
#define KEY_TAB 		9			/* value for the tab key      */

#define AlphabetSize (UCHAR_MAX +1)		/* for portability            */

#ifndef max
#define max(a,b) ((a) >(b) ? (a) : (b))
#endif

FILE *fpIN;		        		/* global file ptr           */

/* function prototypes */

/* acceptch.c */
int wacceptch(WINS *windows, off_t len);
void restoreBorder(WINS *win);
char *inputLine(WINDOW *win, int line, int col);

/* file.c */
void outline(FILE *fp, off_t linenum);
off_t maxLoc(FILE *fp);
void print_usage();
off_t maxLines(off_t len);
int openfile(WINS *win);
void savefile(WINS *win);
off_t hexSearch(FILE *fp, int ch[], off_t startfp, int length);
off_t gotoLine(FILE *fp, off_t currLoc, off_t gotoLoc, off_t maxlines,  WINDOW *windows);
int getLocVal(off_t loc);
bool inHexList(off_t loc);

/* getopt.c */
int hgetopt(int argc, char *const *argv, const char *optstring);

/* hexcurse.c */
off_t parseArgs(int argc, char *argv[]);
/*void printDebug(hexList *head, int loc);*/
int getMinimumAddressLength(off_t len);
RETSIGTYPE catchSegfault(int sig);

/* llist.c */
hexList *deleteNode(hexList *head, off_t loc);
hexList *insertItem(hexList *head, off_t loc, int val);
int searchList(hexList *head, off_t loc);
int writeChanges();
hexList *freeList(hexList *head);

/* screen.c */
void init_menu(WINS *windows);
void free_windows(WINS *windows);
void exit_err(char *err_str);
void init_screen(void);
void screen_exit(int exit_val);
void init_fkeys();
RETSIGTYPE checkScreenSize(int sig);
void refreshall(WINS *win);
WINDOW *drawbox(int y, int x, int height, int width);
void scrollbar(WINS *windows, int currentLine, long maxLines);
void printHelp(WINS *win);
void winscroll(WINS *win, WINDOW *, int n, int currentLine);
void clearScreen(WINS *win);
int  quitProgram(int notChanged, short int ch);
void popupWin(char *msg, int time);
short int questionWin(char *msg);

/* stack.c */
void createStack(hexStack *stack);
void pushStack(hexStack **stack, hexStack *tmpStack);
void popStack(hexStack **stack);
void smashDaStack(hexStack **stack);
