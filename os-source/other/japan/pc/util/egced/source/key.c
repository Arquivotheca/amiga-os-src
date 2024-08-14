#ifndef lint
static char     rcsid[]="$Id: key.c,v 1.1 93/06/08 10:59:03 brummer Exp $";
#endif
/*
*
*     key.c  :   key functions
*
*        vfKey()    : key type getting routine
*        vfGetKey() : key code getting routine
*
*        Copyright(C) 1991 ERGOSOFT Corp. TOKIO Labo.
*        All Rights Reserved
*
*        Designed    by        H.Yanata  1991.Apr.04
*        Coded       by        H.Yanata  1991.Apr.04
*		 Modified    by        T.Koide   1992.09.14
*
*/

#include    <stdio.h>
#include    <string.h>
#ifdef UNIX
#include    <curses.h>
#endif

#include    "egced.h"
#include    "crt.h"
#include    "keybrd.h"

#ifndef        UNIX
#include    <dos.h>
#endif

VOID    vfKey(ubKey)
    UBYTE   ubKey[2];
{
    do {
        vfGetKey(&ubKey[0]);
        switch(ubKey[0]){
            case    UP:
            case    DWN:
            case    RGT:
            case    LFT:
            case    RUP:
            case    RDWN:
                ubKey[1] = CSR_MOVE;
                break;
            case    ESC:
            case    TAB:
            case    MENU:
            case    MASK:
#ifndef        UNIX
            case    AUTO_UP:
            case    AUTO_DWN:
            case    AUTO_PAGE_UP:
            case    AUTO_PAGE_DWN:
#endif
                ubKey[1] = COMMAND;
                break;
            default    :
                ubKey[1] = ILLEGAL;
                BEEP();
                break;
        }
    } while(ubKey[1] != CSR_MOVE && ubKey[1] != COMMAND);

}

/*------------------------------------------------------*/
/*    get charactor code                                */
/*------------------------------------------------------*/
VOID    vfGetKey(ubKey)
    UBYTE    *ubKey;
{
#ifdef UNIX
    noecho();
    raw();
    *ubKey = getchar();
    noraw();
	echo();
#else
    *ubKey = (UBYTE)waitgch();
#endif

}

#ifndef        UNIX
/*------------------------------------------------------*/
/*    get charactor code(wait input)                    */
/*------------------------------------------------------*/
int        waitgch(VOID)
{
    iRegs.h.ah = 0x0c;
    intdos(&iRegs, &oRegs);
    iRegs.h.ah = 0x07;
    intdos(&iRegs, &oRegs);
    return (oRegs.h.al);
}

/*------------------------------------------------------*/
/*    get charactor code(non wait)                      */
/*------------------------------------------------------*/
int        nonwaitgch(VOID)
{
    iRegs.h.ah = 0x06;
    iRegs.h.dl = 0xff;
    do {
		intdos(&iRegs, &oRegs);
	} while (oRegs.h.al == 0);
    return  (oRegs.h.al);
}
#endif    /* not UNIX */

/*------------------------------------------------------*/
/*    get string                                        */
/*------------------------------------------------------*/
VOID    vfGetString(ubStrBuf, iLength)
    UBYTE       *ubStrBuf;        /* data area                */
    int         iLength;          /* maximum data length      */
{
    UBYTE   *ubPtr;               /* data pointer             */
    UBYTE    ubCode;              /* character code           */
    REG      i;

    ubPtr = ubStrBuf;
    i = 0;
    do {
        /*---------- get charactor code ----------*/
#ifndef    UNIX
        ubCode = (UBYTE)nonwaitgch();
#else
        vfGetKey(&ubCode);
#endif    /* not UNIX */

        if (ubCode >= 0x21 || ubCode == BS || ubCode == ESC){
            /*---------- bak space ----------*/
            if (ubCode == BS && i > 0) {
                i--;
                *(--ubPtr) = 0x00;
                printf("\033[1D \033[1D");
            /*---------- cancel ----------*/
            } else if (ubCode == ESC){
                memset(ubStrBuf, 0x00, iLength);
                return;
            /*---------- input ----------*/
            } else if (ubCode != BS && i < iLength){
                *(ubPtr++) = ubCode;
                printf("%c", ubCode);
                i++;
            } else    if (ubCode != RET)    BEEP();
        } else    BEEP();
    } while(ubCode != RET);
    for (; i < iLength; i++)    ubStrBuf[i] = 0x00;
}
