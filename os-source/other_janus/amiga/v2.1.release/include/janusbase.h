#ifndef JANUS_JANUSBASE_H
#define JANUS_JANUSBASE_H

/************************************************************************
 * (Amiga side file)
 * 
 * JanusBase.h  --  Primary include file for janus.library
 * 
 * This file contains all AMIGA SPECIFIC definitions and does not contain
 * any definitions required on the PC
 *
 * Copyright (c) 1986, 1987, 1988, Commodore Amiga Inc.
 * All rights reserved
 * 
 * Date        Name               Description
 * --------   ---------------     ----------------------------------------
 * Early 86 - Katin/Burns clone - Created this file!
 * 02-12-88 - RJ Mical          - Added JanusRemember structure
 * 07-15-88 - Bill Koester      - Modified for self inclusion of required files
 * 07-25-88 - Bill Koester      - Added jb_Reserved to JanusBase
 * 
 ************************************************************************/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif

#ifndef EXEC_INTERRUPTS_H
#include <exec/interrupts.h>
#endif



/*
 * === ===================================================================== 
 * === JanusBase Structure =================================================
 * === ===================================================================== 
 * JanusBase -- the main janus.library data structure.
 * This is the structure that you must declare a pointer to:
 *
 *   struct JanusBase *JanusBase = NULL;
 *
 * and initialize by opening janus.library:
 *
 *   JanusBase = OpenLibrary(JANUSNAME, myJANUSVERSION);
 *
 *  before using any of the Janus routines.  
 */

struct JanusBase {

   struct   Library     jb_LibNode;

   ULONG    jb_IntReq;          /* software copy of outstanding requests   */
   ULONG    jb_IntEna;          /* software copy of enabled interrupts     */
   UBYTE    *jb_ParamMem;       /* ptr to (byte arranged) param mem        */
   UBYTE    *jb_IoBase;         /* ptr to base of io register region       */
   UBYTE    *jb_ExpanBase;      /* ptr to start of shared memory           */
   APTR     jb_ExecBase;        /* ptr to exec library                     */
   APTR     jb_DOSBase;         /* ptr to DOS library                      */
   APTR     jb_SegList;         /* ptr to loaded code                      */
   struct   Interrupt   **jb_IntHandlers;/* base array of int handler ptrs */
   struct   Interrupt   jb_IntServer;    /* INTB_PORTS server              */
   struct   Interrupt   jb_ReadHandler;  /* JSERV_READAMIGA handler        */

   UWORD   jb_KeyboardRegisterOffset;    /* exactly that                   */
   UWORD   jb_ATFlag;                    /* 1 if this is an AT             */
   UWORD   jb_ATOffset;                  /* offset to the AT ROM bank      */

   struct   ServiceBase *jb_ServiceBase; /* Amiga Services data structure  */

   ULONG    jb_Reserved[4];
   };



/*
 * === ===================================================================== 
 * === Miscellaneous ======================================================= 
 * === ===================================================================== 
 * hide a byte field in the lib_pad field 
 */

#define jb_SpurriousMask LIB_pad


/*
 * === ===================================================================== 
 * === Miscellaneous ======================================================= 
 * === ===================================================================== 
 */

/************************************************************************
 *
 * data structure for SetupJanusSig() routine
 *
 ************************************************************************/

struct SetupSig {

   struct   Interrupt ss_Interrupt;
   APTR     ss_TaskPtr;
   ULONG    ss_SigMask;
   APTR     ss_ParamPtr;
   ULONG    ss_ParamSize;
   UWORD    ss_JanusIntNum;
   };


/*
 * JanusResource - an entity which keeps track of the reset state of the 8088
 * if this resource does not exist, it is assumed the 8088 can be reset
 */

struct JanusResource {

   APTR     jr_BoardAddress;     /* Address of Janus board                */
   UBYTE    jr_Reset;            /* non-zero indicates 8088 is held reset */
   UBYTE    jr_Pad0;
   };



#define JANUSNAME "janus.library"


#endif /* End of JANUS_JANUSBASE_H conditional compilation                */



