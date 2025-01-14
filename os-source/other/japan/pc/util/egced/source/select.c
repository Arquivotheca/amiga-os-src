#ifndef lint
static char     rcsid[]="$Id: select.c,v 40.0 93/06/08 10:56:29 brummer Exp $";
#endif
/*
*
*     select.c  :  command select
*
*        Copyright(C) 1991 ERGOSOFT Corp. TOKIO Labo.
*        All Rights Reserved
*
*        Designed    by        H.Yanata  1991.Apr.04
*        Coded       by        H.Yanata  1991.Apr.04
*		 Modified    by 	   T.Koide   1992.09.14
*
*/

#include    <stdio.h>
#include	<string.h>
#include    "egced.h"
#include    "keybrd.h"
#include    "crt.h"

#ifndef        UNIX
#include       <dos.h>
#include       <conio.h>
#define        printf    cprintf
#endif


/*    command items    */
#ifdef    UNIX
    static char    *ubMenu[] = {
        "\033[%d;38mF:����", "\033[%d;38mM:�C��",
        "\033[%d;38mD:�폜", "\033[%d;38mE:�o�^",
        "\033[%d;38mC:����", "\033[%d;38mI:�}��",
        "\033[%d;38mW:�ۑ�", "\033[%d;38mP:���", 
        "\033[%d;38mQ:�I��"
    };
#else
    static char    *ubMenu[] = {
        "\033[%d;33mF\033[%d;38m:����", "\033[%d;33mM\033[%d;38m:�C��",
        "\033[%d;33mD\033[%d;38m:�폜", "\033[%d;33mE\033[%d;38m:�o�^",
        "\033[%d;33mC\033[%d;38m:����", "\033[%d;33mI\033[%d;38m:�}��",
        "\033[%d;33mW\033[%d;38m:�ۑ�", "\033[%d;33mP\033[%d;38m:���", 
        "\033[%d;33mQ\033[%d;38m:�I��"
    };
#endif

