
* *****************************************************************************
* 
* defs.asm -- define some magic constants for janus library
* 
* Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
* 
* Date       Name               Description
* ---------  -----------        -----------------------------------------------
* 15-Jul-88  -RJ                Changed all files to work with new includes
* 
* *****************************************************************************


		INCLUDE "exec/types.i"
		INCLUDE "janus/janusbase.i"


		XDEF	SysBaseOffset

SysBaseOffset	EQU	jb_ExecBase



		END



