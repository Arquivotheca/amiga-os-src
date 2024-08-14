/*	structs.h - Amiga system structure defines
**	$VER: structs.h 37.1 (18.6.91)
**
**	900109 (MEW) -- created with 1.3
**	900928 (MEW) -- updated to v36
**	910618 (MEW) -- updated to v37
*/


#ifndef	STRUCTS_H /*(*/
#define	STRUCTS_H


/* devices/audio.h */
#define	IOAUDIO		struct IOAudio

/* devices/bootblock.h */
#define	BOOTBLOCK	struct BootBlock

/* devices/clipboard.h */
#define	CLIPBOARDUNITPARTIAL	struct ClipboardUnitPartial
#define	IOCLIPREQ	struct IOClipReq
#define	SATISFYMSG	struct SatisfyMsg
/* v36 */
#define CLIPHOOKMSG	struct ClipHookMsg

/* devices/console.h */

/* devices/conunit.h */
#define	CONUNIT		struct ConUnit

/* devices/gameport.h */
#define	GAMEPORTTRIGGER	struct GamePortTrigger

/* devices/hardblocks.h */
#define	RIGIDDISKBLOCK	struct RigidDiskBlock
#define	BADBLOCKENTRY	struct BadBlockEntry
#define	BADBLOCKBLOCK	struct BadBlockBlock
#define	PARTITIONBLOCK	struct PartitionBlock
#define	FILESYSHEADERBLOCK	struct FileSysHeaderBlock
#define	LOADSEGBLOCK	struct LoadSegBlock

/* devices/input.h */

/* devices/inputevent.h */
#define	INPUTEVENT	struct InputEvent
/* v36 */
#define IEPOINTERPIXEL	struct IEPointerPixel
#define IEPOINTERTABLET	struct IEPointerTablet

/* devices/keyboard.h */

/* devices/keymap.h */
#define	KEYMAP		struct KeyMap
#define	KEYMAPNODE	struct KeyMapNode
#define	KEYMAPRESOURCE	struct KeyMapResource

/* devices/narrator.h */
#define	NARRATOR_RB	struct narrator_rb
#define	MOUTH_RB	struct mouth_rb

/* devices/parallel.h */
#define	IOPARRAY	struct IOPArray
#define	IOEXTPAR	struct IOExtPar

/* devices/printer.h */
#define	IOPRTCMDREQ	struct IOPrtCmdReq
#define	IODRPREQ	struct IODRPReq

/* devices/prtbase.h */
#define	DEVICEDATA	struct DeviceData
#define	PRINTERDATA	struct PrinterData
#define	PRINTEREXTENDEDDATA	struct PrinterExtendedData
#define	PRINTERSEGMENT	struct PrinterSegment

/* devices/prtgfx.h */
#define	PRTINFO		struct PrtInfo

/* devices/scsidisk.h */
#define	SCSICMD		struct SCSICmd

/* devices/serial.h */
#define	IOTARRAY	struct IOTArray
#define	IOEXTSER	struct IOExtSer

/* devices/timer.h */
#define	TIMEVAL		struct timeval
#define	TIMEREQUEST	struct timerequest
/* v36 */
#define ECLOCKVAL	struct EClockVal

/* devices/trackdisk.h */
#define	IOEXTTD		struct IOExtTD
#define	TDU_PUBLICUNIT	struct TDU_PublicUnit
/* v36 */
#define DRIVEGEOMETRY	struct DriveGeometry

/* dos/datetime.h */
/* v36 */
#define DATETIME	struct DateTime

/* dos/dos.h */
#define	DATESTAMP	struct DateStamp
#define	FILEINFOBLOCK	struct FileInfoBlock
#define	INFODATA	struct InfoData

/* dos/dosasl.h */
/* v36 */
#define ANCHORPATH	struct AnchorPath
#define ACHAIN	struct AChain

/* dos/dosextens.h */
/* v36 */
#define	PROCESS		struct Process
#define	FILEHANDLE	struct FileHandle
#define	DOSPACKET	struct DosPacket
#define	STANDARDPACKET	struct StandardPacket
#define	DOSLIBRARY	struct DosLibrary
#define	ROOTNODE	struct RootNode
#define	DOSINFO		struct DosInfo
#define	COMMANDLINEINTERFACE	struct CommandLineInterface
#define	DEVICELIST	struct DeviceList
#define	DEVINFO		struct DevInfo
#define	DOSLIST		struct DosList
#define	FILELOCK	struct FileLock
/* v36 */
#define ERRORSTRING	struct ErrorString
#define CLIPROCLIST	struct CliProcList
#define ASSIGNLIST	struct AssignList
#define DEVPROC	struct DevProc
/* v37 */
#define SEGMENT		struct Segment

