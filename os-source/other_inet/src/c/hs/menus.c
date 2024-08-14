/* -----------------------------------------------------------------------
 * menus.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: menus.c,v 1.1 91/05/09 16:32:08 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/menus.c,v 1.1 91/05/09 16:32:08 bj Exp $
 *
 * $Log:	menus.c,v $
 * Revision 1.1  91/05/09  16:32:08  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

#include    "termall.h"
#include    "arexxmenu.h"
#include    "transfermenu.h"
#include    "phonemenu.h"
#include    "terminalmenu.h"
#include    "setupmenu.h"
#include    "projectmenu.h"
#include    "titlemenu.h"
#include    "fcntl.h"
#include    "rexx/storage.h"


/*
* Menu definition
*/

/*----------------------------------------------------------------------------*/

void InitMenu ()
  {
    /*
    * Set the Menu strip
    */
    if ( Screen )
      {
        SetMenuStrip ( Window, &project );
        if ( FindPort ( "REXX" ) )
            arexxitem.Flags |=  ITEMENABLED;
        else
            arexxitem.Flags &= ~ITEMENABLED;
      }
  }

/*----------------------------------------------------------------------------*/

/***
*
* Process IDCMP Request
*
***/
void ProcIDCMPReq ( class, code )
unsigned long   class;
unsigned short  code;
  {
    struct MenuItem     *ItemAddress (),
                        *item;
    unsigned short      menu_number;
    unsigned short      color;
    unsigned long int   line;
    unsigned char       extension[32];

    switch ( class )
      {
        case ACTIVEWINDOW :
            memcpy ( Title_line + 35, "Active", 6 );
            ShowTitle ( Screen, TRUE );
            break;
        case INACTIVEWINDOW :
            memcpy ( Title_line + 35, "      ", 6 );
            ShowTitle ( Screen, TRUE );
            break;
        case MENUPICK :
            menu_number = code;
            while ( menu_number != MENUNULL )
              {
                item = ItemAddress ( &project, (long) menu_number );
                switch ( MENUNUM(menu_number) )
                  {
                    case 0: /* Project Menu */
                        switch ( ITEMNUM( menu_number ) )
                          {
                            case 0:  /* Quit Option */
                                if ( !ScreenSafe () )
                                    break;
                                Quitflag = 1;
                                break;
                            case 1: /* Save Parameters  As ... */
                                SetPattern ( "parms" );
                                if ( !FileReq ( "Save Parameters into file",
                                          Nvr ) )
                                    break;
                                /*
                                if ( !GetStringFromUser ( 
                                     "Save Parameters into file", Nvr,
                                     FNAMESIZE ) )
                                    break;
                                */
                                stcgfe ( extension, Nvr );
                                if ( stricmp ( extension, "parms" ) != 0 )
                                    strcat ( Nvr, ".parms" );
                                SaveNVR ();
                                break;
                            case 2: /* Load Parameters  As ... */
                                if ( !ScreenSafe () )
                                    break;
                            
                                SetPattern ( "parms" );
                                if ( !FileReq ( "Load Parameters from file",
                                          Nvr ) )
                                    break;
                                /*
                                if ( !GetStringFromUser ( 
                                     "Load Parameters from file",Nvr,
                                     FNAMESIZE ) )
                                    break;
                                */
                                stcgfe ( extension, Nvr );
                                if ( stricmp ( extension, "parms" ) != 0 )
                                    strcat ( Nvr, ".parms" );
                                LoadNVR ();
                                RemoveCursor ();
                                PlaceCursor ();
                                RefreshMenu ();
                                ConfigureSerialPorts ();
                                SetColors ();
                                ConfigureScreen ();
                                break;
                            case 3: /* Alter function key definitions. */
                                SetFunctionKeys ();
                                break;
                            case 4: /* Set Colourrs */
                                if ( Screen->BitMap.Depth == 2 )
                                  {
									StopBlinkTask ();
                                    color = nvmodes.color3;
                                    SetRGB4 ( VPort, ( 1L << Screen->BitMap.Depth ) - 1,
                                            (long)color >> 8, ( color >> 4 ) & 0xfL, color & 0xfL );
                                  }
        
                                DoColorWindow ( Screen, 200, 80, 0, 1 );
                                SaveColors ();
                                if ( Screen->BitMap.Depth == 2 )
                                  {
                                    nvmodes.color3 = GetRGB4 ( VPort->ColorMap,
                                          ( 1L << Screen->BitMap.Depth ) - 1 );
									StartBlinkTask ();
                                  }
        
                                break;
                            case 5: /* Reset parameters */
                                if ( !ScreenSafe () )
                                    break;
                                TermInit ();
                                RefreshMenu ();
                                ConfigureSerialPorts ();
                                SetColors ();
                                ConfigureScreen ();
                                StopSerialTasks ();
                                Vcls ( 1, vcb.screenlen );
                                Vlocate ( 1, 1);
                                StartSerialTasks ();
                                break;
                            case 6: /* Send Break      */
                                ser_req.IOSer.io_Flags  &= ~IOF_QUICK;
                                ser_req.IOSer.io_Command = SDCMD_BREAK;
                                DoIO ( (struct IORequest *) &ser_req );
                                break;
                            case 7: /* Create Icons? */
                                switch ( SUBNUM( menu_number ) )
                                  {
                                    case 0:
                                        nvmodes.icons = RESET;
                                        break;
                                    case 1:
                                        nvmodes.icons = SET;
                                        break;
                                  }
                                break;
                            case 8: /* Save screen to file */
                                switch ( SUBNUM( menu_number ) )
                                  {
                                    case 0:
                                        if ( Imagefilehandle )
                                          {
                                            _dclose ( Imagefilehandle );
                                            Imagefilehandle = 0;
                                          }
                                        if ( access ( Imagefile, 0 ) == -1 )
                                            strcpy ( Imagefile, "" );
                                        if ( !FileReq (
                                            "Write Screen Images to file",
                                            Imagefile ) )
                                            break;
                                        Imagefilehandle =
                                            _dopen ( Imagefile, MODE_NEWFILE );
                                        if ( !Imagefilehandle )
                                          {
                                            Error ( "Failed to open Imagefile" );
                                            break;
                                          }
                                        screenclose.Flags  |= ITEMENABLED;
                                        screenappend.Flags |= ITEMENABLED;
                                        break;
                                    case 1:
                                        if ( Imagefilehandle )
                                          {
                                            _dclose ( Imagefilehandle );
                                            Imagefilehandle = 0;
                                          }
                                        screenclose.Flags  &= ~ITEMENABLED;
                                        screenappend.Flags &= ~ITEMENABLED;
                                        break;
                                    case 2:
                                      {
                                        unsigned char line_image[133];
                                        unsigned char *ptr;
                                        
                                        if ( !Imagefilehandle )
                                          {
                                            Error ( "No Image File open" );
                                            break;
                                          }
                                        
                                        for ( line = 0;
                                              line < vcb.screenlen;
                                              line++ )
                                          {
                                            strncpy ( line_image, 
                                                      ScreenImage + line * 132,
                                                      vcb.linelen );
                                            line_image[vcb.linelen] = '\0';
                                            
                                            ptr = strrev ( line_image );
                                            ptr = stpblk ( ptr );
                                            ptr = strrev ( ptr );
                                            _dwrite ( Imagefilehandle,
                                                      ptr,
                                                      (unsigned) strlen ( ptr ) );
                                            _dwrite ( Imagefilehandle, "\n", 1L );
                                          }
                                        _dwrite ( Imagefilehandle,
                                                  "<<<<<-- END OF SCREEN -->>>>>\n",
                                                  30L );
                                      }
                                        break;
                                    case 3:
                                        ToPrinter ( 1, vcb.screenlen, 0 );
                                        break;
                                  }
                                break;
                            case 9: /* About Handshake */
                                ShowAboutRequester ();
                                WaitForAbout ();
                                RemoveAboutRequester ();
                                break;
                            case 10: /* FileRequester selection */
                                nvmodes.freqtype = SUBNUM ( menu_number );
                                break;
                            case 11: /* Printer submenu */
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0:
                                        strcpy ( nvmodes.printer, "PRT:" );
                                        screenprint.Flags |= ITEMENABLED;
                                        break;
                                    case 1:
                                        strcpy ( nvmodes.printer, "PAR:" );
                                        screenprint.Flags |= ITEMENABLED;
                                        break;
                                    case 2:
                                        screenprint.Flags &= ~ITEMENABLED;
                                        *nvmodes.printer = '\0';
                                        break;
                                  }
                                break;
                            case 12: /* Font type */
                                GetStringFromUser (
                                    "Enter Font Name",
                                    nvmodes.font,
                                    31 );
                                CloseFonts ();
                                FontInit ();
                                ConfigureScreen ();
                                break;
                            
                          }
                        break;
                    case 1: /* Setup Menu   */
                        switch ( ITEMNUM( menu_number ) )
                          {
                            case 0 : /* Baud rate SubMenu   */
                                switch ( SUBNUM( menu_number ) )
                                  { 
                                    case 0 : /*  300 baud */
                                        nvmodes.lspeed = 300;
                                        break;
                                    case 1 : /* 450 baud */
                                        nvmodes.lspeed = 450;
                                        break;
                                    case 2 : /* 600 baud */
                                        nvmodes.lspeed = 600;
                                        break;
                                    case 3 : /* 1200 baud */
                                        nvmodes.lspeed = 1200;
                                        break;
                                    case 4 : /* 1800 baud */
                                        nvmodes.lspeed = 1800;
                                        break;
                                    case 5 : /* 2400 baud */
                                        nvmodes.lspeed = 2400;
                                        break;
                                    case 6 : /* 3600 baud */
                                        nvmodes.lspeed = 3600;
                                        break;
                                    case 7 : /* 4800 baud */
                                        nvmodes.lspeed = 4800;
                                        break;
                                    case 8 : /* 9600 baud */
                                        nvmodes.lspeed = 9600;
                                        break;
                                    case 9 : /* 19200 baud */
                                        nvmodes.lspeed = 19200;
                                        break;
                                  }
                                break;
                            case 1 : /* Data Bit SubMenu */
                                switch ( SUBNUM( menu_number ) )
                                  {
                                    case 0 : /* Seven data bits */
                                        nvmodes.bpc = 7;
                                        break;
                                    case 1 : /* Eight data bits */
                                        nvmodes.bpc = 8;
                                        break;                    
                                  }
                                break;
                            case 2 : /* Stop Bit SubMenu */
                                nvmodes.stpbits = SUBNUM( menu_number );
                                break;
                            case 3 : /* Parity SubMenu */
                                nvmodes.parity = SUBNUM( menu_number );
                                break;
                            case 4 : /* Handshaking */
                                close_ports = 1;
                                switch  ( SUBNUM( menu_number ) )
                                  {
                                    case 0 :
                                        nvmodes.seven_wire = 1;
                                        break;
                                    case 1 :
                                        nvmodes.seven_wire = 0;
                                        break;
                                  }
                                break;
                            case 5 : /* Flow control */
                                switch ( SUBNUM( menu_number ) )
                                  {
                                    case 0 : /* XON XOFF */
                                        nvmodes.axonxoff = 1;
                                        break;
                                    case 1 : /* none     */
                                        nvmodes.axonxoff = 0;
                                        break;
                                  }
                                break;
			                case 6 :
                                close_ports = 1;
			    	            nvmodes.serial_port = SUBNUM( menu_number );
				                break;
                            case 7:
                                close_ports = 1;
                                GetStringFromUser ( "Serial Device Driver",
                                                    nvmodes.serial_driver,
                                                    256 );
                          }
                        ConfigureSerialPorts ();
                        close_ports = 0;
                        break;
                    case 2: /* Terminal Menu */
                        switch ( ITEMNUM(menu_number ) )
                          {
                            case 0 : /* Mode SubMenu */
                                switch ( SUBNUM( menu_number ) )
                                  {
                                    case 0 : /* VT52 */
                                        nvmodes.decanm = VT52;
                                        tmodes.controls  =
                                        tmodes.ocontrols = 7;
                                        break;
                                    case 1 : /* VT100 */
                                        nvmodes.decanm = VT100;
                                        tmodes.controls  =
                                        tmodes.ocontrols = 7;
                                        g[0] = &NormalFont;
                                        g[1] = &SGCSFont;
                                        glset = 0;
                                        grset = 1;
                                        break;
                                    case 2 : /* VT200 7 bit controls */
                                        nvmodes.decanm   = VT200_7;
                                        tmodes.controls  =
                                        tmodes.ocontrols = 7;
                                        g[0] = &NormalFont;
                                        g[1] = &SGCSFont;
                                        g[2] = &DSCSFont;
                                        glset = 0;
                                        grset = 2;
                                        break;
                                    case 3: /* VT200 8 bit controls */
                                        nvmodes.decanm   = VT200_8;
                                        tmodes.controls  =
                                        tmodes.ocontrols =
                                        nvmodes.bpc      = 8;
                                        g[0] = &NormalFont;
                                        g[1] = &SGCSFont;
                                        g[2] = &DSCSFont;
                                        glset = 0;
                                        grset = 2;
                                        break;
                                  }
                                so_msg.opcode = SER_OUT_SET_KEYBOARD;
                                PutMsg ( so_task_port, (struct Message *) &so_msg );
                                break;
                            case 1 : /* Wrap Submenu */
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0 : /* Off */
                                        nvmodes.decawm = SET;
                                        break;
                                    case 1 : /* On  */
                                        nvmodes.decawm = RESET;
                                        break;
                                  }
                                break;
                            case 2: /* Scroll Submenu */
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0 : /* Jump */
                                        tmodes.decsclm = RESET;
                                        break;
                                    case 1 : /* Smooth */
                                        tmodes.decsclm = SET;
                                        break;
                                  }
                                break;
                            case 3: /* Cursor Type Submenu */
                                RemoveCursor ();
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0: /* Block        */
                                        nvmodes.ctype &= 0x02;
                                        break;
                                    case 1: /* Underline    */
                                        nvmodes.ctype |= 0x01;
                                        break;
                                    case 2: /* Non-Blinking     */
                                        nvmodes.ctype &= 0x01;
                                        break;
                                    case 3: /* Blinking */
                                        nvmodes.ctype |= 0x02;
                                        break;
                                  }
                                PlaceCursor ();
                                break;
                            case 4 : /* Local Echo Submenu */
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0 : /* On */
                                        nvmodes.echo = SET;
                                        break;
                                    case 1 : /* Off  */
                                        nvmodes.echo = RESET;
                                        break;
                                  }
                                break;
                            case 5: /* Set Tabs */
                                if ( !TabRequest ( nvmodes.tabvec ) )
                                    break;
                                TabSet ();
                                break;
                            case 6: /* Answer back message */
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    unsigned char *ptr;
                                    
                                    case 0:
                                        GetStringFromUser (
                                        "Answer Back Message",
                                        nvmodes.answrbck, 20 );
                                        break;
                                    case 1:
                                        for ( ptr = nvmodes.answrbck; *ptr;
                                              SendChar ( *ptr++ ) );
                                        break;
                                  }
                                break;
                            case 7 : /* Character set Submenu */
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0 : /* US */
                                        nvmodes.ukset = RESET;
                                        break;
                                    case 1 : /* UK */
                                        nvmodes.ukset = SET;
                                        break;
                                  }
                                break;
                            case 8 : /* New Line Mode Submenu */
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0 : /* On */
                                        nvmodes.lnm = SET;
                                        break;
                                    case 1 : /* Off */
                                        nvmodes.lnm = RESET;
                                        break;
                                  }
                                break;
                            case 9 : /* Screen Mode Submenu */
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0 : /* NORMAL */
                                        if ( nvmodes.decscnm != RESET )
                                            SwapFgBg ();
                                        nvmodes.decscnm = RESET;
                                        break;
                                    case 1 : /* REVERSE */
                                        if ( nvmodes.decscnm != SET )
                                            SwapFgBg ();
                                        nvmodes.decscnm = SET;
                                        break;
                                  }
                                break;
                            case 10 : /* Columns Submenu */
                                home_cursor = 1;
                                SleepTasks ();

                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0 : /* 80 */
                                        vcb.linelen = 80;
                                        tmodes.deccolm = RESET;
                                        break;
                                    case 1 : /* REVERSE */
                                        vcb.linelen = 132;
                                        tmodes.deccolm = SET;
                                        break;
                                  }

                                WakeTasks ();
                                home_cursor = 0;
                                break;
                            case 11 : /* Interlace submenu */
                                if ( !ScreenSafe () )
                                    break;
                                nvmodes.decinlm = SUBNUM ( menu_number );
                                ConfigureScreen ();
                                break;
                            case 12 : /* Backspace delete submenu */
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0 : /* Normal */
                                        nvmodes.bkspdel = 1;
                                     SetNormalBksp ();
                                        break;
                                    case 1 : /* Reversed */
                                        nvmodes.bkspdel = 0;
                                     SetReverseBksp ();
                                        break;
                                  }
                                break;
                            case 13 : /* Bell type submenu */
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0 : /* ON */
                                        nvmodes.bell_type = 1;
                                        break;
                                    case 1 : /* OFF */
                                        nvmodes.bell_type = 2;
                                        break;
                                    case 2 : /* OFF */
                                        nvmodes.bell_type = 3;
                                        break;
                                    case 3 : /* OFF */
                                        nvmodes.bell_type = 0;
                                        break;
                                  }
                                break;
                            case 14 : /* Margin Bell submenu */
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0 : /* ON */
                                        nvmodes.marbell = SET;
                                        break;
                                    case 1 : /* OFF */
                                        nvmodes.marbell = RESET;
                                        break;
                                  }
                                break;
                            case 15 : /* Bit plane submenu */
                                if ( !ScreenSafe () )
                                    break;
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0 : /* 2 */
                                        nvmodes.bitplane= 2;
                                        break;
                                    case 1 : /* 3 */
                                        nvmodes.bitplane= 3;
                                        break;
                                  }
                                ConfigureScreen ();
                                break;
                          }
                        break;
                    case 3: /* Modem Menu */
                        switch ( ITEMNUM(menu_number ) )
                          {
                            case 0 : /* Dialing Requester */
                                if ( !DialRequest () )
                                    break;
                                redialitem.Flags |= ITEMENABLED;
                                break;
                            case 1 : /* Redial last number */
                                ReDial ();
                                break;
                            case 2 : /* Hangup phone */
                                HangUpPhone ();
                                break;
                            case 3 : /* Dialing Mode */
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0: /* Automatic */
                                        nvmodes.dial_mode = 1;
                                        break;
                                    case 1: /* Single dial */
                                        nvmodes.dial_mode  = 0;
                                        break;
                                  }
                                break;
                            case 4: /* Dialing string */
                                GetStringFromUser (
                                     "Modem dialing string", nvmodes.dial_str,
                                     63 );
                                break;
                            case 5: /* Dial Timeout   */
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0: /*  0 seconds */
                                        nvmodes.dialtime = 20;
                                        break;
                                    case 1: /* 30 seconds */
                                        nvmodes.dialtime = 30;
                                        break;
                                    case 2: /* 45 seconds */
                                        nvmodes.dialtime = 45;
                                        break;
                                    case 3: /* 60 seconds */
                                        nvmodes.dialtime = 60;
                                        break;
                                    case 4: /* 90 seconds */
                                        nvmodes.dialtime = 90;
                                        break;
                                    case 5: /* 120 seconds */
                                        nvmodes.dialtime = 120;
                                        break;
                                  }
                                break;
                            case 6: /* Redial Delay   */
                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0: /* 00 seconds */
                                        nvmodes.dialdelay = 0;
                                        break;
                                    case 1: /* 30 seconds */
                                        nvmodes.dialdelay = 30;
                                        break;
                                    case 2: /* 60 seconds */
                                        nvmodes.dialdelay = 60;
                                        break;
                                    case 3: /* 90 seconds */
                                        nvmodes.dialdelay = 90;
                                        break;
                                    case 4: /* 120 seconds */
                                        nvmodes.dialdelay = 120;
                                        break;
                                    case 5: /* 180 seconds */
                                        nvmodes.dialdelay = 180;
                                        break;
                                    case 6: /* 300 seconds */
                                        nvmodes.dialdelay = 300;
                                        break;
                                  }
                                break;
                            case 7: /* Hangup before dial */
                                nvmodes.hangup = SUBNUM ( menu_number );
                                break;
                          }
                        break;
                    case 4: /* Transfer Menu */
                        switch ( ITEMNUM(menu_number ) )
                          {
                            /* ASCII end of line processing parms. */
                            case 0 :
                                nvmodes.txcr = SUBNUM ( menu_number );
                                break;
                            case 1 :
                                nvmodes.txnl = SUBNUM ( menu_number );
                                break;
                            case 2 :
                                nvmodes.rxcr = SUBNUM ( menu_number );
                                break;
                            case 3 :
                                nvmodes.rxnl = SUBNUM ( menu_number );
                                break;
                            case 4 : /* Begin ASCII receive number */
                                if ( !BeginASCIIReceive ( NULL ) )
                                    break;
                                captureonitem.Flags     &= ~ITEMENABLED;
                                captureoffitem.Flags    |= ITEMENABLED;
                                asciisenditem.Flags     &= ~ITEMENABLED;
                                break;
                            case 5 : /* End ASCII Receive */
                                EndASCIIReceive ();
                                captureonitem.Flags     |= ITEMENABLED;
                                captureoffitem.Flags    &= ~ITEMENABLED;
                                asciisenditem.Flags     |= ITEMENABLED;
                                break;
                            case 6 : /* Begin ASCII Send     */
                                BeginASCIISend ( NULL );
                                break;
                            case 7 : /* Binary protocol selection*/
                                xprinititem.Flags &= ~ITEMENABLED;

                                switch ( SUBNUM ( menu_number ) )
                                  {
                                    case 0:
                                        nvmodes.protocol = XMODEM;
                                        break;
                                    case 1:
                                        nvmodes.protocol = XMODEM_CRC;
                                        break;
                                    case 2:
                                        nvmodes.protocol = YMODEM;
                                        break;
                                    case 3:
                                        nvmodes.protocol = YMODEM_BATCH;
                                        break;
                                    case 4:
                                        nvmodes.protocol = KERMIT;
                                        break;
                                    case 5:
                                        nvmodes.protocol = KERMIT_7BIT;
                                        break;
                                    case 6:
                                        nvmodes.protocol = KERMIT_TEXT;
                                        break;
                                    case 7:
										if (!*nvmodes.xprotocol)
                                            strcpy ( nvmodes.xprotocol, "LIBS:" );

                                        SetPattern ( "^xpr" );
                                        if ( !FileReq ( "Select External Protocol",
                                             nvmodes.xprotocol ) )
                                            break;

                                        nvmodes.protocol = XPROTOCOL;
                                        InitializeXProtocol ();
                                        xprinititem.Flags |= ITEMENABLED;
                                        break;
                                  }
                                break;
                            case 8 : /* Begin Binary Receive */
                                ReceiveFile ( (unsigned short int)
                                              nvmodes.protocol, NULL );
                                break;
                            case 9 : /* Begin Binary Send    */
                                TransmitFile ( (unsigned short int )
                                               nvmodes.protocol, NULL );
                                break;
                            case 10 :
                                XProtocolParams ();
                                break;
                          }
                        break;
                    case 5: /* AREXX Menu */
                        switch ( ITEMNUM(menu_number ) )
                          {
                            case 0:
                              {
                                SetPattern ( "rexx" );
                                if ( !FileReq ( "Select Macro to Run",
                                          nvmodes.rexx_file ) )
                                    break;
                                asyncRexxCmd ( RXCOMM, nvmodes.rexx_file );
                                break;
                              }
                            case 1:
                              {
                                unsigned char buffer[40];
                                
                                sprintf ( buffer, "NewWSH CON:S%08lx/300/20/320/50/WShell/c", (long)Screen );
                                if ( system ( buffer ) != 0 )
                                  {
                                    sprintf ( buffer, "NewShell CON:S%08lx/300/20/320/50/Shell", (long)Screen );
                                    if ( system ( buffer ) != 0 )
                                      {
                                        sprintf ( buffer, "NewCLI CON:S%08lx/300/20/320/50/CLI", (long)Screen );
                                        if ( system ( buffer ) != 0 )
                                                Error ( "Unable to invoke a command window" );
                                      }
                                  }
                                break;
                              }
                            case 2:
                              tmodes.rexx_abort = 1;
                              break;
                          }
                        break;
                    default:
                        Error ( "Invalid Menu number" );
                        break;
                  }
                menu_number = item->NextSelect;             
              }
            break;
        default :
		  {
			unsigned char string[20];

            Error ( "Invalid IDCMP message type" );
			sprintf ( string, "%04x", class );
			Error ( string ); 
            break;
		  }
      }
  }

