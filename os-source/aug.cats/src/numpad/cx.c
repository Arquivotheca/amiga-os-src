
   /***********************************************************************
   *                                                                      *
   *                            COPYRIGHTS                                *
   *                                                                      *
   *   UNLESS OTHERWISE NOTED, ALL FILES ARE                              *
   *   Copyright (c) 1990  Commodore-Amiga, Inc.  All Rights Reserved.    *
   *                                                                      *
   ***********************************************************************/

/*
 * cx.c -- Commodities interface
 *
 * This module handles all the command messages from commodities.library.
 * Commands such as HIDE SHOW ENABLE DISABLE KILL are sent to commidities
 * from the Exchange program and are processed here.
 */

#include "local.h"

CxObj *broker = NULL;            /* Our broker */

/* a little global for showing the user the hotkey   */
char   hotkeybuff[ 257 ];

struct NewBroker mynb = {
   NB_VERSION,                        /* Library needs to know version */
   COM_NAME,                          /* broker internal name          */
   COM_TITLE,                         /* commodity title               */
   COM_DESCR,                         /* description                   */
   NBU_NOTIFY | NBU_UNIQUE,           /* We want to be the only broker */
                                      /* with this name and we want to */
                                      /* be notified of any attempts   */
                                      /* to add a commodity with the   */
                                      /* same name                     */
   0,                                 /* flags                         */
   0,                                 /* default priority              */
   NULL,                              /* port, will fill in            */
   0                                  /* channel (reserved)            */
};

/****i* Blank.ld/handleCxMsg() ******************************************
*
*   NAME
*        handleCxMsg -- Handle incoming commodities messages.
*
*   SYNOPSIS
*        handleCxMsg(msg)
*
*        VOID handleCxMsg(Struct Message *msg);
*
*   FUNCTION
*        This function handles commodities messages sent to the brokers
*        message port. This is were the standard commodities features
*        such as Enable/Disable Show/Hide Quit and HotKey PopUp are
*        handled. If the  message is not for the standard handler then the
*        function handleCustomCXMsg() is called to handle application
*        specific IEVENTS otherwise handleCustomCXCommand() is called
*        to handle application specific commodities command messages.
*
*   INPUTS
*        msg = A commodities message;
*
*   RESULT
*        Either the standard commodities function is performed or the
*        custom handler is called.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID handleCxMsg(struct Message *msg)
{
   ULONG   msgid;
   ULONG   msgtype;

   msgid   = CxMsgID(   (CxMsg *)msg );
   msgtype = CxMsgType( (CxMsg *)msg );

   D1( printf("cx.c: handleCxMsg() enter\n") );

   ReplyMsg(msg);

   switch(msgtype)
   {
      case CXM_IEVENT:
         D1( printf("cx.c: handleCxMsg(CXM_IEVENT)\n") );
         switch(msgid)
         {
            W(
            case POP_KEY_ID:
                  D1( printf("cx.c: handleCxMsg(POP_KEY_ID)\n") );
                  setupWindow();
                  break;
            )
            default:
                  D1( printf("cx.c: handleCxMsg(Custom Message)\n") );
                  handleCustomCXMsg(msgid);
                  break;
         }
         break;
      case CXM_COMMAND:
         D1( printf("cx.c: handleCxMsg(CXM_COMMAND)\n") );
         switch(msgid)
         {
            case CXCMD_DISABLE:
               D1( printf("cx.c: handleCxMsg(CXCMD_DISABLE)\n") );
               ActivateCxObj(broker,0L);
               break;
            case CXCMD_ENABLE:
               D1( printf("cx.c: handleCxMsg(CXCMD_ENABLE)\n") );
               ActivateCxObj(broker,1L);
               break;
            case CXCMD_APPEAR:   /* Time to pop up the window         */
            case CXCMD_UNIQUE:   /* Someone has tried to run us again */
               D1( printf("cx.c: handleCxMsg(CXCMD_APPEAR|CXCMD_UNIQUE)\n") );
               /* If someone tries to run us a second time the second copy
                * of the program will fail and we will be sent a
                * CXCMD_UNIQUE message. If we support a window then we
                * Make our window appear since that is what the user wanted.
                * If we do not support a window then we kill the currently
                * running version 'this one' so that things like autopoint
                * can be toggled on/off by running them a second time.
                */
               if(WINDOW)
               {
                  W( setupWindow(); )
               } else {
                  terminate();
               }
               break;            /* the window                        */
            case CXCMD_DISAPPEAR:
               D1( printf("cx.c: handleCxMsg(CXCMD_DISAPPEAR)\n") );
               W(
                  shutdownWindow();
               )
               break;
            case CXCMD_KILL:
               D1( printf("cx.c: handleCxMsg(CXCMD_KILL)\n") );
               terminate();
               break;
            default:
               D1( printf("cx.c: handleCxMsg(Custom Command)\n") );
               handleCustomCXCommand(msgid);
               break;
         }     /* end switch(command) */
         break;
   }     /* end switch(msgtype) */
}
/****i* Blank.ld/setupCX() ******************************************
*
*   NAME
*        setupCX -- Initialize commodities.library specific features.
*
*   SYNOPSIS
*        result = setupCX(ttypes)
*
*        BOOL setupCX(char **ttypes);
*
*   FUNCTION
*        This function performs all the commodities.library specific
*        setup required for a standard commodity. It sets up the brokers
*        message port, and priority. And sets certain flags in the
*        broker structure so the exchange program will know what
*        features this broker supports. If the commodity supports a window
*        the windows hotkey is added to the broker and then the function
*        setupCustomCX() is called to setup the application specific
*        commodities objects. If all this goes well the broker is activated
*        and the function returns TRUE.
*
*   INPUTS
*        ttypes - NULL terminated Argument array containing this
*        applications TOOLTYPES strings.
*
*   RESULT
*        Returns TRUE if all went OK else returns FALSE.
*
*   EXAMPLE
*        if(setupCX(ttypes))
*        {
*           printf("Commodities successfully initialized.\n");
*        } else {
*           printf("Commodities initialization error!\n");
*        }
*
*   NOTES
*        This function can be called at anytime to reinitialize the
*        commodities code from a new set of arguments.
*
*   BUGS
*
*   SEE ALSO
*        setupCustomCX();
*        shutdownCX();
*        shutodwnCustomCX();
*
*****************************************************************************
*
*/
BOOL setupCX(char **ttypes )
{
   LONG   error;
   W( char   *str; )

   D1( printf("cx.c: setupCX() enter\n"); )

   shutdownCX();                          /* remove what was and create */
                                          /* everything from scratch    */

   cxport=CreatePort(mynb.nb_Name,0L);    /* Create Message port        */
   if( ! cxport )
   {
      D1( printf("cx.c: setupCX() Could not create message port\n"); )
      return(FALSE);
   }
   D1( printf("cx.c: setupCX() cxport=0x%lx\n",cxport); )

   cxsigflag = 1L << cxport->mp_SigBit;   /* Create signal mask for Wait*/

   /* Set the brokers priority from the TOOLTYPES or from default if no */
   /* TOOLTYPES are available. Set the brokers Message port.            */
   mynb.nb_Pri  = ArgInt( ttypes, PRIORITY_TOOL_TYPE, CX_DEFAULT_PRIORITY );
   mynb.nb_Port = cxport;

   D1( printf("cx.c: setupCX() mynb.nb_pri=0x%lx\n",mynb.nb_Pri); )

   /* If this commodity supports a window then set the SHOW/HIDE flag   */
   /* so the Exchange controller can ghost its gadgets appropriately    */
   W( mynb.nb_Flags |= COF_SHOW_HIDE; )

   /* Attempt to create our broker */
   if ( ! ( broker = CxBroker( &mynb, NULL ) ) )
   {
      D1( printf("cx.c: setupCX() could not create broker\n"); )
      shutdownCX();
      return(FALSE);
   }
   D1( printf("cx.c: setupCX() broker=0x%lx\n",broker); )

   /* If this commodity supports a window then add its HotKey now       */
   W(
      /* install a hotkey for popping up window   */
      AttachCxObj(broker,
               HotKey(str=ArgString(ttypes,POPKEY_TOOL_TYPE,CX_DEFAULT_POP_KEY),
               cxport,POP_KEY_ID) );
      strncpy(hotkeybuff,str,sizeof(hotkeybuff)-1);
   )

   /* Setup all application specific commodities objects */
   if( ! setupCustomCX() )
   {
      D1( printf("cx.c: setupCX() setupCustomCX failed\n"); )
      shutdownCX();
      return(FALSE);
   }

   /* Check for accumulated error */
   if ( error = CxObjError( broker ) )
   {
      D1( printf("cx.c: setupCX() accumulated broker error %ld\n",error) );
      shutdownCX();
      return (FALSE);
   }

   /* All went well so activate our broker */
   ActivateCxObj(broker,1L);

   D1( printf("cx.c: setupCX() returns TRUE") );
   return (TRUE);
}