/* dos/doshunks.h */

/* dos/dostags.h */

/* dos/exall.h */
/* v36 */
#define EXALLDATA	struct ExAllData
#define EXALLCONTROL	struct ExAllControl

/* dos/filehandler.h */
#define	DOSENVEC	struct DosEnvec
#define	FILESYSSTARTUPMSG	struct FileSysStartupMsg
#define	DEVICENODE	struct DeviceNode

/* dos/notify.h */
/* v36 */
#define NOTIFYMESSAGE	struct NotifyMessage
#define NOTIFYREQUEST	struct NotifyRequest

/* dos/rdargs.h */
/* v36 */
#define CSOURCE	struct CSource
#define RDARGS	struct RDArgs

/* dos/record.h */
/* v36 */
#define RECORDLOCK	struct RecordLock

/* dos/stdio.h */

/* dos/var.h */
/* v36 */
#define LOCALVAR	struct LocalVar

/* exec/alerts.h */

/* exec/devices.h */
#define	DEVICE		struct Device
#define	UNIT		struct Unit

/* exec/errors.h */

/* exec/exec.h */

/* exec/execbase.h */
#define	EXECBASE	struct ExecBase

/* exec/initializers.h */

/* exec/interrupts.h */
#define	INTERRUPT	struct Interrupt
#define	INTVECTOR	struct IntVector
#define	SOFTINTLIST	struct SoftIntList

/* exec/io.h */
#define	IOREQUEST	struct IORequest
#define	IOSTDREQ	struct IOStdReq

/* exec/libraries.h */
#define	LIBRARY		struct Library

/* exec/lists.h */
#define	EXECLIST	struct List		/** LIST already used by iff/iff.h **/
#define	MINLIST		struct MinList

/* exec/memory.h */
#define	MEMCHUNK	struct MemChunk
#define	MEMHEADER	struct MemHeader
#define	MEMENTRY	struct MemEntry
#define	MEMLIST		struct MemList

/* exec/nodes.h */
#define	NODE		struct Node
#define	MINNODE		struct MinNode

/* exec/ports.h */
#define	MSGPORT		struct MsgPort
#define	MESSAGE		struct Message

/* exec/resident.h */
#define	RESIDENT	struct Resident

/* exec/semaphores.h */
#define	SEMAPHORE	struct Semaphore
#define	SEMAPHOREREQUEST	struct SemaphoreRequest
#define	SIGNALSEMAPHORE	struct SignalSemaphore

/* exec/tasks.h */
#define	TASK		struct Task

/* exec/types.h */

/* graphics/clip.h */
#define	LAYER		struct Layer
#define	CLIPRECT	struct ClipRect

/* graphics/collide.h */

/* graphics/copper.h */
#define	COPINS		struct CopIns
#define	CPRLIST		struct cprlist
#define	COPLIST		struct CopList
#define	UCOPLIST	struct UCopList
#define	COPINIT		struct copinit

/* graphics/display.h */

/* graphics/displayinfo.h */
/* v36 */
#define QUERYHEADER	struct QueryHeader
#define DISPLAYINFO	struct DisplayInfo
#define DIMENSIONINFO	struct DimensionInfo
#define MONITORINFO	struct MonitorInfo
#define NAMEINFO	struct NameInfo

/* graphics/gels.h */
/* #define	VSPRITE	struct VSprite *** #define already in use *************/
#define	BOB			struct Bob
#define	ANIMCOMP	struct AnimComp
#define	ANIMOB		struct AnimOb
#define	DBUFPACKET	struct DBufPacket
#define	COLLTABLE	struct collTable

/* graphics/gfx.h */
#define	RECTANGLE	struct Rectangle
#define	TPOINT		struct tPoint
#define	BITMAP		struct BitMap
/* v36 */
#define RECT32	struct Rect32

/* graphics/gfxbase.h */
#define	GFXBASE		struct GfxBase

/* graphics/gfxmacros.h */

/* graphics/gfxnodes.h */
/* v36 */
#define EXTENDEDNODE	struct ExtendedNode

/* graphics/graphint.h */
#define	ISRVSTR		struct Isrvstr

/* graphics/layers.h */
#define	LAYER_INFO	struct Layer_Info

/* graphics/monitor.h */
/* v36 */
#define MONITORSPEC	struct MonitorSpec
#define ANALOGSIGNALINTERVAL	struct AnalogSignalInterval
#define SPECIALMONITOR	struct SpecialMonitor

/* graphics/rastport.h */
#define	AREAINFO	struct AreaInfo
#define	TMPRAS		struct TmpRas
#define	GELSINFO	struct GelsInfo
#define	RASTPORT	struct RastPort

/* graphics/regions.h */
#define	REGIONRECTANGLE	struct RegionRectangle
#define	REGION		struct Region

