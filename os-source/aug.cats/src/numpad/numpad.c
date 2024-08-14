   /***********************************************************************
   *                                                                      *
   *                            COPYRIGHTS                                *
   *                                                                      *
   *   UNLESS OTHERWISE NOTED, ALL FILES ARE                              *
   *   Copyright (c) 1990  Commodore-Amiga, Inc.  All Rights Reserved.    *
   *                                                                      *
   ***********************************************************************/

#include "app.h"
#include <intuition/intuitionbase.h>

extern char **ttypes;

#define KCNT	7
#define CODE_LSHIFT	0x60
#define CODE_RSHIFT	0x61
#define CODE_CONTROL	0x63
#define CODE_LALT	0x64
#define CODE_RALT	0x65
#define CODE_LAMIGA	0x66
#define CODE_RAMIGA	0x67


/* real row 1 numeric pad */
#define NOPAREN_KEY 0x5A
#define NCPAREN_KEY 0x5B
#define NSLASH_KEY  0x5C
#define NASTER_KEY  0x5D

/* real row 2 numeric pad */
#define N7_KEY 0x3D
#define N8_KEY 0x3E
#define N9_KEY 0x3F
#define NMINUS_KEY  0x4A

/* real row 3 numeric pad */
#define N4_KEY 0x2D
#define N5_KEY 0x2E
#define N6_KEY 0x2F
#define NPLUS_KEY   0x5E

/* real row 4 numeric pad */
#define N1_KEY 0x1D
#define N2_KEY 0x1E
#define N3_KEY 0x1F
#define NENTER_KEY  0x43

/* real row 5 numeric pad */
#define N0_KEY 0x0F
#define NPERIOD_KEY 0x3C

/*
8 9 0 -
i o p [
k l ; '
, . / RETURN
SPACE \
*/

#define NKCNT 18

UWORD numkeys[NKCNT] = {
NOPAREN_KEY,	NCPAREN_KEY,	NSLASH_KEY,	NASTER_KEY,
N7_KEY,		N8_KEY,		N9_KEY,		NMINUS_KEY,
N4_KEY,		N5_KEY,		N6_KEY,		NPLUS_KEY,
N1_KEY,		N2_KEY,		N3_KEY,		NENTER_KEY,
N0_KEY,		NPERIOD_KEY };
 
UWORD fakenumkeys[NKCNT] = {
0x08,		0x09,		0x0A,		0x0B,
0x17,		0x18,		0x19,		0x1A,
0x27,		0x28,		0x29,		0x2A,
0x38,		0x39,		0x3A,		0x44,
0x40,		0x0D };


/* shift is OK - we'll leave shift alone */
UWORD qmask =   IEQUALIFIER_LALT     | IEQUALIFIER_RALT |
		IEQUALIFIER_LCOMMAND | IEQUALIFIER_RCOMMAND |
		IEQUALIFIER_CONTROL;

UWORD newqual = 0;

/* Means emulate numeric pad */
UWORD umask = IEQUALIFIER_LALT | IEQUALIFIER_CONTROL;


__saveds VOID AppHandler(struct CxMsg *cxm, CxObj *co)
{
struct	InputEvent	*ie;
UWORD	kcode, kqual;
int k;
BOOL Done;

    ie=(struct InputEvent *)CxMsgData(cxm);

    if (ie->ie_Class==IECLASS_RAWKEY)
	{
	kcode = ie->ie_Code & (~IECODE_UP_PREFIX);
	kqual = ie->ie_Qualifier & qmask;

	if((kqual & umask) == umask)
	    {
	    for(k=Done=0; (k < NKCNT) && (!Done); k++)
		{
		if(fakenumkeys[k] == kcode)
		    {
		    Done = TRUE;
		    ie->ie_Code = (ie->ie_Code & IECODE_UP_PREFIX) | numkeys[k];
		    ie->ie_Qualifier &= (~umask);
		    ie->ie_Qualifier |= IEQUALIFIER_NUMERICPAD;
		    }
		}
	    }
	}	    
}

static long oldpri;
struct InputXpression ix = { 0 };

#define SHIFTS (IEQUALIFIER_RSHIFT|IEQUALIFIER_LSHIFT)
#define ALTS   (IEQUALIFIER_RALT|IEQUALIFIER_LALT)

BOOL setupApp(VOID)
{
CxObj	*objectlist;
/*
UBYTE *qualname;

    if(qualname = (UBYTE *)ArgString(ttypes,"QUALIFIER",""))
	{
	if(!(stricmp(qualname,"lalt"))) umask |= IEQUALIFIER_LALT;
	if(!(stricmp(qualname,"ralt")))  umask |= IEQUALIFIER_RALT;
	if(!(stricmp(qualname,"control")))  umask |= IEQUALIFIER_CONTROL;
	}
*/
    oldpri=SetTaskPri(FindTask(NULL),21);

    objectlist=CxCustom(AppHandler,NULL);

    if (CxObjError(objectlist))
    {
	DeleteCxObjAll(objectlist);
	objectlist=NULL;
    }
    else AttachCxObj(broker,objectlist);

    return(objectlist!=NULL);
}

VOID closeApp(VOID)
{
	SetTaskPri(FindTask(NULL),oldpri);
}

VOID MyHandleCustomSignal(VOID)
{

}
