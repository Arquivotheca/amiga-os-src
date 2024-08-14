/************************************************************************

  Module :  Postscript "PostScript Monitor"		© Commodore-Amiga
            Written by P. Jones (started March 1991)

  Purpose:  This file contains C-entries for the Debug Package


  Conventions:


  NOTES: 

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <m68881.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/graphics.h>

#include "pstypes.h"
#include "stream.h"
#include "memory.h"
#include "objects.h"

#include "misc.h"
#include "stack.h"
#include "gstate.h"
#include "context.h"

#include "scanner.h"

/**************************************************************************/

#define DEBUG_BIT	0x02


/* Debug Structures and variables */

struct IntuiMessage *msg;
struct NewWindow nw =
{
    0,0,
    680,250,
    -1,-1,
	MOUSEBUTTONS,
	SMART_REFRESH|GIMMEZEROZERO|WINDOWDRAG|WINDOWDEPTH,
	NULL,
	NULL,
"                                                          PostScript Monitor",
	NULL,    
	NULL,
	10,10,-1,-1,
	WBENCHSCREEN
};

SHORT Vector1[] = {
	0,12,
	0,0,
	110,0
};
SHORT Vector2[] = {
	110,0,
	110,12,
	0,12
};
/* LVA **!!
struct Border BorderTrue2 ={
	-1,-1,
	2,0,JAM2,
	3,
	Vector2,
	NULL
};

struct Border BorderTrue1 ={
	-1,-1,
	1,0,JAM2,
	3,
	Vector1,
	&BorderTrue2
};

struct Gadget Gad1 = {
	NULL,
	250,114,
	50,50,
	(GFLG_GADGHBOX),
	(GACT_TOGGLESELECT),
	BOOLGADGET,
	(APTR)&BorderTrue1,
	NULL,
	NULL,
	NULL,
	NULL,
	1,
	NULL
};
*/	


char Blank[] = "      --       ";
char Dict[]  = "  DICTIONARY   ";
char Mark[]  = "     MARK      ";
char Arry[]  = "     ARRAY     ";
char PArry[] = "  PACKED ARRAY ";
char Proc[]  = "   PROCEDURE   ";
char Unk[]   = "    UNKNOWN    ";
char Strm[]  = "     FILE      ";
char Oprt[]  = "    OPERATOR   ";
char Tru[]   = "     TRUE      ";
char Fal[]   = "     FALSE     ";
char Nul[]   = "     NULL      ";
char Sav[]   = "     SAVE      ";
char *GString[11];

#define DEBUG_NORMAL	1L
#define DEBUG_STEP		2L


/**************************************************************************/



int InitDebug(DPSContext,int);
void ClearDisplay(DPSContext );
void RefreshScreen(DPSContext);
void UpdateStacks(DPSContext,int);
void Monitor(DPSContext);
void Box(struct RastPort *,int,int,int,int,int);

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;


/**********************************************************************/

int InitDebug(DPSContext ctxt,int mode) {
	int i;
	struct RastPort *rp;
	
	if(ctxt->wind==NULL) {
		if((ctxt->wind = (struct Window *)OpenWindow(&nw))==NULL||
			!(IntuitionBase)) {
			return(TRUE) ;
		}
	rp = ctxt->wind->RPort;
		ctxt->Debg.OperSp = OPERSP(ctxt);
		ctxt->Debg.ExecSp = EXECSP(ctxt);
		ctxt->Debg.DictSp = DICTSP(ctxt);
		ctxt->Debg.GsavSp = GSAVSP(ctxt);
		ctxt->Debg.ExecMode = mode;
		ctxt->Debg.Del = 0;
		ctxt->Debg.Onum = -1;
		ctxt->Debg.Enum = -1;
		ctxt->Debg.Dnum = -1;
		ctxt->Debg.Gnum = -1;

		for(i=0;i<11;i++) {
			GString[i] = AllocMem(16,MEMF_CLEAR);
		}
	

		UpdateStacks(ctxt,OPER);
		UpdateStacks(ctxt,EXEC);
		UpdateStacks(ctxt,DICT);
		UpdateStacks(ctxt,GSAVE);
	}

	return FALSE;
}

