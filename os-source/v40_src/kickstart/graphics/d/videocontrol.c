/******************************************************************************
*
*   $Id: videocontrol.c,v 39.21 93/03/12 13:58:59 chrisg Exp $
*
*   $Locker:  $
*
******************************************************************************/

#include <exec/types.h>
#include <utility/tagitem.h>
#include "/gfxbase.h"
#include "/view.h"
#include "/videocontrol.h"
#include "/displayinfo.h"
#include "d.protos"
#include "/macros.h"
#include "/displayinfo_internal.h"
#include "/c/c.protos"
#include <hardware/custom.h>
#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>

extern struct Custom custom;

/* need to consider GlobalVideoControl issues at length -- bart */

/* need to consider GlobalVideoControl issues at length -- bart */

/* returns the number of pens currently set up for chroma_key */

int __regargs num_chroma_pens(cm) 
struct ColorMap *cm;
{
    int i, num = 0;
    WORD *p = cm->ColorTable;

    Forbid();	/* like this really protects us! */

    for(i=0; i<cm->Count; i++)
	if ((*p++)<0) num++;		/* check high bit of color table */

    Permit();

    return(num); 
}

/* This function may then be renamed to LocalVideoControl */

#define AA_CHIPS (GBASE->ChipRevBits0 & GFXF_AA_LISA)

