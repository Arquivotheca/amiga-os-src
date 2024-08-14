/*
 * Berkeley -> Amiga exit() procedure.  Maps error return codes as
 * follows:
 *
 *		1	- RETURN_FAIL
 *		0 	- RETURN_OK
 *		-1	- RETURN_WARN
 */

#include <libraries/dos.h>

/* Copyright (C) 1986,1987 by Manx Software Systems, Inc. */

int (*cls_)();

exit(code)
{
	switch(code){
	case 1:
		code = RETURN_ERROR;
		break;

	case 0:
		code = RETURN_OK;
		break;

	case -1:
		code = RETURN_WARN;
		break;
	}

	if (cls_)
		(*cls_)();
	_exit(code);
}