/* graphics/scale.h */
/* v37 */
#define BITSCALEARGS	struct BitScaleArgs

/* graphics/sprite.h */
#define	SIMPLESPRITE	struct SimpleSprite

/* graphics/text.h */
#define	TEXTATTR	struct TextAttr
#define	TEXTFONT	struct TextFont
/* v36 */
#define TTEXTATTR	struct TTextAttr
#define TEXTFONTEXTENSION	struct TextFontExtension
#define COLORFONTCOLORS	struct ColorFontColors
#define COLORTEXTFONT	struct ColorTextFont
#define TEXTEXTENT	struct TextExtent

/* graphics/videocontrol.h */

/* graphics/view.h */
#define	COLORMAP	struct ColorMap
#define	VIEWPORT	struct ViewPort
#define	VIEW		struct View
#define	RASINFO		struct RasInfo
/* v36 */
#define VIEWEXTRA	struct ViewExtra
#define VIEWPORTEXTRA	struct ViewPortExtra

/* hardware/adkbits.h */

/* hardware/blit.h */
#define	BLTNODE		struct bltnode

/* hardware/cia.h */
/* #define	CIA	struct CIA *** struct name already all capitals *********/

/* hardware/custom.h */
/* #define	CUSTOM	struct Custom *** #define already in use ****************/

/* hardware/dmabits.h */

/* hardware/intbits.h */

/* intuition/cghooks.h */
/* v36 */
#define	GADGETINFO	struct GadgetInfo

/* intuition/classes.h */
#define ICLASS		struct IClass
/* #define _OBJECT		struct _Object */

/* intuition/classusr.h */
#define OPSET		struct opSet
#define OPUPDATE	struct opUpdate
#define OPGET		struct opGet
#define OPADDTAIL	struct opAddTail
#define OPMEMBER	struct opMember

/* intuition/gadgetclass.h */
#define GPHITTEST	struct gpHitTest
#define GPRENDER	struct gpRender
#define GPINPUT		struct gpInput
#define GPGOINACTIVE	struct gpGoInactive

/* intuition/icclass.h */

/* intuition/imageclass.h */
#define IMPFRAMEBOX	struct impFrameBox
#define IMPDRAW		struct impDraw
#define IMPERASE	struct impErase
#define IMPHITTEST	struct impHitTest

/* intuition/intuition.h */
#define	MENU		struct Menu
#define	MENUITEM	struct MenuItem
#define	REQUESTER	struct Requester
#define	GADGET		struct Gadget
#define	BOOLINFO	struct BoolInfo
#define	PROPINFO	struct PropInfo
#define	STRINGINFO	struct StringInfo
#define	INTUITEXT	struct IntuiText
#define	BORDER		struct Border
#define	IMAGE		struct Image
#define	INTUIMESSAGE	struct IntuiMessage
#define	WINDOW		struct Window
#define	NEWWINDOW	struct NewWindow
#define	REMEMBER	struct Remember
/* v36 */
#define	IBOX		struct IBox
#define COLORSPEC	struct ColorSpec
#define EASYSTRUCT	struct EasyStruct
#define EXTNEWWINDOW	struct ExtNewWindow

/* intuition/intuitionbase.h */
#define	INTUITIONBASE	struct IntuitionBase

/* intuition/preferences.h */
#define	PREFERENCES	struct Preferences

/* intuition/screens.h */
#define	SCREEN		struct Screen
#define	NEWSCREEN	struct NewScreen
/* v36 */
#define DRAWINFO	struct DrawInfo
#define EXTNEWSCREEN	struct ExtNewScreen
#define PUBSCREENNODE	struct PubScreenNode

/* intuition/sghooks.h */
/* v36 */
/* temporarily disabled cuz of duplicate in iobsolete.h...
#define STRINGEXTEND	struct StringExtend
*/
#define STRINGEXT	struct StringExtend
#define SGWORK	struct SGWork

/* libraries/asl.h */
/* v36 */
#define FILEREQUESTER	struct FileRequester
#define FONTREQUESTER	struct FontRequester

/* libraries/commodities.h */
/* v36 */
#define NEWBROKER	struct NewBroker
/*#define INPUTXPRESSION	struct InputXpression		/* use IX instead */

/* libraries/configregs.h */
#define	EXPANSIONROM	struct ExpansionRom
#define	EXPANSIONCONTROL	struct ExpansionControl
#define	DIAGAREA	struct DiagArea

/* libraries/configvars.h */
#define	CONFIGDEV	struct ConfigDev
#define	CURRENTBINDING	struct CurrentBinding

