/* dispatch.c
 *
 */

#include "classbase.h"

/*****************************************************************************/

#define	private	static

/*****************************************************************************/

#define	G(o)		((struct Gadget *)o)
#define	PROC_NAME	"Sound_ObjectHandler"

/*****************************************************************************/

/* Max playable sample in one IO request is 128K */
#define MAXSAMPLE	131072

#define	CHIPSPLIT	8192

/*****************************************************************************/

#define	MAX_IOS		2

/*****************************************************************************/

struct StartupMsg
{
    struct Message	 sm_Message;		/* Embedded message */
    struct ClassBase	*sm_CB;			/* Pointer to the library base */
    Class		*sm_Class;		/* Pointer to the class */
    Object		*sm_Object;		/* Pointer to the object */
    ULONG		 sm_Return;		/* Return value */
};

/*****************************************************************************/

struct localData
{
    struct MsgPort	*lod_MP;
    struct IOAudio	*lod_IO;
    struct IOAudio	*lod_IOC[MAX_IOS];
    struct IOAudio	*lod_IOVolume;

    /* Application Controller */
    struct Process	*lod_Process;		/* Player process */
    struct Task		*lod_SignalTask;	/* Task to signal */
    ULONG		 lod_SignalBit;		/* Signal bit */

    struct VoiceHeader	 lod_VHDR;		/* This is longword aligned */
    struct StartupMsg	 lod_Message;		/* So is this */
    ULONG		 lod_Flags;

    UBYTE		*lod_Sample;
    ULONG		 lod_SampleLength;

    UBYTE		*lod_Buffer;
    UBYTE		*lod_Buffer1;
    UBYTE		*lod_Buffer2;
    UBYTE		*lod_Buffer3;
    ULONG		 lod_BufferLen;

    UBYTE		*lod_CBuffer;
    UBYTE		*lod_ChipBuff[2];

    UWORD		 lod_Period;
    UWORD		 lod_Volume;
    UWORD		 lod_Cycles;
    UWORD		 lod_Going;

    struct BitMap	 lod_BMap;
};

/*****************************************************************************/

#define	LODF_RUNNING	(1L<<0)
#define	LODF_CONTINUOUS	(1L<<1)
#define	LODF_PLAYING	(1L<<2)
#define	LODF_PLAYED1	(1L<<3)
#define	LODF_PLAYED2	(1L<<4)
#define	LODF_NEWSAMPLE	(1L<<5)
#define	LODF_NOTCHIP	(1L<<6)
#define	LODF_IMMEDIATE	(1L<<7)
#define	LODF_LOOP	(1L<<8)

/*****************************************************************************/

