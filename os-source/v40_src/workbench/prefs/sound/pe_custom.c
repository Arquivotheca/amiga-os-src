
/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/soundclass.h>
#include <graphics/text.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/locale.h>
#include <prefs/locale.h>
#include <dos/dos.h>
#include <string.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>
#include <clib/asl_protos.h>
#include <clib/datatypes_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/datatypes_pragmas.h>

/* application includes */
#include "pe_custom.h"
#include "pe_strings.h"
#include "pe_utils.h"
#include "pe_iff.h"
#include "8svx.h"


#define SysBase ed->ed_SysBase


/*****************************************************************************/


/* The IFF chunks known to this prefs editor. IFFPrefChunkCnt says how many
 * chunks there are
 */
#define IFFPrefChunkCnt 2
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_SOND,
};


/*****************************************************************************/


/* The PrefHeader structure this editor outputs */
static struct PrefHeader far IFFPrefHeader =
{
    0,				/* version */
    0,				/* type    */
    0				/* flags   */
};

/*****************************************************************************/

APTR newdtobject (EdDataPtr ed, STRPTR name, Tag tag1,...)
{
    return (NewDTObjectA (name, (struct TagItem *) & tag1));
}

/*****************************************************************************/

ULONG getdtattrs (EdDataPtr ed, Object * o, ULONG data,...)
{
    return (GetDTAttrsA (o, (struct TagItem *) & data));
}

/*****************************************************************************/

ULONG setdtattrs (EdDataPtr ed, Object * o, ULONG data,...)
{
    return (SetDTAttrsA (o, NULL, NULL, (struct TagItem *) & data));
}

/*****************************************************************************/

EdStatus InitEdData (EdDataPtr ed)
{
    EdStatus status = ES_NORMAL;

    ed->ed_PrefsDefaults.sop_DisplayQueue = TRUE;
    ed->ed_PrefsDefaults.sop_AudioQueue = FALSE;
    ed->ed_PrefsDefaults.sop_AudioType = SPTYPE_BEEP;
    ed->ed_PrefsDefaults.sop_AudioVolume = 64;
    ed->ed_PrefsDefaults.sop_AudioPeriod = 1500;
    ed->ed_PrefsDefaults.sop_AudioDuration = 10;

    ed->ed_PrefsWork = ed->ed_PrefsDefaults;
    ed->ed_PrefsInitial = ed->ed_PrefsDefaults;

    ed->ed_PeriodStash = ed->ed_PrefsDefaults.sop_AudioPeriod;
    ed->ed_DurationStash = ed->ed_PrefsDefaults.sop_AudioDuration;

    ed->ed_FilterHook.h_Entry = Filter;
    ed->ed_FilterHook.h_Data = ed;

    if (!(ed->ed_DataTypesBase = OpenLibrary ("datatypes.library", 39)))
	status = ES_REQUIRES_DATATYPES;

    return (status);
}


/*****************************************************************************/


VOID CleanUpEdData (EdDataPtr ed)
{
    if (ed->ed_DataObject)
	DisposeDTObject (ed->ed_DataObject);
    CloseLibrary (ed->ed_DataTypesBase);
    FreeAslRequest (ed->ed_SoundReq);
}


/*****************************************************************************/


EdStatus ReadPrefs (EdDataPtr ed, struct IFFHandle * iff, struct ContextNode * cn)
{
    if (cn->cn_ID != ID_SOND || cn->cn_Type != ID_PREF)
	return (ES_IFF_UNKNOWNCHUNK);

    if (ReadChunkBytes (iff, &ed->ed_PrefsWork, sizeof (struct SoundPrefs)) == sizeof (struct SoundPrefs))
    {
	ed->ed_PeriodStash = ed->ed_PrefsWork.sop_AudioPeriod;
	ed->ed_DurationStash = ed->ed_PrefsWork.sop_AudioDuration;

	if (strlen (ed->ed_PrefsWork.sop_AudioFileName) > 0)
	{
	    if (ed->ed_DataObject = newdtobject (ed, ed->ed_PrefsWork.sop_AudioFileName,
					     DTA_SourceType, DTST_FILE,
					     TAG_DONE))
	    {
	    }
	    else
	    {
		/* Couldn't find the sample! */
		ed->ed_ErrorFileName = ed->ed_PrefsWork.sop_AudioFileName;
		return (ES_DOSERROR);
	    }
	}

	return (ES_NORMAL);
    }

    return (ES_IFFERROR);
}