/**********************************************************************/

void ClearDisplay(DPSContext ctxt) {
	int i;

	if(ctxt->wind!=NULL) {
		for(i=0;i<11;i++) {
			FreeMem(GString[i],16);
		}
		while((msg=(struct IntuiMessage *)GetMsg(ctxt->wind->UserPort))!=NULL) {
			ReplyMsg((struct Message *)msg);
		}
		CloseWindow(ctxt->wind);
		ctxt->wind = NULL;
		ctxt->space.state &= ~DEBUG_BIT;

	}
}
#define DOFF 82

/**********************************************************************/

void RefreshScreen(DPSContext ctxt) {
	long x,y;
	struct RastPort *rp;
	rp = ctxt->wind->RPort;
	SetAPen(rp,1);
	for(x=DOFF;x<=(496+DOFF);x+=124) {
		Move(rp,x,20); Draw(rp,x,120);
	}
	for(y=20;y<130;y+=10) {
		Move(rp,DOFF,y); Draw(rp,DOFF+496,y);
	}
	SetAPen(rp,2);
	Move(rp,DOFF,120); Draw(rp,DOFF,20); Draw(rp,DOFF+496,20);

	SetAPen(rp,2);

	Move(rp,DOFF+26,12);
	Text(rp,"Execution",9);
	Move(rp,DOFF+158,12);
	Text(rp,"Operand",7);
	Move(rp,DOFF+270,12);
	Text(rp,"Dictionary",10);
	Move(rp,DOFF+404,12);
	Text(rp,"Graphics",8);
	Box(rp,DOFF,2,496,14,FALSE);

	Box(rp,DOFF,123,496,12,TRUE);

	
	if(ctxt->Debg.ExecMode == DEBUG_STEP) {
		Box(rp,DOFF,184,110,12,TRUE);
		Box(rp,DOFF,204,110,12,FALSE);
	} else {
		Box(rp,DOFF,184,110,12,FALSE);
		Box(rp,DOFF,204,110,12,TRUE);
	}
	
	Box(rp,DOFF,164,110,12,FALSE);
	SetAPen(rp,2);
	Move(rp,DOFF+12,173); Text(rp,"   ACTIVE  ",11);
	Move(rp,DOFF+12,193); Text(rp,"SINGLE STEP",11);
	Move(rp,DOFF+12,213); Text(rp,"NORMAL EXEC",11);
	
	Box(rp,DOFF+130,204,15,14,FALSE); Box(rp,DOFF+130+15+1,204,25,14,FALSE);
	Box(rp,DOFF+130+15+1+25+1,204,15,14,FALSE);


}

/**********************************************************************/

void Box(struct RastPort *rp,int x,int y,int w,int h,int flg) {
	
	if(flg==TRUE) SetAPen(rp,1); else SetAPen(rp,2);
	Move(rp,x,y+h);
	Draw(rp,x,y); Draw(rp,x+w,y);
	if(flg==TRUE) SetAPen(rp,2); else SetAPen(rp,1);
	Draw(rp,x+w,y+h); Draw(rp,x,y+h);
}

/**********************************************************************/

