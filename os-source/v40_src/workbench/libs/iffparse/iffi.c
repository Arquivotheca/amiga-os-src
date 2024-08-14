
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <devices/clipboard.h>

#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "iffprivate.h"
#include "iffparsebase.h"
#include "iffa.h"
#include "iffi.h"


/*****************************************************************************/


#define	MAXOFF 0x7fffffffL


/*****************************************************************************/


/* Prototypes for functions defined in iffi.c */
static LONG __asm DOS_Stream(register __a0 struct Hook * ,
                             register __a2 struct IFFHandle * ,
                             register __a1 struct IFFStreamCmd * );
static LONG __asm CB_Stream(register __a0 struct Hook * ,
                            register __a2 struct IFFHandle * ,
                            register __a1 struct IFFStreamCmd * );


/*****************************************************************************/


LONG streamaction(struct IFFHandleP *iffp, LONG cmd, LONG clienterr, char *buf, LONG size)
{
struct IFFStreamCmd sc;

    sc.sc_Command  = cmd;
    sc.sc_Buf      = (APTR) buf;
    sc.sc_NBytes   = size;

    if (CallAHook(iffp->iffp_StreamHook, iffp, (LONG *) &sc))
        return (clienterr);

    return (0);
}


/*****************************************************************************/



/* UserRead() and UserWrite() are now macros */
#if 0
/* Gives access to the user-supplied read function.
 * Here we access the user function through the generic
 * stub function or directly if none is provided.  In C the result is
 * the same, but ultimately this will all be in assembly.  (Won't it?)
 */
LONG UserRead(struct IFFHandleP *iffp, char *buf, LONG size)
{
    return (streamaction(iffp,IFFSCC_READ,IFFERR_READ,buf,size));
}

/* Interface for user write function */
LONG UserWrite(struct IFFHandleP *iffp, char *buf, LONG size)
{
    return (streamaction(iffp,IFFSCC_WRITE,IFFERR_WRITE,buf,size));
}
#endif


/*****************************************************************************/


/* Perform the seek with the user-provided interface.
 * This assumes that a seek function is provided.  If not,
 * we must do something else.
 */
#define MAXBUFFER 512

LONG UserSeek(struct IFFHandleP *iffp, LONG nbytes)
{
	register LONG	error;

	/*
	 * Test for cases we can't handle:
	 * A reverse seek without random access seek capability.
	 */
	if (nbytes < 0  &&  !(iffp->iffp_Flags & IFFF_RSEEK))
		return (IFFERR_SEEK);

	/*
	 * For the case of forward seeks without any seek capability,
	 * we fake it with a series of reads.
	 * This capability is *required* to be able to read IFF files.
	 * RSEEK implies FSEEK.
	 */
	if (nbytes > 0  &&
	    !(iffp->iffp_Flags & (IFFF_FSEEK|IFFF_RSEEK)))
	{
		register BYTE	*buf;
		register LONG	bsiz, rsiz;

		/*
		 * Might try smaller units if this allocation fails.
		 */
		bsiz = MAXBUFFER;
		if (nbytes < bsiz)
			bsiz = nbytes;
		if (!(buf = AllocMem (bsiz, 0L)))
			return (IFFERR_SEEK);

		error = 0;
		while (!error  &&  nbytes) {
			rsiz = (nbytes < MAXBUFFER ? nbytes : bsiz);
			error = UserRead (iffp, buf, rsiz);
			nbytes -= rsiz;
		}
		FreeMem (buf, bsiz);
		return (error);
	}

	/*
	 * If we get here, that means we can seek.
	 */
	return (streamaction (iffp, IFFSCC_SEEK, IFFERR_SEEK, NULL, nbytes));
}


/*****************************************************************************/


VOID ASM InitIFFLVO(REG(a0) struct IFFHandleP *iffp,
                    REG(d0) LONG flags,
                    REG(a1) struct Hook *hook)
{
    iffp->iffp_StreamHook = hook;
    iffp->iffp_Flags      = flags;
}


/*****************************************************************************/


/* These functions are examples of the interface defined for user-defined
 * stream management vectors.  They can be coded in assembly and be called
 * directly, or they can be coded in a high-level language and be called
 * via the generic stub function.  The interface is defined in terms of
 * the assembly interface, but the translation is straighforward for
 * HLL programmers.
 */
static LONG ASM DOS_Stream(REG(a0) struct Hook *hook,
                           REG(a2) struct IFFHandle *iff,
                           REG(a1) struct IFFStreamCmd *actionpkt)
{
LONG nbytes;
APTR buf;
BPTR stream;

    stream  = (BPTR)iff->iff_Stream;
    buf     = actionpkt->sc_Buf;
    nbytes  = actionpkt->sc_NBytes;

    switch (actionpkt->sc_Command)
    {
        case IFFSCC_READ  : return ((LONG) (Read (stream, buf, nbytes) != nbytes));
        case IFFSCC_WRITE : return ((LONG) (Write (stream, buf, nbytes) != nbytes));
        case IFFSCC_SEEK  : return ((LONG) (Seek (stream, nbytes, OFFSET_CURRENT) == -1));
        default           : return (0);
    }
}


/*****************************************************************************/


/* NOTE:  This routine assumes the supplied stream is fully seekable.
 * The client is responsible for determining, using whatever method,
 * if the supplied DOS stream is fully seekable or not, and adjusting the
 * iff_Flags field appropriately.
 */
