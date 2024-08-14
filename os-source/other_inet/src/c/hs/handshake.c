/* -----------------------------------------------------------------------
 * handshake.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: handshake.c,v 1.1 91/05/09 15:21:13 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/handshake.c,v 1.1 91/05/09 15:21:13 bj Exp $
 *
 * $Log:	handshake.c,v $
 * Revision 1.1  91/05/09  15:21:13  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* A not so simple terminal program for the Amiga.
*
***/

#include    "termall.h"
/* #include    "libraries/arpbase.h" */
#include    "rexx/storage.h"

static unsigned short int SerialPort = -1;
static unsigned char      SerialDriver[256];
static unsigned short int DisplayDisabled = 0;
extern BPTR               _Backstdout;

/*---------------------------------------------------------------------------*/

/***
*
* Main Task for HandShake. This task starts the input and output tasks and
* does the necessary window management.
*
***/
void __stdargs main ( argc, argv )
int    argc;
char   **argv;
  {
	unsigned char port_string[20];

	/*
	* Determine which copy of Handshake we are.
	*/
	for ( InvokationNumber = 1; TRUE ;InvokationNumber++ )
	  {
		sprintf ( port_string, "HANDSHAKE%d", InvokationNumber );
		if ( !FindPort ( port_string ) )
			break;
	  }
	
    /*
    * Find our own task control block
    */
    
    main_task = FindTask ( 0L );
    
    /*
    * Process the input arguments
    */
    
    if ( !argc )
      {
        /*
        * Started from workbench
        */
        nvmodes.bell_type = 1;
        ProcessWorkbenchArguments ( dial_descriptor );
      }
    else
      {
        /*
        * Started from command line
        */
        ProcessCommandLine ( argc, argv, dial_descriptor );
      }
      
    /***
    *
    * Open the libraries. The result returned by this call is
    * used to connect your program to the actual library routines
    * in ROM. If the result of this call is equal to zero, something
    * is wrong and the library you requested is not available.
    * so your program should exit immediately.
    *
    ***/

/* =========================== ARP MUST DIE! ====================
 *   if ( access ( "libs:arp.library", 0 ) )
 *       ArpBase = NULL;
 *   else
 *       ArpBase = (struct ArpBase * ) OpenLibrary ( ArpName, ArpVersion );
 * ========================================================*/


    IntuitionBase = (struct IntuitionBase *)
        OpenLibrary ( "intuition.library", INTUITION_REV );
    if ( IntuitionBase == NULL )
        ShutDown ();

    GfxBase = (struct GfxBase *)
        OpenLibrary ( "graphics.library", GRAPHICS_REV );
    if ( GfxBase == NULL )
        ShutDown ();
        
    /*
    * Initialize the Non-volatile memory.
    */
    
    NVMInit ();
    
    if ( SerialPort != -1 )
        nvmodes.serial_port = SerialPort;
        
    if ( *SerialDriver )
        strcpy ( nvmodes.serial_driver, SerialDriver );
        
    /*
    * Open the serial port
    */
    OpenSerialPort ();
    if ( _Backstdout )
      {
        Close ( _Backstdout );
        _Backstdout = NULL;
      }

    /*
    * Initialize the timer port.
    */
    timeout_port = InitTimer ( &timeout_req, "Timeout timer" );
    
    /*
    * Allocate a communications port for this task.
    */

    main_task_port = CreatePort ( /*"HandShake Main Port"*/ NULL, 0 );
    keyb_msg.vt_msg.mn_Node.ln_Type   = NT_MESSAGE;
    keyb_msg.vt_msg.mn_Length         = sizeof ( unsigned short int );
    keyb_msg.vt_msg.mn_ReplyPort      = main_task_port;
    echo_msg.vt_msg.mn_Node.ln_Type   = NT_MESSAGE;
    echo_msg.vt_msg.mn_Length         = sizeof ( unsigned short int );
    echo_msg.vt_msg.mn_ReplyPort      = main_task_port;
    blink1_msg.vt_msg.mn_Node.ln_Type = NT_MESSAGE;
    blink1_msg.vt_msg.mn_Length       = sizeof ( unsigned short int );
    blink1_msg.vt_msg.mn_ReplyPort    = main_task_port;
    si_msg.vt_msg.mn_Node.ln_Type     = NT_MESSAGE;
    si_msg.vt_msg.mn_Length           = sizeof ( unsigned short int );
    si_msg.vt_msg.mn_ReplyPort        = main_task_port;
    so_msg.vt_msg.mn_Node.ln_Type     = NT_MESSAGE;
    so_msg.vt_msg.mn_Length           = sizeof ( unsigned short int );
    so_msg.vt_msg.mn_ReplyPort        = main_task_port;
    
    /*
    * Open up the emulation window
    */
    
    if ( ! DisplayDisabled )
      {
        CharAttr    = HSAllocMem ( 132L * 48L, MEMF_CLEAR );
        if ( !CharAttr )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
    
        ScreenImage = HSAllocMem ( 132L * 48L, MEMF_CLEAR );
        if ( !ScreenImage )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
    
        ConfigureScreen ();
        ScreenInit ( vcb.screenlen, vcb.linelen );
    
        window_ptr = ( ( struct Process * )main_task )->pr_WindowPtr;
        ( ( struct Process * )main_task )->pr_WindowPtr = (APTR) Window;

        vcb.ln          = 15;
        vcb.col         = 1;
        vcb.cursor_ln   = 15;
        vcb.cursor_col  = 1;
        Vsetcup ();
        PlaceCursor ();

		/*
		Delay ( 50L ); /*ERH Weird Delay required to let Intuition Catch up */
		*/
    
        /*
        * Initialize fonts
        */
        FontInit ();

      }

    /*
    * Initialize Allocate storage for sub-task stacks.
    */
    if ( !DisplayDisabled )
      {
        blink1_stack  = HSAllocMem ( STACK_SIZE, MEMF_CLEAR );
        if ( !blink1_stack )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
      }

    ser_in_stack  = HSAllocMem ( STACK_SIZE, MEMF_CLEAR );
    if ( !ser_in_stack )
      {
        DisplayAlert ( AG_NoMemory, MemMessage, 60L );
        ShutDown ();
      }
      
    ser_out_stack = HSAllocMem ( STACK_SIZE, MEMF_CLEAR );
    if ( !ser_out_stack )
      {
        DisplayAlert ( AG_NoMemory, MemMessage, 60L );
        ShutDown ();
      }

    /*
    * Start the serial input output tasks.
    */
    StartSerialTasks ();

    /*
    * Start the blink task.
    */
    if ( !DisplayDisabled )
        StartBlinkTask ();

    /*
    * Set the menu to reflect the current state of affairs.
    */
    ConfigureSerialPorts ();

    /*
    * Draw the logo
    */
    if ( !DisplayDisabled )
        DrawLogo ();

    /*
    * Start up the AREXX port if possible
    */
    StartRexx ();

    /*
    * Dial initial number if specified on command line.
    */
    DialInitialNumber ( dial_descriptor );
    
    /*
    * Run initial AREXX macro if there is one.
    */
    if ( StartMacro )
        asyncRexxCmd ( RXCOMM, StartMacro );

    /*
    * Main loop
    */
    for (;;)
      {
        /*
        * Set the Menu strip
        */
        InitMenu ();
        IDCMP_in_progress = 0;

		/*
		ModifyIDCMP ( Window, MENUPICK | ACTIVEWINDOW | INACTIVEWINDOW |
							  GADGETUP | MENUVERIFY );
        */

        ProcessUntil ( IDCMP_EVENT );

        /*
        * Process all  the IDCMP messages we have received
        */
        if ( !DisplayDisabled )
          {
            IDCMP_in_progress = 1;
            while ( message = (struct IntuiMessage *) GetMsg ( Window->UserPort ) )
              {
                class   = message->Class;
                code    = message->Code;

				/*
				if ( class == MENUVERIFY )
				  {
					if ( code == MENUHOT )
					  {
						StopBlinkTask ();

						ModifyIDCMP ( Window, MENUPICK | ACTIVEWINDOW |
								          INACTIVEWINDOW | GADGETUP );
					  }
					ReplyMsg ( (struct Message *) message );
				  }
				else
				  {
				*/
					if ( class == MENUPICK )
					  {
						/*
						StartBlinkTask ();
						*/
						ClearMenuStrip ( Window );
					  }

					ReplyMsg ( (struct Message *) message );
                	ProcIDCMPReq ( class, code );
				/*
				  }
				*/

                /*
                * If a close request, go away nicely
                */
                if ( Quitflag )
                    ShutDown ();
              }
          }
      }
  }

