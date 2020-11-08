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

/********************************************************\
 * Description: deletes an item from the linked list and*
 *	 	returns the head of the linked list.    *
\********************************************************/	
hexList *deleteNode(hexList *head, off_t loc)
{
    if (head == NULL)					/* if NULL            */
        return head;					/* just return        */
    else if (head->loc > loc)
	return head;
    else if (head->loc < loc) 				/* if loc > current   */
        head->next = deleteNode(head->next, loc);	/* go to next list    */
    else if (head->loc == loc)				/* if correct loc     */
    {
        hexList *tmpHead = head;
        head = head->next;				/* "delete" it        */
        free(tmpHead);					/* free the memory    */
    }

        /*printDebug(head, loc);*/

    return head;
}

/********************************************************\
 * Description: inserts an item into the linked list and*
 *	 	returns the head of the linked list.    *
\********************************************************/	
hexList *insertItem(hexList *head, off_t loc, int val)
{
    hexList *curr = head,
            *prev = NULL,
            *newHead = head,
            *newItem;

    while (curr != NULL && loc > curr->loc)  /* iterate until correct */
    {                                        /* position for loc      */
        prev = curr;                         /* or end of list        */
        curr = curr->next;
    }
    newItem  = llalloc();                  /* allocate space        */
    if (prev == NULL) newHead = newItem;   /* new head of list      */
    else prev->next = newItem;             /* or point previous     */
    newItem->loc = loc;                    /* store the location    */
    newItem->val = val;                    /* store the value       */
    newItem->next = curr;                  /* point next to current */
    return newHead;
}

/********************************************************\
 * Description: Search through the linked list for to   *
 *		check if loc exist in the list. The     *
 *		function returns -1 if not found or a   *
 *		positve int of the modified value       * 
\********************************************************/
int searchList(hexList *head, off_t loc)
{   
    hexList *tmpHead;					/* allocate temp space*/
    tmpHead = head;					/* temp points to head*/
    while (tmpHead != NULL)				/* while not null     */
    {
        if (tmpHead->loc == loc)			/*if loc == return val*/
			return tmpHead->val;
		else
			tmpHead = tmpHead->next;	/* move to next item  */
    }
    return -1;
}

/********************************************************\
 * Description: write the changes to either the current *
 *		file or to a specified output file	*
\*******************************************************/
int writeChanges()
{   
    FILE *fpOUT = NULL;
    FILE *fptmp = NULL;
    
    off_t buff,prev_loc;					/* declare llist vars */
    hexList *tmpHead = head;

    if (fpOUTfilename && fpIN && (fpOUT = fopen(fpOUTfilename, "w+")))
    {							/* open the write file*/
	fptmp = fpOUT;
        rewind(fpIN);					/* set file loc to 0  */
	rewind(fpOUT);					
	while ((buff = fgetc(fpIN)) != EOF)		/*write to file buffer*/
		fputc(buff, fpOUT);
    }
    else if (fpIN)				/* if no output file  */
    {
	fptmp = fpIN;
	fpIN = fopen(fpINfilename, "r+");
	if (!fpIN)
	{
		fpIN = fptmp;
		popupWin("Cannot write to file: bad permissions", -1);
		return 1;
	}
	fclose(fptmp);
	fptmp = fpIN;
    }
    else
    {
	popupWin("No data written.", -1);
	return 1;
    }
    
    rewind(fpIN);
    rewind(fptmp);
    prev_loc = -1;
    while (tmpHead != NULL)		/* write to file      */
    {
	/* only print the latest change  from the linked list*/
	if (prev_loc != tmpHead->loc) { 
		fseeko(fptmp, tmpHead->loc, SEEK_SET);
		fputc(tmpHead->val, fptmp);
	}
	prev_loc = tmpHead->loc;
	tmpHead = tmpHead->next;
    }

    fflush(fptmp);					/* flush buffto disk  */
    
    rewind(fpIN);					/* reset file pointer */
    if (fpIN != fptmp)
	fclose(fptmp);

    return 0;

} 

/********************************************************\
 * Description: recursivly frees all the memory that was*
 *		allocated via malloc(), this avoids     *
 *		memory leaks that exist far too many    *
 *		programs that you have to pay to use    *
\********************************************************/
hexList *freeList(hexList *head)
{
    if (head != NULL) 						/* while head != NULL */
    {
        freeList(head->next);				/* check next item    */
        free(head);							/* free the memory    */
    }
    return NULL;							/* return NULL to head*/
} 
