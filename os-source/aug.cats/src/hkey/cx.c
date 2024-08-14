/*
 * cx.c -- Commodities interface
 *
 * This module handles all the command messages from commodities.library.
 * Commands such as HIDE SHOW ENABLE DISABLE KILL are sent to commidities
 * from the Exchange program and are processed here.
 */

#include "mycxapp.h"

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


VOID handleCxMsg( msg )
struct Message   *msg;
{
   ULONG   msgid;
   ULONG   msgtype;

   msgid   = CxMsgID( msg );
   msgtype = CxMsgType( msg );

   D1( printf("cx: handleCxMsg() enter\n") );

   ReplyMsg(msg);

   switch(msgtype)
   {
      case CXM_IEVENT:
         D1( printf("cx: handleCxMsg(CXM_IEVENT)\n") );
         switch(msgid)
         {
            W(
            case POP_KEY_ID:
                  setupWindow();
                  break;
            )
            default:
                  handleCustomCXMsg(msgid);
                  break;
         }
         break;
      case CXM_COMMAND:
         D1( printf("cx: handleCxMsg(CXM_COMMAND)\n") );
         switch(msgid)
         {
            case CXCMD_DISABLE:
               D1( printf("cx: handleCxMsg(CXCMD_DISABLE)\n") );
               ActivateCxObj(broker,0L);
               break;
            case CXCMD_ENABLE:
               D1( printf("cx: handleCxMsg(CXCMD_ENABLE)\n") );
               ActivateCxObj(broker,1L);
               break;
            case CXCMD_APPEAR:   /* Time to pop up the window         */
            case CXCMD_UNIQUE:   /* Someone has tried to run us again */
               D1( printf("cx: handleCxMsg(CXCMD_APPEAR|CXCMD_UNIQUE)\n") );
               W(
                  setupWindow();    /* might be a good time to bring up  */
               )
               break;            /* the window                        */
            case CXCMD_DISAPPEAR:
               D1( printf("cx: handleCxMsg(CXCMD_DISAPPEAR)\n") );
               W(
                  shutdownWindow();
               )
               break;
            case CXCMD_KILL:
               D1( printf("cx: handleCxMsg(CXCMD_KILL)\n") );
               terminate();
               break;
            default:
               handleCustomCXCommand(msgid);
               break;
         }     /* end switch(command) */
         break;
   }     /* end switch(msgtype) */
}

/* may be called anytime to reinitialize commodities
 * elements for a new set of arguments
 */
BOOL setupCX( ttypes )
char   **ttypes;      /* "tooltypes" (null terminated) array   */
{
   LONG   error;
   W( char   *str; )

   shutdownCX();      /* remove what was   */

   /* create everything from scratch   */

   /* cx available to main program loop   */
   cxport = CreatePort( mynb.nb_Name, 0L );
   if ( ! cxport ) return ( 0 );
   cxsigflag = 1L << cxport->mp_SigBit;

   /* set up some broker data from ttypes   */
   mynb.nb_Pri  = ArgInt( ttypes, PRIORITY_TOOL_TYPE, CX_DEFAULT_PRIORITY );
   mynb.nb_Port = cxport;
   W( mynb.nb_Flags |= COF_SHOW_HIDE; )

   /* init my commodities broker   */
   if ( ! ( broker = CxBroker( &mynb, NULL ) ) )
   {
      D1( printf("cx: broker failed to create\n") );
      shutdownCX();
      return (0);
   }

   W(
      /* install a hotkey for popping up window   */
      AttachCxObj(broker,
               HotKey(str=ArgString(ttypes,POPKEY_TOOL_TYPE,CX_DEFAULT_POP_KEY),
               cxport,POP_KEY_ID) );
      strncpy(hotkeybuff,str,sizeof(hotkeybuff)-1);
   )

   if( ! setupCustomCX() )
   {
      shutdownCX();
      return(0);
   }

   if ( error = CxObjError( broker ) )
   {
      D1( printf("accumulated broker error %ld\n",error) );
      shutdownCX();
      return (0);
   }

   ActivateCxObj(broker,1L);

   return (1);
}

VOID shutdownCX()
{
   struct Message   *msg;

   D1( printf("cx: shutdownCX.  broker now: %lx\n", broker ) );
   shutdownCustomCX();

   if(cxport)
   {
      DeleteCxObjAll(broker);      /* safe, even if NULL   */

      /* now that messages are shut off, clear port   */
      while(msg=GetMsg(cxport)) ReplyMsg(msg);
      DeletePort(cxport);

      cxport    = NULL;
      cxsigflag = 0;
      broker    = NULL;
   }
}