/*---------------------------------------------------------------------------*/

void ProcessWorkbenchArguments ( dial_descriptor )
char    *dial_descriptor;
  {
    extern struct WBStartup     *WBenchMsg;
    BPTR                        olddir;
    char                        *DefaultTool;
    struct DiskObject           *diskobj;
    unsigned short int          i;

    if ( ( IconBase = OpenLibrary ( "icon.library", 0 ) ) == NULL )
      {
        Error ( "Could not open icon.library\n" );
        *dial_descriptor = 0x00;
        return;  
      }
      
    
    for ( i = 0; i < WBenchMsg->sm_NumArgs; i++ )
      {    
    
        olddir = CurrentDir ( WBenchMsg->sm_ArgList[i].wa_Lock );
        diskobj = GetDiskObject ( WBenchMsg->sm_ArgList[i].wa_Name );
    
        if ( diskobj == NULL )
          {
            Error ( "No Icon Found" );
            CurrentDir ( olddir );
            CloseLibrary ( (struct Library *) IconBase );
            break;
         }
    
        
        if ( diskobj->do_Type == WBPROJECT || diskobj->do_Type == WBTOOL )
          {
            DefaultTool = FindToolType ( diskobj->do_ToolTypes, "DIALUP" );
            if ( DefaultTool)
              {
                strncpy ( dial_descriptor, DefaultTool, 31 );
              }
    
            DefaultTool = FindToolType ( diskobj->do_ToolTypes, "MACRO" );
            if ( DefaultTool)
              {
                strncpy ( StartMacro, DefaultTool, 256 );
              }
    
            DefaultTool = FindToolType ( diskobj->do_ToolTypes, "PORT" );
            if ( DefaultTool)
              {
                int ivalue;
            
                stcd_i ( DefaultTool, &ivalue );
                if ( ivalue >= 0 && ivalue <= 8 )
                    SerialPort = ivalue;
              }
    
            DefaultTool = FindToolType ( diskobj->do_ToolTypes, "DRIVER" );
            if ( DefaultTool)
              {
                strncpy ( SerialDriver, DefaultTool, 256 );
              }

            DefaultTool = FindToolType ( diskobj->do_ToolTypes, "DISPLAY" );
            if ( DefaultTool)
              {
                if ( DefaultTool[1] == 'F' || DefaultTool[1] == 'f' )
                    DisplayDisabled = 1;
              }
          }
        CurrentDir ( olddir );
    
        strncpy ( Nvr, WBenchMsg->sm_ArgList[i].wa_Name, 256 );
        if ( diskobj->do_Type == WBTOOL )
            strcat ( Nvr, ".parms" );
        Nvr[ 256 ] = 0x00;
    
        FreeDiskObject ( diskobj );
      }
    CloseLibrary ( (struct Library *) IconBase );

    return;
  }

/*---------------------------------------------------------------------------*/

void ProcessCommandLine ( argc, argv, dial_descriptor )
int    argc;
char   *argv[];
char   *dial_descriptor;
  {
    int                 argn = 1;
    unsigned char       *optd,
                        optc;

    /*
    * Process the command line.
    */
    while ( optd = argopt ( argc, argv, "PpMm", &argn, &optc ) )
        switch ( (int) optc )
          {
            case 'M': /* specify rexx macro to run */
            case 'm':
                if ( optd )
                  {
                    strncpy (StartMacro, optd, 256 );
                    StartMacro[ 256 ] = 0x00;
                  }
                break;
            case 'P' : /* Select parameter file */
            case 'p' :
                if  ( optd )
                  {
                    strncpy ( Nvr, optd, 65 );
                    Nvr[ 65 ] = 0x00;
                  }
                break;
            case 'S': /* Select serial port */
            case 's':
              {
                int ivalue;
                stcd_i ( optd, &ivalue );
                if ( ivalue < 0 || ivalue > 8 )
                    break;
                SerialPort = ivalue;
                break;
              }
            case 'D': /* Select device driver */
            case 'd':
                strcpy ( SerialDriver, optd );
                break;

            case 'N':
            case 'n':
                DisplayDisabled = 1;
          }

    if ( argn < argc )
      {
        strncpy ( dial_descriptor, argv[ argn ], 31 );
        dial_descriptor[ 31 ] = 0x00;
      }
    else
        *dial_descriptor = 0x00;
  }