void UncheckMenu ( itemptr )
struct MenuItem *itemptr;
  {
    do
      {
        itemptr->Flags &= ~CHECKED;
        if ( itemptr->SubItem )
            UncheckMenu ( itemptr->SubItem );
      } while ( itemptr = itemptr->NextItem );
  }

void UncheckMenuStrip ( )
  {
    struct Menu *menuptr;

    menuptr = &project;
    do
      {
        UncheckMenu ( menuptr->FirstItem );
      } while ( menuptr = menuptr->NextMenu );
  }

void CheckFreq ()
  {
    if ( !ArpBase )
      {
        nvmodes.freqtype = 0;
      }
    else
        arpyes.Flags |=  ITEMENABLED;
        
    switch ( nvmodes.freqtype )
      {
        case 0:
            arpno.Flags  |=  CHECKED;
            break;
        case 1:
            arpyes.Flags |=  CHECKED;
            break;
      }
  }
  
void CheckPrinter ()
  {
    if ( strcmp ( nvmodes.printer, "PRT:" ) == 0 )
      {
        printprt.Flags  |=  CHECKED;
        screenprint.Flags |= ITEMENABLED;
      }
    else if ( strcmp ( nvmodes.printer, "PAR:" ) == 0 )
      {
        printpar.Flags  |=  CHECKED;
        screenprint.Flags |= ITEMENABLED;
      }
    else
      {
        *nvmodes.printer =  '\0';
        printnone.Flags |=  CHECKED;
        screenprint.Flags &= ~ITEMENABLED;
      }
  }

