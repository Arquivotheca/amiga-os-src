#ifndef lint
static char     rcsid[]="$Id: csrmove.c,v 3.4.1.1 91/06/25 21:07:15 katogi GM Locker: katogi $";
#endif
/*
*
*     csrmove.c  :   cursor moving routhin
*
*        Copyright(C) 1991 ERGOSOFT Corp. TOKIO Labo.
*        All Rights Reserved
*
*        Designed    by        H.Yanata  1991.Apr.04
*        Coded       by        H.Yanata  1991.Apr.04
*		 Modified    by		   T.Koide   1992.09.14
*
*/

#include    <stdio.h>
#include    "egced.h"
#include    "keybrd.h"
#include    "crt.h"


VOID    vfCsrMove(sDAT, ubKey)
    sDATASET    *sDAT;        /*    pointer to data buffer    */
    UBYTE        ubKey;       /*    key code (ascii)          */
{
	switch(ubKey){
		case	UP:
			if (sDAT->wIndexNum > INDEX_MIN){
			    if(sDAT->wIndexNum < sDAT->IndexMax){
				    sDAT->iCsrY --;
				    sDAT->wIndexNum --;
				    if (sDAT->iCsrY < CSR_TOP){
					    sDAT->iCsrY = CSR_TOP;
					    sDAT->wDispNum --;
					    if (sDAT->wDispNum < INDEX_MIN)
						    sDAT->wDispNum = INDEX_MIN;
				    }
				}
			} else	BEEP();
			break;
		case	DWN:
			if (sDAT->wIndexNum < sDAT->IndexMax-1) { 
				sDAT->iCsrY ++;
				sDAT->wIndexNum ++;
				if (sDAT->iCsrY > CSR_BOTTOM){
					sDAT->iCsrY = CSR_BOTTOM;
					sDAT->wDispNum ++;
					if (sDAT->wDispNum > sDAT->IndexMax-1)
						sDAT->wDispNum = sDAT->IndexMax-1;
				}
			} else {
				BEEP();
			}
				break;
		case	RGT:
			sDAT->iCsrX ++;
			if (sDAT->iCsrX > CSR_RIGHT)	sDAT->iCsrX = CSR_LEFT;
			break;
		case	LFT:
			sDAT->iCsrX --;
			if (sDAT->iCsrX < CSR_LEFT)	sDAT->iCsrX = CSR_RIGHT;
			break;
		case	RUP:
			if (sDAT->wIndexNum > INDEX_MIN){
				sDAT->wIndexNum -= DISPMAX;
				sDAT->wDispNum -= DISPMAX;
				if (sDAT->wDispNum < INDEX_MIN){
					BEEP();
					sDAT->wIndexNum = INDEX_MIN;
					sDAT->wDispNum = INDEX_MIN;
	    			sDAT->iCsrY = CSR_TOP;
				}
			} else 	BEEP();
			break;
		case	RDWN:
			if (sDAT->wDispNum + DISPMAX < sDAT->IndexMax){
				sDAT->wIndexNum += DISPMAX;
				sDAT->wDispNum += DISPMAX;
				if (sDAT->wIndexNum > sDAT->IndexMax - 1){
					BEEP();
					sDAT->wIndexNum = sDAT->IndexMax - 1;
					sDAT->iCsrY = sDAT->wIndexNum - sDAT->wDispNum;
				}

			} else {
				BEEP();
			}
			break;
		default	:
			BEEP();
			break;
	}
}