/*---------------------------------------------------------------------------*/

void ProcessUntil ( until_opcode )
unsigned short int until_opcode;
  {
    static VT_message           *temp_echo_msg = NULL,
                                *temp_keyb_msg = NULL;
    VT_message                  *main_msg      = NULL;

    unsigned long  int  signals;
    unsigned short int  current_opcode;

    /*
    * Wait for an IDCMP event or a request to flush the capture buffer.
    */
    /*printf ( "Waiting for %d...", until_opcode );
    */
    do
      {
        for ( ;; )
          {
            if (  until_opcode == IDCMP_EVENT && IDCMP_pending )
              {
                IDCMP_pending = 0;
                return;
              }

            if ( main_msg = (VT_message *) GetMsg ( main_task_port ) )
                break;
            else
              {
                if ( !DisplayDisabled )
                    signals = Wait ( 1L << main_task_port->mp_SigBit  |
                                    1L << Window->UserPort->mp_SigBit |
                                    RexxMask );
                else
                    signals = Wait ( 1L << main_task_port->mp_SigBit  |
                                    RexxMask );

                if ( !DisplayDisabled )
                    if ( signals & 1L << Window->UserPort->mp_SigBit )
                      {
                        /*
                        printf ( "IDCMP received\n" );
                        */
                        IDCMP_pending = 1;
                      }

                if ( signals & 1L << main_task_port->mp_SigBit )
                  {
                    main_msg = (VT_message *) GetMsg ( main_task_port );
                    if ( main_msg )
                        break;
                  }
                if ( signals & RexxMask )
                  {
                    dispRexxPort ();
                  }
              }
          }

        current_opcode = main_msg->opcode;
        /*
        printf ( "Processing %d from %08x\n", current_opcode, main_msg );
        */
        switch ( current_opcode )
          {
            case SI_SUBTASK_AWAKE:
                break;
            case SO_SUBTASK_AWAKE:
                break;
            case SI_SUBTASK_ASLEEP:
                break;
            case SO_SUBTASK_ASLEEP:
                break;
            case ECHO_CHARACTER:
                if ( Si_asleep )
                  {
                    main_msg->opcode = SER_OUT_ECHOED;
                    ReplyMsg ( (struct Message *) main_msg );
                  }
                else
                  {
                    temp_echo_msg = main_msg;
                    echo_msg.opcode = SER_IN_ECHO;
                    PutMsg ( si_task_port, (struct Message *) &echo_msg );
                  }
                break;
            case ECHO_COMPLETE:
                temp_echo_msg->opcode = SER_OUT_ECHOED;
                ReplyMsg ( (struct Message *) temp_echo_msg );
                break;
            case RESET_TERMINAL:
                ReplyMsg ( (struct Message *) main_msg );
                TermInit ();
                SetColors ();
                SetOrigFgBg ();
                if ( nvmodes.decscnm == SET )
                  {
                    SwapFgBg ();
                  }
                close_ports = 1;
                ConfigureSerialPorts ();
                close_ports = 0;
                RefreshMenu ();
                break;
            case SWITCH_KEYBOARD:
                if ( So_asleep )
                  {
                    main_msg->opcode = SER_IN_KEYBOARD_RESET;
                    ReplyMsg ( (struct Message *) main_msg );
                  }
                else
                  {
                    temp_keyb_msg = main_msg;
                    keyb_msg.opcode = SER_OUT_SWITCH_KEYBOARD;
                    PutMsg ( so_task_port, (struct Message *) &keyb_msg );
                  }
                break;
            case KEYBOARD_SWITCHED:
                temp_keyb_msg->opcode = SER_OUT_ECHOED;
                ReplyMsg ( (struct Message *) temp_keyb_msg );
                break;
            case FLUSH_CAPTURE_BUFFER:
                FlushCaptureBuffer ();
                main_msg->opcode = SER_IN_WAKE;
                ReplyMsg ( (struct Message *) main_msg );
                break;
            case PRINT_LINES:
                ToPrinter ( Print_from_line, Print_to_line, Print_form_feed );
                main_msg->opcode = SER_IN_WAKE;
                ReplyMsg ( (struct Message *) main_msg );
                break;
            case CONTROL_PRINT :
                switch ( tmodes.prtcntrl )
                  {
                    case 1:
                        _dwrite ( tmodes.printerhandle, &Print_char, 1L );
                        break;
                    case 2:
                        if ( tmodes.printerhandle )
                          {
                            _dclose ( tmodes.printerhandle );
                            tmodes.printerhandle = 0;
                          }
                        break;
                    case 3:
                        if ( !*nvmodes.printer )
                          {
                            Error ( "No printer selected" );
                            break;
                          }
                          
                        if ( tmodes.printerhandle )
                            break;
                            
                        tmodes.printerhandle = _dopen ( nvmodes.printer,
                                                        MODE_NEWFILE );
                        if ( !tmodes.printerhandle )
                          {
                            Error ( "Unable to open printer" );
                            break;
                          }
                        break;
                  }
                main_msg->opcode = SER_IN_WAKE;
                ReplyMsg ( (struct Message *) main_msg );
                break;
            case SHOW_TITLE:
                ShowTitle ( Screen, TRUE );
                main_msg->opcode = SER_IN_WAKE;
                ReplyMsg ( (struct Message *) main_msg );
                break;
            case RESTART_LINE:
                HandleLockedLine ( &ser_req );
                main_msg->opcode = SER_OUT_WAKE;
                ReplyMsg ( (struct Message *) main_msg );
                break;
            default:
                /*
                printf ( "INVALID OPCODE\n" );
                */
                break;
          }
      } while ( current_opcode != until_opcode );
  }

/*---------------------------------------------------------------------------*/

