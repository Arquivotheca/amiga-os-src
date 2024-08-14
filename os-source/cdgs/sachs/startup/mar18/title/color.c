/* :ts=4
*
*	color.c
*
*	William A. Ware						D303
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright 1993, Silent Software, Incorporated.							*
*   All Rights Reserved.													*
****************************************************************************/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/memory.h>
#include <proto/exec.h>

#include <proto/graphics.h>

#include <cdtv/debox.h>
#include <cdtv/deboxproto.h>

#include "color.h"
#include "title.h"

// SPOOF THE PRAGMAS
//
#define	GfxBase		(td->GfxBase)
#define DeBoxBase	(td->DeBoxBase)

void FreeCNodeData( struct TData *td, struct CNode *cn )
{
	if (cn->Table) FreeBMInfoRGB32( cn->Table );
	cn->Table = NULL;
	cn->Flags |= CNF_FINISHED;
}

BOOL CutCNode(struct TData *td, 
				struct CNode *cn, struct BMInfo *bmi,ULONG *table, 
				WORD start, WORD stop )
{
	int size;

	FreeCNodeData( td,cn );
	
	if (!(cn->Table = GetBMInfoRGB32( bmi,(size = stop-start+1),start )))
		return FALSE;
	cn->From = start;
	cn->To  = stop;
	
	cn->Size = size;
	cn->Bottom = start-size;
	cn->Flags = 0;

	if (table) MemSet( &table[1+start*3], 0,size * 12 );
	return TRUE;
}

BOOL GetCNodeData( struct TData *td, 
					struct CNode *cn, UBYTE *data, WORD start,WORD stop )
{
	struct BMInfo *bmi;

	if (bmi = DecompBMInfo( NULL,NULL,data ))
	{
		if (!CutCNode( td,cn,bmi,NULL,start,stop )) return FALSE;
		FreeBMInfo( bmi );
	}
}

//---------------------------------------------------------------------


static BOOL cnMoveUp( struct TData *td, struct CNode *cn, ULONG *table, 
					  BOOL fake )
{
	BOOL	ans = TRUE;
	WORD	bottom,size,start;
	ULONG	*from,*to;

	if (cn->Flags & CNF_FINISHED) return FALSE;
	cn->Bottom++;
	bottom = cn->Bottom;
	
	if (bottom > cn->To) 
	{
		bottom = cn->To;
		ans = FALSE;
		cn->Flags |= CNF_FINISHED;
	}
	
	if (fake)
	{
		if (bottom >= cn->From)
		{
			from = &cn->Table[1];
			to = &table[1+3*bottom];
			*to++ = *from++;
			*to++ = *from++;
			*to++ = *from++;
		}
		return ans;
	}
	
	start  = 0;
	size   = cn->Size;
	
	if (bottom < cn->From)
	{
		size -= cn->From-bottom; 
		start = cn->From - bottom;
		bottom = cn->From;
	}
	
	
	if (bottom + size > cn->To)
		size = cn->To - bottom + 1;

	if (size > 0)
	{
		CopyMem( &cn->Table[1+3*start],&table[1+3*bottom],size * 12 );
	}

	return ans;
}


static BOOL cnMoveDown( struct TData *td, struct CNode *cn, ULONG *table,
						BOOL fake )
{
	BOOL	ans = TRUE;
	WORD	bottom,size,start;
	ULONG	*from,*to;

	if (cn->Flags & CNF_FINISHED) return FALSE;

	bottom = --cn->Bottom;

	if (bottom <= cn->From - cn->Size + 1)
	{
		bottom = cn->From - cn->Size + 1;
		ans = FALSE;
		cn->Flags |= CNF_FINISHED;
	}

	if (fake)
	{
		if ((bottom += cn->Size-1) <= cn->To)
		{
			from = &cn->Table[1+3*(cn->Size-1)];
			to = &table[1+3*bottom];
			*to++ = *from++;
			*to++ = *from++;
			*to++ = *from++;
		}
		return ans;
	}

	start  = 0;
	size   = cn->Size;
	
	if (bottom < cn->From)
	{
		size -= cn->From-bottom; 
		start = cn->From - bottom;
		bottom = cn->From;
	}
	
	
	if (bottom + size > cn->To)
		size = cn->To - bottom + 1;

	if (size > 0)
	{
		CopyMem( &cn->Table[1+3*start],&table[1+3*bottom],size * 12 );
	}
	return ans;
}