EdStatus OpenPrefs (EdDataPtr ed, STRPTR name)
{

    return (ReadIFF (ed, name, IFFPrefChunks, IFFPrefChunkCnt, ReadPrefs));
}


/*****************************************************************************/


EdStatus WritePrefs (EdDataPtr ed, struct IFFHandle * iff, struct ContextNode * cn)
{

    if (!PushChunk (iff, 0, ID_SOND, sizeof (struct SoundPrefs)))
	if (WriteChunkBytes (iff, &ed->ed_PrefsWork, sizeof (struct SoundPrefs)) == sizeof (struct SoundPrefs))
	    if (!PopChunk (iff))
		return (ES_NORMAL);

    return (ES_IFFERROR);
}


EdStatus SavePrefs (EdDataPtr ed, STRPTR name)
{

    return (WriteIFF (ed, name, &IFFPrefHeader, WritePrefs));
}


/*****************************************************************************/


struct EdMenu far EM[] =
{
    {NM_TITLE, MSG_PROJECT_MENU, EC_NOP, 0},
    {NM_ITEM, MSG_PROJECT_OPEN, EC_OPEN, 0},
    {NM_ITEM, MSG_PROJECT_SAVE_AS, EC_SAVEAS, 0},
    {NM_ITEM, MSG_NOTHING, EC_NOP, 0},
    {NM_ITEM, MSG_PROJECT_QUIT, EC_CANCEL, 0},

    {NM_TITLE, MSG_EDIT_MENU, EC_NOP, 0},
    {NM_ITEM, MSG_EDIT_RESET_TO_DEFAULTS, EC_RESETALL, 0},
    {NM_ITEM, MSG_EDIT_LAST_SAVED, EC_LASTSAVED, 0},
    {NM_ITEM, MSG_EDIT_RESTORE, EC_RESTORE, 0},

    {NM_TITLE, MSG_OPTIONS_MENU, EC_NOP, 0},
    {NM_ITEM, MSG_OPTIONS_SAVE_ICONS, EC_SAVEICONS, CHECKIT | MENUTOGGLE},

    {NM_END, MSG_NOTHING, EC_NOP, 0}
};

#define NW_WIDTH     397
#define NW_HEIGHT    145
#define	NW_IDCMP     (IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | BUTTONIDCMP | CHECKBOXIDCMP | SLIDERIDCMP | CYCLEIDCMP | TEXTIDCMP)
#define	NW_FLAGS     (WFLG_ACTIVATE | WFLG_DEPTHGADGET | WFLG_DRAGBAR | WFLG_SIMPLE_REFRESH)
#define NW_MINWIDTH  NW_WIDTH
#define NW_MINHEIGHT NW_HEIGHT
#define NW_MAXWIDTH  NW_WIDTH
#define NW_MAXHEIGHT NW_HEIGHT
#define ZOOMWIDTH    200

/* main display gadgets */
struct EdGadget far EG[] =
{
    {BUTTON_KIND, 8, 128, 87, 14, MSG_SAVE_GAD, EC_SAVE, 0},
    {BUTTON_KIND, 152, 128, 87, 14, MSG_USE_GAD, EC_USE, 0},
    {BUTTON_KIND, 303, 128, 87, 14, MSG_CANCEL_GAD, EC_CANCEL, 0},

    {CHECKBOX_KIND, 8, 4, 12, 14, MSG_SND_FLASH_GAD, EC_TOGGLEFLASH, PLACETEXT_RIGHT},
    {CHECKBOX_KIND, 190, 4, 12, 14, MSG_SND_MAKESOUND_GAD, EC_TOGGLESOUND, PLACETEXT_RIGHT},

    {BUTTON_KIND, 190, 108, 200, 14, MSG_SND_TEST_GAD, EC_TEST, 0},
    {CYCLE_KIND, 190, 19, 200, 14, MSG_SND_SOUNDTYPE_GAD, EC_SOUNDTYPE, 0},

    {SLIDER_KIND, 238, 35, 152, 11, MSG_SND_VOLUME_GAD, EC_VOLUME, 0},
    {SLIDER_KIND, 238, 48, 152, 11, MSG_SND_PITCH_GAD, EC_PITCH, 0},
    {SLIDER_KIND, 238, 61, 152, 11, MSG_SND_DURATION_GAD, EC_DURATION, 0},

