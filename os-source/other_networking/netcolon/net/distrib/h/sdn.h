/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1990 The Software Distillery.                    *
* |. o.| ||          All Rights Reserved                                  *
* | .  | ||          Written by John Toebes and Doug Walker               *
* | o  | ||          The Software Distillery                              *
* |  . |//           207 Livingstone Drive                                *
* ======             Cary, NC 27513                                       *
*                    BBS:(919)-460-7430                                   *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef D_SDN_H
#define D_SDN_H

/*
**      $Filename: sdn.h $ 
**      $Author: J_Toebes $
**      $Revision: 1.11 $
**      $Date: 91/01/10 22:12:35 $
**      $Log:	sdn.h,v $
 * Revision 1.11  91/01/10  22:12:35  J_Toebes
 * Correct RPBase definition.
 * 
 * Revision 1.10  91/01/10  22:00:45  J_Toebes
 * Reshuffle rpacket order to allow quick buffer to reside
 * at the end.  Add SDN_FAIL, SDN_KNOWN codes, added macro to access
 * RPacket data base.
 * 
**
**/


struct RPacket       /* Packet sent to remote node */
{
   char  *DataP;        /* always contains at least MINBUFSIZE bytes    */
   char  *AllocP;       /* always contains at least MINBUFSIZE bytes    */
   short Type;          /* Type of request (AmigaDos packet type)       */
   short DLen;          /* Length of data in Data buffer                */
   void  *handle;       /* Generic handle for association               */
   LONG  Arg1,          /* Equivalent to dp_Arg? in normal devices      */
         Arg2,          /* when transmitting, dp_Res? when receiving    */
         Arg3,
         Arg4;
};

#define RPBASE(rp)    ((char *)&((rp)->Type))
#define RPSIZEN       (sizeof(struct RPacket)-8)

#define SDN_OK   0   /* Successful completion                   */
#define SDN_ERR  1   /* Some error, operation not done          */
#define SDN_NONE 2   /* No packets pending                      */
#define SDN_PEND 3   /* Packet was sent but is now pending      */
#define SDN_FAIL 4   /* Pended packet failed                    */
#define SDN_DOWN 5   /* Node is known but not currently up      */

/* Prototypes for functions defined in sdninet.c */
int SDNSerInit          (APTR *drgp,
                         ULONG *maskp,
                         char *data
                        );
int SDNSerTerm          (APTR drglobal);
int SDNHanInit          (APTR *drgp,
                         ULONG *maskp,
                         char *data
                        );
int SDNHanTerm          (APTR drglobal);
int SDNInitNode         (APTR drglobal,
                         char *name,
                         APTR *con
                        );
void SDNTermNode        (APTR drglobal,
                         APTR con
                        );
int SDNAccept           (APTR drglobal,
                         struct RPacket **rp
                        );
int SDNReply            (APTR drglobal,
                         struct RPacket *rp
                        );
int SDNSend             (APTR drglobal,
                         struct RPacket *rp
                        );
int SDNReceive          (APTR drglobal,
                         ULONG mask,
                         struct RPacket **rp
                        );

struct RPacket *
    SDNAllocRPacket     (APTR drglobal,
                         APTR con,
                         int len
                        );

struct RPacket *
    SDNDupRPacket       (APTR drglobal,
                         struct RPacket *rp
                        );

void SDNFreeRPacket     (APTR drglobal,
                         struct RPacket *rp
                        );

void *SDNGetConData     (APTR drglobal,
                         struct RPacket *rp
                        );

void SDNSetConData      (APTR drglobal,
                         struct RPacket *rp,
                         void *data
                        );
#include "sdn_pragmas.h"
#endif /* D_SDN_H */
