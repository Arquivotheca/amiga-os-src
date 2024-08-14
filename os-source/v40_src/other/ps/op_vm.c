/************************************************************************

  Module :	Postscript "virtual memory" operators

  Purpose:	This file contains C-entries for operators called directly
			by the interpreter using the Postscript execution model.

  Conventions: -The order in which functions appear is identical to the one
				in the Adobe Red Book (Chapter 6 Operator Summary).
	    	   -Variables called 'tos' and 'nos' point to the Top Of Stack
				and Next Of Stack elements resp. (on Operand stack).

To do/unfinished:
- save, restore

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
error ps_vmstatus	(DPSContext);
error ps_save		(DPSContext);
error ps_restore	(DPSContext);
//--------------------------------------------------------------------

/************************************************************************/
error ps_vmstatus(DPSContext ctxt) { // - vmstatus - savelevel used max 

ps_obj result;
int u,m;

	ENOUGH_ROOM(3);				// need to push three results

	result.type=TYPE_INT|ATTR_LITERAL;
	result.tag=result.len=0;	// initialise an integer result

	result.obj.intval=ctxt->space.save_level;
	PUSHOPER(ctxt,&result);				// push the savelevel argument

	m=GetTotalVM(ctxt->space.MemPool);	// get total memory allocated for pool
	u=m-GetFreeVM(ctxt->space.MemPool);// calculate how much has been used

	result.obj.intval=u;
	PUSHOPER(ctxt,&result);				// push the amount used

	result.obj.intval=m;
	PUSHOPER(ctxt,&result);				// push the amount allocated

	return ERR_OK;
}
/************************************************************************/
error ps_save(DPSContext ctxt) {
ps_obj save;

	ENOUGH_ROOM(1);
	save.type=TYPE_SAVE;
	save.tag=save.len=0;
	save.obj.saveval=0;
	PUSHOPER(ctxt,&save);
	++ctxt->space.save_level;
	return ERR_OK;
}
/************************************************************************/
error ps_restore(DPSContext ctxt) {

ps_obj *tos;
	NEED_ARGS(1);
	tos=OPERSP(ctxt);
	if(OBJ_TYPE(tos)==TYPE_SAVE) {
		--ctxt->space.save_level;
		POPOPER(ctxt);
		return ERR_OK;
	}
	else return ERR_typecheck;
}

/************************************************************************/