void CheckIcons ()
  {
    switch ( nvmodes.icons )
      {
        case RESET :
            iconno.Flags  |=  CHECKED;
            break;
        case SET :
            iconyes.Flags |=  CHECKED;
            break;
      }
  }

void CheckBaudRate ()
  {
    switch ( nvmodes.lspeed )
      {
        case 300:
            b300.Flags |= CHECKED;
            break;
        case 450:
            b450.Flags |= CHECKED;
            break;
        case 600:
            b600.Flags |= CHECKED;
            break;
        case 1200:
            b1200.Flags |= CHECKED;
            break;
        case 1800:
            b1800.Flags |= CHECKED;
            break;
        case 2400:
            b2400.Flags |= CHECKED;
            break;
        case 3600:
            b3600.Flags |= CHECKED;
            break;
        case 4800:
            b4800.Flags |= CHECKED;
            break;
        case 9600:
            b9600.Flags |= CHECKED;
            break;
        case 19200:
            b19200.Flags |= CHECKED;
            break;
      }
  }

void CheckDataBits ()
  {
    switch ( nvmodes.bpc )
      {
        case 7:
            db7.Flags |= CHECKED;
            break;
        case 8:
            db8.Flags |= CHECKED;
            break;
      }
  }

void CheckStopBits ()
  {
    switch ( nvmodes.stpbits )
      {
        case 0:
            sb0.Flags |=  CHECKED;
            break;
        case 1:
            sb1.Flags |=  CHECKED;
            break;
        case 2:
            sb2.Flags |= CHECKED;
            break;
      }
  }

