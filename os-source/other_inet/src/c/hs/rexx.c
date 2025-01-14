/* -----------------------------------------------------------------------
 * rexx.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: rexx.c,v 1.1 91/05/09 15:42:35 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/rexx.c,v 1.1 91/05/09 15:42:35 bj Exp $
 *
 * $Log:	rexx.c,v $
 * Revision 1.1  91/05/09  15:42:35  bj
 * Initial revision
 * 
 * Revision 1.1  91/05/09  15:39:47  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/*
*   We read in our own personal little include.
*/
#include "termall.h"
#include "rexx/rexxsys.h"
#include "minrexx.h"
#include "rexx/errors.h"

/*
*   All of our local globals, hidden from sight.
*/
static struct MsgPort *rexxPort ;          /* this is *our* rexx port */
static int bringerdown ;                   /* are we trying to shut down? */
static long stillNeedReplies ;             /* how many replies are pending? */
static long rexxPortBit ;                  /* what bit to wait on for Rexx? */
static char *extension ;                   /* the extension for macros */

/*
*   Our library base.  Don't you dare close this!
*/
static struct RxsLib *RexxSysBase ;

/****************************************************************************/

#define QUIT        1
#define STRNG       2
#define DIAL        3
#define DELAY       4
#define COLOR       5
#define MONO        6
#define GETSTR      7
#define TIMEOUT     8
#define PROTOCOL    9
#define TRANSMIT    10
#define RECEIVE     11
#define ENDASCII    12
#define BAUD        13
#define DATABITS    14
#define PARITY      15
#define STOPBITS    16
#define LOADPARMS   17
#define SAVEPARMS   18
#define HANGUP      19
#define CLS         20
#define SHOW        21
#define HIDE        22
#define GETLENGTH   23

static unsigned short int   RexxTimeout = 10;

static struct rexxCommandList CommandList[] =
  {
    { "HS_QUIT"     , QUIT      },
    { "HS_DELAY"    , DELAY     },
    { "HS_COLOR"    , COLOR     },
    { "HS_MONO"     , MONO      },
    { "HS_TIMEOUT"  , TIMEOUT   },
    { "HS_PROTOCOL" , PROTOCOL  },
    { "HS_ENDASCII" , ENDASCII  },
    { "HS_BAUD"     , BAUD      },
    { "HS_DATABITS" , DATABITS  },
    { "HS_PARITY"   , PARITY    },
    { "HS_STOPBITS" , STOPBITS  },
    { "HS_HANGUP"   , HANGUP    },
    { "HS_CLS"      , CLS       },
    { "HS_SHOW  "   , SHOW      },
    { "HS_HIDE"     , HIDE      },
    { "HS_GETLENGTH", GETLENGTH },
    { NULL          , NULL      }
  };
  
static struct rexxCommandList FunctionList[] =
  {
    { "HS_STRING"   , STRNG     },
    { "HS_DIAL"     , DIAL      },
    { "HS_GETSTR"   , GETSTR    },
    { "HS_TRANSMIT" , TRANSMIT  },
    { "HS_RECEIVE"  , RECEIVE   },
    { "HS_LOADPARMS", LOADPARMS },
    { "HS_SAVEPARMS", SAVEPARMS },
    { NULL          , NULL      }
  };
  
static struct rexxCommandList ProtocolList[] =
  {
    { "ASCII"       ,   XON_XOFF     },
    { "XMODEM"      ,   XMODEM       },
    { "XMODEM/CRC"  ,   XMODEM_CRC   },
    { "YMODEM"      ,   YMODEM       },
    { "YMODEM/BATCH",   YMODEM_BATCH },
    { "KERMIT/TEXT" ,   KERMIT_TEXT  },
    { "KERMIT/7BIT" ,   KERMIT_7BIT  },
    { "KERMIT/8BIT" ,   KERMIT       },
    { NULL          ,   -1           }
  };

static struct rexxCommandList ParityList[] =
  {
    { "NONE"        ,   1 },
    { "EVEN"        ,   2 },
    { "ODD"         ,   3 },
    { "MARK"        ,   4 },
    { "SPACE"       ,   5 },
    { NULL          ,   NULL         }
  };

/****************************************************************************/