void Monitor(DPSContext ctxt) {
	SHORT mousex,mousey;
	long class,code;
	struct RastPort *rp;

	rp = ctxt->wind->RPort;
	Box(rp,DOFF,164,110,12,TRUE);
	
/*
	if(ctxt->Debg.OperSp != OPERSP(ctxt)) {
		UpdateStacks(ctxt,OPER);
		ctxt->Debg.OperSp = OPERSP(ctxt);
	}
*/
		UpdateStacks(ctxt,OPER);
/*
	if(ctxt->Debg.ExecSp != EXECSP(ctxt)) {
		UpdateStacks(ctxt,EXEC);
		ctxt->Debg.ExecSp = EXECSP(ctxt);
	}
*/
	UpdateStacks(ctxt,EXEC);
	if(ctxt->Debg.DictSp != DICTSP(ctxt)) {
		UpdateStacks(ctxt,DICT);
		ctxt->Debg.DictSp = DICTSP(ctxt);
	}
/*
	if(ctxt->Debg.GsavSp != GSAVSP(ctxt)) {
		UpdateStacks(ctxt,GSAVE);
		ctxt->Debg.GsavSp = GSAVSP(ctxt);
	}
*/
	while(TRUE) {
		if((msg = (struct IntuiMessage *)GetMsg(ctxt->wind->UserPort))!=NULL) {
			class = msg->Class;
			code = msg->Code;
			mousex =  msg->MouseX; mousey = msg->MouseY;
			ReplyMsg((struct Message *)msg);
			if(code & IECODE_UP_PREFIX) {
				mousey -= 20; 
				if(mousex>=DOFF && mousex<=(DOFF+110)) {
					if(mousey>=184 && mousey<=(184+12)) {
						Box(rp,DOFF,184,110,12,TRUE);
						Box(rp,DOFF,204,110,12,FALSE);
						ctxt->Debg.ExecMode = DEBUG_STEP;
						Box(rp,DOFF,164,110,12,FALSE);
						return ;
					} else if(mousey>=204 && mousey<=(204+12)) {
						Box(rp,DOFF,184,110,12,FALSE);
						Box(rp,DOFF,204,110,12,TRUE);
						ctxt->Debg.ExecMode = DEBUG_NORMAL;
						if(ctxt->Debg.Del) Delay(ctxt->Debg.Del);
						Box(rp,DOFF,164,110,12,FALSE);
						return;
					}
				}			
			}
		} else if(ctxt->Debg.ExecMode == DEBUG_NORMAL) {
				Box(rp,DOFF,184,110,12,FALSE);
				Box(rp,DOFF,204,110,12,TRUE);
				if(ctxt->Debg.Del) Delay(ctxt->Debg.Del);
				Box(rp,DOFF,164,110,12,FALSE);
				return ;
		}
	}
	Box(rp,DOFF,164,110,12,FALSE);
	return ;
}

/**********************************************************************/

