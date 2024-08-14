#ifndef CE_CUSTOM_H
#define CE_CUSTOM_H


/*****************************************************************************/


#include <exec/types.h>
#include <libraries/commodities.h>
#include <workbench/startup.h>
#include "mouseblanker_rev.h"

#define CX_MOUSEBLANKER
#include <localestr/commodities.h>


/*****************************************************************************/


/* define the editor's command-line template here
 * OPT_PRIORITY, OPT_POPKEY, OPT_POPUP and OPT_COUNT must all
 * be defined. If any of these do not apply, artificially bump OPT_COUNT
 * by one, and set the value of the unused arguments to OPT_COUNT-1
 */
#define TEMPLATE     "CX_PRIORITY/N/K" VERSTAG
#define OPT_PRIORITY 0
#define OPT_POPKEY   1
#define OPT_POPUP    1
#define OPT_COUNT    2


/*****************************************************************************/


/* Commodities specific definitions.
 *
 * CE_NAME        - Name of the program
 * CE_TITLE       - Title, for window and Exchange program
 * CE_DESCRIPTION - Description of what the program does, used by Exchange
 * CE_PRIORITY    - default priority for this commody's broker
 * CE_POPKEY      - default hotkey sequence to open this commoditie's window
 * CE_POPUP       - whether to open this commodity's window on startup
 * CE_WINDOW      - set to 1 if this commodity has a window, 0 if not
 */
#define CE_NAME         MSG_MB_NAME
#define CE_TITLE        MSG_MB_TITLE
#define CE_DESCRIPTION  MSG_MB_DESCRIPTION
#define CE_PRIORITY     0
#define CE_POPKEY       ""
#define CE_POPUP        FALSE
#define CE_WINDOW       0


/*****************************************************************************/


BOOL CreateCustomCx(CxObj *broker);
VOID DisposeCustomCx(VOID);

BOOL ProcessCustomArgs(struct WBStartup *wbMsg, struct DiskObj *diskObj, ULONG *cliOpts);
VOID ProcessCustomCxMsg(ULONG cmd);
VOID ProcessCustomCxCmd(ULONG cmd);
VOID ProcessCustomCxSig(VOID);


/*****************************************************************************/


#endif /* CE_CUSTOM_H */