void CheckParity ()
  {
    switch ( nvmodes.parity )
      {
        case 0:
            pnone.Flags  |=  CHECKED;
            break;
        case 1:
            podd.Flags   |=  CHECKED;
            break;
        case 2:
            peven.Flags  |=  CHECKED;
            break;
        case 3:
            pmark.Flags  |=  CHECKED;
            break;
        case 4:
            pspace.Flags |=  CHECKED;
            break;
      }
  }

void CheckHandshake ()
  {
    switch ( nvmodes.seven_wire )
      {
        case 0:
            con3wire.Flags |=  CHECKED;
            break;
        case 1:
            con7wire.Flags |=  CHECKED;
            break;
      }
  }

void CheckFlow ()
  {
    switch ( nvmodes.axonxoff )
      {
        case 0:
            xonxoffoff.Flags |= CHECKED;
            break;
        case 1:
            xonxoffon.Flags  |= CHECKED;
            break;
      }
  }

void CheckPort ()
  {
    switch ( nvmodes.serial_port )
      {
        case 0:
            port0.Flags |=  CHECKED;
            break;
        case 1:
            port1.Flags |=  CHECKED;
            break;
        case 2:
            port2.Flags |=  CHECKED;
            break;
        case 3:
            port3.Flags |=  CHECKED;
            break;
        case 4:
            port4.Flags |=  CHECKED;
            break;
        case 5:
            port5.Flags |=  CHECKED;
            break;
        case 6:
            port6.Flags |=  CHECKED;
            break;
        case 7:
            port7.Flags |=  CHECKED;
            break;
        case 8:
            port8.Flags |=  CHECKED;
            break;
      }
  }

