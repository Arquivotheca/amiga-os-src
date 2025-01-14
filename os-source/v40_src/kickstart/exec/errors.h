#ifndef	EXEC_ERRORS_H
#define	EXEC_ERRORS_H
/*
**	$Id: errors.h,v 39.0 91/10/15 08:26:51 mks Exp $
**
**	Standard Device IO Errors (returned in io_Error)
**
**	(C) Copyright 1985,1986,1987,1988 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#define IOERR_OPENFAIL	 (-1) /* device/unit failed to open */
#define IOERR_ABORTED	 (-2) /* request terminated early [after AbortIO()] */
#define IOERR_NOCMD	 (-3) /* command not supported by device */
#define IOERR_BADLENGTH	 (-4) /* not a valid length (usually IO_LENGTH) */
#define IOERR_BADADDRESS (-5) /* invalid address (misaligned or bad range) */
#define IOERR_UNITBUSY	 (-6) /* device opens ok, but requested unit is busy */
#define IOERR_SELFTEST   (-7) /* hardware failed self-test */

#endif	/* EXEC_ERRORS_H */
