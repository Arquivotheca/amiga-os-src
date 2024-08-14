#ifndef JANUS_MEMRW_H
#define JANUS_MEMRW_H

/************************************************************************
 * (Amiga side file)
 *
 * memrw.h -- parameter area definition for access to other processors mem
 *
 * Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved
 *
 * 7-15-88 - Bill Koester - Modified for self inclusion of required files
 ************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

/*
 * this is the parameter block for the JSERV_READPC and JSERV_READAMIGA
 * services -- read and/or write the other processors memory.
 */

struct MemReadWrite {

   UWORD    mrw_Command;      /* see below for list of commands         */
   UWORD    mrw_Count;        /* number of bytes to transfer            */
   ULONG    mrw_Address;      /* local address to access.  This is      */
                              /* a machine pointer for the 68000, and   */
                              /* a segment/offset pair for the 808x.    */
                              /* The address is arranged so the native  */
                              /* processor may read it directly.        */
   UWORD    mrw_Buffer;       /* The offset in buffer memory for the    */
                              /* other buffer.                          */
   UWORD    mrw_Status;       /* See below for status.                  */
   };


/* command definitions                                                  */

#define MRWC_NOP        0     /* do nothing -- return OK status code    */
#define MRWC_READ       1     /* xfer from address to buffer            */
#define MRWC_WRITE      2     /* xfer from buffer to address            */
#define MRWC_READIO     3     /* only on 808x -- read from IO space     */
#define MRWC_WRITEIO    4     /* only on 808x -- write to IO space      */
#define MRWC_WRITEREAD  5     /* write from buffer, then read back      */


/* status definitions                                                   */

#define MRWS_INPROGRESS 0xffff /* we've noticed command and are working */
#define MRWS_OK         0x0000 /* completed OK                          */
#define MRWS_ACCESSERR  0x0001 /* some sort of protection violation     */
#define MRWS_BADCMD     0x0002 /* unknown command                       */


#endif /* End of JANUS_MEMRW_H conditional compilation                  */


