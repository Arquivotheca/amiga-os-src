/************************************************************************

  Module :	Display Postscript "Multiple Execution Context Operators"
			© Commodore-Amiga

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
error ps_condition			(DPSContext);
error ps_currentcontext		(DPSContext);
error ps_fork				(DPSContext);
error ps_join				(DPSContext);
error ps_lock				(DPSContext);
error ps_monitor			(DPSContext);
error ps_notify				(DPSContext);
error ps_wait				(DPSContext);
error ps_yield				(DPSContext);
//--------------------------------------------------------------------

/************************************************************************/
/**************** Multiple Execution Contexts Operators *****************/
/************************************************************************/

error ps_condition(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_currentcontext(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_fork(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_join(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_lock(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_monitor(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_notify(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_wait(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_yield(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