private UWORD chip ImageData[] =
{
/* Plane 0 */
    0x0000, 0x0010, 0x0200, 0x0000, 0x0000, 0x0028, 0x0400, 0x0000,
    0x0000, 0x0068, 0x0800, 0x0400, 0x0000, 0x00A8, 0x1000, 0x0C00,
    0x0000, 0x0128, 0x2000, 0x0C00, 0x0000, 0xF228, 0x00C0, 0x0C00,
    0x0001, 0x0E28, 0x0300, 0x0C00, 0x0001, 0x0A28, 0x0C00, 0x0C00,
    0x0001, 0x0A28, 0x0000, 0x0C00, 0x0001, 0x0A28, 0x0000, 0x0C00,
    0x0001, 0x0A28, 0x07E0, 0x0C00, 0x0001, 0x0A28, 0x0000, 0x0C00,
    0x0001, 0x0A28, 0x0000, 0x0C00, 0x0001, 0x0A28, 0x0C00, 0x0C00,
    0x0001, 0x0E28, 0x0300, 0x0C00, 0x0000, 0xF228, 0x00C0, 0x0C00,
    0x0000, 0x0128, 0x2000, 0x0C00, 0x0000, 0x00A8, 0x1000, 0x0C00,
    0x0000, 0x0068, 0x0800, 0x0C00, 0x0000, 0x0028, 0x0400, 0x0C00,
    0x0000, 0x0010, 0x0200, 0x0C00, 0x0000, 0x0000, 0x0000, 0x0C00,
    0x0000, 0x0000, 0x0000, 0x0C00, 0x7FFF, 0xFFFF, 0xFFFF, 0xFC00,
/* Plane 1 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0010, 0x0000, 0x0000,
    0xFFFF, 0xFF93, 0xF77F, 0xF800, 0xD555, 0x5551, 0x4455, 0x5000,
    0xD555, 0x54D1, 0x5555, 0x5000, 0xD555, 0x05D1, 0x5115, 0x5000,
    0xD554, 0xF1D1, 0x5455, 0x5000, 0xD554, 0xF5D1, 0x5145, 0x5000,
    0xD554, 0xF5D1, 0x5515, 0x5000, 0xD554, 0xF5D1, 0x5455, 0x5000,
    0xD554, 0xF5D1, 0x5015, 0x5000, 0xD554, 0xF5D1, 0x5555, 0x5000,
    0xD554, 0xF5D1, 0x5405, 0x5000, 0xD554, 0xF5D1, 0x5155, 0x5000,
    0xD554, 0xF1D1, 0x5455, 0x5000, 0xD555, 0x01D1, 0x5415, 0x5000,
    0xD555, 0x00D1, 0x5515, 0x5000, 0xD555, 0x4151, 0x4545, 0x5000,
    0xD555, 0x5511, 0x5555, 0x5000, 0xD555, 0x5551, 0x5155, 0x5000,
    0xD555, 0x5541, 0x5555, 0x5000, 0xD555, 0x5551, 0x5455, 0x5000,
    0xD555, 0x5551, 0x5555, 0x5000, 0x8000, 0x0000, 0x0000, 0x0000,
};

#define	DEPTH	2
#define	WIDTH	54
#define	HEIGHT	24

/*****************************************************************************/

private ULONG setdtattrs (struct ClassBase * cb, Object * o, ULONG data,...)
{
    return (SetDTAttrsA (o, NULL, NULL, (struct TagItem *) & data));
}

/*****************************************************************************/

private ULONG notifyAttrChanges (struct ClassBase * cb, Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...)
{
    return DoMethod (o, OM_NOTIFY, &tag1, ginfo, flags);
}

/*****************************************************************************/

private struct Process *createnewproc (struct ClassBase * cb, Tag tag1,...)
{
    return (CreateNewProc ((struct TagItem *) & tag1));
}

/*****************************************************************************/

private ULONG triggerMethod (struct ClassBase * cb, Class * cl, Object * o, struct dtTrigger * dtt)
{
    register struct localData *lod = INST_DATA (cl, o);
    register ULONG retval = 0;

    switch (dtt->dtt_Function)
    {
	case STM_PLAY:
	    if (lod->lod_Sample)
	    {
		Signal (lod->lod_Process, SIGBREAKF_CTRL_D);
		retval = 1;
	    }
	    break;
    }

    return (retval);
}

/*****************************************************************************/

private struct TagItem bools[] =
{
    {SDTA_Continuous, LODF_CONTINUOUS},
    {TAG_END}
};

/*****************************************************************************/

/* Set attributes of an object */
private ULONG setAttrsMethod (struct ClassBase * cb, Class * cl, Object * o, struct opSet * msg)
{
    register struct localData *lod = INST_DATA (cl, o);
    register struct TagItem *tag;
    register ULONG tidata;

    struct TagItem *tags = msg->ops_AttrList;
    UWORD oldPeriod = lod->lod_Period;
    UWORD oldVolume = lod->lod_Volume;
    struct TagItem *tstate;
    ULONG refresh = 0L;

    /* Handle boolean tags */
    lod->lod_Flags = PackBoolTags (lod->lod_Flags, tags, bools);

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case SDTA_Sample:
		lod->lod_Sample = lod->lod_Buffer3 = (UBYTE *) tidata;
		lod->lod_Flags |= LODF_NEWSAMPLE;
		if (tidata)
		{
		    /* If we are playing continuous sound then start doing this sound */
		    if (lod->lod_Flags & LODF_CONTINUOUS)
		    {
			if (lod->lod_Flags & LODF_PLAYING)
			{
			    lod->lod_Buffer3 = (UBYTE *) tidata;
			}
			else if (lod->lod_Buffer)
			{
			    CopyMem (lod->lod_Sample, lod->lod_Buffer, lod->lod_BufferLen);
			    if (lod->lod_Buffer == lod->lod_Buffer1)
				lod->lod_Buffer = lod->lod_Buffer2;
			    else
				lod->lod_Buffer = lod->lod_Buffer1;
			}
		    }
		}
		break;

	    case SDTA_SampleLength:
		lod->lod_SampleLength = lod->lod_BufferLen = (ULONG) tidata;

		/* Allocate buffers for the continuous sounds */
		if (lod->lod_Flags & LODF_CONTINUOUS)
		{
		    FreeVec (lod->lod_Buffer1);
		    FreeVec (lod->lod_Buffer2);
		    lod->lod_Buffer1 = AllocVec (tidata, MEMF_CHIP);
		    lod->lod_Buffer2 = AllocVec (tidata, MEMF_CHIP);
		    lod->lod_Buffer = lod->lod_Buffer1;
		}
		break;

	    case SDTA_Period:
		lod->lod_Period = (UWORD) tidata;
		break;

	    case SDTA_Volume:
		lod->lod_Volume = (UWORD) tidata;
		break;

	    case SDTA_Cycles:
		lod->lod_Cycles = (UWORD) tidata;
		break;

	    case SDTA_SignalTask:
		lod->lod_SignalTask = (struct Task *) tidata;
		break;

	    case SDTA_SignalBit:
		lod->lod_SignalBit = (ULONG) tidata;
		break;
	}
    }

    if (lod->lod_Process && lod->lod_Sample &&
	((oldPeriod != lod->lod_Period) || (oldVolume != lod->lod_Volume)))
	Signal (lod->lod_Process, SIGBREAKF_CTRL_F);

    return (refresh);
}

/*****************************************************************************/

/* Trigger methods we support */
struct DTMethod tm[] =
{
    {"Play", "PLAY", STM_PLAY},
    {NULL,},
};

/*****************************************************************************/

/* Methods we support */
ULONG m[] =
{
    OM_NEW,
    OM_GET,
    OM_SET,
    OM_UPDATE,
    OM_DISPOSE,
    GM_LAYOUT,
    GM_HITTEST,
    GM_GOACTIVE,
    GM_HANDLEINPUT,
    GM_RENDER,
    DTM_TRIGGER,
    DTM_COPY,
    DTM_WRITE,
    ~0,
};

