
* *** services.asm ************************************************************
* 
* Service Routines  --  Janus Library
* 
* Copyright (C) 1988, Commodore-Amiga, Inc.
* 
* CONFIDENTIAL and PROPRIETARY
* 
* Date       Name           Description
* ---------  -------------  ---------------------------------------------------
* 20-Oct-88  =RJ Mical=     Created this file!
* 
* *****************************************************************************


      INCLUDE "assembly.i"

      NOLIST
      INCLUDE "exec/types.i"
      INCLUDE "exec/memory.i"
      INCLUDE "libraries/dosextens.i"
      INCLUDE "janus/janusbase.i"
      LIST

      INCLUDE "asmsupp.i"

      XLIB   Open
      XLIB   Execute
      XLIB   Lock
      XLIB   UnLock

      XDEF   RunAutoloadBack


RunAutoloadBack:
* BOOL   RunAutoloadBack()
*    BOOL   retvalue;
*    ULONG   lock;

RUNBACK_REGS   REG   D2-D5/A6

      MOVEM.L   RUNBACK_REGS,-(SP)

   PUTMSG   2,<'%s/RunAutoloadBack'>

      MOVEA.L   jb_DOSBase(A6),A6

*    retvalue = FALSE;
*    lock = 0;
      MOVEQ.L   #0,D4
      MOVE.L   D4,D5

*    lock = Lock(name, ACCESS_READ); 
      LEA.L   AutoloadName(PC),A0
      MOVE.L   A0,D1
      MOVE.L   #ACCESS_READ,D2
      CALLSYS   Lock

*    if (! lock) goto DONE;
      MOVE.L   D0,D5
      BEQ   DONE

*    /* Set up our null i/o file handler */
*    nilfh = Open("NIL:", MODE_NEWFILE); 
      LEA.L   NILName(PC),A0
      MOVE.L   A0,D1
      MOVE.L   #MODE_NEWFILE,D2
      CALLSYS   Open
      TST.L   D0
      BEQ   DONE
      ;------   else NIL: is here with us forever

*    /* The full command passed to Execute looks like this:
*     *
*     *   "run >NIL: <NIL: sys:pc/services/autoload >NIL: <NIL:"
*     *
*     */
*    retvalue = Execute( &commandstring[0] , nilfh, nilfh);
      LEA.L   RunAutoload(PC),A0
      MOVE.L   A0,D1
      MOVE.L   D0,D2
      MOVE.L   D0,D3
      CALLSYS   Execute
      TST.L   D0
      BEQ   DONE

*    if (retvalue)
*       {
*       /* Execute, in this case, returns IMMEDIATELY.  The process
*        * that is loading the code that is to be run as a background
*        * process is working to get everything in and started.  
*        */
*       retvalue = TRUE;
*       }
;      MOVE.L   #TRUE,D4  **** BILL KLUDGE ****
      MOVE.L   #1,D4

DONE:
*    if (lock) UnLock(lock);
      MOVE.L   D5,D1
      BEQ   REALLY_DONE
      CALLSYS   UnLock

REALLY_DONE:
*    return(retvalue);
      MOVE.L   D4,D0

      MOVEM.L   (SP)+,RUNBACK_REGS
      RTS



AutoloadName:
      DC.B   'sys:pc/services/autoload',0
      CNOP   0,2
NILName:
      DC.B   'NIL:',0
      CNOP   0,2
RunAutoload:
      DC.B   'run >NIL: <NIL: sys:pc/services/autoload >NIL: <NIL:',0
      CNOP   0,2

      END
