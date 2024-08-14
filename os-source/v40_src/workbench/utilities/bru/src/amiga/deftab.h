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

char *deftab = 
"df0:|df1:|df2:|df3: \\\n\
    size=880K seek=512 bufsize=22K noreopen qfwrite \\\n\
    prerr=5 pwerr=5 zrerr=5 zwerr=5 frerr=5 fwerr=5 wperr=30 \n\
- \\\n\
    size=0 seek=0 \\\n\
    prerr=0 pwerr=0 zrerr=0 zwerr=0 frerr=0 fwerr=0 wperr=0 \n\
";