/****i* Blank.ld/shutdownCX() ******************************************
*
*   NAME
*        shutdownCX -- Cleanup all commodities brokers and handlers.
*
*   SYNOPSIS
*        shutdownCX()
*
*        VOID shutdownCX(VOID);
*
*   FUNCTION
*        Shuts down and cleans up all variables and data used for supporting
*        the commodities.library side of this commodity. This function
*        MUST be set up so that it can be called regardless of the current
*        state of the program. This function handles all the standard
*        cleanup and calls shutdownCustomCX(); to cleanup the application
*        specific code.
*
*   INPUTS
*        None.
*
*   RESULT
*        The commodities specific code is cleaned up and made ready for
*        a terminate(); or a call to setupCX();.
*
*   EXAMPLE
*
*   NOTES
*        The first thing that setupCX() does is call this routine. Therefore
*        this function must work even before your commodity has been
*        initialized.
*
*   BUGS
*
*   SEE ALSO
*        setupCX();
*        setupCustomCX();
*        shutdownCustomCX();
*
*****************************************************************************
*
*/
VOID shutdownCX()
{
   struct Message   *msg;

   D1( printf("cx.c: shutdownCX() enter broker now: %lx\n", broker ) );
   shutdownCustomCX();

   if(cxport)
   {
      D1( printf("cx.c: shutdownCX() Deleting all objects\n") );
      DeleteCxObjAll(broker);      /* safe, even if NULL   */

      /* now that messages are shut off, clear port   */
      while(msg=GetMsg(cxport)) ReplyMsg(msg);
      DeletePort(cxport);

      cxport    = NULL;
      cxsigflag = 0;
      broker    = NULL;
   }
   D1( printf("cx.c: shutdownCX() returns\n") );
}

