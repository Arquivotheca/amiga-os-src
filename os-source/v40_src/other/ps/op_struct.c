/************************************************************************

  Module :	Display Postscript "Structured Output Operators" © Commodore-Amiga

  Purpose:	This file contains C-entries for operators called directly
			by the interpreter using the Postscript execution model.

  Conventions: -Variables called 'tos' and 'nos' point to the Top Of Stack
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
error ps_currentobjectformat	(DPSContext);
error ps_printobject			(DPSContext);
error ps_setobjectformat		(DPSContext);
error ps_writeobject			(DPSContext);
//--------------------------------------------------------------------

/************************************************************************/
/******************* Structured Output Operators ************************/
/************************************************************************/

error ps_currentobjectformat(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_printobject(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_setobjectformat(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_writeobject(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