/***
*
* Shut down admin task
*
***/
void ShutDown ()
  {
    StopRexx ();
    ShutXProtocol ();
    
    if ( !DisplayDisabled )
        ( ( struct Process * ) main_task )->pr_WindowPtr = window_ptr;
    
    StopSerialTasks ();

    if ( Screen )
        StopBlinkTask ();

    if ( timeout_port )
      {
        CloseDevice ( (struct IORequest *) &timeout_req );
        DeletePort  ( timeout_port );
      }

    if ( ser_port )
      {
        CloseDevice ( (struct IORequest *) &ser_req );
        DeletePort  ( ser_port );
        DeletePort  ( temp_ser_port );
        Dtr ( 0 );
      }

    FTCBClose ();
    if ( Imagefilehandle )
      {
        _dclose ( Imagefilehandle );
        Imagefilehandle = 0;
      }

    if ( !DisplayDisabled )
      {
        CloseFonts ();

        if ( Window )
            CloseWindow ( Window );
        if ( Screen )
            CloseScreen ( Screen );
    
        if ( CharAttr )
            HSFreeMem ( CharAttr,    132L * 48L );
        if (ScreenImage )
            HSFreeMem ( ScreenImage, 132L * 48L );
      }

    if ( Screen && blink1_stack )
        HSFreeMem ( blink1_stack,  STACK_SIZE );
    if ( ser_in_stack )
        HSFreeMem ( ser_in_stack,  STACK_SIZE );
    if ( ser_out_stack )
        HSFreeMem ( ser_out_stack, STACK_SIZE );

    if ( ArpBase )
        CloseLibrary ( (struct Library *) ArpBase );
    if ( IntuitionBase )
        CloseLibrary ( (struct Library *) IntuitionBase );
    if ( GfxBase )
        CloseLibrary ( (struct Library *) GfxBase       );

    if ( main_task_port )
        DeletePort ( main_task_port );
        
    if ( _Backstdout )
        Close ( _Backstdout );
        
    _exit ( 0 );
 }

/*---------------------------------------------------------------------------*/

void NVMInit ()
  {
    int i;

    tmodes.dsplayon   = TRUE;
    tmodes.printing   = FALSE;
    tmodes.prtcntrl   = FALSE;
    tmodes.tracing    = FALSE;
    tmodes.ss2        = -1;
    tmodes.ss3        = -1;
    tmodes.kam        = RESET;
    tmodes.irm        = RESET;
    tmodes.srm        = SET;
    tmodes.decckm     = RESET;
    tmodes.deccolm    = RESET;
    tmodes.decsclm    = RESET;
    tmodes.decom      = RESET;
    tmodes.decarm     = SET;
    tmodes.deckpm     = RESET;
    tmodes.dectcem    = SET;
    tmodes.sol        = 1;
    tmodes.savedrc.r  = 1;
    tmodes.savedrc.c  = 1;
    tmodes.controls   = 7;
    tmodes.ocontrols  = 7;
    tmodes.font       = FASCII;
    tmodes.eraseattr  = ERASE;
    tmodes.fontstyle  = GR_NORMAL;
    nvmodes.serial_port = 0;
    nvmodes.decinlm   = 0;
    nvmodes.bitplane  = 2;
    nvmodes.decscnm   = RESET;
    nvmodes.decpff    = RESET;
    nvmodes.decpex    = SET;
    nvmodes.decanm    = VT100;
    nvmodes.ctype     = 2; /* Blinking block cursor */
    nvmodes.marbell   = RESET;
    nvmodes.bell_type = 3;
    nvmodes.ukset     = RESET;
    nvmodes.echo      = RESET;
    nvmodes.decawm    = RESET;
    nvmodes.lnm       = RESET;
    nvmodes.axonxoff  = RESET;
    nvmodes.bpc       = 0x08;
    nvmodes.lspeed    = 1200;
    nvmodes.stpbits   = 0x01;
    nvmodes.seven_wire= 0;
    nvmodes.termid    = RESET;
    nvmodes.txcr      = 1;
    nvmodes.txnl      = 2;
    nvmodes.rxcr      = 1;
    nvmodes.rxnl      = 2;
    nvmodes.hangup    = 1;
    strcpy(nvmodes.serial_driver, SERIALNAME );
    strcpy(nvmodes.answrbck,"");
    strcpy(nvmodes.dial_str,"ATDP");
    strcpy(nvmodes.dest_path,"");
    strcpy(nvmodes.pc_file,"");
    strcpy(nvmodes.font, "topaz.font");
    nvmodes.direction  = RESET;
    nvmodes.protocol   = XMODEM;
    nvmodes.dialtime   = 30;
    nvmodes.dialdelay  = 60;
    nvmodes.dial_mode  = 0;
    nvmodes.bkspdel    = 0;
    nvmodes.icons      = 0;
    nvmodes.pfk_cr     = '~';
    nvmodes.pfk_nl     = '`';
    for (i=0 ; i < 132 ; i++   ) nvmodes.tabvec[i] = ' ';
    for (i=0 ; i < 132 ; i += 8) nvmodes.tabvec[i] = 'T';
    close_ports = 0;
    home_cursor = 0;
    nvmodes.colormap[0] = 0x0000;
    nvmodes.colormap[1] = 0x000f;
    nvmodes.colormap[2] = 0x00f0;
    nvmodes.colormap[3] = 0x00ff;
    nvmodes.colormap[4] = 0x0f00;
    nvmodes.colormap[5] = 0x0f0f;
    nvmodes.colormap[6] = 0x0ff0;
    nvmodes.colormap[7] = 0x0fff;
    SetReverseBksp ();

    LoadNVR ();
    if ( access ( nvmodes.dest_path, 0 ) == -1 )
        strcpy ( nvmodes.dest_path, "" );
    if ( access ( nvmodes.pc_file,   0 ) == -1 )
        strcpy ( nvmodes.pc_file,   "" );
    glset=0;
    grset = ( nvmodes.decanm < VT200_7 ) ? 1 : 2;
    FTCBInit ();
    vcb.linelen = 80;
  }
    
/*---------------------------------------------------------------------------*/

void ScreenInit ( unsigned short int rows, unsigned short int cols )
  {
    VCBInit ( rows, cols );
    if ( nvmodes.decscnm == SET )
      {
        SwapFgBg ();
      }
      
    memset ( CharAttr,    FASCII , 132L * 48L );
    memset ( ScreenImage, ' ',     132L * 48L );
    
    RefreshMenu ();
  }

