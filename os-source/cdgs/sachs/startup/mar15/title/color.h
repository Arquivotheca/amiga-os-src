/* :ts=4
*
*	color.h
*
*	William A. Ware						D303
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright 1993, Silent Software, Incorporated.							*
*   All Rights Reserved.													*
****************************************************************************/

#ifndef COLOR_H
#define COLOR_H

#include <exec/nodes.h>


struct CNode
{
	struct MinNode		Node;
	struct CNode		*NextCNode;
	struct MinList		*ReturnList;
	ULONG				*Table;
	BOOL				(*Func)(struct TData *,struct CNode *,...);
	
	WORD				Size;
	WORD				Bottom;
	
	WORD				From,To;
	
	LONG				Speed,Counter;

	ULONG				Flags;

	LONG				CycleWait;
	UBYTE				Type;
	UBYTE				Pad1;

	WORD				Pad2;
};

#define		CNF_FINISHED	(1<<1)
#define		CNF_CYCLE		(1<<2)

enum
{
	CNTYPE_MOVEUP,
	CNTYPE_ONECOLOR,
	CNTYPE_SPRITECOLOR,
	CNTYPE_MOVEDOWN
};



//---------------------------------------------------------------------------
void FreeCNodeData( struct TData *td,struct CNode *cn );
BOOL CutCNode(struct TData *td,struct CNode *cn, struct BMInfo *bmi,ULONG *table, 
				WORD start, WORD stop );
BOOL GetCNodeData( struct TData *td,struct CNode *cn, UBYTE *data, WORD start,WORD stop );


void StartCNode(struct TData *td,struct MinList *,struct CNode *, WORD, WORD, LONG,LONG,UWORD );
BOOL RunCNodes( struct TData *td,struct MinList *, ULONG *, LONG );


void FadeRGB32( struct TData *td,ULONG *,ULONG *, LONG, WORD, WORD );

void SwitchCNode( struct TData *td, struct CNode *cn );

#endif //COLOR_H