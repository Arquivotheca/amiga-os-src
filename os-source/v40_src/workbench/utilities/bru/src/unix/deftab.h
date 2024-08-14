/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */


/*
 *	A default table to be used if no table can be loaded.
 */

static char *deftab = 
#if xenix		/* Can't handle big strings */
"/dev/mt0 \\\n\
   size=0 seek=0 \\\n\
   prerr=0 pwerr=0 zrerr=EIO zwerr=0 frerr=0 fwerr=0 wperr=0 \\\n\
   reopen tape advance \n\
/dev/rmt0 \\\n\
   size=0 seek=0 \\\n\
   prerr=0 pwerr=0 zrerr=EIO zwerr=0 frerr=0 fwerr=0 wperr=0 \\\n\
   reopen rawtape tape advance \n\
- \\\n\
   size=0 seek=0 \\\n\
   prerr=0 pwerr=0 zrerr=0 zwerr=0 frerr=0 fwerr=0 wperr=0 \n\
";
#else
#if AUX
"/dev/rfloppy0 \\\n\
	size=800K seek=512 \\\n\
	prerr=ENOTTY pwerr=ENOTTY zrerr=ENOTTY zwerr=ENXIO frerr=EIO \\\n\
	fwerr=EIO wperr=0 reopen eject format\n\
/dev/rfloppy1 \\\n\
	size=800K seek=512 \\\n\
	prerr=ENOTTY pwerr=ENOTTY zrerr=ENOTTY zwerr=ENXIO frerr=EIO \\\n\
	fwerr=EIO wperr=0 reopen eject format\n\
- \\\n\
   size=0 seek=0 \\\n\
   prerr=0 pwerr=0 zrerr=0 zwerr=0 frerr=0 fwerr=0 wperr=0 \n\
";
#else
"/dev/rmt0 \\\n\
   size=0 seek=0 \\\n\
   prerr=0 pwerr=0 zrerr=EIO zwerr=0 frerr=0 fwerr=0 wperr=0 \\\n\
   reopen rawtape tape advance \n\
/dev/nrmt0 \\\n\
   size=0 seek=0 \\\n\
   prerr=0 pwerr=0 zrerr=EIO zwerr=0 frerr=0 fwerr=0 wperr=0 \\\n\
   noreopen tape rawtape norewind advance \n\
- \\\n\
   size=0 seek=0 \\\n\
   prerr=0 pwerr=0 zrerr=0 zwerr=0 frerr=0 fwerr=0 wperr=0 \n\
";
#endif
#endif