/*****************************************************************************/

/* Inquire attribute of an object */
private ULONG getAttrMethod (Class * cl, Object * o, struct opGet * msg)
{
    register struct localData *lod = INST_DATA (cl, o);

    switch (msg->opg_AttrID)
    {
	case DTA_TriggerMethods:
	    *msg->opg_Storage = (ULONG) tm;
	    break;

	case DTA_Methods:
	    *msg->opg_Storage = (ULONG) m;
	    break;

	case DTA_NominalVert:
	    *msg->opg_Storage = (ULONG) HEIGHT;
	    break;

	case DTA_NominalHoriz:
	    *msg->opg_Storage = (ULONG) WIDTH;
	    break;

	case SDTA_VoiceHeader:
	    *msg->opg_Storage = (ULONG) & lod->lod_VHDR;
	    break;

	case SDTA_Sample:
	    *msg->opg_Storage = (ULONG) lod->lod_Sample;
	    break;

	case SDTA_SampleLength:
	    *msg->opg_Storage = (ULONG) lod->lod_SampleLength;
	    break;

	case SDTA_Period:
	    *msg->opg_Storage = (ULONG) lod->lod_Period;
	    break;

	case SDTA_Volume:
	    *msg->opg_Storage = (ULONG) lod->lod_Volume;
	    break;

	case SDTA_Cycles:
	    *msg->opg_Storage = (ULONG) lod->lod_Cycles;
	    break;

	default:
	    return (DoSuperMethodA (cl, o, msg));
    }

    return (1L);
}

/*****************************************************************************/

private BOOL WriteBody (struct ClassBase * cb, struct IFFHandle * iff, APTR sample, ULONG len)
{
    if (PushChunk (iff, 0, ID_BODY, len) == 0)
	if (WriteChunkBytes (iff, sample, len) == len)
	    if (PopChunk (iff) == 0)
		return (TRUE);
    return (FALSE);
}

/*****************************************************************************/

private BOOL WriteVHDR (struct ClassBase * cb, struct IFFHandle * iff, struct VoiceHeader * vhdr)
{
    register ULONG msize = sizeof (struct VoiceHeader);

    if (PushChunk (iff, 0, ID_VHDR, msize) == 0)
	if (WriteChunkBytes (iff, vhdr, msize) == msize)
	    if (PopChunk (iff) == 0)
		return (TRUE);
    return (FALSE);
}

/*****************************************************************************/

private BOOL WriteNAME (struct ClassBase * cb, struct IFFHandle * iff, STRPTR name)
{
    register ULONG msize;

    if (!name)
	return (TRUE);
    msize = strlen (name) + 1;

    if (PushChunk (iff, 0, ID_NAME, msize) == 0)
	if (WriteChunkBytes (iff, name, msize) == msize)
	    if (PopChunk (iff) == 0)
		return (TRUE);
    return (FALSE);
}

/*****************************************************************************/

private BOOL writeObject (struct ClassBase * cb, struct IFFHandle * iff, Object * o, struct localData * lod)
{
    struct VoiceHeader vhdr;
    BOOL success = FALSE;
    STRPTR title = NULL;

    /* Get the title */
    GetAttr (DTA_ObjName, o, (ULONG *) & title);

    /* Copy the voice header */
    vhdr = *(&lod->lod_VHDR);
    vhdr.vh_Compression = CMP_NONE;

    if (OpenIFF	(iff, IFFF_WRITE) == 0)
    {
	if (PushChunk (iff, ID_8SVX, ID_FORM, IFFSIZE_UNKNOWN) == 0)
	{
	    /* Write the title of the object */
	    WriteNAME (cb, iff,	title);

	    if (WriteVHDR (cb, iff, &vhdr))
		success	= WriteBody (cb, iff, lod->lod_Sample, lod->lod_SampleLength);
	    PopChunk (iff);
	}
	CloseIFF (iff);
    }

    return (success);
}

/*****************************************************************************/

private ULONG copyMethod (struct ClassBase * cb, Class * cl, Object * o, struct dtGeneral * msg)
{
    register struct localData *lod = INST_DATA (cl, o);
    register struct IFFHandle *iff;
    BOOL success = FALSE;

    if (iff = AllocIFF ())
    {
	if (iff->iff_Stream = (ULONG) OpenClipboard (0L))
	{
	    InitIFFasClip (iff);
	    success = writeObject (cb, iff, o, lod);
	    CloseClipboard ((struct ClipboardHandle *) iff->iff_Stream);
	}
	FreeIFF (iff);
    }

    if (!success)
	DisplayBeep (NULL);

    return ((ULONG) success);
}

/*****************************************************************************/

private ULONG writeMethod (struct ClassBase * cb, Class * cl, Object * o, struct dtWrite * msg)
{
    register struct localData *lod = INST_DATA (cl, o);
    register struct IFFHandle *iff;
    BOOL success = FALSE;

    if (iff = AllocIFF ())
    {
	if (iff->iff_Stream = msg->dtw_FileHandle)
	{
	    InitIFFasDOS (iff);
	    success = writeObject (cb, iff, o, lod);
	}
	FreeIFF (iff);
    }

    if (!success)
	DisplayBeep (NULL);
    return ((ULONG) success);
}

/*****************************************************************************/