/*---------------------------------------------------------------------------*/

void TermInit ()
  {
    NVMInit ();
    ScreenInit ( (unsigned short int)((nvmodes.decinlm == 3) ? 48 : 24), 80);
  }

/*---------------------------------------------------------------------------*/

void OpenSerialPort ()
  {
    int error;

    ser_port = InitSerialIO ( &ser_req, "ser_admin" );
    if ( !ser_port )
      {
        if ( _Backstdout )
            Write ( _Backstdout, "Failed to open serial port\n", 27 );
        ShutDown ();
      }

    error = SetParams ( &ser_req, 1 );
    if ( error )
      {
        if ( _Backstdout )
            Write ( _Backstdout, "Failed set serial port parameters\n", 34 );
        ShutDown ();
      }

    temp_ser_req = ser_req;
    temp_ser_port = CreatePort (/*"temp serial"*/ NULL, 0 );
    temp_ser_req.IOSer.io_Message.mn_ReplyPort = temp_ser_port;
  }

/*---------------------------------------------------------------------------*/

void SleepTasks ( void )
  {
    /*
    * Put the Serial I/O subtasks to sleep, and have them close all
    * their serial ports.
    */
    
    if ( ser_in_task && !Si_asleep )
      {
        Si_asleep = 1;
        si_msg.opcode = SER_IN_SLEEP;
        PutMsg ( si_task_port, (struct Message *) &si_msg );
        ProcessUntil ( SI_SUBTASK_ASLEEP );
      }

    if ( ser_out_task && !So_asleep )
      {
        So_asleep = 1;
        so_msg.opcode = SER_OUT_SLEEP;
        PutMsg ( so_task_port, (struct Message *) &so_msg );
        ProcessUntil ( SO_SUBTASK_ASLEEP );
      }
    Delay ( 10 );
  }
    
/*---------------------------------------------------------------------------*/

void WakeTasks ( void )
  {
    /*
    * Wake up the Serial I/O Subtasks
    */

    if ( ser_out_task && So_asleep )
      {
        so_msg.opcode = SER_OUT_WAKE;
        PutMsg ( so_task_port, (struct Message *) &so_msg );
        ProcessUntil ( SO_SUBTASK_AWAKE );
      }

    if ( ser_in_task && Si_asleep )
      {
        si_msg.opcode = SER_IN_WAKE;
        PutMsg ( si_task_port, (struct Message *) &si_msg );
        ProcessUntil ( SI_SUBTASK_AWAKE );
      }
    Delay ( 10 );
  }

/*---------------------------------------------------------------------------*/

void ConfigureSerialPorts ()
  {
    int error;

    SleepTasks ();

    /*
    * Shut down all the serial ports if necessary.
    */
    if ( close_ports )
      {
        if ( ser_port )
          {
            CloseDevice ( (struct IORequest *) &ser_req );
            DeletePort  ( ser_port );
            ser_port = 0L;
          }
          
        /*
        * Reopen main task serial ports. Thereby setting the parms.
        */
        OpenSerialPort ();
        ser_in_req  = ser_req;
        ser_rp_req  = ser_req;
        ser_out_req = ser_req;
        ser_in_req.IOSer.io_Message.mn_ReplyPort  = ser_in_port;
        ser_rp_req.IOSer.io_Message.mn_ReplyPort  = ser_rp_port;
        ser_out_req.IOSer.io_Message.mn_ReplyPort = ser_out_port;
      }
    else
      {
        /*
        * Set the Serial I/O parameters without closing the main ports.
        */
        error = SetParams ( &ser_req, 1 );
        if ( error )
          {
            Error ( "Unable to set Serial Parameters" );
          }
      }

    WakeTasks ();
  }

/*---------------------------------------------------------------------------*/

void StartSerialTasks ()
  {
    /*
    * Start the serial input task.
    */
    if ( !ser_in_task )
      {
        ser_in_task = (struct Task *) HSAllocMem ( sizeof (struct Task),
                                             MEMF_PUBLIC | MEMF_CLEAR );
        if ( !ser_in_task )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }

        ser_in_req  = ser_req;
        ser_rp_req  = ser_req;

        ser_in_task->tc_SPLower = (APTR) &ser_in_stack[0];
        ser_in_task->tc_SPUpper = (APTR) &ser_in_stack[STACK_SIZE -  2];
        ser_in_task->tc_SPReg   = (APTR) &ser_in_stack[STACK_SIZE - 12];
        ser_in_task->tc_Node.ln_Pri = 1;
        ser_in_task->tc_Node.ln_Name = /*"VT serial input"*/ NULL;
        AddTask ( ser_in_task, (char *) SerialInput, 0 );

        ProcessUntil ( SI_SUBTASK_AWAKE );

        SetTaskPri ( ser_in_task, 0 );
        Si_asleep = 0;
      }

    /*
    * Start the serial output task.
    */
    if ( !ser_out_task )
      {
        ser_out_task = (struct Task *) HSAllocMem ( sizeof (struct Task),
                                                MEMF_PUBLIC | MEMF_CLEAR );
        if ( !ser_out_task )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
      
        ser_out_req = ser_req;

        ser_out_task->tc_SPLower = (APTR) &ser_out_stack[0];
        ser_out_task->tc_SPUpper = (APTR) &ser_out_stack[STACK_SIZE -  2];
        ser_out_task->tc_SPReg   = (APTR) &ser_out_stack[STACK_SIZE - 12];
        ser_out_task->tc_Node.ln_Pri = 1;
        ser_out_task->tc_Node.ln_Name = /*"VT serial output"*/ NULL;
        AddTask ( ser_out_task, (char *) SerialOutput, 0 );

        ProcessUntil ( SO_SUBTASK_AWAKE );

        SetTaskPri ( ser_out_task, 0 );
        So_asleep = 0;
      }

  }

/*---------------------------------------------------------------------------*/