static BOOL cnOneColor( struct TData *td,struct CNode *cn, ULONG *table,
						BOOL fake )
{
	BOOL			ans = TRUE;
	register ULONG	*from,*to;
	int				i;

	if (cn->Flags & CNF_FINISHED) return FALSE;
	cn->Bottom++;

	if (cn->Bottom >= cn->Size)
	{
		cn->Flags |= CNF_FINISHED;
		cn->Bottom = cn->Size-1;
		ans = FALSE;
	}
	
	if (fake && (ans == TRUE)) return TRUE;

	for(i=cn->From,from = &cn->Table[1+3*cn->Bottom],to=&table[1+3*cn->From];
		i <= cn->To;i++)
	{
		*to++ = from[0];
		*to++ = from[1];
		*to++ = from[2];
	}
	return ans;
}

static BOOL cnSpriteColor( struct TData *td,struct CNode *cn, ULONG *table,
							BOOL fake)
{
	BOOL			ans = TRUE;
	register ULONG	*from,*to;
	int				i;

	if (cn->Flags & CNF_FINISHED) return FALSE;
	cn->Bottom++;

	if (cn->Bottom >= cn->Size)
	{
		cn->Flags |= CNF_FINISHED;
		cn->Bottom = cn->Size-1;
		ans = FALSE;
	}

	if (fake && (ans == TRUE)) return TRUE;

	for(i=0,from = &cn->Table[1+3*cn->Bottom],to=&table[1+3*cn->From];
		i < 4;i++)
	{
		to[0] = from[0];
		to[1] = from[1];
		to[2] = from[2];
		to = &to[3 * 4];
	}
	return ans;
}


static BOOL cnCycle( struct TData *td,struct CNode *cn, ULONG *table,
							BOOL fake)
{
	BOOL			ans = TRUE;
	register ULONG	*from,*to;
	WORD			start,i;
	

	if (cn->Flags & CNF_FINISHED) return FALSE;
	cn->Bottom++;
	
	if (cn->Bottom > cn->To)
		cn->Bottom = cn->To - cn->Size;
	
	if (!(cn->Flags & CNF_CYCLE)) 
	{
		cn->Flags |= CNF_FINISHED;
		ans = FALSE;
	}
	
	if (cn->Bottom >= cn->From)
		start = cn->Size - (cn->Bottom - cn->From);
	else
		start = cn->From - cn->Bottom;

	if (fake && (ans == TRUE)) return TRUE;

	for(i=cn->From, from = &cn->Table[1+3*start], to = &td->Table[1+3*cn->From ];
		i<=cn->To;i++)
	{
		if (start++ >= cn->Size) 
		{
			start = 0;
			from = &cn->Table[1];
		}
		*to++ = *from++;
		*to++ = *from++;
		*to++ = *from++;
	}
	return ans;
}
	



void InsertCNode( struct TData *td,struct MinList *list, struct CNode *cn )
{
	struct CNode	*c;
	
	for(c = (struct CNode *)list->mlh_Head;c->Node.mln_Succ;
		c = (struct CNode *)c->Node.mln_Succ )
	{
		if ((c->From == cn->From) && (c->To == cn->To)) 
		{
			Insert( (struct List *)list, (struct Node *)cn, (struct Node *)c );
			return;
		}
	}
	AddTail( (struct List *)list,(struct Node *)cn );
}


void SwitchCNode( struct TData *td, struct CNode *cn )
{
	if (cn->Func == (BOOL (*)(struct TData *,struct CNode *,...))cnMoveUp)
		cn->Func = (BOOL (*)(struct TData *,struct CNode *,...))cnMoveDown;
	else
	{
		if (cn->Func == (BOOL (*)(struct TData *,struct CNode *,...))cnMoveDown)
			cn->Func = (BOOL (*)(struct TData *,struct CNode *,...))cnMoveUp;
		else return;
	}
}

void StartCNode(struct TData *td,
				struct MinList *list,
				struct CNode *cn, WORD start, WORD stop, 
				LONG speed, LONG initcounter,
				UWORD type )
{

	if (cn->Node.mln_Succ != NULL)
		Remove( (struct Node *)cn );
	
	FOFF( cn->Flags, CNF_FINISHED );

	cn->Type = type;
	cn->Speed = speed; 
	cn->Counter = initcounter;

	cn->From = start; 
	cn->To = stop; 

	switch( type )
	{
		default:
		case CNTYPE_MOVEUP: 
			cn->Func = (BOOL (*)(struct TData *,struct CNode *,...))cnMoveUp;
			cn->Bottom = cn->From - cn->Size;
			break;
		case CNTYPE_MOVEDOWN: 
			cn->Func = (BOOL (*)(struct TData *,struct CNode *,...))cnMoveDown;
			cn->Bottom = cn->To;
			break;
		case CNTYPE_ONECOLOR:
			cn->Func = (BOOL (*)(struct TData *,struct CNode *,...))cnOneColor;
			cn->Bottom = 0;
			break;
		case CNTYPE_SPRITECOLOR:
			cn->Func = (BOOL (*)(struct TData *,struct CNode *,...))cnSpriteColor;
			cn->Bottom = 0;
			break;
		case CNTYPE_CYCLE:
			FON(cn->Flags,CNF_CYCLE);
			cn->Func = (BOOL (*)(struct TData *,struct CNode *,...))cnCycle;
			if ((cn->Bottom < cn->To - cn->Size) || (cn->Bottom >= cn->From))  
				cn->Bottom = cn->To - cn->Size;
			break;
	}


	if (list)
		InsertCNode( td,list, cn );
}


