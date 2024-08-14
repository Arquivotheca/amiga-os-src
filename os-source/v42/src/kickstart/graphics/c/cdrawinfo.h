/******************************************************************************
*
*	$Id: cdrawinfo.h,v 42.0 93/06/16 11:17:42 chrisg Exp $
*
******************************************************************************/


struct cdraw_info
{
    WORD dx,dy;
    WORD absdx,absdy;
	UWORD	con1;
	char	code1,code2;
	char	xmajor;	/* major axis is x octants 1,4,5,8 */
};