void StartBlinkTask ()
  {
	if ( blink1_task || !Screen || Screen->BitMap.Depth != 2 )
		return;

   	blink1_task = (struct Task *) HSAllocMem ( sizeof (struct Task),
                                             	MEMF_PUBLIC | MEMF_CLEAR );
    if ( !blink1_task )
      {
        DisplayAlert ( AG_NoMemory, MemMessage, 60L );
        ShutDown ();
      }
    blink1_task->tc_SPLower = (APTR) &blink1_stack[0];
    blink1_task->tc_SPUpper = (APTR) &blink1_stack[STACK_SIZE -  2];
    blink1_task->tc_SPReg   = (APTR) &blink1_stack[STACK_SIZE - 12];
    blink1_task->tc_Node.ln_Pri = 1;
    blink1_task->tc_Node.ln_Name = /*"Blink 1"*/ NULL;
    AddTask ( blink1_task, (char *) BlinkTask, 0 );

    SetTaskPri ( main_task, 1 );
  }

/*---------------------------------------------------------------------------*/

void StopSerialTasks ()
  {
    /*
	unsigned short int  count;
	*/
    
    /*
    * Shutdown the Sub-Tasks
    */
    So_asleep = 1;
    if ( ser_out_task )
      {
        so_msg.opcode = SER_OUT;
        PutMsg ( so_task_port, (struct Message *) &so_msg );
        
        SetTaskPri ( ser_out_task, 5L );
		/* ERH
        for ( count = 0; FindTask ( "VT serial output" ) && count < 5; count++ )
          {
            Delay ( 10 );
          }
        if ( FindTask ( "VT serial output" ) )
          {
            Error ( "Messy shutdown of output task" );
            RemTask ( ser_out_task );
            CloseDevice ( (struct IORequest *) &con_in_req    );
            if ( con_in_port )
                DeletePort  ( con_in_port    );
            if ( con_kb_port )
                DeletePort  ( con_kb_port    );
            if ( ser_out_port )
                DeletePort  ( ser_out_port   );

            CloseDevice ( (struct IORequest *) &so_timeout_req );
            if ( so_timeout_port )
                DeletePort  ( so_timeout_port );
            if ( so_timeout_port )
            DeletePort  ( so_task_port    );
          }
		*/
        HSFreeMem ( (char *) ser_out_task, sizeof (struct Task) );
        ser_out_task = NULL;
      }

    Si_asleep = 1;
    if ( ser_in_task )
      {
        si_msg.opcode = SER_IN;
        PutMsg ( si_task_port, (struct Message *) &si_msg );
        
        SetTaskPri ( ser_in_task, 5L );
		/*
        for ( count = 0; FindTask ( "VT serial input" ) && count < 5; count++ )
          {
            Delay ( 10 );
          }
        if ( FindTask ( "VT serial input" ) )
          {
            Error ( "Messy shutdown of input task" );
            RemTask ( ser_in_task );
            if ( ser_in_port )
                DeletePort  ( ser_in_port   );
            if ( ser_rp_port )
                DeletePort  ( ser_rp_port   );
            if ( si_task_port );
                DeletePort  ( si_task_port );
          }
		*/
        HSFreeMem ( (char *) ser_in_task, sizeof (struct Task) );
        ser_in_task = NULL;
      }
  }

/*---------------------------------------------------------------------------*/

void StopBlinkTask ()
  {
	unsigned short int color;
   
    if ( blink1_task && Screen )
      {
        blink1_msg.opcode = BLINK1;
        PutMsg ( blink1_task_port, (struct Message *) &blink1_msg );
        ProcessUntil ( BLINK1 );
        /*
		while ( FindTask ( "Blink 1" ) )
            Delay ( 10 );
		*/
        HSFreeMem ( (char *) blink1_task,  sizeof ( struct Task ) );
		blink1_task = NULL;

        if ( Screen && Screen->BitMap.Depth == 2 )
		  {
			color = nvmodes.color3;
       		SetRGB4 ( VPort, 3L,
               	(long)  color >>  8L,
               	(long)( color >>  4L ) & 0xfL,
               	(long)  color & 0xfL );
		  }
      }
  }

/*---------------------------------------------------------------------------*/

int LoadNVR ()
  {
    unsigned char       copy_of_nvr[ 257 ];
    unsigned long int   file_size;
    long int            fh;

    
    if ( !*Nvr )
      {
        if ( access ( "HandShake.parms", 0 ) == 0 )
            strcpy ( Nvr, "Handshake.parms" );
        else if ( access ( "S:Handshake.parms", 0 ) == 0 )
            strcpy ( Nvr, "S:Handshake.parms" );
        else
            strcpy ( Nvr, "s:Handshake.parms" );
      }
    
    if ( -1 == ( fh = _dopen ( Nvr, MODE_OLDFILE ) ) )
      {
        strcpy ( copy_of_nvr, Nvr );
        StrToUpper ( copy_of_nvr );
        if ( *Nvr &&
             strcmp ( copy_of_nvr, "HANDSHAKE.PARMS" ) &&
             strcmp ( copy_of_nvr, "S:HANDSHAKE.PARMS" ) )
            Error ( "Could not open paramter file" );

        return ( 0 );
      }

    file_size = _dseek ( fh, 0L, 2 );

    if ( file_size != sizeof(NVM) )
      {
        Error ( "Invalid parameter file" );
        _dclose ( fh );
        return ( 0 );
      }

    _dseek ( fh, 0L, 0 );
    _dread  ( fh, (char *) &nvmodes, sizeof ( NVM ) );
    _dclose ( fh );
    
    if ( !nvmodes.pfk_cr )
       nvmodes.pfk_cr = '~';
    if ( !nvmodes.pfk_nl )
       nvmodes.pfk_nl = '`';
    if ( !nvmodes.bitplane )
       nvmodes.bitplane = 2;
    if ( nvmodes.decanm < VT200_8 )
        tmodes.controls = tmodes.ocontrols = 7;
    else
        tmodes.controls = tmodes.ocontrols = 8;
       
    if ( nvmodes.colormap[0] == nvmodes.colormap[1] ||
         nvmodes.colormap[1] == nvmodes.colormap[2] ||
         nvmodes.colormap[2] == nvmodes.colormap[3] ||
         nvmodes.colormap[3] == nvmodes.colormap[4] ||
         nvmodes.colormap[4] == nvmodes.colormap[5] ||
         nvmodes.colormap[5] == nvmodes.colormap[6] ||
         nvmodes.colormap[6] == nvmodes.colormap[7] )
      {
        nvmodes.colormap[0] = 0x0000;
        nvmodes.colormap[1] = 0x000f;
        nvmodes.colormap[2] = 0x00f0;
        nvmodes.colormap[3] = 0x00ff;
        nvmodes.colormap[4] = 0x0f00;
        nvmodes.colormap[5] = 0x0f0f;
        nvmodes.colormap[6] = 0x0ff0;
        nvmodes.colormap[7] = 0x0fff;
      }
      
    if ( nvmodes.color3 == nvmodes.smallmap[0] ||
         nvmodes.color3 == nvmodes.smallmap[1] )
        nvmodes.color3 = 0x0abc;
      
    if ( nvmodes.bkspdel )
     SetNormalBksp ();
    else
     SetReverseBksp ();
      
    if (!*nvmodes.font)
        strcpy ( nvmodes.font, "topaz.font" );
    TermFont.ta_Name = nvmodes.font;

    UpdateFunctionKeys ();

    if ( nvmodes.protocol == XPROTOCOL )
        InitializeXProtocol ();

    return ( 1 );
  }

