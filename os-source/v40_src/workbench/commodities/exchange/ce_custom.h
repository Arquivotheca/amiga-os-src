#ifndef CE_CUSTOM_H
#define CE_CUSTOM_H


/*****************************************************************************/


#include <exec/types.h>
#include <libraries/commodities.h>
#include <workbench/startup.h>
#include "exchange_rev.h"

#define CX_EXCHANGE
#include <localestr/commodities.h>


/*****************************************************************************/


/* define the editor's command-line template here
 * OPT_PRIORITY, OPT_POPKEY, OPT_POPUP and OPT_COUNT must all
 * be defined. If any of these do not apply, artificially bump OPT_COUNT
 * by one, and set the value of the unused arguments to OPT_COUNT-1
 */
#define TEMPLATE        "CX_PRIORITY/N/K,CX_POPKEY/K,CX_POPUP/K" VERSTAG
#define OPT_PRIORITY    0
#define OPT_POPKEY      1
#define OPT_POPUP       2
#define OPT_COUNT       3


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
#define CE_NAME         MSG_EX_NAME
#define CE_TITLE        MSG_EX_TITLE
#define CE_DESCRIPTION  MSG_EX_DESCRIPTION
#define CE_PRIORITY     0
#define CE_POPKEY       "control alt help"
#define CE_POPUP        TRUE
#define CE_WINDOW       1


/*****************************************************************************/


#define CE_POPKEYID 86


/*****************************************************************************/


#define WINDOW_LEFT   0
#define WINDOW_TOP    0
#define WINDOW_WIDTH  566
#define WINDOW_HEIGHT 82

#define IDCMPFLAGS  BUTTONIDCMP | LISTVIEWIDCMP | IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_MENUPICK
#define WINDOWFLAGS WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_ACTIVATE


/*****************************************************************************/


BOOL CreateCustomCx(CxObj *broker);
VOID DisposeCustomCx(VOID);

BOOL ProcessCustomArgs(struct WBStartup *wbMsg, struct DiskObject *diskObj, ULONG *cliOpts);
VOID ProcessCustomCxMsg(ULONG cmd);
VOID ProcessCustomCxCmd(ULONG cmd);
VOID ProcessCustomCxSig(VOID);
BOOL ProcessIntuiMsg(struct IntuiMessage *intuiMsg);

BOOL CreateCustomGadgets(VOID);
BOOL CreateCustomMenus(VOID);
VOID RefreshWindow(BOOL refreshEvent);


/*****************************************************************************/


#endif /* CE_CUSTOM_H */
