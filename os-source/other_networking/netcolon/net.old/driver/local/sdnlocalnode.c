/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1990 The Software Distillery.                    *
* |. o.| ||          All Rights Reserved                                  *
* | .  | ||          Written by John Toebes and Doug Walker               *
* | o  | ||          The Software Distillery                              *
* |  . |//           207 Livingstone Drive                                *
* ======             Cary, NC 27513                                       *
*                    BBS:(919)-460-7430                                   *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "netcomm.h"
#include "sdnlocal.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNInitNode                                                        */
/*                                                                             */
/* PURPOSE: Establish a new node connection                                    */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    rc = SDNInitNode(drglobal, nnode);                                       */
/*                                                                             */
/*    drglobal   APTR             (in)     Driver-specific global data pointer */
/*                                                                             */
/*    nnode      struct NetNode * (in/out) On input, name of node is set.      */
/*                                         On output, driver sets the 'status' */
/*                                            and 'ioptr' fields.              */
/*                                                                             */
/*    rc         int              (ret)    SDN_ERR, SDN_OK                     */
/*                                                                             */
/* NOTES:                                                                      */
/*    Called by the handler to establish communications with a specific        */
/*    remote node.                                                             */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int LIBENT LOCALSDNInitNode     (register __a0 LOCALGLOBAL tg,
                                register __a1 char *name,
                                register __a2 APTR *con,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   int len;
   
   len = strlen(name);
   if (len > MAXNAME)
      return(SDN_ERR);

   if (tg->nodename[0])
   {
      if (stricmp(tg->nodename, name))
         return(SDN_ERR);
      return(SDN_OK);
   }

   strcpy(tg->nodename, name);
   tg->state = CSTATE_CLOSED;
   if (tg->sendport == NULL)
      return(SDN_DOWN);
   return(SDN_OK);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNTermNode                                                        */
/*                                                                             */
/* PURPOSE: Free Up all Driver resources associated with a Node                */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    SDNTermNode(drglobal, nnode);                                            */
/*                                                                             */
/*    drglobal  APTR              (in)     Driver-specific global data pointer */
/*                                                                             */
/*                                                                             */
/*    nnode     struct NetNode *  (in)     NetNode to terminate                */
/*                                                                             */
/* NOTES:                                                                      */
/*    This routine is void.                                                    */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void LIBENT LOCALSDNTermNode    (register __a0 LOCALGLOBAL tg,
                                register __a1 APTR con,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   tg->nodename[0] = 0;
   tg->state = CSTATE_CLOSED;
}

