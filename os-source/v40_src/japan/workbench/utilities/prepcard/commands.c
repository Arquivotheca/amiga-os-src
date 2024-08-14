#include "prepcard.h"

LONG DoCommand( struct cmdVars *cmv, ULONG command )
{
LONG error;

	error = RETURN_OK;

	switch( command )
	{

		case CM_MAKEDISK:
			error = PrepCard( cmv, TRUE );
			break;

		case CM_MAKERAM:
			error = PrepCard( cmv, FALSE );
			break;

		case CM_ADV:
			MakeAdvDisplay( cmv );
			break;

		case CM_QUIT:
			cmv->cv_IsBusy = FALSE;
			break;

		default:
			error = RETURN_FAIL;
			break;
	}

	return( error );
}

