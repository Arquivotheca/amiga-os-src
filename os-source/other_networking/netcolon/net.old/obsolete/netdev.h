
/************************************************************************
*                                                                       *
*    Copyright (C) 1989, The Software Distillery.  All Rights Reserved. *
*                                                                       *
************************************************************************/



#ifndef DEVICES_NETDEV_H
#define DEVICES_NETDEV_H

#ifndef EXEC_IO_H
#include "exec/io.h"
#endif !EXEC_IO_H

#ifndef EXEC_DEVICES_H
#include "exec/devices.h"
#endif !EXEC_DEVICES_H



/*
 *--------------------------------------------------------------------
 *
 * Driver Specific Commands
 *
 *--------------------------------------------------------------------
 */

#define CHF_HANDLER (1<<15)              /* for internal use only! */
#define CHF_SERVER  (1<<14)              /* for internal use only! */

/*
 *--------------------------------------------------------------------
 *
 * Commands with the CHF_HANDLER bit set are only allowed on the handler
 * unit.  Reads and Writes with the handler bit set can only be
 * satisfied by Writes or Reads from the remote node with the CHF_SERVER
 * bit set.
 *
 *--------------------------------------------------------------------
 */
#define HCH_READ         (CMD_READ|CHF_HANDLER) /* Read data from a server*/
#define HCH_WRITE        (CMD_WRITE|CHF_HANDLER)/* Write data to a server */

#define HCH_ATTACH       ((CMD_NONSTD+0)|CHF_HANDLER) /* Attach to new server */
#define HCH_DETACH       ((CMD_NONSTD+1)|CHF_HANDLER) /* Detach from a server */
#define HCH_STATUS       ((CMD_NONSTD+2)|CHF_HANDLER) /* Give node status     */
#define HCH_ADDCHANGEINT ((CMD_NONSTD+3)|CHF_HANDLER) /* Install softint      */
#define HCH_REMCHANGEINT ((CMD_NONSTD+4)|CHF_HANDLER) /* Remove softint set   */
                                                      /*  by ADDCHANGEINT     */

/*
 *--------------------------------------------------------------------
 *
 * Commands with the CHF_SERVER bit set are only allowed on the server  
 * unit.  Writes with the server bit set can only be satisfied by Reads  
 * from the node specified in the io_Offset field that have the          
 * CHF_HANDLER bit set.  Reads with the server bit set are posted with an
 * io_Offset of -1 specified, and may be satisfied by a write with the   
 * CHF_HANDLER bit set from any node.                                    
 *
 *--------------------------------------------------------------------
 */
#define SCH_READ         (CMD_READ|CHF_SERVER)  /* Read data from a handler */
#define SCH_WRITE        (CMD_WRITE|CHF_SERVER) /* Write data to a server */
#define SCH_NAME         (CMD_NONSTD+5|CHF_SERVER) /* Set new name        */

/*
 *--------------------------------------------------------------------
 *
 * Predefined unit numbers  :
 * Handlers open the CHU_HANDLER unit; servers open the CHU_SERVER
 * unit.  Commands with the CHF_HANDLER bit set are valid only on
 * the handler unit; commands with the CHF_SERVER bit set are valid
 * only on the server unit.
 *
 *--------------------------------------------------------------------
 */

#define CHU_HANDLER -1   /* Handler unit number */
#define CHU_SERVER  -2   /* Server unit number  */

/*
 *--------------------------------------------------------------------
 *
 * Driver error defines
 *
 *--------------------------------------------------------------------
 */

#define CHERR_NotSpecified  41  /* general catchall */
#define CHERR_BadNodeID     42  /* Invalid node ID passed */
#define CHERR_NodeDown      43  /* Specified node is down */
#define CHERR_UnitInUse     44  /* Unit number in use */
#define CHERR_BadRequest    45  /* IO request invalid or unknown */
#define CHERR_IOErr         46  /* Physical I/O failure */
#define CHERR_NameConflict  47  /* Name set request failed */
                                        
#endif DEVICES_COMMHAND_H
