/******************************************************************************
*
*	$Id: cdrawinfo.h,v 39.0 91/08/21 17:15:03 chrisg Exp $
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
