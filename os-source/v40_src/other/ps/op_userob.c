/************************************************************************

  Module :	Display Postscript "User Objects Operators"		© Commodore-Amiga

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
error ps_defineuserobject	(DPSContext);
error ps_execuserobject		(DPSContext);
error ps_undefineuserobject	(DPSContext);
//--------------------------------------------------------------------

/************************************************************************/
/************************* User Object Operators ************************/
/************************************************************************/

error ps_defineuserobject(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_execuserobject(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_undefineuserobject(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
