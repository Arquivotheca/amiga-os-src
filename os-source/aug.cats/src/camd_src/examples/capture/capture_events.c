
/* Events source file */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <midi/camd.h>
#include <string.h>

#include "capture.h"
#include "/camdlistreq/listreq.h"
#include "/camdlistreq/camdlists.h"

#include <clib/gadtools_protos.h>
#include <pragmas/gadtools_pragmas.h>

/* Prototypes */

BOOL EventFunc_CLEAR(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_LOAD(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_SAVE(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_ABOUT(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_QUIT(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_SETOUTPU(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_SETINPUT(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_SEND(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);

/* NewMenu UserData structures */

TMOBJECTDATA tmobjectdata_CLEAR = { EventFunc_CLEAR };
TMOBJECTDATA tmobjectdata_LOAD = { EventFunc_LOAD };
TMOBJECTDATA tmobjectdata_SAVE = { EventFunc_SAVE };
TMOBJECTDATA tmobjectdata_ABOUT = { EventFunc_ABOUT };
TMOBJECTDATA tmobjectdata_QUIT = { EventFunc_QUIT };
TMOBJECTDATA tmobjectdata_SETOUTPU = { EventFunc_SETOUTPU };
TMOBJECTDATA tmobjectdata_SETINPUT = { EventFunc_SETINPUT };

/* NewGadget UserData structures */

TMOBJECTDATA tmobjectdata_SEND = { EventFunc_SEND };

/* Menu event functions */

static UBYTE 			filename[ 256 ];
extern struct MidiLink	*in_link,
						*out_link;

extern LONG				buffer_size;
extern struct ListRequester	*lreq;

BOOL SetMidiLinkAttrs(struct MidiLink *mi, Tag tag, ...);

BOOL EventFunc_CLEAR(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
{
	ClearBuffer();
	GT_SetGadgetAttrs(
		GadgetInfo_BYTESINB.Gadget,
		WindowInfo_CAPTUREM.Window,
		NULL,
		GTNM_Number, buffer_size,
		TAG_END );

	return(FALSE);
}

BOOL EventFunc_ABOUT(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
{
	TM_Request( imsg->IDCMPWindow,
				"Program Information",
				"MIDI Capture ©1993 Sylvan technical Arts\nWritten by Talin\n\nThis program captures all MIDI events into\na buffer (without timing information) and\nallows that data to be manipulated.",
				"Continue", NULL, NULL );
	return(FALSE);
}

BOOL EventFunc_QUIT(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {
  return(TRUE);
  }

BOOL EventFunc_SETOUTPU(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
{
	char				*oldname;
	UBYTE 				name[ 32 ];

	GetMidiLinkAttrs( out_link, MLINK_Location, &oldname, TAG_DONE );
	if (oldname) strncpy( name, oldname, sizeof name - 1);
	else name[0] = '\0';

	if (SelectCluster(	lreq, name, sizeof name,
						LISTREQ_Window, 		imsg->IDCMPWindow,
						LISTREQ_TitleText,		"Select Output Link",
						LISTREQ_PositiveText,	"Select",
						LISTREQ_NegativeText,	"Cancel",
						TAG_END ))
	{	SetMidiLinkAttrs( out_link, MLINK_Location, name, TAG_END );
	}
	return(FALSE);
}

BOOL EventFunc_SETINPUT(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
{

	char				*oldname;
	UBYTE 				name[ 32 ];

	GetMidiLinkAttrs( in_link, MLINK_Location, &oldname, TAG_DONE );
	if (oldname) strncpy( name, oldname, sizeof name - 1);
	else name[0] = '\0';

	if (SelectCluster(	lreq, name, sizeof name,
						LISTREQ_Window, 		imsg->IDCMPWindow,
						LISTREQ_TitleText,		"Select Input Link",
						LISTREQ_PositiveText,	"Select",
						LISTREQ_NegativeText,	"Cancel",
						TAG_END ))
	{	SetMidiLinkAttrs( in_link, MLINK_Location, name, TAG_END );
	}
	return(FALSE);
}

BOOL EventFunc_LOAD(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
{
	if (getfilename( TMData, "Load Buffer", filename, sizeof filename, imsg->IDCMPWindow, FALSE ))
	{	LoadMIDI( TMData, filename );

		GT_SetGadgetAttrs(
			GadgetInfo_BYTESINB.Gadget,
			WindowInfo_CAPTUREM.Window,
			NULL,
			GTNM_Number, buffer_size,
			TAG_END );
	}
	return(FALSE);
}

BOOL EventFunc_SAVE(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
{
	if (getfilename( TMData, "Save Buffer", filename, sizeof filename, imsg->IDCMPWindow, TRUE ))
	{	SaveMIDI( TMData, filename );
	}
	return(FALSE);
}

/* Gadget event functions */

BOOL EventFunc_SEND(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
{
	SendMIDI( TMData );
	return(FALSE);
}