void CheckMode ()
  {
    switch ( nvmodes.decanm )
      {
        case VT52:
            vt52.Flags    |= CHECKED;
            break;
        case VT100:
            vt100.Flags   |= CHECKED;
            break;
        case VT200_7:
            vt200_7.Flags |= CHECKED;
            break;
        case VT200_8:
            vt200_8.Flags |= CHECKED;
            break;
      }
  }

void CheckWrap ()
  {
    switch ( nvmodes.decawm )
      {
        case RESET:
            wrpoff.Flags |= CHECKED;
            break;
        case SET:
            wrpon.Flags  |= CHECKED;
            break;
      }
  }

void CheckEcho ()
  {
    switch ( nvmodes.echo )
      {
        case RESET:
            echooff.Flags |= CHECKED;
            break;
        case SET:
            echoon.Flags  |= CHECKED;
            break;
      }
  }

void CheckScroll ()
  {
    switch ( tmodes.decsclm )
      {
        case RESET:
            scrfast.Flags |= CHECKED;
            break;
        case SET:
            scrslow.Flags |= CHECKED;
            break;
      }
  }

void CheckCursor ()
  {
    if ( nvmodes.ctype & 0x01 )
        cursorul.Flags |= CHECKED;
    else
        cursorbk.Flags |= CHECKED;
    
    if ( nvmodes.ctype & 0x02 )
        cursorblink.Flags   |= CHECKED;
    else
        cursorsteady.Flags  |= CHECKED;
  }