int __stdargs RexxComm ( rmsg, rc )
struct RexxMsg *rmsg;
struct rexxCommandList *rc;
  {
    struct rexxCommandList  *rcl;
    unsigned char           *string = rmsg->rm_Args[0];
    signed int              int1;
    
    for ( ; *string && *string <= ' '; string++ ); /* Skip leading blanks */
    for ( ; *string && *string >  ' '; string++ ); /* Skip Command        */
    for ( ; *string && *string <= ' '; string++ ); /* Skip blanks         */
    
    switch ( (int)(rc->userdata) )
      {
        case QUIT: /*Command*/
            if ( !ScreenSafe () )
              {
                replyRexxCmd ( rmsg, RC_WARN, RC_OK, NULL );
                return ( 1 );
              }
            replyRexxCmd ( rmsg, RC_OK, RC_OK, NULL );
            ShutDown ();
            break;
            
        case DELAY: /*Command*/
            stcd_i ( string, &int1 );
            if ( int1 )
                Delay ( 50L * (long)int1 );
            break;
            
        case COLOR: /*Command*/
            if ( !ScreenSafe () )
              {
                replyRexxCmd ( rmsg, RC_WARN, 0, NULL );
                return ( 1 );
              }
            nvmodes.bitplane = 3;
            NewScreen.Depth  = 3;
            ConfigureScreen ();
            RefreshMenu ();
            break;
            
        case MONO: /*Command*/
            if ( !ScreenSafe () )
              {
                replyRexxCmd ( rmsg, RC_WARN, 0, NULL );
                return ( 1 );
              }
            nvmodes.bitplane = 2;
            NewScreen.Depth  = 2;
            ConfigureScreen ();
            RefreshMenu ();
            break;
            
        case TIMEOUT: /*Command*/
            stcd_i ( string, &int1 );
            RexxTimeout = int1;
            break;
          
        case PROTOCOL: /*Command*/
            for (rcl = ProtocolList; rcl->name; rcl++)
              {
		if (rcl->name[0] == toupper ( *string )
                    && cmdcmp(rcl->name, string) == 0)
                  {
                    nvmodes.protocol = rcl->userdata;
                    int1 = 1;
                    break;
                  }
              }
            if ( rcl->userdata == -1 )
              {
                replyRexxCmd ( rmsg, RC_ERROR, 0, NULL );
                return ( 1 );
              }
            RefreshMenu ();
            break;
            
        case ENDASCII:  /*Command*/
            EndASCIIReceive ();
            break;

        case BAUD:      /*Command*/
            stcd_i ( string, &int1 );
            if ( int1 < 110 || int1 > 50000 )
              {
                replyRexxCmd ( rmsg, RC_ERROR, 0, NULL );
                return ( 1 );
              }
            nvmodes.lspeed = int1;
            ConfigureSerialPorts ();
            RefreshMenu ();
            break;

        case DATABITS:  /*Command*/
            stcd_i ( string, &int1 );
            if ( int1 != 7 && int1 != 8 )
              {
                replyRexxCmd ( rmsg, RC_ERROR, 0, NULL );
                return ( 1 );
              }
            nvmodes.bpc = int1;
            ConfigureSerialPorts ();
            RefreshMenu ();
            break;
            
        case PARITY:    /*Command*/
            for (rcl = ParityList; rcl->name; rcl++)
              {
                if (rcl->name[0] == toupper ( *string )
                    && cmdcmp(rcl->name, string) == 0)
                  {
                    nvmodes.parity = rcl->userdata - 1;
                    int1 = 1;
                    break;
                  }
              }
            if ( !rcl->userdata )
              {
                replyRexxCmd ( rmsg, RC_ERROR, 0, NULL );
                return ( 1 );
              }
            ConfigureSerialPorts ();
            RefreshMenu ();
            break;

        case STOPBITS:  /*Command*/
            stcd_i ( string, &int1 );
            if ( int1 < 0 || int1 > 2 )
              {
                replyRexxCmd ( rmsg, RC_ERROR, 0, NULL );
                return ( 1 );
              }
            nvmodes.stpbits = int1;
            ConfigureSerialPorts ();
            RefreshMenu ();
            break;
            
        case HANGUP: /*Command*/
            HangUpPhone ();
            break;

        case CLS:
            /*
            * Put the Serial I/O subtasks to sleep, and have them close all
            * their serial ports.
            */
            home_cursor = 1;
            SleepTasks ();
            
            /*
            * Wake up the Serial I/O Subtasks
            */
            WakeTasks ();
            home_cursor = 0;
            break;

        case SHOW:
            ScreenToFront ( Screen );
            break;

        case HIDE:
            ScreenToBack ( Screen );
            break;

        case GETLENGTH: /*Command*/
            stcd_i ( string, &int1 );
            GetLength = int1;
            break;
      }
    return ( 0 );
  }

