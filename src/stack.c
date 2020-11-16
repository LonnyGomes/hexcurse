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
 * Description: this pushes a structure of type       *
 *		hexStack to the stack.  NULL being the*
 *		initial state of the stack            *
\******************************************************/
void pushStack(hexStack **stack, off_t cl, int val)
{
    hexStack *oldStack, *newStack;
    oldStack = *stack;

    /* calloc() is used because it NULLS out all returned memory  */
    newStack = (hexStack *) calloc(1, sizeof(hexStack));
    newStack->currentLoc = cl;
    newStack->savedVal = val;

    if (oldStack == NULL)				/* begining of stack  */
        *stack = newStack;
    else 
    {
        newStack->prev = oldStack;
        *stack = newStack;
    }
}

/******************************************************\
 * Description: This function pops a structure off of *
 *		the stack and then free's it's        *
 *		previously allocated memory.  If the  *
 *		stack == NULL that means the bottom of*
 *		the stack has been reached.           *
\******************************************************/
void popStack(hexStack **stack)
{
    hexStack *tmpStack = *stack; 

    if (tmpStack != NULL)
    {						/* set to prev val in llist   */
	*stack = tmpStack->prev;		/* pop off the stack          */
	free(tmpStack);				/* free allocated memory      */
    }
    
}

/******************************************************\
 * Description: This will pop off all values from the *
 *		stack and wil free all the allocated  *
 *		memory avoiding potential memory leaks*
\******************************************************/
void smashDaStack(hexStack **stack)
{
    hexStack *tmpStack = *stack; 

    while (tmpStack != NULL)
    {
        *stack = tmpStack->prev;
	free(tmpStack);
	tmpStack = *stack;
    }
}
