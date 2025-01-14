#include <exec/types.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <exec/ports.h>
#include <exec/lists.h>
#include <exec/io.h>
#include <exec/nodes.h>
#include <exec/resident.h>
#include <exec/tasks.h>
#include <exec/execbase.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>

#include <devices/trackdisk.h>

#include <workbench/startup.h>
#include <workbench/workbench.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <graphics/text.h>

#include <resources/cia.h>
#include <hardware/cia.h>
#include <resources/card.h>
#include <libraries/asl.h>

#include "flash_rev.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/locale_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/cia_protos.h>
#include <clib/cardres_protos.h>
#include <clib/alib_protos.h>
#include <clib/asl_protos.h>

extern struct Library *SysBase;
extern struct Library *DOSBase;

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/cardres_pragmas.h>
#include <pragmas/asl_pragmas.h>

#pragma libcall cv->cv_CIABase AddICRVector 6 90E03
#pragma libcall cv->cv_CIABase RemICRVector C 90E03
#pragma libcall cv->cv_CIABase AbleICR 12 0E02
#pragma libcall cv->cv_CIABase SetICR 18 0E02

/* define for data cache control enable */

#define IFCACHECONTROL	1

/* Gadget actions */

#define CM_GADGET_SOURCE	1
#define CM_GADGET_ERASE		2
#define CM_GADGET_CHECK		3
#define CM_GADGET_DUP		4
#define CM_GADGET_ERASEON	5
#define CM_GADGET_WVERIFY	6
#define CM_GADGET_INSTALL	7
#define CM_GADGET_ABORT		8
#define CM_GADGET_CONFIRM	9
#define	CM_GADGET_MANUFACTURER	10
#define	CM_GADGET_TOTALSIZE	11
#define	CM_GADGET_ZONESIZE	12
#define	CM_GADGET_SPEED		13

/* For ReadArgs	*/

#define TEMPLATE    "HELP/S" VERSTAG
#define OPT_HELP	0
#define OPT_COUNT	1

/* c->asm flash support interface structure */

struct ZoneHandle {
	APTR	zh_to;
	APTR	zh_from;
	ULONG	zh_size;
	APTR	zh_timer;
	ULONG	*zh_signals;
	ULONG	zh_sigmask;
	APTR	zh_cv;
};

#define FILENAME_SIZEOF	128

struct cmdVars {
	struct  MsgPort	cv_ParentPort;

	struct	ExecBase *cv_SysBase;
	struct	Library *cv_DOSBase;
	struct	Library *cv_IntuitionBase;
	struct	Library *cv_GfxBase;
	struct	Library *cv_GadToolsBase;
	struct	Library *cv_LocaleBase;
	struct  Library *cv_CardResource;
	struct	Library *cv_WorkbenchBase;
	struct	Library *cv_AslBase;
	struct	CIABase *cv_CIABase;

	struct	WBStartup *cv_WBenchMsg;

	struct 	FileRequester	*cv_freq;
	struct	Task		*cv_Task;
	struct	CardMemoryMap	*cv_CardMemMap;
	struct	DrawInfo	*cv_di;

	ULONG			cv_Signal;

	LONG	cv_opts[OPT_COUNT];	

	struct	CardHandle	cv_CardHandle;

	struct	Interrupt	cv_Inserted;
	struct	Interrupt	cv_Removed;
	struct	Interrupt	cv_StatusChange;
	struct	Interrupt	cv_ciabint;


	struct	ZoneHandle	cv_ZoneHandle;

	struct	Font		*cv_font;
	struct	Screen		*cv_sp;
	APTR			cv_VI;

	struct	Window		*cv_win;
	struct	Window		*cv_awin;
	struct	Menu		*cv_begmenu;

	struct	Gadget		*cv_gadgets;
	struct	Gadget		*cv_GADS;

	struct	Gadget		*cv_Source;

	struct	Gadget		*cv_EraseOnly;
	struct	Gadget		*cv_CheckBlank;
	struct	Gadget		*cv_Duplicate;

	struct	Gadget		*cv_EraseFirst;
	struct	Gadget		*cv_VerifyWrite;
	struct	Gadget		*cv_Bootable;

	struct	Gadget		*cv_Abort;
	struct	Gadget		*cv_Confirm;

	struct	Gadget		*cv_Manufacturer;
	struct	Gadget		*cv_TotalSize;
	struct	Gadget		*cv_ZoneSize;
	struct	Gadget		*cv_Speeds;