/****************************************************************************/

int __stdargs RexxFunc ( rmsg, rc )
struct RexxMsg *rmsg;
struct rexxCommandList *rc;
  {
    long                    rc2;
    unsigned char           *string = rmsg->rm_Args[0];
    
    for ( ; *string && *string <= ' '; string++ ); /* Skip leading blanks */
    for ( ; *string && *string >  ' '; string++ ); /* Skip Command        */
    for ( ; *string && *string <= ' '; string++ ); /* Skip blanks         */
    
    switch ( (int)(rc->userdata) )
      {
        case STRNG: /*Function*/
			/*
			TransferMessage ( "ERH Outputting string\r\n" );
			*/
            string = rmsg->rm_Args[1];
            rc2 = 1;
            while ( *string )
                if ( !Aputc ( *string++ ) )
                    rc2 = 0;
            replyRexxCmd ( rmsg, RC_OK, rc2, NULL );
            return ( 1 );
            
        case DIAL: /*Function*/
            if ( isdigit ( rmsg->rm_Args[1][0] ) )
                rc2 = Dial ( rmsg->rm_Args[1] );
            else
                rc2 = DialInitialNumber ( rmsg->rm_Args[1] );
            replyRexxCmd ( rmsg, RC_OK, rc2, NULL );
            return ( 1 );
            
        case GETSTR: /*Function*/
          {
            struct GetSMsg *GetStrMsg;
            
            GetStrMsg = (struct Getmsg *) HSAllocMem ( sizeof (struct GetSMsg),
                                                     (long)MEMF_PUBLIC );
            GetStrMsg->Message.mn_Node.ln_Type = NT_MESSAGE;
            GetStrMsg->Message.mn_ReplyPort    = main_task_port;
            GetStrMsg->Message.mn_Length       = 6;
            GetStrMsg->opcode  = REXX_GETSTR;
            GetStrMsg->timeout = RexxTimeout;
            GetStrMsg->rmsg    = rmsg;
            
            /*
            sprintf ( rmsg->rm_Args[0],"%08lx",rmsg->rm_Action );
            Error (rmsg->rm_Args[0]);
            */
            PutMsg ( si_task_port, (struct Message *) GetStrMsg );
            ProcessUntil ( REXX_GETSTR );
            
            if ( !GetStrMsg->rmsg )
              {
                HSFreeMem ( (char *) GetStrMsg, sizeof (struct GetSMsg) );
                replyRexxCmd ( rmsg, RC_OK, 0, "HSERR_TIMEOUT" );
                return ( 1 );
              }
            
            HSFreeMem ( (char *) GetStrMsg, sizeof (struct GetSMsg) );
            replyRexxCmd ( rmsg, RC_OK, 0, (char *)GetStrMsg->rmsg );
            return ( 1 );
            break;
          }
          
        case TRANSMIT: /*Function*/
            rc2 = TransmitFile ( (unsigned short int) nvmodes.protocol,
                                 rmsg->rm_Args[1] );
            replyRexxCmd ( rmsg, RC_OK, rc2, NULL );
            return ( 1 );
            
        case RECEIVE: /*Function*/
            rc2 = ReceiveFile ( (unsigned short int) nvmodes.protocol,
                                rmsg->rm_Args[1] );
            replyRexxCmd ( rmsg, RC_OK, rc2, NULL );
            return ( 1 );
            
        case LOADPARMS: /*Function*/
            if ( !ScreenSafe () )
              {
                replyRexxCmd ( rmsg, RC_WARN, 0, NULL );
                return ( 1 );
              }
            strcpy ( Nvr, rmsg->rm_Args[1] );
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
            replyRexxCmd ( rmsg, RC_OK, 1, NULL );
            return ( 1 );
            
        case SAVEPARMS: /*Function*/
            strcpy ( Nvr, rmsg->rm_Args[1] );
            stcgfe ( extension, Nvr );
            if ( stricmp ( extension, "parms" ) != 0 )
                strcat ( Nvr, ".parms" );
            SaveNVR ();
            replyRexxCmd ( rmsg, RC_OK, 1, NULL );
            return ( 1 );
            
      }
    return ( 0 );
  }