void CheckCharSet ()
  {
    switch ( nvmodes.ukset )
      {
        case RESET:
            charsetUS.Flags |= CHECKED;
            break;
        case SET:
            charsetUK.Flags |= CHECKED;
            break;
      }
  }

void CheckNewLine ()
  {
    switch ( nvmodes.lnm )
      {
        case RESET:
            newlineoff.Flags |= CHECKED;
            break;
        case SET:
            newlineon.Flags  |= CHECKED;
            break;
      }
  }

void CheckScreen ()
  {
    switch ( nvmodes.decscnm )
      {
        case RESET:
            screennormal.Flags   |= CHECKED;
            break;
        case SET:
            screenreverse.Flags  |= CHECKED;
            break;
      }
  }

void CheckColumns ()
  {
    switch ( tmodes.deccolm )
      {
        case RESET:
            column80.Flags   |= CHECKED;
            break;
        case SET:
            column128.Flags  |= CHECKED;
            break;
      }
  }

void CheckInterlace ()
  {
    switch ( nvmodes.decinlm )
      {
        case 0:
            interlaceoff.Flags   |=  CHECKED;
            break;
        case 1:
            interlaceon.Flags    |=  CHECKED;
            break;
        case 2:
            interlacefull.Flags  |=  CHECKED;
            break;
        case 3:
            lines48.Flags        |=  CHECKED;
            break;
      }
  }