BOOL RunCNodes( struct TData *td,struct MinList *list, ULONG *table, LONG time )
{
	BOOL		 	ans = FALSE;
	struct CNode 	*cn,*next,*prev = NULL;
	struct CNode	*addlist = NULL,*tempcn;
	
	cn = (struct CNode *)list->mlh_Head;
	next = (struct CNode *)cn->Node.mln_Succ;
	
	for(;cn->Node.mln_Succ;
		prev = cn,cn = next,next = (struct CNode *)cn->Node.mln_Succ)
	{
		if ((cn->Counter-=time) < 0)
		{
			while( cn->Counter < 0)
			{
				cn->Counter += cn->Speed;
				ans = TRUE;
		
				if (!(cn->Func( td,cn,table,(cn->Counter < 0 ? TRUE:FALSE))))
				{				
					Remove( (struct Node *)cn );
					cn->Node.mln_Succ = NULL;
					if (tempcn = cn->NextCNode)
					{
						if (!tempcn->Node.mln_Succ)
						{
							tempcn->Node.mln_Succ = addlist;
							addlist = cn->NextCNode;
						}
						cn->NextCNode = NULL;
					}
					if (cn->Flags & CNF_CYCLE)
						StartCNode( td,&td->CnList,cn,cn->From,cn->To,
									cn->Speed,cn->CycleWait,cn->Type );
					else
					{
						if (cn->ReturnList) 
							AddTail((struct List *)cn->ReturnList,(struct Node *)cn);
					}
					break;
				}
			}
		}
	}
	
	while( addlist )
	{
		cn = addlist;
		addlist = (struct CNode *)cn->Node.mln_Succ;
		cn->Node.mln_Succ = NULL;
		InsertCNode( td,list, cn );
	}
	return ans;
}

//-----------------------------------------------------------------------------
void FadeColor( struct TData *td, ULONG *sptr,LONG flv, WORD color )
{
	ULONG lv;
	int temp;
	
	lv = ((ULONG)flv << 16) | (ULONG)flv;

	sptr += 1 + (color * 3);

	for(temp=3;temp-- > 0;)
	{
		if (lv > *sptr) {*sptr++ = 0;}
		else *sptr++ -= lv;
	}
}


void FadeRGB32( struct TData *td,
				register ULONG *sptr , register ULONG *dptr ,LONG flv,
				WORD from, WORD to)
{
	ULONG	lv;
	WORD	temp;
	WORD	*wptr = (WORD *)sptr;
	WORD	size;

	size = wptr[0];

	if (!to || (to >= size)) to = size - 1;
	
	*dptr++ = *sptr++; 	// the control.
	
	if (from) 
	{
		// Copy the lower area.
		CopyMem( sptr, dptr, (temp = from * 3)*4 );
		sptr += temp;
		dptr += temp;
	}
	
	temp = to - from + 1;
	if (flv == 0)
	{
		temp *= 3;
		CopyMem( sptr, dptr, temp * 4 );
		sptr += temp;
		dptr += temp;
		
		goto xit;
	}
	if (flv == 0xffff)
	{
		temp *= 3;
		MemSet( dptr,0, temp * 4 );	
		sptr += temp;
		dptr += temp;
		
		goto xit;
	}
	
	lv = ((ULONG)flv << 16) | (ULONG)flv;

	
	for(temp;temp-- > 0;)
	{
		if (lv > *sptr) {*dptr++ = 0;sptr++;}
		else *dptr++ = *sptr++ - lv;
	}
	
xit:
	if (to <= size-1)
	{
		temp = (size - to - 1)*3;
		CopyMem( sptr, dptr, temp*4 );
		sptr += temp;
		dptr += temp;
	}
	*dptr = 0;
}

	