/****************************************************************************/

static unsigned char port_string[20];
	
void StartRexx ()
  {
	sprintf ( port_string, "HANDSHAKE%d", InvokationNumber );
    RexxMask = upRexxPort ( port_string, "rexx" );
    if ( RexxMask )
        AddFuncHost (  port_string, "-50" );
  }

/****************************************************************************/

void StopRexx ()
  {
    asyncRexxCmd ( RXREMLIB, port_string );
    dnRexxPort ();
  }

/****************************************************************************/

/*
*   This is the main entry point into this code.
*/
long upRexxPort(s, exten)
/*
*   The first argument is the name of your port to be registered;
*   this will be used, for instance, with the `address' command of ARexx.
*/
char *s ;
/*
*   The third argument is the file extension for ARexx macros invoked
*   by this program.  If you supply this argument, any `primitive' not
*   in the association list rcl will be sent out to ARexx for
*   interpretation, thus allowing macro programs to work just like
*   primitives.  If you do not want this behavior, supply a `NULL'
*   here, and those commands not understood will be replied with an
*   error value of RXERRORNOCMD.
*/
char *exten ;
/*
*   upRexxPort() returns the signal bit to wait on for Rexx messages.
*   If something goes wrong, it simply returns a `0'.  Note that this
*   function is safe to call multiple times because we check to make
*   sure we haven't opened already.  It's also a quick way to change
*   the association list or dispatch function.
*/
  {
    /*
    *   If we aren't open, we make sure no one else has opened a port with
    *   this name already.  If that works, and the createport succeeds, we
    *   fill rexxPortBit with the value to return.
    *
    *   Note that rexxPortBit will be 0 iff rexxPort is NULL, so the check
    *   for rexxPort == NULL also insures that our rexxPortBit is 0.
    */
    if (rexxPort == NULL)
      {
        Forbid() ;
        if (FindPort(s)==NULL)
            rexxPort = CreatePort(s, 0L) ;
        Permit() ;
        if (rexxPort != NULL)
            rexxPortBit = 1L << rexxPort->mp_SigBit ;
      }
    /*
    *   Squirrel away these values for our own internal access, and return
    *   the wait bit.
    */
    extension = exten ;
    return(rexxPortBit) ;
}

/****************************************************************************/

/*
*   This function closes the rexx library, but only if it is open
*   and we aren't expecting further replies from REXX.  It's
*   *private*, but it doesn't have to be; it's pretty safe to
*   call anytime.
*/
static void closeRexxLib()
  {
    if (stillNeedReplies == 0 && RexxSysBase)
      {
        CloseLibrary((struct Library *) RexxSysBase) ;
        RexxSysBase = NULL ;
      }
  }

/****************************************************************************/

/*
*   This function closes down the Rexx port.  It is always safe to
*   call, and should *definitely* be made a part of your cleanup
*   routine.  No arguments and no return.  It removes the Rexx port,
*   replies to all of the messages and insures that we get replies
*   to all the ones we sent out, closes the Rexx library, deletes the
*   port, clears a few flags, and leaves.
*/
void dnRexxPort()
  {
    if (rexxPort)
      {
        RemPort(rexxPort) ;
        bringerdown = 1 ;
        while (stillNeedReplies)
          {
            WaitPort(rexxPort) ;
            dispRexxPort() ;
          }
        closeRexxLib() ;
        DeletePort(rexxPort) ;
        rexxPort = NULL ;
     }
    rexxPortBit = 0 ;
}

/****************************************************************************/