void CheckBkspDel ()
  {
    switch ( nvmodes.bkspdel )
      {
        case 0 :
            bkdelrev.Flags  |= CHECKED;
            break;
        case 1 :
            bkdelnorm.Flags |= CHECKED;
            break;
      }
  }

void CheckBellType ()
  {
    unsigned char   err_msg[41];
    
    switch ( nvmodes.bell_type )
      {
        case 0 :
            bellnone.Flags   |= CHECKED;
            break;
        case 1 :
            bellaudio.Flags  |= CHECKED;
            break;
        case 2 :
            bellvisual.Flags |= CHECKED;
            break;
        case 3 :
            bellboth.Flags   |= CHECKED;
            break;
        default:
            sprintf ( err_msg, "Bell type invalid <%d>", nvmodes.bell_type );
            Error ( err_msg );
            break;
      }
  }

void CheckMarBell ()
  {
    switch ( nvmodes.marbell )
      {
        case SET:
            marbellon.Flags  |= CHECKED;
            break;
        case RESET :
            marbelloff.Flags |= CHECKED;
            break;
      }
  }

void CheckBitPlane ()
  {
    switch ( nvmodes.bitplane )
      {
        case 2:
            bitplane2.Flags |= CHECKED;
            break;
        case 3 :
            bitplane4.Flags |= CHECKED;
            break;
      }
  }

void CheckDialMode ()
  {
   switch ( nvmodes.dial_mode )
      {
        case 0:
            autodialoff.Flags |=  CHECKED;
            break;
        case 1:
            autodialon.Flags  |=  CHECKED;
            break;
      }
  }

