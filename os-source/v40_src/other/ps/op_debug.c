/************************************************************************

  Module :	Postscript "Debug Operators"		© Commodore-Amiga
			(started March 1991)

  Purpose: This file contains two operators which allow the user
           to turn the debugger on or off.

  Conventions: -The order in which functions appear is identical to the one
				in the Adobe Red Book (Chapter 6 Operator Summary).
		       -Variables called 'tos' and 'nos' point to the Top Of Stack
				and Next Of Stack elements resp. (on Operand stack).

 NOTES: 

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <intuition/intuition.h>
#include <proto/intuition.h>

#include "errors.h"
#include "exec/types.h"
#include "pstypes.h"
#include "stream.h"
#include "memory.h"
#include "objects.h"

#include "misc.h"
#include "stack.h"
#include "gstate.h"
#include "context.h"

#define DEBUG_BIT	0x02

//--------------------------------------------------------------------
error ps_debugon	(DPSContext);
error ps_debugoff	(DPSContext);
error ps_showhash	(DPSContext);
error ps_flash		(DPSContext);
error ps_showtable	(DPSContext);
//--------------------------------------------------------------------

extern void ClearDisplay(DPSContext);
extern void RefreshScreen(DPSContext);
extern void UpdateStacks(DPSContext,int);

extern void Monitor(DPSContext);
extern void Box(int,int,int,int,int);
extern int InitDebug(DPSContext , int);

extern void print_distrib(DPSContext);

/************************************************************************/
error ps_flash(DPSContext ctxt) {
	SHORT *reg;
	int loop;
	reg = (SHORT *)0xdff180;
//	fprintf(OP,"Flash !  Ah Ah, Saviour of the universe\n");
	for(loop=0;loop<500000;loop++) {
		*reg = 0xf00;
	}
	return ERR_OK;
}
/************************************************************************/
error ps_debugon(DPSContext ctxt) {
	if(ctxt->wind==NULL) {
		if(!(InitDebug(ctxt,1))) {
			ctxt->space.state |=DEBUG_BIT;
			RefreshScreen(ctxt);
		}
	}
	return ERR_OK;
}
/************************************************************************/
error ps_debugoff(DPSContext ctxt) {
	if(ctxt->wind!=NULL) {
		ctxt->space.state &= ~DEBUG_BIT;
		ClearDisplay(ctxt);
	}
	return ERR_OK;
}	
/************************************************************************/
error ps_showhash(DPSContext ctxt) {
	print_distrib(ctxt);
	return ERR_OK;
}
/************************************************************************/
error ps_showtable(DPSContext ctxt) {
	int loop;
	for(loop=0;loop<256;loop++) {
		if(ctxt->space.SysNameTab[loop] == NULL) {
//			fprintf(OP,"NULL\t\t%d\n",loop);
		} else {
//			fprintf(OP,"%s\t\t%d\n",ctxt->space.SysNameTab[loop],loop);
		}
	}
	return ERR_OK;
}