/* libraries/diskfont.h */
#define	FONTCONTENTS	struct FontContents
#define	FONTCONTENTSHEADER	struct FontContentsHeader
#define	DISKFONTHEADER	struct DiskFontHeader
#define	AVAILFONTS	struct AvailFonts
#define	AVAILFONTSHEADER	struct AvailFontsHeader
/* v36 */
#define TFONTCONTENTS	struct TFontContents
#define TAVAILFONTS	struct TAvailFonts

/* libraries/dos.h */

/* libraries/dosextens.h */

/* libraries/expansion.h */

/* libraries/expansionbase.h */
#define	EXPANSIONBASE	struct ExpansionBase
/* v36 */
#define BOOTNODE	struct BootNode

/* libraries/filehandler.h */

/* libraries/gadtools.h */
/* v36 */
#define NEWGADGET	struct NewGadget
#define NEWMENU	struct NewMenu

/* libraries/iffparse.h */
/* v36 */
#define IFFHANDLE	struct IFFHandle
#define IFFSTREAMCMD	struct IFFStreamCmd
#define CONTEXTNODE	struct ContextNode
#define LOCALCONTEXTITEM	struct LocalContextItem
#define STOREDPROPERTY	struct StoredProperty
#define COLLECTIONITEM	struct CollectionItem
#define CLIPBOARDHANDLE	struct ClipboardHandle

/* libraries/mathffp.h */

/* libraries/mathieeedp.h */

/* libraries/mathieeesp.h */

/* libraries/mathlibrary.h */
#define	MATHIEEEBASE	struct MathIEEEBase

/* libraries/translator.h */

/* resources/battclock.h */

/* resources/battmem.h */

/* resources/battmembitsamiga.h */

/* resources/battmembitsamix.h */

/* resources/battmembitsshared.h */

/* resources/cia.h */

/* resources/ciabase.h */

/* resources/disk.h */
#define	DISCRESOURCEUNIT	struct DiscResourceUnit
#define	DISCRESOURCE	struct DiscResource

/* resources/filesysres.h */
#define	FILESYSRESOURCE	struct FileSysResource
#define	FILESYSENTRY	struct FileSysEntry

/* resources/mathresource.h */
#define	MATHIEEERESOURCE	struct MathIEEEResource

/* resources/misc.h */

/* resources/potgo.h */

/* rexx/errors.h */

/* rexx/rexxio.h */
#define IOBUFF		struct IoBuff
#define REXXMSGPORT	struct RexxMsgPort

/* rexx/rxslib.h */
#define RXSLIB	struct RxsLib

/* rexx/storage.h */
#define NEXXSTR		struct NexxStr
#define REXXARG		struct RexxArg
#define REXXMSG		struct RexxMsg
#define REXXRSRC	struct RexxRsrc
#define REXXTASK	struct RexxTask
#define SRCNODE		struct SrcNode

/* utility/date.h */
/* v36 */
#define CLOCKDATA	struct ClockData

/* utility/hooks.h */
/* v36 */
#define HOOK	struct Hook

/* utility/tagitem.h */
/* v36 */
#define TAGITEM	struct TagItem

/* workbench/icon.h */

/* workbench/startup.h */
#define	WBSTARTUP	struct WBStartup
#define	WBARG		struct WBArg

/* workbench/workbench.h */
#define	DRAWERDATA	struct DrawerData
#define	DISKOBJECT	struct DiskObject
#define	FREELIST	struct FreeList
/* v36 */
#define OLDDRAWERDATA	struct OldDrawerData
#define APPMESSAGE	struct AppMessage

#define	DOSBASE		struct DOSBase
#define	LAYERSBASE	struct LayersBase

#ifndef	IMPORT_LIBS	// [

#define	IMPORT_LIBS

#define	IMPORT	extern

IMPORT	INTUITIONBASE * IntuitionBase;
IMPORT	GFXBASE		  * GfxBase;
IMPORT	EXECBASE	  * ExecBase;	
//IMPORT	EXPANSIONBASE * ExpansionBase;
IMPORT	MATHIEEEBASE  * MathIEEEBase;
IMPORT	DOSBASE		  * DOSBase;
IMPORT	LAYERSBASE	  * LayersBase;
IMPORT	LIBRARY		  * DiskfontBase;
IMPORT	LIBRARY		  * IconBase;
IMPORT	LIBRARY		  * TranslatorBase;
IMPORT	LIBRARY		  * GadToolsBase;
IMPORT	LIBRARY		  * MathIeeeDoubBasBase;
IMPORT	LIBRARY		  * MathIeeeDoubTransBase;	
IMPORT	LIBRARY		  * RexxSysBase;
IMPORT	LIBRARY		  * DeBoxBase;		

#ifdef	TEXTGAD_LIB			// [
IMPORT	LIBRARY		  * TGadBase;
#endif						// ]

#endif				// ]

#endif	/*)*/
