/* LoadSym.rexx
 *
 * (c) Copyright 1992-1993 Commodore-Amiga, Inc.  All rights reserved.
 *
 * This software is provided as-is and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 *
 * Figures out where the symbols for the specified Kickstart are located,
 * and invokes LoadSym.
 *
 * Here's the format of the Kickstart number:
 *
 *	machine:version.revision
 *	machine:revision		(version assumed 39)
 *
 * "machine" can be "2" (A2000/500/600 Kickstart)
 *                  "3" (A3000 Kickstart)
 *                  "4" (A4000 Kickstart)
 *                  "Z" (A2000/500/600 ZKick/ReKick Kickstart)
 *                  "F" (A2000/500/600 F00000 Kickstart)
 */

if ( arg() > 0 ) then
    kickstart = arg(1)

i = index(kickstart,":")

if (i > 0) then
DO
	machine = upper(left(kickstart,i-1))
	kickstart = substr(kickstart,i+1)


	i = index(kickstart,".")
	if (i>0) then
	DO
		version = left(kickstart,i-1)
		revision = substr(kickstart,i+1)
	END
	else
	DO
		version = 39
		revision = kickstart
	END

	if ( machine = "2" ) then
	DO
		name = "A2000/500/600 Kickstart "version"."revision

		v37path = "2000"
		v37file1 = "f0"
		v37file2 = "A300"
		V39file = "A600"
		V40file = "A600"
	END
	else if ( machine = "3" ) then
	DO
		name = "A3000 Kickstart "version"."revision

		v37path = "3000"
		v37file1 = "f8"
		v37file2 = "A3000"
		V39file = "A3000"
		V40file = "A3000"
	END
	else if ( machine = "4" ) then
	DO
		name = "A4000 Kickstart "version"."revision

		v37path = "1000"
		v37file1 = ""
		v37file2 = "A1000"
		V39file = "A1000"
		V40file = "A4000"
	END
	else if ( machine = "Z" ) then
	DO
		name = "A2000/500/600 ZKick/ReKick "version"."revision

		v37path = "2000"
		v37file1 = "20"
		v37file2 = "ZKick"
		V39file = "A600_20"
		V40file = "A600_20"
	END
	else if ( machine = "F" ) then
	DO
		name = "$F00000 Kickstart "version"."revision

		v37path = "2000"
		v37file1 = "f0"
		v37file2 = ""
		V39file = "A600_F0"
		V40file = "A600_F0"
	END


	else
	DO
		exit "*Invalid Kickstart type (must be 2, 3, 4, Z, or F)"
	END

	if ( version = 37 ) then
	DO
		path = "V37:disk/kickstart/"v37path"/"
		path2 = ""
		if ( revision < 200 ) then
		DO
		    file = "symb."revision"."v37file1
		END
		else
		DO
		    file = "sym."v37file2"."version"."revision
		END
	END
	else if ( version = 39 ) then
	DO
		path = "V"version":disk/kickstart/builds/"
		path2 = "V"version":disk/kickstart/ReleaseBuilds/"
		file = "sym."V39file"."version"."revision
	END
	else
	DO
		path = "V"version":disk/kickstart/builds/"
		path2 = "V"version":disk/kickstart/ReleaseBuilds/"
		file = "sym."V40file"."version"."revision
	END
	if ( revision = 999 ) then
	DO
		path = "HD:Kickstart/"
		path2 = ""
		file = "sym."V39file"."version"."revision
	END

	if ( ~exists( path||file".ld" ) & path2 ~= "" ) then
	DO
		path = path2
	END
	if exists( path||file".ld" ) then
	DO
		address command "loadsym >NIL: skipbogus" path||file
		if (rc ~= 0) then
		DO
			exit "*Could not load symbols for" name
		END
	END
	else
	DO
		exit "*Could not find symbols for" name
	END
exit name "symbols"
END
exit "*Bad Kickstart description"