/*
*   Here we dispatch any REXX messages that might be outstanding.
*   This is the main routine for handling Rexx messages.
*   This function is fast if no messages are outstanding, so it's
*   pretty safe to call fairly often.
*
*   If we are bring the system down and flushing messages, we reply
*   with a pretty serious return code RXERRORIMGONE.
*
*   No arguments, no returns.
*/
void dispRexxPort()
  {
    register struct RexxMsg *RexxMsg ;
    register struct rexxCommandList *rcl ;
    register char *p ;
    register int dontreply ;

    /*
    *   If there's no rexx port, we're out of here.
    */
    if (rexxPort == NULL)
        return ;
    /*
    *   Otherwise we have our normal loop on messages.  We check that
    *   each message is actually a Rexx message; if it's not, we just
    *   replymsg() it.
    */
    while (RexxMsg = (struct RexxMsg *)GetMsg(rexxPort))
      {
        if (strcmp(RexxMsg->rm_Node.mn_Node.ln_Name, RXSDIR))
          {
            ReplyMsg((struct Message *)RexxMsg) ;
          }
        /*
        *   If we have a reply to a message we sent, we look at the second
        *   argument.  If it's set, it's a function we are supposed to call
        *   so we call it.  Then, we kill the argstring and the message
        *   itself, decrement the outstanding count, and attempt to close
        *   down the Rexx library.  Note that this call only succeeds if
        *   there are no outstanding messages.  Also, it's pretty quick, so
        *   don't talk to me about efficiency.
        */
        else if (RexxMsg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG)
          {
            if (RexxMsg->rm_Args[14])
              {
                ((int (*)())(RexxMsg->rm_Args[14]))(RexxMsg) ;
              }
            DeleteArgstring(RexxMsg->rm_Args[0]) ;
            if (RexxMsg->rm_Args[1] )
                DeleteArgstring(RexxMsg->rm_Args[1]) ;
            if (RexxMsg->rm_Args[2] )
                DeleteArgstring(RexxMsg->rm_Args[2]) ;
            DeleteRexxMsg(RexxMsg) ;
            stillNeedReplies-- ;
            closeRexxLib() ;
          }
        /*
        *   The default case is we got a message and we need to check it for
        *   primitives.  We skip past any initial tabs or spaces and initialize
        *   the return code fields.
        */
        else
          {
            p = (char *)RexxMsg->rm_Args[0] ;
			/*
			TransferMessage ( p );
			TransferMessage ( "\r\n" );
			*/

            while (*p > 0 && *p <= ' ')
                p++ ;
            RexxMsg->rm_Result1 = 0 ;
            RexxMsg->rm_Result2 = 0 ;
            /*
            *   If somehow the reply is already done or postponed, `dontreply' is
            *   set.
            */
            dontreply = 0 ;
            /*
            *   If the sky is falling, we just blow up and replymsg.
            */
            if (bringerdown || tmodes.rexx_abort )
              {
                tmodes.rexx_abort = 0;
                RexxMsg->rm_Result1 = RXERRORIMGONE ;
              }
            /*
            *   Otherwise we cdr down our association list, comparing commands,
            *   until we get a match.  If we get a match, we call the dispatch
            *   function with the appropriate arguments, and break out.
            */
            else
              {
                if ( RexxMsg->rm_Action & RXFUNC ) /*Function*/
                  {
                    for (rcl = FunctionList; rcl->name; rcl++)
                      {
                        if (rcl->name[0] == toupper ( *p )
                            && cmdcmp(rcl->name, p) == 0)
                          {
							/*
							TransferMessage ( "ERH Matched function\r\n" );
							*/
                            dontreply = RexxFunc(RexxMsg, rcl) ;
                            break ;
                          }
                      }
                    /*
                    *   If we broke out, rcl will point to the function we executed; if we
                    *   are at the end of the list, we didn't understand the function.  In
                    *   this case, we set the return code to specify that we did not find
                    *   the function, and reply to the message
                    */
                    if ( rcl->name == NULL )
                      {
			RexxMsg->rm_Result1 = 5;
                        RexxMsg->rm_Result2 = 1;
			dontreply = 1;
	                ReplyMsg((struct Message *)RexxMsg) ;
                      }
                  }
                else /*Command */
                  {
                    for (rcl = CommandList; rcl->name; rcl++)
                      {
                        if (rcl->name[0] == toupper ( *p )
                            && cmdcmp(rcl->name, p) == 0)
                          {
							/*
							TransferMessage ( "ERH Matched Command\r\n" );
							*/
                            dontreply = RexxComm(RexxMsg, rcl) ;
                            break ;
                          }
                      }
                    /*
                    *   If we broke out, rcl will point to the command we executed; if we
                    *   are at the end of the list, we didn't understand the command.  In
                    *   this case, if we were supplied an extension in upRexxPort, we know
                    *   that we should send the command out, so we do so, synchronously.
                    *   The synchronous send takes care of our reply.  If we were given a
                    *   NULL extension, we bitch that the command didn't make sense to us.
                    */
                    if (rcl->name == NULL)
                      {
                        if (extension)
                          {
                            syncRexxCmd( RXCOMM, RexxMsg->rm_Args[0], RexxMsg);
                            dontreply = 1 ;
                          }
                        else
                          {
                            RexxMsg->rm_Result1 = RXERRORNOCMD ;
                          }
                      }
                  }
              }
            /*
            *   Finally, reply if appropriate.
            */
            if (!dontreply)
	      {
                ReplyMsg((struct Message *)RexxMsg) ;
	      }
          }
      }
  }

