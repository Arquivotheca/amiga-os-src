/************************************************************************

  Module :  Postscript "Support Functions"		© Commodore-Amiga
            (started Jan 1991)

  Purpose:  This file contains the C-entries for support modules used
            by the scanner. 

  Conventions: 


  NOTES:    These are not public functions

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <m68881.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#include "exec/types.h"
#include "pstypes.h"
#include "stream.h"
#include "memory.h"
#include "objects.h"

#include "misc.h"
#include "stack.h"
#include "gstate.h"
#include "context.h"

#include "scanner.h"

/**************************************************************************
 *				Proto's												      *
 **************************************************************************/


int  GetEntireString(DPSContext,stream *,int,ps_obj *);
int  GetEntireHexString(DPSContext,stream *,ps_obj *);
int  Get_Esc_Char(stream *);

#define STACK_BUFFER	1024

extern char FLAGS[256];
/**************************************************************************
 *				          Support Functions 						      *
 **************************************************************************/
	
int GetEntireString(DPSContext ctxt,stream *ip,int repeated,ps_obj *target) {
	int length=0;
	uchar text_buffer[STACK_BUFFER];
	int c,loop;
	int balance=1;
	int quit=FALSE;

	while(!quit) {
		c = sgetc(ip);
		if(length>STACK_BUFFER) {                            /* Make sure length's ok */
			return(SYNTAX_ERROR);
		} else switch(c) {
			case '\\' :
				if(!repeated) c = Get_Esc_Char(ip); /* Found an escape. Handle it */
				break;
			case EOF :
				return(SYNTAX_ERROR);
			case ')' :
				if(balance > 0) {                   /* Strings can be nested */
					balance--;                      /* So do balance checking */
					if(balance == 0) {              /* here  */
						quit = TRUE;
					}
				} 
				break;
			case '(' :
				balance++;
				break;
			default : break;
		}
/* 
 * check for null because get_esc_char may decide that the escape char
 * was meaningless and so would return a NULL ie (ignore this char)
 */
		if((c!=-1)&&!quit) text_buffer[length++] = (uchar)c;
	}

/* Allocate a second buffer, this time the correct size */
	if(length>0) {
		if((target->obj.stringval=(uchar *)AllocVM(VM,length))==NULL) {
			return (ALLOC_FAILED);
		}
		target->len = length;
	} else {
		target->obj.stringval= NULL;
		target->len = 0;
	}

/* Copy the string into this second buffer */
	for(loop=0;loop<length;loop++) {
		target->obj.stringval[loop] = text_buffer[loop];
	}

	target->type = (ATTR_LITERAL|TYPE_STRING);
	SAVE_LEVEL(target);
	return(CONTINUE);
}


int Get_Esc_Char(stream *ip) {
	int c,i;
	int octal_value;

	c = sgetc(ip);

/* Let's see if it's an Octal Digit ! */
	if(is_octal(c)) {

		octal_value = c - '0';
		for(i=0;i<2;i++) {
			c = sgetc(ip);
			if(is_octal(c)) octal_value =(octal_value << 3) | (c - '0');
			else {
				 sungetc(ip,c);
				 break;
			}
		}

/* Mask out any overflow bits because PostScript ignores overflow escapes */
#ifdef DEBUG
//	printf("Just consumed an octal value \\%o %d\n",(octal_value&0xff),(octal_value&0xff));
#endif

		return(octal_value&0xff);
	} else switch(c) {             /* It's not Octal so it must be one of */
		case '(' :                 /* these.   */
			return('(');
		case ')' :
			return(')');
		case '\\' :
			return('\\');
		case 'f' :
			return('\f');
		case 'r' :
			return('\r');
		case 'n' :
			return('\n');
		case 't' :
			return('\t');
		case 'b' :
			return('\b');
		case '\n' :                /* This is a real CR, and is not encoded */
			return(-1);            /* with the string */
		default :
		return(c);
	}
}

int GetEntireHexString(DPSContext ctxt,stream *ip,ps_obj *target) {

	int length=0;
	uchar text_buffer[STACK_BUFFER];
	int c,nibble,loop;
	nibble = 0;

	while((c=sgetc(ip))!=EOF && length<STACK_BUFFER) {
		if(!is_space(c)) {            /* Only allow non-white space through */
			if(c == '>') break;       /* The end ? */
			c = toupper(c)-'0';      
			if(c>9) { c -= 7; }       /* Do a bit of math to convert ASCII */
			if(c>=0 && c<=15) {       /* Digit to Hex Digit */
				if(nibble==0) {
					text_buffer[length] = (uchar)(c<<4); /* High nibble */
					nibble = 1;
				} else {
					text_buffer[length++] |= (uchar)c;   /* Low nibble */
					nibble = 0;	
				}
			} else {
				return(SYNTAX_ERROR);
			}
		}
	}

/* If there wasn't an even number of digits, then add one to the length and
 * append a zero
 */
	if(nibble == 1) { length++; }
	if(length>0) {
		if((target->obj.stringval=(uchar *)AllocVM(VM,length))==NULL) {
			return(ALLOC_FAILED);
		}
		target->len = length;
	} else {
		target->obj.stringval = NULL;
		target->len = length;
	}
	for(loop=0;loop<length;loop++) {
		target->obj.stringval[loop] = text_buffer[loop];
	}
	target->type = (ATTR_LITERAL|TYPE_STRING);
	SAVE_LEVEL(target);
	return(CONTINUE);
}