/*---------------------------------------------------------------------------*/

int SaveNVR ()
  {
  
#   define    G_WIDTH     84
#   define    G_HEIGHT    36
    
    static unsigned short int ProjObjData[] =
      {
  0x0000,
  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
  0x0000,  0x0000,  0x0006,  0x0000,  0x0000,  0x0000,  0x0000,  0x0e00,
  0x0666,  0x0000,  0x0000,  0x0000,  0x0000,  0x01f8,  0x03c3,  0x0000,
  0x0000,  0x0000,  0x0000,  0x000e,  0x03c3,  0x0000,  0x0000,  0x0000,
  0x0000,  0x0002,  0x0fff,  0x0000,  0x0000,  0x0000,  0x0000,  0x00f9,
  0x0fff,  0x0000,  0x0000,  0x0000,  0x0000,  0x0187,  0x03c3,  0x0000,
  0x3fff,  0xffff,  0xc000,  0x00c3,  0x03c3,  0x0000,  0x3fff,  0xffff,
  0xc000,  0x007d,  0x0666,  0x0000,  0x3fff,  0xffff,  0xc000,  0x0003,
  0x0670,  0x0000,  0x3fff,  0xffff,  0xc000,  0x0006,  0x0000,  0x0000,
  0x3fff,  0xffff,  0xc000,  0x0ffc,  0x0000,  0x0000,  0x3fff,  0xffff,
  0xc000,  0x1018,  0x0000,  0x0000,  0x3fff,  0xffff,  0xc000,  0x18f8,
  0x0000,  0x0000,  0x3fff,  0xffff,  0xc000,  0x0f88,  0x0000,  0x0000,
  0x3fff,  0xffff,  0xc000,  0x0008,  0x0000,  0x0000,  0x3fff,  0xffff,
  0xc000,  0x0018,  0x0000,  0x0000,  0x3fff,  0xffff,  0xc000,  0x3ff0,
  0x0000,  0x0000,  0x3fff,  0xffff,  0xc000,  0x6030,  0x0000,  0x0000,
  0x3fff,  0xffff,  0xc000,  0x3078,  0x0000,  0x0000,  0x3fff,  0xffff,
  0xc000,  0x1fc8,  0x003c,  0x0000,  0x3fff,  0xffff,  0xc000,  0x0018,
  0x0000,  0x0000,  0x3fff,  0xffff,  0xc007,  0x8030,  0x0000,  0x0000,
  0x3fff,  0xffff,  0xc00c,  0x8060,  0x0000,  0x0000,  0x3fe0,  0x007f,
  0xc00c,  0xc1c0,  0x0000,  0x0000,  0x3fff,  0xffff,  0xc007,  0xff00,
  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0xc000,  0x0000,  0x0000,
  0x0000,  0x0000,  0x03ff,  0x8000,  0x0000,  0x0000,  0x0000,  0x0000,
  0x0400,  0x0000,  0x0003,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
  0x0000,  0x0000,  0x0000,  0x0000,  0x0600,  0x0000,  0x0000,  0x0000,
  0x0000,  0x0000,  0x03b3,  0x0003,  0xc000,  0x0000,  0x3c00,  0x0000,
  0x0601,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0180,  0x7fff,
  0xffff,  0xffff,  0xffff,  0xe000,  0x0c00,  0xffff,  0xffff,  0xffff,
  0xffff,  0xf000,  0x0003,  0xffff,  0xffff,  0xffff,  0xffff,  0xfe00,
  0x0000,  0xffff,  0xffff,  0xffff,  0xffff,  0xf1f8,  0x03c1,  0xfff8,
  0x7800,  0x0001,  0xe1ff,  0xf00e,  0x00e0,  0xfff8,  0x7800,  0x0001,
  0xe1ff,  0xf002,  0x0000,  0xfff9,  0xffff,  0xffff,  0xf9ff,  0xf0f9,
  0x0600,  0xfff9,  0x8000,  0x0000,  0x19ff,  0xf187,  0x0000,  0xfff9,
  0x8000,  0x0000,  0x19ff,  0xf0c3,  0x0000,  0x7ff1,  0x9f83,  0xfc1f,
  0x98ff,  0xe07d,  0x0001,  0x0001,  0x9f83,  0xfc1f,  0x9800,  0x0003,
  0x066f,  0x0001,  0x8000,  0x0000,  0x1800,  0x0006,  0x03c0,  0x0001,
  0x8000,  0x0000,  0x1800,  0x0ffc,  0x0180,  0x0001,  0x9f83,  0xfc1f,
  0x9800,  0x1018,  0x0ee1,  0x0001,  0x9f83,  0xfc1f,  0x9800,  0x18f8,
  0x03c3,  0x0001,  0x8000,  0x0000,  0x1800,  0x0f88,  0x03b0,  0x0001,
  0x8000,  0x0000,  0x1800,  0x0008,  0x0300,  0x0001,  0x9f83,  0xfc1f,
  0x9800,  0x0018,  0x0000,  0x0001,  0x9f83,  0xfc1f,  0x9800,  0x3ff0,
  0x0080,  0x0001,  0x8000,  0x0000,  0x1800,  0x6030,  0x0000,  0x0001,
  0x8000,  0x0000,  0x1800,  0x3078,  0x0661,  0x0001,  0x9f83,  0xfc1f,
  0x9800,  0x1fc8,  0x00c3,  0x0001,  0x9f83,  0xfc1f,  0x9800,  0x0018,
  0x03c3,  0x0001,  0x8000,  0x0000,  0x1807,  0x8030,  0x000c,  0x0001,
  0x8000,  0x0000,  0x180c,  0x8060,  0x0600,  0x0001,  0x801f,  0xff80,
  0x180c,  0xc1c0,  0x01e6,  0x0001,  0x8000,  0x0000,  0x1807,  0xff00,
  0x03c3,  0x0001,  0x8000,  0x0000,  0x1800,  0xc000,  0x03c6,  0x0001,
  0xffff,  0xffff,  0xfbff,  0x8000,  0x0000,  0x0001,  0xffff,  0xffff,
  0xfc00,  0x0000,  0x00c0,  0x0001,  0xffff,  0xffff,  0xf800,  0x0000,
  0x0000,  0x0001,  0xffff,  0xffff,  0xf800,  0x0000,  0x0000,  0x0001,
  0xffff,  0xffff,  0xf800,  0x0000,  0x0003,  0x0001,  0xffff,  0xffff,
  0xf800,  0x0000,  0x0000,  0x0003,  0xc000,  0x0000,  0x3c00,  0x0000,
  0x03c6,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000

      };
    
    static unsigned char  *ToolTypes[] =
      {
        "DIALUP=",
        "MACRO=",
        "DISPLAY=ON",
        "PORT=0;",
        "DRIVER=serial.device",
        NULL
      };

    static struct Image ProjObjImage =
      {
        0,  0,          /* Left edge, Top edge      */
        G_WIDTH, G_HEIGHT,/* Width, Height          */
        2,              /* Depth, Pixel size        */
        &ProjObjData[0],/* Pointer to inamge data   */
        3, 0,           /* Plane pick, plane on/off */
        NULL            /* Next Data image          */
      };
      
    static struct DiskObject ProjObj =
      {
        WB_DISKMAGIC, WB_DISKVERSION,
          {
            /* GADGET STRUCTURE */
            NULL,
            0, 0,
            G_WIDTH, G_HEIGHT,
            GADGHBOX | GADGIMAGE,
            RELVERIFY | GADGIMMEDIATE,
            BOOLGADGET,
            (APTR) &ProjObjImage,
            NULL, NULL,
            0, 0, 0, 0,
          },
        WBPROJECT,
        "HandShake",
        &ToolTypes[0],
        NO_ICON_POSITION, NO_ICON_POSITION,
        NULL, NULL,
        0
      };
    
    unsigned char    desc[ 65 ] = "DIALUP=";
    unsigned char    mesc[ 65 ] = "MACRO=";
    
    long int fh;
    
    if ( access ( Nvr, 2 ) > 0 )
      {
        Error ( "Could not access parameter file" );
        return ( 0 );
      }

    if ( -1 == ( fh = _dopen ( Nvr, MODE_NEWFILE ) ) )
      {
        Error ( "Could not open parameter file" );
        return ( 0 );
      }
      
    _dwrite ( fh, (char *) &nvmodes, sizeof ( NVM ) );
    _dclose ( fh );
    
    if ( nvmodes.icons == SET )
      {
        if ( ( IconBase = OpenLibrary ( "icon.library", 0 ) ) == NULL )
          {
            Error ( "Could not open Icon Lib" );
            return ( 0 );  
          }
      
        GetStringFromUser ( "Initail dial descriptor (if any)",
                            dial_descriptor, 31 );
        strncpy ( desc + 7, dial_descriptor, 31 );
        desc[64] = 0x00;
        ToolTypes[ 0 ] = desc;

        if ( FindPort ( "REXX" ) )
          {
            GetStringFromUser ( "Startup AREXX Macro (if any)",
                                StartMacro, 256 );
            strncpy ( mesc + 6, StartMacro, 58 );
            desc[64] = 0x00;
            ToolTypes[ 1 ] = mesc;
          }
          
        PutDiskObject ( Nvr, &ProjObj );
    
        CloseLibrary ( (struct Library *) IconBase );
      }

    return ( 1 );
  }

