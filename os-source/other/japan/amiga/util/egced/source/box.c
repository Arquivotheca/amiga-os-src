#ifndef lint
static char     rcsid[]="$Id: box.c,v 3.4.1.1 91/06/25 21:06:52 katogi GM Locker: katogi $";
#endif
/*
*
*     box.c  :  make and remove box functions
*
*        vfMakeBox()    : normal box making routine
*        vfMakeRvsBox() : reverse box making routine
*        vfRemoveBox()  : box removing routines
*
*        Copyright(C) 1991 ERGOSOFT Corp. TOKIO Labo.
*        All Rights Reserved
*
*        Designed    by        H.Yanata  1991.Apr.04
*        Coded       by        H.Yanata  1991.Apr.04
*		 Modified	 by		   T.Koide   1992.09.14
*
*/

#include    <stdio.h>
#include    "egced.h"
#include    "crt.h"

#ifndef        UNIX
#include    <conio.h>
#define     puts    cputs
#define     printf    cprintf

    static    char    *ubBoxFlm[] = {
    /*    0     1     2     3     4     5   : ‚P‚C…    */
        "Â", "Â", "Â", "Â", "Â—", "ÂÛ"
    };
#else
    static    char    *ubBoxFlm[] = {
    /*    0     1     2     3     4     5   : ‚P‚C…    */
        "  ", "  ", "  ", "  ", "  ", "  "
    };
#endif

VOID    vfMakeBox(iLeft, iTop, iRight, iBottom)
    int        iLeft;               /*    point of left     */
    int        iTop;                /*    point of top      */
    int        iRight;              /*    point of right    */
    int        iBottom;             /*    point of bottom   */
{
    REG        i;

    CSROFF();
#ifndef UNIX
    CSR(33);
#endif
    for (i = 0; i < (iRight - iLeft); i += 2){
        CSRXY(iLeft + i, iBottom);
        puts(ubBoxFlm[5]);
        CSRXY(iLeft + i, iTop);
        puts(ubBoxFlm[5]);
    }
    for (i = 0; i < (iBottom - iTop); i++){
        CSRXY(iLeft, iTop + i);
        puts(ubBoxFlm[4]);
        CSRXY(iRight, iTop + i);
        puts(ubBoxFlm[4]);
    }
    CSRXY(iLeft, iTop); puts(ubBoxFlm[0]);
    CSRXY(iRight, iTop); puts(ubBoxFlm[1]);
    CSRXY(iLeft, iBottom); puts(ubBoxFlm[2]);
    CSRXY(iRight, iBottom); puts(ubBoxFlm[3]);
    NORMCSR();
}

VOID    vfMakeRvsBox(iLeft, iTop, iRight, iBottom, iColor)
    int        iLeft;               /*    point of left    */
    int        iTop;                /*    point of top     */
    int        iRight;              /*    point of right   */
    int        iBottom;             /*    point of bottom  */
    int        iColor;              /*    color of box     */
{
    REG        i;
#ifndef        UNIX
    switch(iColor){
        case    WHITE:
            CSR(7;38); break;
        case    BLUE:
            CSR(7;34); break;
        case    RED:
            CSR(7;31); break;
        case    MAGENTA:
            CSR(7;35); break;
        case    CYAN:
            CSR(7;36); break;
        case    GREEN:
            CSR(7;32); break;
        case    YELLOW:
            CSR(7;33); break;
        default:
            CSR(7;36); break;
    }
#endif
    CSROFF();
    for (i = 0; i < (iRight - iLeft); i += 2){
        CSRXY(iLeft + i, iBottom);
        puts(ubBoxFlm[5]);
        CSRXY(iLeft + i, iTop);
        puts(ubBoxFlm[5]);
    }
    for (i = 0; i < (iBottom - iTop); i++){
        CSRXY(iLeft, iTop + i);
        puts(ubBoxFlm[4]);
        CSRXY(iRight, iTop + i);
        puts(ubBoxFlm[4]);
    }
    CSRXY(iLeft, iTop); puts(ubBoxFlm[0]);
    CSRXY(iRight, iTop); puts(ubBoxFlm[1]);
    CSRXY(iLeft, iBottom); puts(ubBoxFlm[2]);
    CSRXY(iRight, iBottom); puts(ubBoxFlm[3]);
    NORMCSR();
}


VOID    vfRemoveBox()
{
    REG        i;
#ifndef		UNIX
	static char		cFlm[] = "                       ";
#else
	static char		cFlm[] = "                        ";
#endif

    NORMCSR();
    for (i = 0; i < 5; i++){
#ifndef		UNIX
        CSRXY(56, 7 + i);
#else
		CSRXY(55, 7 + i);
#endif
        puts(cFlm);
    }
}
