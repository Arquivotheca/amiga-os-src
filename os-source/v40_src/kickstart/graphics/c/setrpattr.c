/******************************************************************************
*
*   $Id: setrpattr.c,v 39.4 93/05/05 15:44:34 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <utility/tagitem.h>
#include "/gfxbase.h"
#include "/gfxpragmas.h"
#include "/macros.h"
#include "/rpattr.h"
#include "/rastport.h"
#include "/gfx.h"
#include "c.protos"
#include <graphics/clip.h>
#include <graphics/layers.h>
#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/exec_pragmas.h>

/****** graphics.library/SetRPAttrA *******************************************
*
*   NAME
*       SetRPAttrA -- modify rastport settings via a tag list
*	SetRPAttrs  -- varargs stub for SetRPAttrA
*
*   SYNOPSIS
*       SetRPAttrA(rp,tags)
*                  a0   a1
*
*	void SetRPAttrA(struct RastPort *, struct TagItem *);
*
*	SetRPAttrs(rp,tag,...);
*
*   FUNCTION
*       Modify settings of a rastport, based on the taglist passed.
*	currently available tags are:
*
*		RPTAG_Font		Font for Text()
*		RPTAG_SoftStyle		style for text (see graphics/text.h)
*		RPTAG_APen		Primary rendering pen
*		RPTAG_BPen		Secondary rendering pen
*		RPTAG_DrMd 		Drawing mode (see graphics/rastport.h)
*		RPTAG_OutLinePen 	Area Outline pen
*		RPTAG_WriteMask 	Bit Mask for writing.
*		RPTAG_MaxPen 		Maximum pen to render (see SetMaxPen())
*
*   INPUTS
*	rp - pointer to the RastPort to modify.
*	tags - a standard tag list
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	SetFont() SetSoftStyle() SetAPen() SetBPen() SetDrMd() SetOutLinePen()
*	SetWriteMask() SetMaxPen() GetRPAttrA() graphics/rpattr.h
*
******************************************************************************/

void __asm SetRPAttrA(register __a0 struct RastPort *rp,register __a1 struct TagItem *tag)
{
	struct TagItem *tstate;
	tstate=tag;
	while (tag=NextTagItem(&tstate))
	{
		register ULONG data=tag->ti_Data;	/* attempt to fool lattice into generating good code */
		switch(tag->ti_Tag)
		{
			case RPTAG_Font:
				gfx_SetFont(rp,data);
				break;
			case RPTAG_APen:
				gfx_SetAPen(rp,data);
				break;
			case RPTAG_BPen:
				gfx_SetBPen(rp,data);
				break;
			case RPTAG_DrMd:
				gfx_SetDrMd(rp,data);
				break;
			case RPTAG_OutLinePen:
				gfx_SetOutlinePen(rp,data);
				break;
			case RPTAG_WriteMask:
				gfx_SetWriteMask(rp,data);
				break;
			case RPTAG_MaxPen:
				gfx_SetMaxPen(rp,data);
				break;
		}
	}
}

/****** graphics.library/GetRPAttrA *******************************************
*
*   NAME
*       GetRPAttrA -- examine rastport settings via a tag list
*	GetRPAttrs  -- varargs stub for GetRPAttrA
*
*   SYNOPSIS
*       GetRPAttrA(rp,tags)
*                  a0   a1
*
*	void GetRPAttrA(struct RastPort *, struct TagItem *);
*
*	GetRPAttrs(rp,attr1,&result1,...);
*
*   FUNCTION
*       Read the settings of a rastport into variables. The
*	ti_Tag field of the TagItem specifies which attribute
*	should be read, and the ti_Data field points at the
*	location where the result hsould be stored. All current
*	tags store the return data as LONGs (32 bits).
*
*	currently available tags are:
*
*		RPTAG_Font		Font for Text()
*		RPTAG_SoftStyle		style for text (see graphics/text.h)
*		RPTAG_APen		Primary rendering pen
*		RPTAG_BPen		Secondary rendering pen
*		RPTAG_DrMd 		Drawing mode (see graphics/rastport.h)
*		RPTAG_OutLinePen 	Area Outline pen
*		RPTAG_WriteMask 	Bit Mask for writing.
*		RPTAG_MaxPen 		Maximum pen to render (see SetMaxPen())
*		RPTAG_DrawBounds	Determine the area that will be rendered
*					into by rendering commands. Can be used
*					to optimize window refresh. Pass a pointer
*					to a rectangle in the tag data. On return,
*					the rectangle's MinX will be greater than
*					its MaxX if there are no active cliprects.
*
*   INPUTS
*	rp - pointer to the RastPort to examine.
*	tags - a standard tag list specifying the attributes to be read,
*		and where to store their values.
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	GetAPen() GetBPen() GetDrMd() GetOutLinePen()
*	GetWriteMask() SetRPAttrA() graphics/rpattr.h
*
******************************************************************************/

void __asm GetRPAttrA(register __a0 struct RastPort *rp,register __a1 struct TagItem *tag)
{
	struct TagItem *tstate;
	tstate=tag;
	while (tag=NextTagItem(&tstate))
	{
		register ULONG *data=(ULONG *) tag->ti_Data;	/* attempt to fool lattice into generating good code */
		switch(tag->ti_Tag)
		{
			case RPTAG_Font:
				*((struct Font **)data)=rp->Font;
				break;
			case RPTAG_APen:
				*data=gfx_GetAPen(rp);
				break;
			case RPTAG_BPen:
				*data=gfx_GetBPen(rp);
				break;
			case RPTAG_DrMd:
				*data=gfx_GetDrMd(rp);
				break;
			case RPTAG_OutLinePen:
				*data=gfx_GetOutlinePen(rp);
				break;
			case RPTAG_WriteMask:
				*data=rp->Mask;
				break;
			case RPTAG_MaxPen:
				{
					BYTE msk=rp->Mask;
					ULONG c=256;
					while (c && (msk>0) ) { c >>=1 ; msk<<=1; }
					*data=c;
				}
				break;
			case RPTAG_DrawBounds:
				GetRPBounds(rp,(struct Rectangle *) data);
				break;
		    }
	}
}