    {TEXT_KIND, 190, 90, 200, 14, MSG_SND_SAMPLENAME_GAD, EC_NOP, 0},
    {BUTTON_KIND, 190, 74, 200, 14, MSG_SND_SELECTSAMPLE_GAD, EC_SELECTSAMPLE, 0}
};


BOOL CreateDisplay (EdDataPtr ed)
{
    UWORD zoomSize[4];

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    ed->ed_CycleLabels[0] = GetString (&ed->ed_LocaleInfo, MSG_SND_BEEP);
    ed->ed_CycleLabels[1] = GetString (&ed->ed_LocaleInfo, MSG_SND_SAMPLEDSOUND);
    ed->ed_CycleLabels[2] = NULL;

    ed->ed_LastAdded = CreateContext (&ed->ed_Gadgets);
    DoPrefsGadget (ed, &EG[0], NULL, TAG_DONE);
    DoPrefsGadget (ed, &EG[1], NULL, TAG_DONE);
    DoPrefsGadget (ed, &EG[2], NULL, TAG_DONE);

    RenderGadgets (ed);

    if ((ed->ed_LastAdded)
	&& (ed->ed_Menus = CreatePrefsMenus (ed, EM))
	&& (ed->ed_Window = OpenPrefsWindow (ed, WA_InnerWidth, NW_WIDTH,
					     WA_InnerHeight, NW_HEIGHT,
					     WA_MinWidth, NW_MINWIDTH,
					     WA_MinHeight, NW_MINHEIGHT,
					     WA_MaxWidth, NW_MAXWIDTH,
					     WA_MaxHeight, NW_MAXHEIGHT,
					     WA_IDCMP, NW_IDCMP,
					     WA_Flags, NW_FLAGS,
					     WA_Zoom, zoomSize,
					     WA_AutoAdjust, TRUE,
					     WA_PubScreen, ed->ed_Screen,
					     WA_Title, GetString (&ed->ed_LocaleInfo, MSG_SND_NAME),
					     WA_NewLookMenus, TRUE,
					     WA_Gadgets, ed->ed_Gadgets,
					     TAG_DONE)))
    {
	return (TRUE);
    }

    DisposeDisplay (ed);

    return (FALSE);
}


/*****************************************************************************/


VOID DisposeDisplay (EdDataPtr ed)
{

    if (ed->ed_Window)
    {
	ClearMenuStrip (ed->ed_Window);
	CloseWindow (ed->ed_Window);
    }
    FreeMenus (ed->ed_Menus);
    FreeGadgets (ed->ed_Gadgets);
}


/*****************************************************************************/