private ULONG renderMethod (struct ClassBase * cb, Class * cl, Object * o, struct gpRender * msg)
{
    register struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    register struct localData *lod = INST_DATA (cl, o);
    register struct RastPort *rp;
    struct IBox *domain;
    LONG width, height;

    GetAttr (DTA_Domain, o, (ULONG *) & domain);

    width = (ULONG) MIN (WIDTH, domain->Width);
    height = (ULONG) MIN (HEIGHT, domain->Height);

    /* Get a pointer to the rastport */
    if (rp = msg->gpr_RPort)
    {
	if ((msg->MethodID == GM_RENDER) && (msg->gpr_Redraw == GREDRAW_REDRAW) &&
	    (si->si_OTopVert != si->si_TopVert) && (si->si_OTopVert != si->si_TopVert))
	{
	    EraseRect (rp,
		       domain->Left, domain->Top,
		       domain->Left + domain->Width - 1,
		       domain->Top + domain->Height - 1);
	}

	BltBitMapRastPort (&lod->lod_BMap, (ULONG) si->si_TopHoriz, (ULONG) si->si_TopVert,
			   rp, domain->Left, domain->Top, width, height, 0xC0);

	si->si_OTopVert  = si->si_TopVert;
	si->si_OTopHoriz = si->si_TopHoriz;
    }

    return (1L);
}

/*****************************************************************************/

private UBYTE audioChannels[] = {1, 2, 4, 8};

/*****************************************************************************/

/* This inits the handler for playing double-buffered continuous sounds */
private ULONG InitObjectHandler (struct ClassBase * cb, struct localData * lod)
{
    register WORD i;

    if ((lod->lod_MP = CreateMsgPort ()) == NULL)
	return ERROR_NO_FREE_STORE;

    for (i = 0; i < MAX_IOS; i++)
	if (!(lod->lod_IOC[i] = (struct IOAudio *) CreateIORequest (lod->lod_MP, sizeof (struct IOAudio))))
	    return ERROR_NO_FREE_STORE;

    if (!(lod->lod_IOVolume = (struct IOAudio *) CreateIORequest (lod->lod_MP, sizeof (struct IOAudio))))
	return ERROR_NO_FREE_STORE;

    lod->lod_IO = lod->lod_IOC[0];

    /* Are we going to be doing a double-buffered continuous sound (like for a movie) */
    if (lod->lod_Flags & LODF_CONTINUOUS)
    {
	/* Initialize the rest of the Audio structure */
	lod->lod_IO->ioa_Request.io_Command = ADCMD_ALLOCATE;
	lod->lod_IO->ioa_Request.io_Flags   = ADIOF_NOWAIT;
	lod->lod_IOC[0]->ioa_Data   = audioChannels;
	lod->lod_IOC[0]->ioa_Length = sizeof (audioChannels);

	/* Open the audio device */
	if (OpenDevice ("audio.device", 0, lod->lod_IOC[0], 0) != 0)
	    return ERROR_NO_FREE_STORE;

	/* Copy the IO request */
	for (i = 1; i < MAX_IOS; i++)
	    CopyMem (lod->lod_IOC[0], lod->lod_IOC[i], sizeof (struct IOAudio));

	CopyMem (lod->lod_IOC[0], lod->lod_IOVolume, sizeof (struct IOAudio));
    }

    return 0;
}

/*****************************************************************************/

private void PlayAudio (struct ClassBase * cb, struct localData * lod, UBYTE * buffer)
{
    lod->lod_IO->ioa_Request.io_Command = CMD_WRITE;
    lod->lod_IO->ioa_Request.io_Flags = ADIOF_PERVOL;
    lod->lod_IO->ioa_Data = buffer;
    lod->lod_IO->ioa_Length = lod->lod_BufferLen;
    lod->lod_IO->ioa_Period = lod->lod_Period;
    lod->lod_IO->ioa_Volume = lod->lod_Volume;
    lod->lod_IO->ioa_Cycles = lod->lod_Cycles;
    BeginIO (lod->lod_IO);
}

/*****************************************************************************/

private void WaitForEndAndCleanup (struct ClassBase *cb, struct localData *lod, ULONG sigs, BOOL abort, BOOL sig)
{
    /* Make sure the device is open */
    if (lod->lod_IOC[0]->ioa_Request.io_Device)
    {
	/* Are we supposed to abort */
	if (abort)
	{
#if 1
	    lod->lod_IOVolume->ioa_Request.io_Command = CMD_FLUSH;
	    lod->lod_IOVolume->ioa_Request.io_Flags   = IOF_QUICK;
	    BeginIO ((struct IORequest *) lod->lod_IOVolume);
#else
	    /* Abort any existing sounds */
	    if (lod->lod_Flags & LODF_PLAYED2)
		AbortIO (lod->lod_IOC[1]);

	    if (lod->lod_Flags & LODF_PLAYED1)
		AbortIO (lod->lod_IOC[0]);
#endif
	}

	/* Wait for completion */
	if (lod->lod_Flags & LODF_PLAYED1)
	    WaitIO (lod->lod_IOC[0]);
	if (lod->lod_Flags & LODF_PLAYED2)
	    WaitIO (lod->lod_IOC[1]);

	/* Close the device */
	CloseDevice (lod->lod_IOC[0]);
	lod->lod_IOC[0]->ioa_Request.io_Device = NULL;

	/* Clear the signals */
	if (sigs)
	    SetSignal (0L, sigs);
    }

    /* Free the extra chip buffers */
    FreeVec (lod->lod_ChipBuff[0]);
    FreeVec (lod->lod_ChipBuff[1]);
    lod->lod_CBuffer = lod->lod_ChipBuff[0] = lod->lod_ChipBuff[1] = NULL;

    /* Signal the parent that a sound has ended */
    if (sig && (lod->lod_Flags & LODF_PLAYING) && lod->lod_SignalTask)
	Signal (lod->lod_SignalTask, lod->lod_SignalBit);

    /* Show that we aren't playing anymore */
    lod->lod_Flags &= ~(LODF_PLAYING | LODF_PLAYED1 | LODF_PLAYED2);
}