/*---------------------------------------------------------------------------*/

static UsedMemory = 0;

char *HSAllocMem ( size, flags )
long size;
long flags;
  {
    UsedMemory +=size;
    /*
    printf ( "Alloc %08d, To Date %08d\n", size, UsedMemory );
    */
    return ( AllocMem ( size, flags ) );
  }
  
void HSFreeMem ( pointer, size )
char *pointer;
long size;
  {
    UsedMemory -= size;
    /*
    printf ( "Free  %08d, To Date %08d\n", size, UsedMemory );
    */
    FreeMem ( pointer, size );
  }

/*---------------------------------------------------------------------------*/

unsigned short int ToPrinter ( unsigned short int from_line,
                               unsigned short int to_line,
                               unsigned char       decpff )
  {
    unsigned short int    line;
    unsigned char         line_image[133];
    unsigned char         *ptr;
  
    if ( !*nvmodes.printer )
      {
        Error ( "No printer selected" );
        return ( 0 );
      }
    
    tmodes.printerhandle = _dopen ( nvmodes.printer, MODE_NEWFILE );
      
    if ( !tmodes.printerhandle )
      {
        Error ( "Unable to open printer" );
        return ( 0 );
      }
    
    for ( line = from_line - 1;
          line < to_line;
          line++ )
      {
        strncpy ( line_image, ScreenImage + line * 132, vcb.linelen );
        line_image[vcb.linelen]= '\0';
        ptr = strrev ( line_image );
        ptr = stpblk ( ptr );
        ptr = strrev ( ptr );
        _dwrite ( tmodes.printerhandle, ptr, (unsigned) strlen ( ptr ) );
        _dwrite ( tmodes.printerhandle, "\n", 1L );
      }
    
    if ( decpff && decpff != '\n' )
        _dwrite ( tmodes.printerhandle, &decpff, 1L );
    
    _dclose ( tmodes.printerhandle );
    tmodes.printerhandle = 0;
    return ( 1 );
  }

/*---------------------------------------------------------------------------*/