VOID RenderGadgets (EdDataPtr ed)
{
    BOOL disabled = TRUE;

    if (ed->ed_PrefsWork.sop_AudioQueue)
    {
	if ((strlen (ed->ed_PrefsWork.sop_AudioFileName) > 0)
	    || (ed->ed_PrefsWork.sop_AudioType == SPTYPE_BEEP))
	    disabled = FALSE;
    }

    ed->ed_FlashDisplay = DoPrefsGadget (ed, &EG[3], ed->ed_FlashDisplay,
					 GTCB_Checked, ed->ed_PrefsWork.sop_DisplayQueue,
					 TAG_DONE);

    ed->ed_MakeSound = DoPrefsGadget (ed, &EG[4], ed->ed_MakeSound,
				      GTCB_Checked, ed->ed_PrefsWork.sop_AudioQueue,
				      TAG_DONE);

    ed->ed_TestSound = DoPrefsGadget (ed, &EG[5], ed->ed_TestSound,
				      GA_Disabled, disabled,
				      TAG_DONE);

    ed->ed_SoundType = DoPrefsGadget (ed, &EG[6], ed->ed_SoundType,
				      GA_Disabled, !ed->ed_PrefsWork.sop_AudioQueue,
				      GTCY_Active, ed->ed_PrefsWork.sop_AudioType,
				      GTCY_Labels, ed->ed_CycleLabels,
				      TAG_DONE);

    ed->ed_Volume = DoPrefsGadget (ed, &EG[7], ed->ed_Volume,
				   GA_Disabled, !ed->ed_PrefsWork.sop_AudioQueue,
				   GTSL_Level, ed->ed_PrefsWork.sop_AudioVolume,
				   GTSL_Min, 0,
				   GTSL_Max, 64,
				   GTSL_MaxLevelLen, 4,
				   GTSL_LevelFormat, "%4lu",
				   GA_Immediate, TRUE,
				   GA_RelVerify, TRUE,
				   TAG_DONE);

    ed->ed_Pitch = DoPrefsGadget (ed, &EG[8], ed->ed_Pitch,
				  GA_Disabled, !ed->ed_PrefsWork.sop_AudioQueue,
				  GTSL_Level, 3000 - (ed->ed_PrefsWork.sop_AudioPeriod - 135),
				  GTSL_Min, 1,
				  GTSL_Max, 3000,
				  GTSL_MaxLevelLen, 4,
				  GTSL_LevelFormat, "%4lu",
				  GA_Immediate, TRUE,
				  GA_RelVerify, TRUE,
				  TAG_DONE);

    ed->ed_Duration = DoPrefsGadget (ed, &EG[9], ed->ed_Duration,
				     GA_Disabled, (!ed->ed_PrefsWork.sop_AudioQueue) || (ed->ed_PrefsWork.sop_AudioType != SPTYPE_BEEP),
				     GTSL_Level, ed->ed_PrefsWork.sop_AudioDuration,
				     GTSL_Min, 1,
				     GTSL_Max, 100,
				     GTSL_MaxLevelLen, 4,
				     GTSL_LevelFormat, "%4lu",
				     GA_Immediate, TRUE,
				     GA_RelVerify, TRUE,
				     TAG_DONE);

    ed->ed_SampleName = DoPrefsGadget (ed, &EG[10], ed->ed_SampleName,
				       GA_Disabled, (!ed->ed_PrefsWork.sop_AudioQueue) || (ed->ed_PrefsWork.sop_AudioType != SPTYPE_SAMPLE),
				       GTTX_Text, FilePart (ed->ed_PrefsWork.sop_AudioFileName),
				       GTTX_Border, TRUE,
				       TAG_DONE);

    ed->ed_SelectSample = DoPrefsGadget (ed, &EG[11], ed->ed_SelectSample,
					 GA_Disabled, (!ed->ed_PrefsWork.sop_AudioQueue) || (ed->ed_PrefsWork.sop_AudioType != SPTYPE_SAMPLE),
					 TAG_DONE);
}


/*****************************************************************************/

ULONG ASM Filter (REG (a0) struct Hook * h, REG (a2) struct FileRequester * fr, REG (a1) struct AnchorPath * ap)
{
    EdDataPtr ed = (EdDataPtr) h->h_Data;
    struct DataType *dtn;
    ULONG use = FALSE;
    UBYTE buffer[300];
    BPTR lock;

    strncpy (buffer, fr->fr_Drawer, sizeof (buffer));
    AddPart (buffer, ap->ap_Info.fib_FileName, sizeof (buffer));
    if (lock = Lock (buffer, ACCESS_READ))
    {
	    if (ap->ap_Info.fib_DirEntryType < 0)
	    {
		if (dtn = ObtainDataTypeA (DTST_FILE, (APTR) lock, NULL))
		{
		    if (dtn->dtn_Header->dth_GroupID == GID_SOUND)
			use = TRUE;
		    ReleaseDataType (dtn);
		}
	    }
	    else
		use = TRUE;

	UnLock (lock);
    }

    return (use);
}

/*****************************************************************************/

BOOL SelectSound (EdDataPtr ed, ULONG tag1,...)
{

    if (!ed->ed_SoundReq)
	ed->ed_SoundReq = AllocAslRequest (ASL_FileRequest, NULL);

    return (AslRequest (ed->ed_SoundReq, (struct TagItem *) & tag1));
}

/*****************************************************************************/

#define ASM __asm
#define REG(x) register __## x

VOID ASM IntuiHook (REG (a0) struct Hook *, REG (a2) struct FileRequester *, REG (a1) struct IntuiMessage *);

/*****************************************************************************/

