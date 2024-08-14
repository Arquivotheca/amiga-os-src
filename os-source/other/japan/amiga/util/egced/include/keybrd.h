/*
 * $Id: keybrd.h,v 3.4.1.1 91/06/25 21:07:25 katogi GM Locker: katogi $
 */
/*---------------------------------------------------------------------------*/
/*                    KEYBORD ASCII CODE : 1BYTE OR 2BYTE                    */
/*---------------------------------------------------------------------------*/
/*
*/

#if !defined    __KEYCODE__
#define         __KEYCODE__
    
#ifdef UNIX
#define      UP       0x10
#define      DWN      0x0E
#define      RGT      0x06
#define      LFT      0x02
#else
#define      UP       0x0b
#define      DWN      0x0a  
#define      LFT      0x08  
#define      RGT      0x0c  
#endif     /* UNIX */

#define      TENUP    0x39
#define      TENDWN   0x33
#define      TENRGT   0x35
#define      TENLFT   0x37
#define      ESC      0x1b
#define      CR       0x0d
#define      RUP      0x11

#ifdef    UNIX
#define      RDWN     0x16
#else
#define      RDWN     0x1a
#endif     /* UNIX */

#define      BS       0x08
#define      TAB      0x09
#define      SPC      0x20
#define      RET      0x0d
#define      MENU     0x0d

#ifndef    UNIX
#define      AUTO_UP        0x2f
#define      AUTO_DWN       0x2a
#define      AUTO_PAGE_UP   0x2b
#define      AUTO_PAGE_DWN  0x3d
#endif    /*    UNIX    */

#endif    /*    __KEY_CODE__    */
