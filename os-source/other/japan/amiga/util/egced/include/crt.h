/*
 * $Id: crt.h,v 3.4.1.1 91/06/25 21:07:14 katogi GM Locker: katogi $
 */
/*
*
*     crt.h  :   screen control macros
*
*        Copyright(C) 1991 ERGOSOFT Corp. TOKIO Labo.
*        All Rights Reserved
*
*        Designed    by        H.Yanata  1991.Apr.04
*        Coded       by        H.Yanata  1991.Apr.04
*
*
*/

#if    !defined    __CRT_CTRL__
#define            __CRT_CTRL__


#define        CSRXY(x,y)        printf("\033[%d;%dH", (y), (x))
#define        CSRON()           printf("\033[>5l")
#define        CSRRVS()          printf("\033[7m")
#define        NORMCSR()         printf("\033[0m")
#define        CSROFF()          printf("\033[>5h")
#define        CLRSCR()          printf("\033[2J")
#define        BEEP()            printf("\007")
#define        VL8025()          printf("\033[>3l")

#define        WHITE             37
#define        BLUE              34
#define        RED               31
#define        MAGENTA           35
#define        GREEN             32
#define        CYAN              36
#define        YELLOW            33


#ifndef        UNIX
#define        VL8020()          printf("\033[>3h")
#define        CSR(mode)         printf("\033["#mode"m")
#endif

#endif
