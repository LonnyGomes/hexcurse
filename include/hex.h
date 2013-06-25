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
#include <ctype.h>				/* char types                 */
#include <errno.h>				/* errors                     */
#include <limits.h>
#include <stdio.h>				/* general includes           */
#include <stdlib.h>				/* standard library           */
#include <signal.h>				/* unix signals               */
#include <string.h>				/* string processing          */
#include <strings.h>				/* string processing          */
#include <unistd.h>				/* unix std routines          */
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
    long int loc;
    int val;
    struct LinkedList *next;
};

struct Stack {					/* struct to be used for stack*/
    int savedVal;
    long int currentLoc;
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
extern int  hex_outline_width;
extern int  hex_win_width;
extern int  ascii_outline_width;
extern int  ascii_win_width;
extern long maxlines;
extern long currentLine;
extern bool editHex;
extern bool printHex;
extern long LastLoc;
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

/* constants */
#define KEY_PGDN        338
#define KEY_PGUP        339
#define HVERSION	"1.55"			/* the version of the source  */
#define MIN_COLS        70                      /* screen has to be 70< cols  */
#define MIN_LINES       7     /* 8 - 1 */       /* the slk crap minuses 1 line*/
#define KEY_TAB 		9			/* value for the tab key      */
#define FN_LEN			255

#define AlphabetSize (UCHAR_MAX +1)		/* for portability            */

#ifndef max
#define max(a,b) ((a) >(b) ? (a) : (b))
#endif

FILE *fpIN, *fpOUT;				/* global file ptrs           */

/* function prototypes */

/* acceptch.c */
int wacceptch(WINS *windows, long len, char *fpINfilename, char *fpOUTfilename);
void restoreBorder(WINS *win);
char *inputLine(WINDOW *win, int line, int col);

/* file.c */
void outline(FILE *fp, int linenum);
int maxLoc(FILE *fp);
void print_usage();
int maxLines(int len);
int openfile(WINS *win, char *fpINfilename);
void savefile(WINS *win, char *fpINfilename, char *fpOUTfilename);
int hexSearch(FILE *fp, int ch[], int startfp, int length);
int gotoLine(FILE *fp, int currLoc, int gotoLoc, int maxlines,  WINDOW *windows);
int getLocVal(long loc);
bool inHexList(long loc);

/* getopt.c */
int hgetopt(int argc, char *const *argv, const char *optstring);

/* hexcurse.c */
long parseArgs(int argc, char *argv[], char *fpINfilename, char *fpOUTfilename);
/*void printDebug(hexList *head, int loc);*/
void catchSegfault(int sig);

/* llist.c */
hexList *deleteNode(hexList *head, int loc);
hexList *insertItem(hexList *head, int loc, int val);
int searchList(hexList *head, int loc);
int writeChanges(WINS *win, FILE *fpIN, FILE *fpOUT, char *fpINfilename, char *fpOUTfilename);
hexList *freeList(hexList *head);

/* screen.c */
void init_menu(WINS *windows);
void exit_err(char *err_str);
void init_screen(void);
void screen_exit(int exit_val);
void init_fkeys();
void checkScreenSize(int sig);
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
