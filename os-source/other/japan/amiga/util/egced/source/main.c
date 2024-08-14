#ifndef lint
static char     rcsid[]="$Id: main.c,v 3.4.1.1 91/06/25 21:07:27 katogi GM Locker: katogi $";
#endif
/*
*
*     main.c  :   egced main routhin
*
*        Copyright(C) 1991 ERGOSOFT Corp. TOKIO Labo.
*        All Rights Reserved
*
*        Designed    by        H.Yanata  1991.Apr.04
*        Coded       by         H.Yanata  1991.Apr.04
*		 Modified    by			T.Koide   1992.09.14
*
*/

#include    "egced.h"
#include    "crt.h"
#include    "keybrd.h"

/*-----------------------------------------------------------*/
/*    main function                                          */
/*-----------------------------------------------------------*/
int        main(argc, argv)
    int     argc; 
    char  **argv;
{
    BOOL        bInitRslt;            /* initialize result           */
    int         iSelect;              /* select mode                 */
    sDATASET    sDAT;                 /* data buffer                 */
    static UBYTE ubKey[] =            /* key code initialize         */
        {(UBYTE)0, (UBYTE)0};
	REG			i;

#ifdef UNIX
    initscr();
#endif

    CLRSCR();

    /*---------- copyright ----------*/
    vfCpyrt();

    /*---------- data initialize ---------*/
	sDAT.ubMask[0] = 0x00;
	sDAT.ubMask[1] = -1;
    sDAT.wIndexNum = 0;               /* current index number initialize   */
    sDAT.wDispNum = 0;                /* current display number initialize */
	sDAT.IndexMax = 0;
    sDAT.iCsrX = sDAT.iCsrY = 0;      /* current cursor point initialize   */
    sDAT.VertHin = NO;

    /*---------- file initialize ----------*/
    bInitRslt = bfFileInit(argc, argv, &sDAT);
    if (! bInitRslt)    return    EXIT_ERROR;    /* file initialize error  */

    /*---------- memory initialize ----------*/
    bInitRslt = bfMemInit(&sDAT);
    if (! bInitRslt)    return    EXIT_ERROR;    /* memory initialize error*/

    /*---------- data initialize ----------*/
    bInitRslt= bfReadRec(&sDAT);
    if (! bInitRslt)    return    EXIT_ERROR;    /* data initialize error  */

	/*---------- data index initialize ---------*/
	for( i = 0; i < sDAT.wIndexMax; i++) 
		sDAT.IndexNum[i] = i;
	sDAT.IndexMax = sDAT.wIndexMax;

    /*---------- cursor off ----------*/
    CSROFF();

    /*---------- screen initialize ----------*/
    CLRSCR();
    VL8025();

    /*---------- display initialize ----------*/
    vfSetMonitor();

    /*---------- ctrl-c off ----------*/
#ifndef        UNIX
    SetStopKey();
#endif
    /*--------------- main loop ---------------*/
    iSelect = CSR_MOVE;
    do {
        /*----- display -----*/
        if (iSelect != ABORT) {
            vfDisplay(&sDAT, ubKey[0]);
			vfBoxDisplay(&sDAT);
		}
        /*---- get key code -----*/
        vfKey(ubKey);
        
        switch(ubKey[1]) {
            /*----- cursor move -----*/
            case    CSR_MOVE:
                iSelect = EDIT;
                vfCsrMove(&sDAT, ubKey[0]);
                break;
            
            /*----- command select -----*/
            case    COMMAND:
                iSelect = ifSelect(&sDAT, ubKey[0]);
                break;
        }
    } while(iSelect != QUIT);
    /*--------------- end of main loop ---------------/

    /*----- clear screen -----*/
    CLRSCR();
    
    /*----- cursor on -----*/
    CSRON();
    
#ifndef        UNIX
    /*----- ctrl-c on -----*/
    RcvStopKey();
#endif
#ifdef UNIX
    endwin();
#endif
    
    /*----- exit normal -----*/
    return    EXIT_SUCCESS;
}
