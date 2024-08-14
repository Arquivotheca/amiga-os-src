/* StructureOffset.rexx
 *
 * (c) Copyright 1992-1993 Commodore-Amiga, Inc.  All rights reserved.
 *
 * This software is provided as-is and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 *
 * Rexx script to parse the Structure.offsets file to figure out the
 * offset of a given structure/member.
 *
 * Syntax:	call StructureOffset structure member
 *
 * If successful, the first four characters are the member offset,
 * the next four characters are the member size, and
 * the rest of the string is the full member name.  If failure, the
 * first four characters will be non-hexadecimal.
 *
 * Also sets ARexx clip variable "StructureMembers" to the lines
 * of the structure-offsets file containing all the members of
 * this structure.
 */

options results
if ( arg() > 0 ) then
DO
	argline = arg(1)
	parse var argline structure" "member

	call open 'structfile', "HD:Structure.offs"

	done = 0
	offset = 0
	DO until ( done ~= 0 )
		structdata = '0a'x||readch('structfile',10000)
		if eof('structfile') then
		DO
			done = -1
		END
		else
		DO
			i = pos( '0a'x||structure":",structdata )
			if ( i > 0 ) then
			DO
				offset = offset + i
				done = 1
			END
			else
			DO
				offset = offset + 9000
			END
			call seek('structfile',offset,'b')
		END
	END
	if ( done > 0 ) then
	DO
		/* Offset is correct, so read "enough" to grab the
		 * whole structure, then discard everything after
		 * the start of the next one:
		 */
		structdata = readch('structfile',20000)
		structdata = substr(structdata,pos(":",structdata)+2)
		structdata = left(structdata,pos(":",structdata))
		structdata = left(structdata,lastpos("0a"x,structdata))

		/* In case someone wants to parse the whole thing */
		call setclip 'StructureMembers',structdata

		i = pos(member,structdata)
		if ( i > 0 ) then
		DO
			start = lastpos( '0a'x, structdata, i )
			end = pos( '0a'x, structdata, i )
			/* Special handling for the "sizeof" line */
			if ( substr(structdata,start+16,7) = 'sizeof(' ) then
			DO
				response = substr(structdata,start+4,4) || substr(structdata,start+10,4) || substr(structdata,start+16,end-start-16)
			END
			else
			DO
				response = substr(structdata,start+4,4) || substr(structdata,start+16,4) || substr(structdata,start+22,end-start-22)
			END
		END
		else
		DO
			response = "Couldn't find member" member "in structure" structure
		END
	END
	else
	DO
		response = "Couldn't find structure" structure
	END
END
else
DO
	response = "Bad syntax"
END

exit response
