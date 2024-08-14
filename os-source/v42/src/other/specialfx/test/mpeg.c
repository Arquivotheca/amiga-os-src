#include "SpecialFX.h"
#include "SpecialFXBase.h"
#include <graphics/videocontrol.h>
#include <graphics/view.h>
#include <intuition/screens.h>
#include "proto/specialfx_protos.h"
#include "proto/specialfx_pragmas.h"


struct SpecialFXBase *SpecialFXBase;

BOOL OpenMPEGScreen(struct Screen *s, ULONG ChromaPen)
{
    BOOL result = FALSE;
    struct TagItem vc[] =
    {
	{VTAG_USERCLIP_SET, TRUE},
	{VTAG_CHROMA_PEN_SET, 0},
	{VTAG_CHROMAKEY_CLR, TRUE},
	{TAG_DONE, NULL},
    };
    struct TagItem ti[] =
    {
	{SFX_InstallEffect, NULL},
	{TAG_DONE, NULL},
    };
    struct ViewPort *vp = &s->ViewPort;
    APTR VideoControlHandle;
    APTR AnimHandle;
    UWORD lines = s->Height;
    ULONG error;

    if (SpecialFXBase = (struct SpecialFXBase *)OpenLibrary("specialfx.library", 39))
    {
	vc[1].ti_Data = ChromaPen;
	if (VideoControl(vp->ColorMap, ti) == NULL)
	{
		if (VideoControlHandle = AllocFX(vp, SFX_FineVideoControl, lines, &error))
		{
			struct FineVideoControl **fvc = (struct FineVideoControl **)VideoControlHandle;
			struct FineVideoControl **first = fvc;
			struct FineVideoControl **vctop, **vcbottom;
			struct TagItem tifvc[2][2] =
			{
				{
					{VTAG_CHROMAKEY_CLR, TRUE},
					{TAG_DONE},
				},
				{
					{VTAG_CHROMAKEY_SET, TRUE},
					{TAG_DONE},
				},
			};
			UWORD top;
			int i;

			for (i = 0; i < lines; i++)
			{
				(*fvc)->fvc_TagList = (ULONG)&tifvc[0];
				(*fvc)->fvc_Line = i;
				(*fvc)->fvc_Count = 1;
				fvc++;
			}
			ti[0].ti_Data = (ULONG)VideoControlHandle;
			if (AnimHandle = InstallFXA(GfxBase->ActiView, vp, ti))
			{
				MakeScreen(s);
				RethinkDisplay();

				/* Now animate */
				top = (lines / 2);
				vctop = (first + top);
				vcbottom = (vctop + 1);
				for (i = top; i > 0; i--)
				{
					(*vctop)->fvc_TagList = (ULONG)&tifvc[1];
					(*vctop)->fvc_Flags = FVCF_MODIFY;
					(*vcbottom)->fvc_TagList = (ULONG)&tifvc[1];
					(*vcbottom)->fvc_Flags = FVCF_MODIFY;

					if (i < top)
					{
						(*vctop + 1)->fvc_Flags = 0;
						(*vcbottom - 1)->fvc_Flags = 0;
					}

					vctop--;
					vcbottom++;
	
					WaitTOF();
					AnimateFX(AnimHandle);
				}
				result = TRUE;
				RemoveFX(AnimHandle);
			}
			FreeFX(VideoControlHandle);
			VideoControl(vp->ColorMap, &tifvc[1]);
			MakeScreen(s);
			RethinkDisplay();
		}
	}
	CloseLibrary(SpecialFXBase);
    }

    return(result);
}