VOID ASM InitIFFasDOSLVO(REG(a0) struct IFFHandleP *iffp)
{
static struct Hook	InternalDOSHook =
{
    { NULL },	/*  MinNode	*/
    DOS_Stream,	/*  h_Entry	*/
    NULL,		/*  h_SubEntry	*/
    NULL		/*  h_Data	*/
};

    /* For now, all DOS files are considered randomly seekable.
     * RSEEK implies FSEEK, but it's best to specify both.
     */
    InitIFF(iffp,IFFF_FSEEK | IFFF_RSEEK,&InternalDOSHook);
}


/*****************************************************************************/


struct ClipboardHandle * ASM OpenClipboardLVO(REG(d0) LONG unit)
{
struct ClipboardHandle	*clip;
LONG			signal1, signal2;

    if (clip = AllocVec(sizeof(struct ClipboardHandle),MEMF_ANY))
    {
        signal1 = AllocSignal(-1L);
        signal2 = AllocSignal(-1L);

        if ((signal1 >= 0) && (signal2 >= 0))
        {
            clip->cbh_CBport.mp_Node.ln_Type      = NT_MSGPORT;
            clip->cbh_CBport.mp_Flags             = PA_SIGNAL;
            clip->cbh_CBport.mp_SigTask           = FindTask(NULL);
            clip->cbh_CBport.mp_SigBit            = signal1;
            clip->cbh_Req.io_Message.mn_ReplyPort = &clip->cbh_CBport;
            NewList(&clip->cbh_CBport.mp_MsgList);

            clip->cbh_SatisfyPort.mp_Node.ln_Type = NT_MSGPORT;
            clip->cbh_SatisfyPort.mp_Flags        = PA_SIGNAL;
            clip->cbh_SatisfyPort.mp_SigBit       = signal2;
            clip->cbh_SatisfyPort.mp_SigTask      = FindTask(NULL);
            NewList(&clip->cbh_SatisfyPort.mp_MsgList);

            if (OpenDevice("clipboard.device", unit, &clip->cbh_Req,0L) == 0)
                return (clip);
        }
    }

    CloseClipboard(clip);

    return(NULL);
}


/*****************************************************************************/


VOID ASM CloseClipboardLVO(REG(a0) struct ClipboardHandle *clip)
{
    if (clip)
    {
        CloseDevice(&clip->cbh_Req);
        FreeSignal((LONG)clip->cbh_CBport.mp_SigBit);
        FreeSignal((LONG)clip->cbh_SatisfyPort.mp_SigBit);
        FreeVec(clip);
    }
}


/*****************************************************************************/


static LONG ASM CB_Stream(REG(a0) struct Hook *hook,
                          REG(a2) struct IFFHandle *iff,
                          REG(a1) struct IFFStreamCmd *actionpkt)
{
	register LONG			nbytes;
	register APTR			buf;
	register struct ClipboardHandle	*clip;

	clip	= (struct ClipboardHandle *) iff->iff_Stream;
	buf	= actionpkt->sc_Buf;
	nbytes	= actionpkt->sc_NBytes;

	switch (actionpkt->sc_Command) {
	case IFFSCC_INIT:
		clip->cbh_Req.io_ClipID =
		clip->cbh_Req.io_Offset =
		clip->cbh_Req.io_Error  = 0;
		return (0);

	case IFFSCC_CLEANUP:
		if ((iff->iff_Flags & IFFF_RWBITS) == IFFF_WRITE)
			clip->cbh_Req.io_Command	= CMD_UPDATE;
		else {
			clip->cbh_Req.io_Command	= CMD_READ;
			clip->cbh_Req.io_Data		= NULL;
			clip->cbh_Req.io_Length	= 1;
			clip->cbh_Req.io_Offset	= MAXOFF;
		}
		return ((LONG) DoIO (&clip->cbh_Req));

	case IFFSCC_READ:
		clip->cbh_Req.io_Command	= CMD_READ;
		clip->cbh_Req.io_Data		= buf;
		clip->cbh_Req.io_Length	= nbytes;

		return ((LONG) DoIO (&clip->cbh_Req));

	case IFFSCC_WRITE:
		clip->cbh_Req.io_Command	= CMD_WRITE;
		clip->cbh_Req.io_Data		= buf;
		clip->cbh_Req.io_Length	= nbytes;

		return ((LONG) DoIO (&clip->cbh_Req));

	case IFFSCC_SEEK:
	 {
		register LONG	curoff = clip->cbh_Req.io_Offset;

		if (curoff + nbytes < 0)
			return (IFFERR_SEEK);

		clip->cbh_Req.io_Offset += nbytes;
		return (0);
	 }
	}
}


/*****************************************************************************/


VOID ASM InitIFFasClipLVO(REG(a0) struct IFFHandleP *iffp)
{
static struct Hook InternalCBHook =
{
    { NULL },	/*  MinNode	*/
    CB_Stream,	/*  h_Entry	*/
    NULL,	/*  h_SubEntry	*/
    NULL	/*  h_Data	*/
};

    /* Init stream function vectors.
     * Language independent.
     */
    InitIFF(iffp,IFFF_FSEEK | IFFF_RSEEK,&InternalCBHook);
}