void UpdateStacks(DPSContext ctxt,int stk) {
	
	ps_obj *sp;
	long i,stop_at,startx,y,loop;
	struct RastPort *rp;

	rp = ctxt->wind->RPort;
	switch(stk) {

	case OPER :
		sp = OPERSP(ctxt);
		stop_at = NUMOPER(ctxt);
		break;
	case EXEC :	
		sp = EXECSP(ctxt);
		stop_at = NUMEXEC(ctxt);
		break;

	case DICT :
		sp = DICTSP(ctxt);
		stop_at = NUMDICT(ctxt);
		break;

	case GSAVE :
		sp = GSAVSP(ctxt);
		stop_at = NUMGSAV(ctxt);
		break;
	default : return ;
	}
	if(stop_at >10) stop_at=10;
	sp +=stop_at;

	for(i=0;i<10;i++) {
		sp--;
		if(i>=stop_at) {
			strcpy(GString[i],&Blank[0]);
			*(GString[i]+15) = 2;
		} else {
			switch(sp->type&TYPE_MASK) {
			case TYPE_INT :
				sprintf(GString[i],"%-15d",sp->obj.intval);
				*(GString[i]+15) = 1;
				break;
			case TYPE_REAL :
				sprintf(GString[i],"%-15e",(double)sp->obj.realval);
				*(GString[i]+15) = 1;
				break;
			case TYPE_BOOL :
				if(sp->obj.boolval == TRUE) {
					strcpy(GString[i],&Tru[0]);
				} else strcpy(GString[i],&Fal[0]);
				*(GString[i]+15) = 3;
				break;
			case TYPE_NAME :
				for(loop=0;loop<15;loop++) {
					if((*(GString[i]+loop) = sp->obj.nameval[loop])=='\0') break;
				}
				for( ;loop<15;loop++) *(GString[i]+loop) = ' ';
				*(GString[i]+15) = 1;
				break;
			case TYPE_STRING :
				*(GString[i]+0) = '(';
				if(sp->len<14) {
					for(loop=1;loop<=sp->len;loop++) {
						*(GString[i]+loop) = sp->obj.stringval[loop-1];
					}
				} else {
					for(loop=1;loop<14;loop++) {
						*(GString[i]+loop) = sp->obj.stringval[loop-1];
					}
				}
				*(GString[i]+loop) = ')'; loop++;

				for( ;loop<15;loop++) *(GString[i]+loop) = ' ';
				*(GString[i]+15) = 3;
				break;
			case TYPE_ARRAY :
				strcpy(GString[i],&Arry[0]);
				*(GString[i]+15) = 3;
				break;
			case TYPE_PACKED :
				strcpy(GString[i],&PArry[0]);
				*(GString[i]+15) = 3;
				break;
			case TYPE_DICT :
				strcpy(GString[i],&Dict[0]);
				*(GString[i]+15) = 3;
				break;
			case TYPE_OPERATOR :
				if(sp->len!=0) {
					sprintf(GString[i],"%-15d",sp->len);
					*(GString[i]+15) = 2;
				} else {
					strcpy(GString[i],&Oprt[0]);
					*(GString[i]+15) = 3;
				}
				break;
			case TYPE_FILE :
				strcpy(GString[i],&Strm[0]);
				*(GString[i]+15) = 3;
				break;
			case TYPE_MARK :
				strcpy(GString[i],&Mark[0]);
				*(GString[i]+15) = 3;
				break;
			case TYPE_SAVE :
				strcpy(GString[i],&Sav[0]);
				*(GString[i]+15) = 3;
				break;
			case TYPE_NULL :
				strcpy(GString[i],&Nul[0]);
				*(GString[i]+15) = 3;
				break;
			default :
				strcpy(GString[i],&Unk[0]);
				*(GString[i]+15) = 3;
				break;
			}
		}
	}
	switch(stk) {
	case EXEC :
		startx = DOFF + 4;
		if(ctxt->Debg.Enum != NUMEXEC(ctxt)) {
			ctxt->Debg.Enum = NUMEXEC(ctxt);
			sprintf(GString[10],"%04d",ctxt->Debg.Enum);
			SetAPen(rp,2);
			Move(rp,DOFF+(124*0)+44,132); Text(rp,GString[10],4);
		}
		break;
	case OPER:
		startx = DOFF + 4 + (124*1);
		if(ctxt->Debg.Onum != NUMOPER(ctxt)) {
			ctxt->Debg.Onum = NUMOPER(ctxt);
			sprintf(GString[10],"%04d",ctxt->Debg.Onum);
			SetAPen(rp,2);
			Move(rp,DOFF+(124*1)+44,132); Text(rp,GString[10],4);
		}
		break;
	case DICT:
		startx = DOFF + 4 + (124*2);
		if(ctxt->Debg.Dnum != NUMDICT(ctxt)) {
			ctxt->Debg.Dnum = NUMDICT(ctxt);
			sprintf(GString[10],"%04d",ctxt->Debg.Dnum);
			SetAPen(rp,2);
			Move(rp,DOFF+(124*2)+44,132); Text(rp,GString[10],4);
		}
		break;
	case GSAVE:
		startx = DOFF + 4 + (124*3);
		if(ctxt->Debg.Gnum != NUMGSAV(ctxt)) {
			ctxt->Debg.Gnum = NUMGSAV(ctxt);
			sprintf(GString[10],"%04d",ctxt->Debg.Gnum);
			SetAPen(rp,2);
			Move(rp,DOFF+(124*3)+44,132); Text(rp,GString[10],4);
		}
		break;
	default : break;
	}
	y = 28;  
	for(i=0;i<10;i++) {
		Move(rp,startx,y);
		SetAPen(rp,(long)*(GString[i]+15));
		Text(rp,GString[i],15);
		y+=10;
	}
	return;

}