	ULONG			cv_ciatimerbit;

	ULONG			cv_ZoneBufSize;
	UWORD			cv_SourceIndex;
	UWORD			cv_ManufIndex;
	UWORD			cv_TotalIndex;
	UWORD			cv_ZoneIndex;
	UWORD			cv_SpeedIndex;


	BOOL			cv_EraseOn;
	BOOL			cv_VerifyOn;
	BOOL			cv_BootOn;

	BOOL			cv_FromCLI;

	BOOL			cv_IsInserted;
	BOOL			cv_IsRemoved;
	BOOL			cv_NoCard;
	BOOL			cv_CardInUse;

	BOOL			cv_QuitProgram;

	UBYTE			cv_Filename[FILENAME_SIZEOF];
};

/* pragma redirections */

#define SysBase cv->cv_SysBase
#define DOSBase cv->cv_DOSBase
#define IntuitionBase cv->cv_IntuitionBase
#define GfxBase cv->cv_GfxBase
#define GadToolsBase cv->cv_GadToolsBase
#define LocaleBase cv->cv_LocaleBase
#define CardResource cv->cv_CardResource
#define WorkbenchBase cv->cv_WorkbenchBase
#define AslBase cv->cv_AslBase
#define CIABBase cv->cv_CIABase
#define WBenchMsg cv->cv_WBenchMsg
#define freq cv->cv_freq

/* some strings */

#define FLASH_TOOL	"FlashTool"
#define FLASH_TITLE	"Amiga to Flash"

/* structure for gadget creation */

struct GadGadget {
	ULONG	gg_GadgetKind;
	WORD	gg_LeftEdge;
	WORD	gg_TopEdge;
	WORD	gg_Width;
	WORD	gg_Height;
	WORD	gg_Command;
	UBYTE	*gg_Label;
	ULONG	gg_GadgetFlags;
};

/* Box ID's */

#define WIND_BOX_ID	0
#define	STAT_BOX_ID	1

/* Prototypes */

void SetUpWindow( struct cmdVars *cv, struct Window *win, struct Gadget *gad );
void __asm RemovedInt(register __a1 struct cmdVars *cv );
void __asm InsertInt(register __a1 struct cmdVars *cv );
UBYTE __asm StatusInt(register __a1 struct cmdVars *cv, register __d0 UBYTE status );
void __asm TimerInt(register __a1 struct cmdVars *cv );
void HandleEvents( struct cmdVars *cmv );
void BevelBox( struct cmdVars *cv, struct RastPort *rp, WORD l, WORD t, WORD w, WORD h, ULONG tag1, ...);
void DisplayError( struct cmdVars *cv, STRPTR str );
BOOL DisplayQuery( struct cmdVars *cv, STRPTR str );
LONG DoEasyRequest( struct cmdVars *cv, STRPTR body, STRPTR gadgets );
VOID ChangeAttrs(struct cmdVars *cv, struct Gadget *gad, ULONG tags, ... );
void ConfirmOnOff( struct cmdVars *cv, BOOL disable );
void AbortOnOff( struct cmdVars *cv, BOOL disable );
VOID StatusBox( struct cmdVars *cmv, UWORD col, UWORD row, BOOL centered, UWORD boxid, STRPTR line );
VOID ClearBox(struct cmdVars *cmv, UWORD boxid);
int LookUpFLASH( struct cmdVars *cv );
int GetStatus( struct cmdVars *cv );
void DisplayCardStatus( struct cmdVars *cv, STRPTR str, int status, BOOL blink );
void Stabilize1MS( struct ZoneHandle *zh );
BOOL EraseZone( struct ZoneHandle *zh );
BOOL WriteZone( struct ZoneHandle *zh );
BOOL CheckBlank( struct ZoneHandle *zh );
void DoCheck( struct cmdVars *cmv );
void DoErase( struct cmdVars *cmv );
void DoProgram( struct cmdVars *cmv );
void DoSprintF( UBYTE *buf, STRPTR fmt, long arg1, ... );
int PromptForInsert( struct cmdVars *cv, STRPTR str );
BOOL __asm CheckAbort(register __a1 struct cmdVars *cv );
BOOL VerifyData( struct ZoneHandle *zh);
void RadBootBlock( struct cmdVars *cv );


#define FREQ_LOAD 0
#define FREQ_SAVE 1

UBYTE  *getfilename(struct cmdVars *cv, UBYTE *title,UBYTE *buffer,ULONG bufsize,
			struct Window *win, BOOL IsSave);

