#ifndef PREP_CARD_H
#define	PREP_CARD_H

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <exec/ports.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/resident.h>
#include <exec/tasks.h>
#include <exec/execbase.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>

#include <workbench/startup.h>
#include <workbench/workbench.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <graphics/text.h>

#include <resources/card.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/locale_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/diskfont_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/cardres_pragmas.h>

#include "prepcard_rev.h"

#define PREPCARD
#define CATCOMP_NUMBERS 1
#include "prepcard_text.h"

#ifdef  DEBUG
extern void kprintf(char *,...);
#define D(a)    kprintf a
#else
#define D(a)
#endif

/* defines */

#define CATALOG		"sys/prepcard.catalog"

/* Basic Functions */

#define	CM_DEFAULT	0
#define CM_MAKEDISK	1
#define CM_MAKERAM	2
#define CM_ADV		3
#define CM_QUIT		4

/* Gadget ID's */

#define CM_GADGET_TYPE	0
#define CM_GADGET_SPEED	1
#define CM_GADGET_SIZE	2
#define CM_GADGET_UNITS	3
#define CM_GADGET_EDC	4
#define CM_GADGET_BSIZE	5
#define CM_GADGET_SECT	6
#define CM_GADGET_TRKC	7
#define CM_GADGET_CYLS	8
#define CM_GADGET_TSEC	9
#define CM_GADGET_GEO	10
#define CM_GADGET_RESET	11
#define	CM_GADGET_CONT	12

/* For ReadArgs	*/

#define TEMPLATE    "DISK/S,RAM/S" VERSTAG
#define OPT_DISK	0
#define OPT_RAM		1
#define OPT_COUNT	2

/* port names */

#define PARENT_PORT	"PrepCard.Parent"
#define CHILD_PORT	"PrepCard.Child"

/* commands to child */

#define	CHILD_QUIT	0
#define	CHILD_COPYTUPLE	1
#define CHILD_WAKEME	2

/* others */

#define MINDISKBLOCKS	15L
#define MAXDISKSIZE	(ULONG)(4*1024*1024)

/* structure, and its a PORT too!! */

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
        struct  Library *cv_DiskfontBase;

	struct	Catalog *cv_Catalog;

	struct	MsgPort *cv_ReplyPort;
	struct  MsgPort *cv_ChildPort;

	struct	WBStartup *cv_WBenchMsg;

	LONG	cv_opts[OPT_COUNT];	

	ULONG	cv_Signal;
	ULONG	cv_ChildSignal;

	struct	Task		*cv_Task;
	struct	Task		*cv_ChildTask;
	struct	CardMemoryMap	*cv_CardMemMap;

	struct	CardHandle	cv_CardHandle;

	struct	Interrupt	cv_Inserted;
	struct	Interrupt	cv_Removed;

	struct	List		cv_StatsList;

	struct	SignalSemaphore	cv_CardSemaphore;

	struct	Gadget		*cv_gadgets;
	struct	Gadget		*cv_GADS;
	struct	Gadget		*cv_FormatGad;
	struct	Gadget		*cv_RamGad;
	struct	Gadget		*cv_QuitGad;

	struct  Gadget		*cv_advgadgets;
	struct	Gadget		*cv_advGADS;
	struct	Gadget		*cv_TypeGad;
	struct	Gadget		*cv_SpeedGad;
	struct	Gadget		*cv_UnitsGad;
	struct	Gadget		*cv_UnitNumGad;
	struct	Gadget		*cv_TotalSizeGad;
	struct	Gadget		*cv_EDCGad;
	struct	Gadget		*cv_BKSZGad;
	struct  Gadget		*cv_MaxBlocksGad;
	struct	Gadget		*cv_EnterBlkGad;
	struct	Gadget		*cv_EnterSecGad;
	struct	Gadget		*cv_EnterTrkGad;
	struct	Gadget		*cv_EnterCylGad;
	struct	Gadget		*cv_EnableGeoGad;
	struct	Gadget		*cv_AdvCanGad;
	struct	Gadget		*cv_AdvConGad;
	struct	Gadget		*cv_DevLabGad;
	struct	Gadget		*cv_DiskLabGad;
	struct	Gadget		*cv_GeoLabGad;

	struct	Font		*cv_font;
	struct	Screen		*cv_sp;
	APTR			cv_VI;

	struct	Window		*cv_win;
	struct	Window		*cv_awin;
	struct	Menu		*cv_begmenu;

	STRPTR			cv_Types[MSG_PREP_CARD_DRAM - MSG_PREP_CARD_SRAM + 2];
	STRPTR			cv_Speeds[MSG_PREP_SPEED_100NS - MSG_PREP_SPEED_250NS + 2];
	STRPTR			cv_Units[MSG_PREP_UNITS_UNKNOWN - MSG_PREP_UNITS_512 + 2];
	STRPTR			cv_EDC[MSG_PREP_EDCC_CRC - MSG_PREP_EDCC_NONE + 2];
	STRPTR			cv_BKSZ[MSG_PREP_BKSZ_2048 - MSG_PREP_BKSZ_128 + 2];

	ULONG			cv_TotalSize;
	ULONG			cv_TotalBlocks;
	ULONG			cv_GeoBlocks;
	ULONG			cv_GeoSecs;
	ULONG			cv_GeoTrks;
	ULONG			cv_GeoCyls;

	UWORD			cv_DefaultType;
	UWORD			cv_DefaultSpeed;
	UWORD			cv_DefaultUnits;
	UWORD			cv_DefaultUnitNum;
	UWORD			cv_DefaultEDC;
	UWORD			cv_DefaultBKSZ;
	UWORD			cv_DefaultGEO;

	UWORD			cv_RetryCard;

	BOOL			cv_IsCORRUPT;

	BOOL			cv_FromCLI;

	BOOL			cv_IsInserted;
	BOOL			cv_IsRemoved;
	BOOL			cv_NoCard;
	BOOL			cv_CardInUse;

	BOOL			cv_IsBusy;
	BOOL			cv_PrepBusy;

	BOOL			cv_IsAdvanced;

	BOOL			cv_GadUnitNumDis;
	BOOL			cv_GadGeoDis;

	BOOL			cv_FirstInfoDraw;

	UBYTE			cv_TotalSizeText[12];
	UBYTE			cv_MaxBlocksText[12];

};