/*****************************************************************************/

private void ShutdownObjectHandler (struct ClassBase * cb, struct localData * lod)
{
    register WORD i;

    if (lod->lod_IO)
	WaitForEndAndCleanup (cb, lod, NULL, TRUE, TRUE);

    DeleteIORequest ((struct IORequest *) lod->lod_IOVolume);
    for (i = 0; i < MAX_IOS; i++)
	DeleteIORequest ((struct IORequest *) lod->lod_IOC[i]);

    DeleteMsgPort (lod->lod_MP);
}

/*****************************************************************************/

#undef	SysBase

/*****************************************************************************/

private void ObjectHandler (void)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    register struct localData *lod;
    struct StartupMsg *sm;
    struct ClassBase *cb;
    struct IOAudio *msg;
    struct Process *pr;
    ULONG ssize;
    ULONG sigs;
    ULONG psig;
    ULONG sigr;
    Class *cl;
    Object *o;

    /* Double buffering */
    struct IOAudio *iod = NULL;
    BYTE *samptr = NULL;
    ULONG dBuffLen = 0;
    LONG size = 0;
    LONG len;

    /* Find out who we are */
    pr = (struct Process *) FindTask (NULL);

    /* Get the startup message */
    WaitPort (&pr->pr_MsgPort);
    sm = (struct StartupMsg *) GetMsg (&pr->pr_MsgPort);

    /* Pull all the information from the startup message */
    cb  = sm->sm_CB;
    cl  = sm->sm_Class;
    o   = sm->sm_Object;
    lod = INST_DATA (cl, o);

    /* Show that we are running (so that OM_DISPOSE doesn't fall out from under us) */
    lod->lod_Flags |= LODF_RUNNING;

    /* Initialize the object handler */
    sm->sm_Return = InitObjectHandler (cb, lod);

    /* Reply to the message */
    ReplyMsg ((struct Message *) sm);

    /* Are we supposed to run? */
    if (sm->sm_Return == 0)
    {
	psig = 1L << lod->lod_MP->mp_SigBit;

	/* All the signals that we're waiting on */
	sigs = psig | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F;

	/* Keep going until we are told to stop */
	while (lod->lod_Going)
	{
	    if (size > 0)
	    {
		len = (size > dBuffLen) ? dBuffLen : size;
		lod->lod_IO->ioa_Request.io_Command = CMD_WRITE;
		lod->lod_IO->ioa_Request.io_Flags = ADIOF_PERVOL;

		if (lod->lod_Flags & LODF_NOTCHIP)
		{
		    CopyMem (samptr, lod->lod_CBuffer, len);
		    lod->lod_IO->ioa_Data = lod->lod_CBuffer;
		}
		else
		    lod->lod_IO->ioa_Data = samptr;
		lod->lod_IO->ioa_Length = len;
		lod->lod_IO->ioa_Period = lod->lod_Period;
		lod->lod_IO->ioa_Volume = lod->lod_Volume;
		lod->lod_IO->ioa_Cycles = 1;
		iod = lod->lod_IO;
		BeginIO (lod->lod_IO);
		if (!(lod->lod_IO->ioa_Request.io_Flags & IOF_QUICK))
		{
		    if (lod->lod_IO == lod->lod_IOC[0])
			lod->lod_Flags |= LODF_PLAYED1;
		    else
			lod->lod_Flags |= LODF_PLAYED2;
		}

		size = (size > dBuffLen) ? size - dBuffLen : 0;
		samptr += dBuffLen;
	    }

	    /* Wait for something to happen */
	    sigr = Wait (sigs);

	    /* Sample ended */
	    if (sigr & psig)
	    {
		if (lod->lod_Flags & LODF_CONTINUOUS)
		{
		    if (msg = (struct IOAudio *) GetMsg (lod->lod_MP))
		    {
			if (msg == lod->lod_IOC[0])
			{
			    lod->lod_Flags &= ~LODF_PLAYED1;
			    lod->lod_Buffer = lod->lod_Buffer1;
			    lod->lod_IO = lod->lod_IOC[0];
			}
			else if (msg == lod->lod_IOC[1])
			{
			    lod->lod_Flags &= ~LODF_PLAYED2;
			    lod->lod_Buffer = lod->lod_Buffer2;
			    lod->lod_IO = lod->lod_IOC[1];
			}

			if (lod->lod_Buffer3 && lod->lod_Buffer)
			{
			    if (lod->lod_Flags & LODF_NEWSAMPLE)
			    {
				lod->lod_Flags &= ~LODF_NEWSAMPLE;
				CopyMem (lod->lod_Buffer3, lod->lod_Buffer, lod->lod_BufferLen);
			    }
			    else
			    {
				memset (lod->lod_Buffer, 0, lod->lod_BufferLen);
			    }
			    PlayAudio (cb, lod, lod->lod_Buffer);
			}
			else if ((lod->lod_Sample == NULL) && (lod->lod_Buffer3 == NULL))
			    lod->lod_Flags &= ~LODF_PLAYING;
		    }
		}
		else if (iod)
		{
		    if (msg = (struct IOAudio *) GetMsg (lod->lod_MP))
		    {
			if (msg == lod->lod_IOC[0])
			{
			    lod->lod_Flags &= ~LODF_PLAYED1;
			    lod->lod_CBuffer = lod->lod_ChipBuff[0];
			    lod->lod_IO = lod->lod_IOC[0];
			}
			else if (msg == lod->lod_IOC[1])
			{
			    lod->lod_Flags &= ~LODF_PLAYED2;
			    lod->lod_CBuffer = lod->lod_ChipBuff[1];
			    lod->lod_IO = lod->lod_IOC[1];
			}
		    }

		    if ((size == 0) && (lod->lod_IO == iod))
		    {
			/* Perform cleanup of sound */
			WaitForEndAndCleanup (cb, lod, sigs, TRUE, TRUE);
			size = 0;
		    }
		}
		else if (lod->lod_IOC[0]->ioa_Request.io_Device)
		{
		    /* Perform cleanup of sound */
		    WaitForEndAndCleanup (cb, lod, sigs, TRUE, TRUE);
		    size = 0;
		}
	    }

	    /* Time to dispose of the object */
	    if (sigr & SIGBREAKF_CTRL_C)
	    {
		lod->lod_Going = FALSE;
		size = 0;
	    }

	    /* Start playing sample */
	    if (sigr & SIGBREAKF_CTRL_D)
	    {
		/* Are we doing a continuous sound? */
		if (lod->lod_Flags & LODF_CONTINUOUS)
		{
		    if (!(lod->lod_Flags & LODF_PLAYING))
		    {
			/* Start playing */
			lod->lod_IO = lod->lod_IOC[0];
			PlayAudio (cb, lod, lod->lod_Buffer1);

			lod->lod_IO = lod->lod_IOC[1];
			PlayAudio (cb, lod, lod->lod_Buffer2);

			/* Set up the swap pointers */
			lod->lod_IO = lod->lod_IOC[0];
			lod->lod_Buffer = lod->lod_Buffer1;

			/* Show that we are playing */
			lod->lod_Flags |= LODF_PLAYING | LODF_PLAYED1 | LODF_PLAYED2;
		    }
		}
		else
		{
		    /* Abort any playing sound */
		    WaitForEndAndCleanup (cb, lod, sigs, TRUE, FALSE);
		    size = 0;

		    /* Reset this pointer */
		    lod->lod_IO = lod->lod_IOC[0];

		    /* Clear the chip flag */
		    lod->lod_Flags &= ~LODF_NOTCHIP;

		    /* Open the audio device */
		    lod->lod_IOC[0]->ioa_Request.io_Command = ADCMD_ALLOCATE;
		    lod->lod_IOC[0]->ioa_Request.io_Flags = ADIOF_NOWAIT;
		    lod->lod_IOC[0]->ioa_Data   = audioChannels;
		    lod->lod_IOC[0]->ioa_Length = sizeof (audioChannels);
		    if (OpenDevice ("audio.device", 0, lod->lod_IOC[0], 0) == 0)
		    {
			lod->lod_IOC[0]->ioa_Request.io_Command = CMD_WRITE;
			lod->lod_IOC[0]->ioa_Request.io_Flags = ADIOF_PERVOL;
			lod->lod_IOC[0]->ioa_Cycles = lod->lod_Cycles;
			lod->lod_IOC[0]->ioa_Length = lod->lod_BufferLen;
			lod->lod_IOC[0]->ioa_Data   = lod->lod_Sample;
			lod->lod_IOC[0]->ioa_Period = lod->lod_Period;
			lod->lod_IOC[0]->ioa_Volume = lod->lod_Volume;

			CopyMem (lod->lod_IOC[0], lod->lod_IOVolume, sizeof (struct IOAudio));
			CopyMem (lod->lod_IOC[0], lod->lod_IOC[1], sizeof (struct IOAudio));

			/* If we weren't in chip, then let's put it there */
			dBuffLen = MAXSAMPLE;
			if (!(TypeOfMem (lod->lod_Sample) & MEMF_CHIP))
			{
			    /* New double buffer size */
			    dBuffLen = CHIPSPLIT;

			    /* Show that the original wasn't in chip memory */
			    lod->lod_Flags |= LODF_NOTCHIP;

			    /* Get the size of the first buffer */
			    ssize = (lod->lod_SampleLength > dBuffLen) ? dBuffLen : lod->lod_SampleLength;
			    if (lod->lod_ChipBuff[0] = AllocVec (ssize, MEMF_CHIP))
			    {
				CopyMem (lod->lod_Sample, lod->lod_ChipBuff[0], ssize);
				lod->lod_IOC[0]->ioa_Data = lod->lod_ChipBuff[0];
			    }
			}

			if (lod->lod_SampleLength <= dBuffLen)
			{
			    iod = NULL;
			    BeginIO (lod->lod_IOC[0]);
			    if (!(lod->lod_IOC[0]->ioa_Request.io_Flags & IOF_QUICK))
				lod->lod_Flags |= LODF_PLAYED1;
			}
			else
			{
			    /* See if we need to prepare the second buffer */
			    if (lod->lod_Flags & LODF_NOTCHIP)
			    {
				ssize = lod->lod_SampleLength - dBuffLen;
				if (ssize > dBuffLen)
				    ssize = dBuffLen;

				if (lod->lod_ChipBuff[1] = AllocVec (ssize, MEMF_CHIP))
				{
				    lod->lod_CBuffer = lod->lod_ChipBuff[1];
				}
				else
				{
				    lod->lod_IOC[0]->ioa_Data = lod->lod_Sample;
				    FreeVec (lod->lod_ChipBuff[0]);
				    lod->lod_ChipBuff[0] = NULL;
				}
			    }

			    iod = lod->lod_IOC[0];
			    lod->lod_IOC[0]->ioa_Length = dBuffLen;
			    lod->lod_IOC[0]->ioa_Cycles = 1;
			    BeginIO (lod->lod_IOC[0]);
			    if (!(lod->lod_IOC[0]->ioa_Request.io_Flags & IOF_QUICK))
				lod->lod_Flags |= LODF_PLAYED1;

			    samptr = lod->lod_Sample + dBuffLen;
			    size = (LONG)(lod->lod_SampleLength - dBuffLen);
			    lod->lod_IO = lod->lod_IOC[1];
			}

			/* Show that we are playing */
			lod->lod_Flags |= LODF_PLAYING;
		    }
		}
	    }

	    /* Time to remove the object */
	    if (sigr & SIGBREAKF_CTRL_E)
		lod->lod_Going = FALSE;

	    /* Change in period, volume or state */
	    if (sigr & SIGBREAKF_CTRL_F)
	    {
		/* Are we doing a continuous sound? */
		if (lod->lod_Flags & LODF_CONTINUOUS)
		{
		}
		/* If we have a sample going, the update it */
		else if (lod->lod_IOC[0]->ioa_Request.io_Device)
		{
		    /* They want to change the volume or period */
		    lod->lod_IOVolume->ioa_Request.io_Command = ADCMD_PERVOL;
		    lod->lod_IOVolume->ioa_Request.io_Flags   = ADIOF_PERVOL | ADIOF_SYNCCYCLE;
		    lod->lod_IOVolume->ioa_Volume = lod->lod_Volume;
		    lod->lod_IOVolume->ioa_Period = lod->lod_Period;
		    lod->lod_IOVolume->ioa_Cycles = lod->lod_Cycles;
		    lod->lod_IOVolume->ioa_Length = lod->lod_BufferLen;
		    lod->lod_IOVolume->ioa_Data = lod->lod_Sample;
		    DoIO (lod->lod_IOVolume);
		}
	    }
	}
    }

    /* Time to shutdown the handler */
    ShutdownObjectHandler (cb, lod);

    /* Exit now */
    Forbid ();

    /* Show that we aren't running anymore */
    lod->lod_Flags &= ~LODF_RUNNING;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#define SysBase			cb->cb_SysBase

