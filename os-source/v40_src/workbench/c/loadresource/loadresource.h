#ifndef LOADRESOURCE_H
#define LOADRESOURCE_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/ports.h>
#include "loadresource_rev.h"


/*****************************************************************************/


#define PROGRAMNAME "« LoadResource »"


/*****************************************************************************/


/* WARNING: If adding new arguments, you should bump the version of the
 *          CLA_VERSION #define below
 */
#define TEMPLATE    "NAME/M,LOCK/S,UNLOCK/S" VERSTAG
#define OPT_NAME    0
#define OPT_LOCK    1
#define OPT_UNLOCK  2
#define OPT_COUNT   3


/*****************************************************************************/


#define CLA_VERSION 0

/* The version number allows the C:LoadResource file to change even if
 * there is already a version of it currently running. In order to remain
 * compatible, always leave the first 4 fields of the structure the same.
 * The other fields can change from version to version. If the sub-process
 * detects a version difference, it'll either deal with it, or return an
 * appropriate error in the cla_Result? fields
 */

struct CmdLineArgs
{
    struct Message cla_Message;
    UWORD          cla_Version;   /* must be CLA_VERSION */
    LONG           cla_Result1;
    LONG           cla_Result2;

    /* info on the originator of a this packet */
    BPTR           cla_CurrentDir;
    BPTR           cla_Input;
    BPTR           cla_Output;

    /* the actual options in this packet */
    LONG           cla_Arguments[OPT_COUNT];
};


/*****************************************************************************/


STRPTR ProgramName(VOID);
VOID LoadResource(struct MsgPort *port);


/*****************************************************************************/


#endif /* LOADRESOURCE_H */