/****************************************************************************/

/*
*   This is the function we use to see if the command matches
*   the command string.  Case sensitive, and the command only need
*   be a prefix of the command string.
*/
static int cmdcmp(c, m)
register char *c, *m ;
  {
    unsigned char   command[80];
    unsigned char   *cptr;
    
    for  ( cptr = command; *m > ' '; *cptr++ = *m++ );
    *cptr = '\x00';
    return ( stricmp ( c, command ) );
  }

/****************************************************************************/

/*
*   Opens the Rexx library if unopened.  Returns success (1) or
*   failure (0).  This is another function that is *private* but
*   that doesn't have to be.
*/
static int openRexxLib()
  {
   if (RexxSysBase)
      return(1) ;
   return((RexxSysBase = (struct RxsLib *) OpenLibrary(RXSNAME, 0L)) != NULL) ;
  }

/****************************************************************************/

/*
*   This is the general ARexx command interface, but is not the one
*   you will use most of the time; ones defined later are easier to
*   understand and use.  But they all go through here.
*/
struct RexxMsg *sendRexxCmd(type, s, f, p1, p2, p3)
long type;
char *s ;
/*
*   The first parameter is the command to send to Rexx.
*/
int (*f)() ;
/*
*   The second parameter is either NULL, indicating that the command
*   should execute asynchronously, or a function to be called when the
*   message we build up and send out here finally returns.  Please note
*   that the function supplied here could be called during cleanup after
*   a fatal error, so make sure it is `safe'.  This function always is
*   passed one argument, the RexxMsg that is being replied.
*/
STRPTR p1, p2, p3 ;
/*
*   These are up to three arguments to be stuffed into the RexxMsg we
*   are building up, making the values available when the message is
*   finally replied to.  The values are stuffed into Args[2]..Args[4].
*/
  {
    register struct MsgPort *rexxport ;
    register struct RexxMsg *RexxMsg ;

    /*
    *   If we have too many replies out there, we just return failure.
    *   Note that you should check the return code to make sure your
    *   message got out!  Then, we forbid, and make sure that:
    *      - we have a rexx port open
    *      - Rexx is out there
    *      - the library is open
    *      - we can create a message
    *      - we can create an argstring
    *
    *   If all of these succeed, we stuff a few values and send the
    *   message, permit, and return.
    */
     
    if (stillNeedReplies > MAXRXOUTSTANDING-1)
        return(NULL) ;
    RexxMsg = NULL ;
    Forbid() ;
    if ( rexxPort &&
         (rexxport = FindPort(RXSDIR)) &&
         openRexxLib() &&
         (RexxMsg = 
            CreateRexxMsg(rexxPort, extension, rexxport->mp_Node.ln_Name)) &&
         (RexxMsg->rm_Args[0] = CreateArgstring(s, (long)strlen(s))))
      {
        RexxMsg->rm_Action   = type;
        RexxMsg->rm_Args[14] = (STRPTR)f ;
        RexxMsg->rm_Args[15] = p1 ;
        if ( p2 )
            RexxMsg->rm_Args[1] = CreateArgstring(p2, (long)strlen(p2));
        if ( p3 )
            RexxMsg->rm_Args[2] = CreateArgstring(p3, (long)strlen(p3));
        RexxMsg->rm_Node.mn_Node.ln_Name = RXSDIR ;
        PutMsg(rexxport, (struct Message *)RexxMsg) ;
        stillNeedReplies++ ;
        Permit() ;
        return(RexxMsg) ;
      }
    Permit() ;
    if (RexxMsg)
        DeleteRexxMsg(RexxMsg) ;
    closeRexxLib() ;
    return(NULL) ;
  }