/* structure for gadget creation */

struct GadGadget {
	ULONG	gg_GadgetKind;
	WORD	gg_LeftEdge;
	WORD	gg_TopEdge;
	WORD	gg_Width;
	WORD	gg_Height;
	WORD	gg_Command;
	ULONG	gg_Label;
	ULONG	gg_GadgetFlags;
};

/* Parent/Child message */

struct PrepMsg {
	struct	Message pm_msg;
	ULONG		pm_Command;
	APTR		pm_Data;
};	

/* Pass args to Child for CopyTuple */

struct TupleMsg {
	struct CardHandle	*tm_ch;
	UBYTE 			*tm_tuplebuf;
	ULONG 			tm_tuplecode;
	ULONG			tm_tuplesize;
	BOOL			tm_result;

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
#define DiskfontBase cv->cv_DiskfontBase
#define WBenchMsg cv->cv_WBenchMsg


/* protos for functions */

/* main */

LONG main( VOID );
LONG StartUI( struct cmdVars *cmv ); 
void __asm InsertInt(register __a1 struct cmdVars *cv );
void __asm RemovedInt(register __a1 struct cmdVars *cv );
VOID PutChildMsg( struct cmdVars *cmv, ULONG command, APTR data );

/* childtask */

VOID __asm PrepCardTask( register __a2 struct cmdVars *cv, register __a3 struct MsgPort *childport );

/* startchild */

VOID StartChild( VOID );

/* preptext */

STRPTR GetString( struct cmdVars *cv, ULONG id );
VOID DisplayError( struct cmdVars *cv, ULONG id );
BOOL DisplayQuery( struct cmdVars *cv, ULONG id );
LONG DoEasyRequest( struct cmdVars *cv, STRPTR body, STRPTR gadgets );

/* ui */

LONG StartGUI( struct cmdVars *cmv );
LONG StartCLI( struct cmdVars *cmv );

/* gadgets */

struct Gadget *CreateAGadget( struct cmdVars *cmv, struct Gadget **GADS, struct GadGadget *gg, ULONG tags, ... );
BOOL MakeGadgets( struct cmdVars *cmv );
BOOL MakeAdvGadgets( struct cmdVars *cmv );
VOID Defaults( struct cmdVars *cmv );
VOID MakeAdvDisplay( struct cmdVars *cmv );
VOID DisplayUnits( struct cmdVars *cmv );
VOID DisplayMaxBlocks( struct cmdVars *cmv );
VOID DisplayGeoStuff( struct cmdVars *cmv, UWORD code );
VOID NewGadget( struct cmdVars *cmv, struct Gadget *gad, UWORD code );
VOID ChangeAdvAttrs( struct cmdVars *cv, struct Gadget *gad, ULONG tags, ... );
VOID ChangeBegAttrs(struct cmdVars *cv, struct Gadget *gad, ULONG tags, ... );
VOID DisabledPrep( struct cmdVars *cv, BOOL disable );
VOID DoSPrintF( UBYTE *buf, STRPTR fmt, long arg1, ... );
VOID CalcBestGeometry( LONG blocks, LONG *sectrk, LONG *trkcyl, LONG *cyl, BOOL linear);

/* windows */

BOOL MakeWindow( struct cmdVars *cmv );
BOOL MakeAdvWindow( struct cmdVars *cmv );
VOID CloseAdvWindow( struct cmdVars *cv );
VOID SetUpWindow( struct cmdVars *cv, struct Window *win, struct Gadget *gad );

/* events */

VOID HandleEvents( struct cmdVars *cmv );

/* commands */

LONG DoCommand( struct cmdVars *cmv, ULONG command );

/* check */

VOID CardCheck( struct cmdVars *cmv );
BOOL ChildCopyTuple( struct cmdVars *cmv, struct CardHandle *ch, UBYTE *tuplebuf, ULONG tuplecode, ULONG tuplesize );
VOID DrawDeviceInfo( struct cmdVars *cmv, ULONG type, ULONG speed, ULONG size );
VOID DrawFormatInfo( struct cmdVars *cmv, ULONG devtype, BOOL isFORMAT, BOOL isXIP, BOOL isDISK, BOOL isPSEUDO, BOOL isCARDMARK );
VOID DrawGeoInfo( struct cmdVars *cmv, ULONG nblocks, UWORD blocksize, BOOL isGEO, UWORD SecTrk, UWORD TrkCyl, UWORD Cyls );
VOID DrawBatteryInfo( struct cmdVars *cmv, ULONG devtype, UBYTE status );
VOID DrawVers1Info( struct cmdVars *cmv, struct TP_Vers_1 *tpv );
VOID DrawVers2Info( struct cmdVars *cmv, struct TP_Vers_2 *tpv, BOOL isVERS2 );
UBYTE *IsString( UBYTE *start, UBYTE *max );
VOID PrintLine(UBYTE *buf, UWORD maxlen, BOOL center, STRPTR fmt, long arg1, ... );
VOID InfoBox( struct cmdVars *cmv, STRPTR label, UWORD x, UWORD y, UWORD maxlen, STRPTR lines[] );
VOID BevelBox( struct cmdVars *cv, struct RastPort *rp, WORD l, WORD t, WORD w, WORD h, ULONG tag1, ...);

/* menus */

struct Menu *CreateAMenuStrip( struct cmdVars *cv, struct NewMenu *nm, struct TagList *tags );
BOOL DoLayoutMenus( struct cmdVars *cv, struct Menu *menu, ULONG tags, ... );
BOOL MakeBegMenu( struct cmdVars *cmv );

/* prep */

ULONG PrepCard( struct cmdVars *cmv, BOOL disk );

/* tinysprintf.asm */

VOID SPrintF( UBYTE *buf, STRPTR fmt, LONG *arg1 );
VOID SPrintC( ULONG *len, STRPTR fmt, LONG *arg1 );

/* protos for card.resource - temporary */
#ifndef CLIB_CARDRES_PROTOS_H

struct CardMemoryMap *GetCardMap( VOID );
struct CardHandle *OwnCard( struct CardHandle * );
BOOL CopyTuple( struct CardHandle *ch, UBYTE *buf, ULONG code, ULONG size );
BOOL CardForceChange( VOID );
VOID ReleaseCard( struct CardHandle *ch, ULONG flags );
ULONG DeviceTuple( UBYTE *data, struct DeviceTData *store );
UBYTE ReadCardStatus( VOID );
struct Resident *IfAmigaXIP( struct CardHandle *);
BOOL BeginCardAccess( struct CardHandle *);
BOOL EndCardAccess( struct CardHandle *);
#endif	/* CLIB_CARDRES_PROTOS_H */
#endif	/* PREP_CARD_H */