int        ifSelect(sDAT, ubCommand)
    sDATASET    *sDAT;              /* dictionary data        */
    UBYTE        ubCommand;         /* key code (ascii)       */
{
       int         iSelect;
       REG         i,iCsr = 0;
       char        ubKey;
static char        ubErace[] = "                                                                               ";   /* space * 79             */
#ifdef    UNIX
    static char        ubCom[] = "[ESC:Cancel]";
#else
    static char        ubCom[] = "\033[33m[ESC:Cancel]";
#endif

    switch(ubCommand){
        /*----- hinshi edit -----*/
        case    TAB:
            iSelect = ABORT;    /* not display data after hinshi edit    */
            vfHinshiEdit(sDAT);
            vfDispHin(sDAT);
			vfGetIndex(sDAT);
            break;
		case	MASK:
			iSelect = EDIT;
			sDAT->ubMask[0] = ubDataMask(&(sDAT->ubMask[1]));
			vfGetIndex(sDAT);
			if (sDAT->VertHin == YES) {
	            CLRSCR();
				vfSetMonitor();
    	        vfDisplay(sDAT, NO_CSR_MOVE);
				vfBoxDisplay(sDAT);
           		NORMCSR();
			}
			break;

#ifndef    UNIX
        /*----- auto scroll -----*/
        case    AUTO_UP:
            vfAutoScrl(sDAT, UP);
            break;
        case    AUTO_DWN:
            vfAutoScrl(sDAT, DWN);
            break;
        case    AUTO_PAGE_UP:
            vfAutoScrl(sDAT, RUP);
            break;
        case    AUTO_PAGE_DWN:
            vfAutoScrl(sDAT, RDWN);
            break;
#endif
        case    DISPHIN:
            iSelect = EDIT;
            if(sDAT->VertHin == YES){
                CLRSCR();
                vfSetMonitor();
                NORMCSR();
                if (strlen(sDAT->ubFile) > 12){
                    i = 0;
                    while(sDAT->ubFile[i] != '\0')        i++;
                    while(sDAT->ubFile[i] != DELIMITER)   i--;
                    i++;
                    CSRXY(65, 3);
                    printf("%-12s",sDAT->ubFile + i);
                } else {
                    CSRXY(65, 3);
                    printf("%-12s",sDAT->ubFile);
                }
				CSRXY(65, 4);
				if(sDAT->iDicLen == DICSIZ32)
					printf("%-12s","32 byte");
				else if(sDAT->iDicLen == DICSIZ48)
					printf("%-12s","48 byte");
				else 
					printf("%-12s","80 byte");

                CSRXY(65, 5);
                printf("%-5d", sDAT->IndexMax);
                sDAT->VertHin = NO;
            } else {
                sDAT->VertHin = YES;
                vfDispVertHin(sDAT);
            }
            break;
        /*----- command -----*/
        case    MENU:
        if (sDAT->VertHin == YES){
            CLRSCR();
            vfDisplay(sDAT, NO_CSR_MOVE);
			vfBoxDisplay(sDAT);
          	vfSetMonitor();
            NORMCSR();
            i = 0;
            if (strlen(sDAT->ubFile) > 12){
                i = 0;
                while(sDAT->ubFile[i] != '\0')        i++;
                while(sDAT->ubFile[i] != DELIMITER)   i--;
                i++;
                CSRXY(65, 3);
                printf("%-12s",sDAT->ubFile + i);
            } else {
                CSRXY(65, 3);
                printf("%-12s",sDAT->ubFile);
            }

			CSRXY(65, 4);
			if(sDAT->iDicLen == DICSIZ32)
				printf("%-12s","32 byte");
			else if(sDAT->iDicLen == DICSIZ48)
				printf("%-12s","48 byte");
			else 
				printf("%-12s","80 byte");

            CSRXY(65, 5);
            printf("%-5d", sDAT->IndexMax);
        }

        /*----- get command mode -----*/
            CSRXY(1, 1); printf(ubCom);
            do {
                CSROFF();
                vfShowMenu();
                CSRXY(14 + iCsr*7, 1);
                printf(ubMenu[iCsr], 7, 7);
                vfGetKey(&ubKey);
                if (ubKey == RGT){
                    iCsr++;
                    if (iCsr > 8) iCsr = 0;
                }
                if (ubKey == LFT){
                    iCsr--;
                    if (iCsr == -1) iCsr = 8;
                }
            } while(ubKey != ESC && ubKey != RET && ubKey != TAB &&
                    /*---------        dilect command         --------*/
                    ubKey != 'f' && ubKey != 'F' && ubKey != '�' &&
                    ubKey != 'd' && ubKey != 'D' && ubKey != '�' &&
                    ubKey != 'e' && ubKey != 'E' && ubKey != '�' &&
                    ubKey != 'c' && ubKey != 'C' && ubKey != '�' &&
                    ubKey != 'w' && ubKey != 'W' && ubKey != '�' &&
                    ubKey != 'm' && ubKey != 'M' && ubKey != '�' &&
                    ubKey != 'i' && ubKey != 'I' && ubKey != '�' &&
                    ubKey != 'p' && ubKey != 'P' && ubKey != '�' &&
                    ubKey != 'q' && ubKey != 'Q' && ubKey !='�' );
            
            switch(ubKey){
                /*----- abort -----*/
                case    ESC:
                case    TAB:
                    iSelect = ABORT;
                    break;
                case    RET:
                /*----- cursor move -----*/
                    switch(iCsr){
                        case    0:            /*find*/
                            iSelect = EDIT;
                            vfFind(sDAT);
                            break;
                        case    1:            /*modify*/
                            iSelect = EDIT;
                            vfModify(sDAT);
							vfGetIndex(sDAT);
                            break;
                        case    2:            /*delete*/
                            iSelect = EDIT;
                            vfDelete(sDAT);
                            vfGetIndex(sDAT);
							break;
                        case    3:            /*entry*/
                            iSelect = EDIT;
                            vfEntry(sDAT);
							vfGetIndex(sDAT);
                            break;
                        case    4:            /*change*/
                            iSelect = EDIT;
                            vfChange(sDAT);
							vfGetIndex(sDAT);
                            break;
                        case    5:            /*insert*/
                            iSelect = EDIT;
                            vfInsert(sDAT);
							vfGetIndex(sDAT);
                            break;
                        case    6:            /*save*/
                            iSelect = EDIT;
                            if (sDAT->iReadOnly == YES)
                                vfDispReadOnly();
                            else
                                bfWriteRec(sDAT);
                            break;
                        case    7:
                            iSelect = EDIT;
                            vfPrintCtrl(sDAT);
                            break;
                        case    8:
                            iSelect = ifQuit(sDAT);
                            break;
                        default    :
                            break;
                    }
                    break;
                /*----- dilect command -----*/
                case    'f': case    'F': case    '�':    /*find*/
                    /*----- search -----*/
                    iSelect = EDIT;
                    vfShowMenu();
                    CSRXY(14, 1);
                    printf(ubMenu[0], 7, 7);
                    vfFind(sDAT);
                    break;
                case    'm': case    'M': case    '�':    /*modify*/
                    /*----- modify -----*/
                    vfShowMenu();
                    CSRXY(21, 1);
                    printf(ubMenu[1], 7, 7);
                    vfModify(sDAT);
					vfGetIndex(sDAT);
                    break;
                case    'd': case    'D': case    '�':    /*delete*/
                    iSelect = EDIT;
                    vfShowMenu();
                    CSRXY(28, 1);
                    printf(ubMenu[2], 7, 7);
                    vfDelete(sDAT);
					vfGetIndex(sDAT);
                    break;
                case    'e': case    'E': case    '�':    /*entry*/
                    iSelect = EDIT;
                    vfShowMenu();
                    CSRXY(35, 1);
                    printf(ubMenu[3], 7, 7);
                    vfEntry(sDAT);
					vfGetIndex(sDAT);
                    break;
                case    'c': case    'C': case    '�':    /*change*/
                    iSelect = EDIT;
                    vfShowMenu();
                    CSRXY(42, 1);
                    printf(ubMenu[4], 7, 7);
                    vfChange(sDAT);
					vfGetIndex(sDAT);
                    break;
                case    'i': case    'I': case    '�':    /*insert*/
                    iSelect = EDIT;
                    vfShowMenu();
                    CSRXY(49, 1);
                    printf(ubMenu[5],7,7);
                    vfInsert(sDAT);
					vfGetIndex(sDAT);
                    break;
                case    'w': case    'W': case    '�':    /*save*/
                    iSelect = EDIT;
                    vfShowMenu();
                    CSRXY(56, 1);
                    printf(ubMenu[6], 7, 7);
                    if (sDAT->iReadOnly == YES){
                        vfDispReadOnly();
                    } else {
                        bfWriteRec(sDAT);
                    }
                    break;
                case    'p': case    'P': case    '�':    /*print*/
                    iSelect = EDIT;
                    vfShowMenu();
                    CSRXY(63, 1);
                    printf(ubMenu[7],7,7);
                    vfPrintCtrl(sDAT);
                    break;
                case    'q': case    'Q': case    '�':    /*quit*/
                    vfShowMenu();
                    CSRXY(70, 1);
                    printf(ubMenu[8],7,7);
                    iSelect = ifQuit(sDAT);
                    break;
                default    :
                    break;
            }
            break;
        default:
            break;
    }
    
    NORMCSR();
    CSRXY(1, 1);  puts(ubErace);
    
    if (sDAT->VertHin == YES)    vfDispVertHin(sDAT);
    
    return        iSelect;
}

