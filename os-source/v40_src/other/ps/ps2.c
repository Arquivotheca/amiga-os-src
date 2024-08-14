/************************************************************************

  Module :  Postscript "Main C entry Point"		© Commodore-Amiga
            Written by P. Jones (started 26/03/1991)

  Purpose:  This file contains the initialisation code for the a PostScript
            context. It also contains the entry point for the Interpreter.

  Conventions: --

  NOTES: --

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <m68881.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/graphics.h>

#include "errors.h"
#include "pstypes.h"
#include "stream.h"
#include "memory.h"
#include "objects.h"

#include "misc.h"
#include "stack.h"
#include "gstate.h"
#include "context.h"

#include "scanner.h"

#include "ps.h"

//--------------------------------------------------------------------
//-------- Internal Debugger values ----------------------------------
//--------------------------------------------------------------------

#define DEBUG_NORMAL	1L
#define DEBUG_STEP		2L

#define DEBUG_BIT	0x02

//--------------------------------------------------------------------
//------- Main Entry Point -------------------------------------------
//--------------------------------------------------------------------

void main(int argc,char **argv) {

DPSContext ctxt;
int mode;
int err;


	if((ctxt = NewContext(argv[argc-1]))==FALSE) {
		printf("Error Creating context\n");
		return ;
	}

	printf("New Context Has Been created \n");
	ctxt->space.state = DEBUG_BIT;
	mode = DEBUG_NORMAL;

	if((err=Interpret(ctxt))) {
		printf("**** Fatal Error From Interpreter %d ****\n",err);
		
	}

	DestroyContext(ctxt);
	printf("Context Destroyed \n");
	printf("GOODBYE\n");
}