VOID ProcessSpecialCommand (EdDataPtr ed, EdCommand ec)
{
    char path[NAMEBUFSIZE];
    struct Gadget *gadget;
    struct Requester req;
    struct Hook hook;
    EdStatus result;
    ULONG duration;
    ULONG period;
    BOOL refresh;
    UWORD icode;
    BPTR lock;
    BOOL bool;

    icode = ed->ed_CurrentMsg.Code;
    gadget = ed->ed_CurrentMsg.IAddress;
    refresh = FALSE;

    switch (ec)
    {
	case EC_TOGGLEFLASH:
	    ed->ed_PrefsWork.sop_DisplayQueue = (SELECTED & gadget->Flags);
	    break;

	case EC_TOGGLESOUND:
	    ed->ed_PrefsWork.sop_AudioQueue = (SELECTED & gadget->Flags);
	    refresh = TRUE;
	    break;

	case EC_SOUNDTYPE:
	    ed->ed_PrefsWork.sop_AudioType = icode;
	    period = ed->ed_PrefsWork.sop_AudioPeriod;
	    duration = ed->ed_PrefsWork.sop_AudioDuration;
	    ed->ed_PrefsWork.sop_AudioPeriod = ed->ed_PeriodStash;
	    ed->ed_PrefsWork.sop_AudioDuration = ed->ed_DurationStash;
	    ed->ed_PeriodStash = period;
	    ed->ed_DurationStash = duration;
	    refresh = TRUE;
	    break;

	case EC_TEST:
	    InitRequester (&req);
	    bool = Request (&req, window);

	    if ((result = DoAudio (ed, ed->ed_PrefsWork.sop_AudioFileName, TRUE)) != ES_NORMAL)
	    {
		ShowError2 (ed, result);
	    }

	    if (bool)
		EndRequest (&req, window);

	    break;

	case EC_VOLUME:
	    ed->ed_PrefsWork.sop_AudioVolume = icode;
	    break;

	case EC_PITCH:
	    ed->ed_PrefsWork.sop_AudioPeriod = (3000 - icode) + 135;
	    break;

	case EC_DURATION:
	    ed->ed_PrefsWork.sop_AudioDuration = icode;
	    break;

	case EC_SELECTSAMPLE:
	    strcpy (path, ed->ed_PrefsWork.sop_AudioFileName);
	    *PathPart (path) = 0;
	    hook.h_Entry = IntuiHook;

	    if (SelectSound (ed, ASLFR_TitleText, GetString (&ed->ed_LocaleInfo, MSG_SND_REQ_LOAD_SOUND),
			     ASLFR_Window, ed->ed_Window,
			     ASLFR_InitialDrawer, path,
			     ASLFR_InitialFile, FilePart (ed->ed_PrefsWork.sop_AudioFileName),
			     ASLFR_IntuiMsgFunc, &hook,
			     ASLFR_SleepWindow, TRUE,
			     ASLFR_RejectIcons, TRUE,
			     ASLFR_FilterFunc, ((DataTypesBase) ? &ed->ed_FilterHook : NULL),
			     TAG_DONE))
	    {
		result = ES_DOSERROR;

		strcpy (path, ed->ed_SoundReq->fr_Drawer);
		AddPart (path, ed->ed_SoundReq->fr_File, sizeof (path));

		if (ed->ed_DataObject)
		    DisposeDTObject (ed->ed_DataObject);

		if (ed->ed_DataObject = newdtobject (ed, path,
						     DTA_SourceType, DTST_FILE,
						     DTA_GroupID, GID_SOUND,
						     TAG_DONE))
		{
		    getdtattrs (ed, ed->ed_DataObject,
				SDTA_Period, (ULONG *)&ed->ed_SamplePeriod,
				TAG_DONE);
		    ed->ed_PrefsWork.sop_AudioPeriod = ed->ed_SamplePeriod;

		    if (lock = Lock (path, ACCESS_READ))
		    {
			if (NameFromLock (lock, path, sizeof (path)))
			{
			    strcpy (ed->ed_PrefsWork.sop_AudioFileName, path);
			    result = ES_NORMAL;
			    refresh = TRUE;
			}
			UnLock (lock);
		    }
		}

		if (!refresh)
		{
		    ed->ed_SecondaryResult = IoErr ();
		    switch (ed->ed_SecondaryResult)
		    {
			case 2000:	/* Unknown datatype */
			    result = ES_IFF_NOT_8SVX;
			    break;

			case 2002:	/* Couldn't open */
			    ed->ed_SecondaryResult = ERROR_OBJECT_NOT_FOUND;
			    break;
		    }
		    ed->ed_ErrorFileName = path;
		    ShowError2 (ed, result);
		}
	    }

	    break;


	default:
	    break;
    }

    if (refresh)
	RenderGadgets (ed);
}

/*****************************************************************************/


VOID GetSpecialCmdState (EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{

    state->cs_Available = TRUE;
    state->cs_Selected = FALSE;
}