void CheckDialTime ()
  {
   switch ( nvmodes.dialtime )
      {
        case 20:
            dialtime20.Flags   |=  CHECKED;
            break;
        case 30:
            dialtime30.Flags   |=  CHECKED;
            break;
        case 45:
            dialtime45.Flags   |=  CHECKED;
            break;
        case 60:
            dialtime60.Flags   |=  CHECKED;
            break;
        case 90:
            dialtime90.Flags   |=  CHECKED;
            break;
        case 120:
            dialtime120.Flags  |=  CHECKED;
            break;
      }
  }

void CheckDialDelay ()
  {
   switch ( nvmodes.dialdelay )
      {
        case 0:
            dialdelay0.Flags    |=  CHECKED;
            break;
        case 30:
            dialdelay30.Flags   |=  CHECKED;
            break;
        case 60:
            dialdelay60.Flags   |=  CHECKED;
            break;
        case 90:
            dialdelay90.Flags   |=  CHECKED;
            break;
        case 120:
            dialdelay120.Flags  |=  CHECKED;
            break;
        case 180:
            dialdelay180.Flags  |=  CHECKED;
            break;
        case 300:
            dialdelay300.Flags  |=  CHECKED;
            break;
      }
  }

void CheckHangUp ()
  {
    switch ( nvmodes.hangup )
      {
        case 0 :
            hangupno.Flags  |=  CHECKED;
            break;
        case 1 :
            hangupyes.Flags |=  CHECKED;
            break;
      }
  }

void CheckTxCR ()
  {
    switch ( nvmodes.txcr )
      {
        case 0 :
            txcrcrnl.Flags   |= CHECKED;
            break;
        case 1 :
            txcrcr.Flags     |= CHECKED;
            break;
        case 2 :
            txcrnl.Flags     |= CHECKED;
            break;
        case 3 :
            txcrignore.Flags |= CHECKED;
            break;
      }
  }

void CheckTxNL ()
  {
    switch ( nvmodes.txnl )
      {
        case 0 :
            txnlcrnl.Flags   |= CHECKED;
            break;
        case 1 :
            txnlcr.Flags     |= CHECKED;
            break;
        case 2 :
            txnlnl.Flags     |= CHECKED;
            break;
        case 3 :
            txnlignore.Flags |= CHECKED;
            break;
      }
  }

void CheckRxCR ()
  {
    switch ( nvmodes.rxcr )
      {
        case 0 :
            rxcrcrnl.Flags   |= CHECKED;
            break;
        case 1 :
            rxcrcr.Flags     |= CHECKED;
            break;
        case 2 :
            rxcrnl.Flags     |= CHECKED;
            break;
        case 3 :
            rxcrignore.Flags |= CHECKED;
            break;
      }
  }

void CheckRxNL ()
  {
    switch ( nvmodes.rxnl )
      {
        case 0 :
            rxnlcrnl.Flags   |= CHECKED;
            break;

        case 1 :
            rxnlcr.Flags     |= CHECKED;
            break;

        case 2 :
            rxnlnl.Flags     |= CHECKED;
            break;

        case 3 :
            rxnlignore.Flags |= CHECKED;
            break;
      }
  }

void CheckProtocol ()
  {
    switch ( nvmodes.protocol )
      {
        case XMODEM:
            xmodemchecksum.Flags |=  CHECKED;
            break;

        case XMODEM_CRC:
            xmodemcrc.Flags      |=  CHECKED;
            break;

        case YMODEM:
            ymodem.Flags         |=  CHECKED;
            break;

        case YMODEM_BATCH :
            ymodembatch.Flags    |=  CHECKED;
            break;
            
        case KERMIT :
            kermit.Flags         |=  CHECKED;
            break;
            
        case KERMIT_7BIT :
            kermit7bit.Flags     |=  CHECKED;
            break;
            
        case KERMIT_TEXT :
            kermittext.Flags     |=  CHECKED;
            break;
            
        case XPROTOCOL :
            xprotocol.Flags      |=  CHECKED;
            break;
      }

    if ( nvmodes.protocol == XPROTOCOL )
        xprinititem.Flags |=  ITEMENABLED;
    else
        xprinititem.Flags &= ~ITEMENABLED;
  }

void CheckMenuStrip ()
  {
    CheckIcons   ();
    CheckFreq    ();
    CheckPrinter ();
    
    CheckBaudRate ();
    CheckDataBits ();
    CheckStopBits ();
    CheckParity ();
    CheckHandshake ();
    CheckFlow ();
    CheckPort ();

    CheckMode ();
    CheckWrap ();
    CheckScroll ();
    CheckEcho ();
    CheckCursor ();
    CheckCharSet ();
    CheckNewLine ();
    CheckScreen ();
    CheckColumns ();
    CheckInterlace ();
    CheckBkspDel ();
    CheckBellType ();
    CheckMarBell ();
    CheckBitPlane ();
    
    CheckDialMode ();
    CheckDialTime ();
    CheckDialDelay ();
    CheckHangUp ();

    CheckTxCR ();
    CheckTxNL ();
    CheckRxCR ();
    CheckRxNL ();
    CheckProtocol ();
  }

void RefreshMenu ()
  {
    UncheckMenuStrip ();
    CheckMenuStrip ();
  }