/****************************************************************************/

/*
 *   This function is used to send out an ARexx message and return
 *   immediately.  Its single parameter is the command to send.
 */
struct RexxMsg *asyncRexxCmd(type, s)
long type;
char *s ;
  {
    return(sendRexxCmd(type, s, NULL, NULL, NULL, NULL)) ;
  }

/****************************************************************************/

/*
 *   This function sets things up to reply to the message that caused
 *   it when we get a reply to the message we are sending out here.
 *   But first the function we pass in, which actually handles the reply.
 *   Note how we get the message from the Args[2]; Args[0] is the command,
 *   Args[1] is this function, and Args[2]..Args[4] are any parameters
 *   passed to sendRexxCmd() as p1..p3.  We pass the result codes right
 *   along.
 */
static void __stdargs replytoit(msg)
register struct RexxMsg *msg ;
  {
    register struct RexxMsg *origmsg ;

    origmsg = (struct RexxMsg *)msg->rm_Args[15] ;
    origmsg->rm_Result1 = msg->rm_Result1 ;
    origmsg->rm_Result2 = msg->rm_Result2 ;
    ReplyMsg((struct Message *)origmsg) ;
  }

/****************************************************************************/

/*
*   This function makes use of everything we've put together so far,
*   and functions as a synchronous Rexx call; as soon as the macro
*   invoked here returns, we reply to `msg', passing the return codes
*   back.
*/
struct RexxMsg *syncRexxCmd(type, s, msg)
long type;
char *s ;
struct RexxMsg *msg ;
  {
    return(sendRexxCmd(type, s, (void *)&replytoit, (STRPTR)msg, NULL, NULL)) ;
  }

/****************************************************************************/

/****************************************************************************/

/*
*   Add specified function as an AREXX Function Host.
*/
struct RexxMsg *AddFuncHost(char *s, char *priority)
  {
    return(sendRexxCmd(RXADDFH, s, NULL, NULL, priority, NULL)) ;
  }

/****************************************************************************/

/*
*   There are times when you want to pass back return codes or a
*   return string; call this function when you want to do that,
*   and return `1' from the user dispatch function so the main
*   event loop doesn't reply (because we reply here.)  This function
*   always returns 1.
*/
int replyRexxCmd(msg, primary, secondary, string)
/*
*   The first parameter is the message we are replying to.
*/
register struct RexxMsg *msg ;
/*
*   The next two parameters are the primary and secondary return
*   codes.
*/
register long primary, secondary ;
/*
*   The final parameter is a return string.  This string is only
*   returned if the primary return code is 0, and a string was
*   requested.
*/
register char *string ;
  {
    /*
    *   Note how we make sure the Rexx Library is open before calling
    *   CreateArgstring . . . and we close it down at the end, if possible.
    */
    if (primary == 0 && (msg->rm_Action & (1L << RXFB_RESULT)))
      {
        if ( openRexxLib() && string )
            secondary = (long)CreateArgstring(string, (long)strlen(string));
        else if ( msg->rm_Action & RXFUNC )
            secondary = (long)CreateArgstring( secondary ? "1" : "0", 1L );
        else
            secondary = 0L;
      }
    msg->rm_Result1 = primary ;
    msg->rm_Result2 = secondary ;
    ReplyMsg((struct Message *)msg) ;
    closeRexxLib() ;
    return(1) ;
  }