long	videocontrol(cm,v)
register struct ColorMap *cm;
register struct TagItem *v;
{
	struct TagItem *tstate;
	int error = FALSE;
	int must_remake = 0;			/* must a remake happen if vtag_immed was set? */
	int *remake_variable=0;			/* where the user wants the remake flag set or null */
	int must_do_bplcon2=0;			/* must we repoke bplcon2? */
	int must_do_bplcon3=0;			/* must we repoke bplcon3? */
	int must_do_bplcon4=0;			/* must we repoke bplcon4? */
	int must_do_colors=0;			/* must we call pokecolors? */
	int must_do_ecsena=0;			/* ecs enable changes */
	if(!cm || (!(cm->Type))) error = -1; /* error, bad colormap */

	tstate=v;
	while((!error) && (v=NextTagItem(&tstate)))
	{
		int TagData=(int) v->ti_Data;
		switch(v->ti_Tag)
		{
			/* handle videocontrol tags */
			case	VTAG_CHROMAKEY_CLR:
				must_do_colors++;
				must_do_bplcon2++;
				cm->Flags &= ~COLORMAP_TRANSPARENCY;
				break;
			case	VTAG_CHROMAKEY_SET:
				must_do_colors++;
				must_do_bplcon2++;
				cm->Flags |=  COLORMAP_TRANSPARENCY;
				break;
			case	VTAG_BITPLANEKEY_CLR:
				must_do_bplcon2++;
				cm->Flags &= ~COLORPLANE_TRANSPARENCY;
				break;
			case	VTAG_BITPLANEKEY_SET:
				must_do_bplcon2++;
				cm->Flags |=  COLORPLANE_TRANSPARENCY;
				break;
			case	VTAG_BORDERBLANK_CLR:
				must_do_bplcon3++;
				must_do_ecsena++;
				cm->Flags &= ~BORDER_BLANKING;
				break;
			case	VTAG_BORDERBLANK_SET:
				must_do_bplcon3++;
				must_do_ecsena++;
				cm->Flags |=  BORDER_BLANKING;
				break;
			case	VTAG_BORDERNOTRANS_CLR:
				must_do_ecsena++;
				must_do_bplcon3++;
				cm->Flags &= ~BORDER_NOTRANSPARENCY;
				break;
			case	VTAG_BORDERNOTRANS_SET:
				must_do_ecsena++;
				must_do_bplcon3++;
				cm->Flags |=  BORDER_NOTRANSPARENCY;
				break;
			case	VTAG_CHROMA_PEN_CLR:
				if (TagData <cm->Count)
				{
				     must_do_colors++;
				     ((UWORD *)(cm->ColorTable))[TagData] &= 0x7fff;
				}
				break;
			case	VTAG_CHROMA_PEN_SET:
				if (TagData <cm->Count)
				{
				    ((UWORD *)(cm->ColorTable))[TagData] |= 0x8000;
				    must_do_colors++;
				}
				break;
			case	VTAG_CHROMA_PLANE_SET:
				if (TagData < 8) {
					must_do_bplcon2++;
					cm->TransparencyPlane = TagData;
				}
				break;
			case	VTAG_ATTACH_CM_SET:
				Forbid();
				{
				    struct ViewPort *vp = 
				    (struct ViewPort *) TagData;
				    must_remake++;
				    if (vp)
				    {
					/* remove old reference */
					struct ColorMap *oldcm = vp->ColorMap;
					if (oldcm) oldcm->cm_vp = 0;
					vp->ColorMap = cm;
					cm->cm_vp = vp;
				    }
				}
				Permit();
				break;
			case	VTAG_NEXTBUF_CM:
				tstate = (struct TagItem *)TagData;
				break;
			case	VTAG_BATCH_CM_CLR:
				cm->Flags &= ~VIDEOCONTROL_BATCH;
				break;
			case	VTAG_BATCH_CM_SET:
				cm->Flags |=  VIDEOCONTROL_BATCH;
				break;
			case	VTAG_NORMAL_DISP_GET:
				v->ti_Tag  = VTAG_NORMAL_DISP_SET;
				v->ti_Data = (ULONG)cm->NormalDisplayInfo;
				break;
			case	VTAG_NORMAL_DISP_SET:
				must_remake++;
				cm->NormalDisplayInfo = (APTR)TagData;
				break;
			case	VTAG_COERCE_DISP_GET:
				v->ti_Tag  = VTAG_COERCE_DISP_SET;
				v->ti_Data = (ULONG)cm->CoerceDisplayInfo;
				break;
			case	VTAG_COERCE_DISP_SET:
				must_remake++;
				cm->CoerceDisplayInfo = (APTR)TagData;
				break;
			case	VTAG_VIEWPORTEXTRA_GET:
				v->ti_Tag  = VTAG_VIEWPORTEXTRA_SET;
				v->ti_Data = (ULONG)cm->cm_vpe;
				break;
			case	VTAG_VIEWPORTEXTRA_SET:
				{
					must_remake++;
					cm->cm_vpe = (struct ViewPortExtra *)TagData;
				}
			case	VTAG_CHROMAKEY_GET:
				v->ti_Tag = VTAG_CHROMAKEY_CLR;
				if(cm->Flags & COLORMAP_TRANSPARENCY)
				{
				    v->ti_Tag = VTAG_CHROMAKEY_SET;
				}
				v->ti_Data = num_chroma_pens(cm);
				break;
			case	VTAG_BITPLANEKEY_GET:
				v->ti_Tag = VTAG_BITPLANEKEY_CLR;
				if(cm->Flags & COLORPLANE_TRANSPARENCY)
				{
				    v->ti_Tag = VTAG_BITPLANEKEY_SET;
				}
				break;
			case	VTAG_BORDERBLANK_GET:
			    v->ti_Tag = VTAG_BORDERBLANK_CLR;
				if (cm->Flags & BORDER_BLANKING)
				{
				    v->ti_Tag = VTAG_BORDERBLANK_SET;
				}
				break;
			case	VTAG_BORDERNOTRANS_GET:
				v->ti_Tag = VTAG_BORDERNOTRANS_CLR;
				if(cm->Flags & BORDER_NOTRANSPARENCY)
				{
				    v->ti_Tag = VTAG_BORDERNOTRANS_SET;
				}
				break;
			case	VTAG_CHROMA_PEN_GET:
				v->ti_Tag=(((WORD *)cm->ColorTable)[TagData] < 0)?VTAG_CHROMA_PEN_SET:VTAG_CHROMA_PEN_CLR;
				break;
			case	VTAG_CHROMA_PLANE_GET:
				v->ti_Tag  = VTAG_CHROMA_PLANE_SET;
				v->ti_Data = (ULONG)cm->TransparencyPlane;
				break;
			case	VTAG_ATTACH_CM_GET:
				v->ti_Tag  = VTAG_ATTACH_CM_SET;
				v->ti_Data = (ULONG)cm->cm_vp;
				break;
			case	VTAG_BATCH_CM_GET:
			    v->ti_Tag  = VTAG_BATCH_CM_CLR;
				if(cm->Flags & VIDEOCONTROL_BATCH)
				{
				    v->ti_Tag  = VTAG_BATCH_CM_SET;
				}
				break;
			case	VTAG_BATCH_ITEMS_GET:
				v->ti_Tag  = VTAG_BATCH_ITEMS_SET;
				v->ti_Data = (ULONG)cm->cm_batch_items;
				break;
			case	VTAG_BATCH_ITEMS_SET:
				cm->cm_batch_items = (struct TagItem *)	TagData;
				break;
			case	VTAG_BATCH_ITEMS_ADD:
				Forbid(); 
				{
				    struct TagItem *oi = cm->cm_batch_items;
				    struct TagItem *ti = (struct TagItem *)
				    TagData;

				    while(ti && (ti->ti_Tag != VTAG_END_CM))
				    {
					if(ti->ti_Tag == VTAG_NEXTBUF_CM)
					{
					    ti = (struct TagItem *)ti->ti_Data;
					}
					else 
					{
					    ti++;
					}
				    }
				    if(ti != (struct TagItem *)TagData) 
				    {
					ti->ti_Tag  = VTAG_NEXTBUF_CM;
					ti->ti_Data = (ULONG)oi;
					ti = (struct TagItem *)TagData;
				    }
				    else
				    {
					ti = oi;
				    }
				    cm->cm_batch_items = ti;
				}
				Permit();
				break;
			case	VTAG_VPMODEID_GET:
				    v->ti_Tag  = VTAG_VPMODEID_SET;
					v->ti_Data = (ULONG)cm->VPModeID;
				break;
			case	VTAG_VPMODEID_SET:
				{
					struct ViewPort *vp = cm->cm_vp;
					ULONG ID = (ULONG)TagData;

					must_remake++;
					ObtainSemaphore(GBASE->ActiViewCprSemaphore);
					cm->VPModeID = ID;
					if ((vp) && (ID != INVALID_ID))
					{
						AttachVecTable(cm->cm_vpe, NULL, ID);
					}
					ReleaseSemaphore(GBASE->ActiViewCprSemaphore);
				}
				break;
			case	VTAG_VPMODEID_CLR:
				{
					must_remake++;
					ObtainSemaphore(GBASE->ActiViewCprSemaphore);
					cm->VPModeID = (ULONG)INVALID_ID;
					if (cm->cm_vpe)
					{
						cm->cm_vpe->VecTable = NULL;
					}
					ReleaseSemaphore(GBASE->ActiViewCprSemaphore);
				}
				break;
			case	VTAG_USERCLIP_GET:
			    	v->ti_Tag = VTAG_USERCLIP_CLR;
				if(cm->Flags & USER_COPPER_CLIP)
				{
				    v->ti_Tag = VTAG_USERCLIP_SET;
				}
				break;
			case	VTAG_USERCLIP_SET:
				must_remake++;
				cm->Flags |=   USER_COPPER_CLIP;
				break;
			case	VTAG_USERCLIP_CLR:
				must_remake++;
				cm->Flags &=  ~USER_COPPER_CLIP;
				break;

			case VTAG_PF2_BASE_SET:
				if (AA_CHIPS) {
					cm->Bp_0_base=(UWORD) TagData; must_do_bplcon4++;
				}
				break;
			case VTAG_PF1_BASE_SET:
				if (AA_CHIPS) { cm->Bp_1_base=(UWORD) TagData; must_do_bplcon4++; }
				break;

			case VTAG_SPEVEN_BASE_SET:
				if (AA_CHIPS) { cm->SpriteBase_Even=(UWORD) TagData; must_do_bplcon4++; }
				break;

			case VTAG_SPODD_BASE_SET:
				if (AA_CHIPS) { cm->SpriteBase_Odd=(UWORD) TagData; must_do_bplcon4++; }
				break;

			case VTAG_PF2_BASE_GET:
				v->ti_Tag = VTAG_PF2_BASE_SET;
				v->ti_Data=cm->Bp_0_base;
				break;

			case VTAG_PF1_BASE_GET:
				v->ti_Tag = VTAG_PF1_BASE_SET;
				v->ti_Data=cm->Bp_1_base;
				break;

			case VTAG_SPEVEN_BASE_GET:
				v->ti_Tag=VTAG_SPEVEN_BASE_SET;
				v->ti_Data=cm->SpriteBase_Even;
				break;

			case VTAG_SPODD_BASE_GET:
				v->ti_Tag=VTAG_SPODD_BASE_SET;
				v->ti_Data=cm->SpriteBase_Odd;
				break;


			case	VTAG_BORDERSPRITE_GET:
			        v->ti_Tag = VTAG_BORDERSPRITE_CLR;
				if(cm->Flags & BORDERSPRITES)
				{
				    v->ti_Tag = VTAG_BORDERSPRITE_SET;
				}
				break;
			case	VTAG_BORDERSPRITE_SET:
				if (AA_CHIPS) { cm->Flags |= BORDERSPRITES; must_do_bplcon3++; }
				break;
			case	VTAG_BORDERSPRITE_CLR:
				if (AA_CHIPS) { cm->Flags &=  ~BORDERSPRITES; must_do_bplcon3++; }
				break;
			case	VTAG_SPRITERESN_SET:
				if ((AA_CHIPS) && (cm->SpriteResolution != TagData))
				{
				 	cm->SpriteResolution = TagData;
					must_do_bplcon3++;
				}
				break;
			case	VTAG_SPRITERESN_GET:
				v->ti_Tag = VTAG_SPRITERESN_SET;
				v->ti_Data = cm->SpriteResolution;
				break;
			case	VTAG_PF1_TO_SPRITEPRI_SET:
				if (cm->cm_vp)
				{
					UBYTE pri = (cm->cm_vp->SpritePriorities & 0x38);	/* preserve pf2 */
					cm->cm_vp->SpritePriorities = (pri | MIN(TagData, 7));
					must_do_bplcon2++;
				}
				break;
			case	VTAG_PF1_TO_SPRITEPRI_GET:
				v->ti_Tag = VTAG_PF1_TO_SPRITEPRI_SET;
				v->ti_Data = 0x4;	/* default */
				if (cm->cm_vp)
				{
					v->ti_Data = (cm->cm_vp->SpritePriorities & 0x7);
				}
				break;
			case	VTAG_PF2_TO_SPRITEPRI_SET:
				if (cm->cm_vp)
				{
					UBYTE pri = (cm->cm_vp->SpritePriorities & 0x7);	/* preserve pf1 */
					must_do_bplcon2++;
					cm->cm_vp->SpritePriorities = (pri | (MIN(TagData, 7) << 3));
				}
				break;
			case	VTAG_PF2_TO_SPRITEPRI_GET:
				v->ti_Tag = VTAG_PF2_TO_SPRITEPRI_SET;
				v->ti_Data = 0x4;	/* default */
				if (cm->cm_vp)
				{
					v->ti_Data = ((cm->cm_vp->SpritePriorities & 0x38) >> 3);
				}
				break;
			case VTAG_IMMEDIATE:
				remake_variable=(int *) (TagData);
				break;

			case VTAG_FULLPALETTE_SET:
				cm->AuxFlags |= CMAF_FULLPALETTE;
				must_remake++;
				break;

			case VTAG_FULLPALETTE_GET:
				v->ti_Tag=VTAG_FULLPALETTE_SET;
				v->ti_Data=(cm->AuxFlags & CMAF_FULLPALETTE)?-1:0;
				break;

			case VTAG_FULLPALETTE_CLR:
				cm->AuxFlags &= ~CMAF_FULLPALETTE;
				break;

			case VTAG_DEFSPRITERESN_SET:
				if ((AA_CHIPS) && (cm->SpriteResDefault != TagData))
				{
					cm->SpriteResDefault=TagData;
					must_do_bplcon3++;
				}
				break;

			case VTAG_DEFSPRITERESN_GET:
				v->ti_Tag=VTAG_DEFSPRITERESN_SET;
				v->ti_Data=cm->SpriteResDefault;
				break;

			default:
/* start of new tag handling tags! separating these makes code smaller */
				switch(v->ti_Tag)
				{
					case VC_IntermediateCLUpdate:
						if (TagData)
							cm->AuxFlags &= ~CMAF_NO_INTERMED_UPDATE;
						else
							cm->AuxFlags |= CMAF_NO_INTERMED_UPDATE;
						break;

					case VC_IntermediateCLUpdate_Query:
						*((ULONG *) TagData)=(cm->AuxFlags & CMAF_NO_INTERMED_UPDATE)?0:-1;
						break;
					case VC_NoColorPaletteLoad:
						must_remake++;
						if (TagData)
						{
							cm->AuxFlags |= CMAF_NO_COLOR_LOAD;
						}
						else
						{
							cm->AuxFlags &= ~CMAF_NO_COLOR_LOAD;
						}
						break;
					case VC_NoColorPaletteLoad_Query:
						*((ULONG *) TagData)=(cm->AuxFlags & CMAF_NO_COLOR_LOAD)?-1:0;
						break;

					case VC_DUALPF_Disable:
						must_remake++;
						if (TagData)
						{
							cm->AuxFlags |= CMAF_DUALPF_DISABLE;
						}
						else
						{
							cm->AuxFlags &= ~CMAF_DUALPF_DISABLE;
						}
						break;
					case VC_DUALPF_Disable_Query:
						*((ULONG *) TagData)=(cm->AuxFlags & CMAF_DUALPF_DISABLE)? -1:0;
						break;
				}
		}
	}



//	kprintf("make=%d con2=%d con4=%d cm->cm_vp=%08lx\n",must_remake,must_do_bplcon2,must_do_bplcon4,cm->cm_vp);

	must_remake = (must_remake || must_do_bplcon2 || must_do_bplcon4 || !cm || !(cm->cm_vp));

	if (remake_variable)
	{
	    *remake_variable=must_remake;
	    if (! must_remake)
	    {
		if (must_do_colors || must_do_bplcon3)
		{
		    ObtainSemaphore(GBASE->ActiViewCprSemaphore);
		    if (must_do_colors) pokecolors(cm->cm_vp);
		    if (must_do_bplcon3) pokevp(cm->cm_vp,REGNUM(bplcon3),get_bplcon3(cm),0x1cfe);
		    if (must_do_ecsena)	pokevp(cm->cm_vp,REGNUM(bplcon0),1,1);
		    update_top_color(GBASE->current_monitor);
		    ReleaseSemaphore(GBASE->ActiViewCprSemaphore);
		}
	    }
	}	


	return (error);	/* ok?, or not... */
}
