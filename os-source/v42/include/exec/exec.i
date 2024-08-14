	IFND	EXEC_EXEC_I
EXEC_EXEC_I	SET	1
**
**	$Id: exec.i,v 39.0 91/10/15 08:27:04 mks Exp $
**
**	Include all other Exec include files in non-overlapping order.
**
**	(C) Copyright 1985,1986,1987,1988,1989,1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

	IFND	EXEC_TYPES_I
	INCLUDE "exec/types.i"
	ENDC
	INCLUDE "exec/macros.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/alerts.i"
	INCLUDE "exec/errors.i"
	INCLUDE "exec/initializers.i"
	INCLUDE "exec/resident.i"
	INCLUDE "exec/strings.i"
	INCLUDE "exec/memory.i"
	INCLUDE "exec/tasks.i"
	INCLUDE "exec/ports.i"
	INCLUDE "exec/interrupts.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/io.i"
	INCLUDE "exec/devices.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/ables.i"
;;;;;;;;INCLUDE "exec/exec_lib.i"    ;special information

	ENDC	; EXEC_EXEC_I