/*****************************************************************************/

private ULONG ASM Dispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    register struct ClassBase *cb = (struct ClassBase *) cl->cl_UserData;
    register struct localData *lod = INST_DATA (cl, o);
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* Create the new object */
	    if (retval = DoSuperMethodA (cl, o, msg))
	    {
		ULONG i, planes, data;

		/* Set reasonable defaults */
		lod = INST_DATA (cl, (Object *) retval);
		lod->lod_Cycles = 1;
		lod->lod_Volume = 64;
		lod->lod_Period = 394;

		/* Initialize the default imagry */
		InitBitMap (&lod->lod_BMap, DEPTH, WIDTH, HEIGHT);
		planes = RASSIZE (WIDTH, HEIGHT);
		data = (ULONG) ImageData;
		for (i = 0; i < DEPTH; i++)
		    lod->lod_BMap.Planes[i] = (PLANEPTR) (data + i * planes);

		/* Set the size */
		setdtattrs (cb, (Object *) retval,
			    DTA_NominalHoriz, WIDTH,
			    DTA_NominalVert, HEIGHT,
			    TAG_DONE);

		/* Set the attributes */
		setAttrsMethod (cb, cl, (Object *) retval, (struct opSet *) msg);

		/* Create a process to monitor this object */
		if (lod->lod_Process = createnewproc (cb,
						      NP_Output, Output (),
						      NP_CloseOutput, FALSE,
						      NP_Input, Input (),
						      NP_CloseInput, FALSE,
						      NP_Name, PROC_NAME,
						      NP_StackSize, 4096L,
						      NP_Entry, ObjectHandler,
						      NP_Priority, 0L,
						      TAG_DONE))
		{
		    /* Initialize the message */
		    lod->lod_Message.sm_Message.mn_Node.ln_Type = NT_MESSAGE;
		    lod->lod_Message.sm_Message.mn_Length       = sizeof (struct StartupMsg);
		    lod->lod_Message.sm_Message.mn_ReplyPort    = &((struct Process *)FindTask(NULL))->pr_MsgPort;
		    lod->lod_Message.sm_CB     = cb;
		    lod->lod_Message.sm_Class  = cl;
		    lod->lod_Message.sm_Object = (Object *) retval;

		    /* Show that we are operational */
		    lod->lod_Going = TRUE;

		    /* Send the information to the process */
		    PutMsg (&(lod->lod_Process->pr_MsgPort), &lod->lod_Message);

		    /* Wait until the new process starts */
		    WaitPort (lod->lod_Message.sm_Message.mn_ReplyPort);

		    /* Get the reply message */
		    GetMsg (lod->lod_Message.sm_Message.mn_ReplyPort);

		    /* No error return value */
		    if (lod->lod_Message.sm_Return == 0)
			return (retval);

		    /* Set the error return value */
		    SetIoErr (lod->lod_Message.sm_Return);
		}

		/* Force disposal */
		CoerceMethod (cl, (Object *) retval, OM_DISPOSE);
	    }
	    return (NULL);
	    break;

	case OM_GET:
	    return getAttrMethod (cl, o, (struct opGet *) msg);

	case DTM_COPY:
	    return copyMethod (cb, cl, o, (struct dtGeneral *) msg);

	case DTM_WRITE:
	    return writeMethod (cb, cl, o, (struct dtWrite *) msg);

	case OM_UPDATE:
	case OM_SET:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    retval += setAttrsMethod (cb, cl, o, (struct opSet *) msg);
	    if (retval)
	    {
		struct RastPort *rp;
		struct gpRender gpr;

		/* Get a pointer to the rastport */
		if (rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo))
		{
		    /* Force a redraw */
		    gpr.MethodID   = GM_RENDER;
		    gpr.gpr_GInfo  = ((struct opSet *) msg)->ops_GInfo;
		    gpr.gpr_RPort  = rp;
		    gpr.gpr_Redraw = GREDRAW_UPDATE;
		    DoMethodA (o, &gpr);

		    /* Release the temporary rastport */
		    ReleaseGIRPort (rp);
		}

		/* Clear the return value */
		retval = 0L;
	    }
	    break;

	case GM_LAYOUT:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    {
		struct IBox *domain;
		STRPTR title;

		GetAttr (DTA_Domain, o, (ULONG *) & domain);
		GetAttr (DTA_ObjName, o, (ULONG *) & title);

		notifyAttrChanges (cb, o, ((struct gpLayout *) msg)->gpl_GInfo, NULL,
				   GA_ID, G (o)->GadgetID,
				   DTA_Busy, FALSE,
				   DTA_Title, title,
				   DTA_TotalHoriz, WIDTH,
				   DTA_HorizUnit, 1L,
				   DTA_VisibleHoriz, MIN (WIDTH, domain->Width),
				   DTA_TotalVert, HEIGHT,
				   DTA_VertUnit, 1L,
				   DTA_VisibleVert, MIN (HEIGHT, domain->Height),
				   DTA_Sync, TRUE,
				   TAG_DONE);
	    }
	    break;

	case GM_HITTEST:
	    return (GMR_GADGETHIT);
	    break;

	case GM_GOACTIVE:
	case GM_HANDLEINPUT:
	    {
		register struct gpInput *gpi = (struct gpInput *) msg;
		register struct InputEvent *ie = gpi->gpi_IEvent;

		if (ie->ie_Code == SELECTUP)
		{
		    if (lod->lod_Sample)
			Signal (lod->lod_Process, SIGBREAKF_CTRL_D);
		    retval = (GMR_NOREUSE | GMR_VERIFY);
		}
		else if (ie->ie_Code == MENUDOWN)
		{
		    retval = GMR_NOREUSE;
		}
		else
		{
		    retval = GMR_MEACTIVE;
		}
	    }
	    break;

	case GM_RENDER:
	    return renderMethod (cb, cl, o, (struct gpRender *) msg);

	case DTM_TRIGGER:
	    return triggerMethod (cb, cl, o, (struct dtTrigger *) msg);

	case OM_DISPOSE:
	    /* Do we have a control process running */
	    if ((lod->lod_Flags & LODF_RUNNING) && lod->lod_Process)
	    {
		/* Tell the control process to go away */
		Signal (lod->lod_Process, SIGBREAKF_CTRL_C);

		/* Wait until they go away */
		while (lod->lod_Flags & LODF_RUNNING)
		    Delay (2);
	    }

	    if (!(lod->lod_Flags & LODF_CONTINUOUS))
		/* THIS WAS A FUCKING STUPID IDEA!!! */
		FreeVec (lod->lod_Sample);

	    FreeVec (lod->lod_Buffer1);
	    lod->lod_Buffer1 = NULL;
	    FreeVec (lod->lod_Buffer2);
	    lod->lod_Buffer2 = NULL;

	    /* Let the superclass handle everything else */
	default:
	    return (ULONG) DoSuperMethodA (cl, o, msg);
    }

    return (retval);
}

/*****************************************************************************/

Class *initClass (struct ClassBase * cb)
{
    register Class *cl;

    if (cl = MakeClass (SOUNDDTCLASS, DATATYPESCLASS, NULL, sizeof (struct localData), 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)()) Dispatch;
	cl->cl_UserData = (ULONG) cb;
	AddClass (cl);
    }

    return (cl);
}
