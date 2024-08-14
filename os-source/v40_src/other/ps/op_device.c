/************************************************************************

  Module :	Display Postscript "Device Setup/Output Operators"	© Commodore-Amiga
			Written by --------- (started May 1991)

  Purpose:	This file contains C-entries for operators called directly
			by the interpreter using the Postscript execution model.

  Conventions:	-Variables called 'tos' and 'nos' point to the Top Of Stack
				and Next Of Stack elements resp. (on Operand stack).

*************************************************************************/

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

//--------------------------------------------------------------------
error ps_showpage		(DPSContext);
error ps_copypage		(DPSContext);
error ps_banddevice		(DPSContext);
error ps_framedevice	(DPSContext);
error ps_nulldevice		(DPSContext);
error ps_renderbands	(DPSContext);
//--------------------------------------------------------------------

/************************************************************************/
/******************* Device Setup and Output Operators ******************/
/************************************************************************/
error ps_showpage	(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_copypage	(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_banddevice	(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_framedevice	(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_nulldevice	(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_renderbands	(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