VOID    vfShowMenu()
{
    REG        i;
    
    for (i = 0; i < 9; i++){
        CSRXY(14 + i*7, 1); 
        printf(ubMenu[i],0,0);
    }
}

#ifndef        UNIX
UBYTE    inkey(VOID)
{
    return    (UBYTE)(bdos(6, 0xff,0) & 0x00ff);
}
void    vfAutoScrl(sDAT, mode)
    sDATASET    *sDAT;
    UBYTE        mode;
{
    REG        i;
    
    switch(mode){
    case    RUP:
        i = sDAT->wDispNum;
        while((i -= DISPMAX) > 0){
            sDAT->wIndexNum -= DISPMAX;
            sDAT->wDispNum -= DISPMAX;
            vfDisplay(sDAT, RUP);
            if (ESC == inkey()) return;
        }
        /* no break */
    case    UP:
        i = sDAT->wDispNum;
        while(i-- > 0){
            sDAT->wIndexNum--;
            sDAT->wDispNum--;
            vfDisplay(sDAT, UP);
            if (ESC == inkey()) return;
        }
        break;
    case    RDWN:
        i = sDAT->IndexMax - sDAT->wIndexNum - 1;
        while((i -= DISPMAX) > 0){
            sDAT->wIndexNum += DISPMAX;
            sDAT->wDispNum += DISPMAX;
            vfDisplay(sDAT, RDWN);
            if (ESC == inkey()) return;
        }
        /* no break */
    case    DWN:
        i = sDAT->wIndexMax - sDAT->wDispNum - 1;
        while(i-- > 0){
            if (sDAT->wDispNum + sDAT->iCsrY > sDAT->IndexMax - 2)
                break;
            sDAT->wIndexNum++;
            sDAT->wDispNum++;
            vfDisplay(sDAT, DWN);
            if (ESC == inkey()) return;
        }
        break;
    default:
        break;
    }
}
#endif

