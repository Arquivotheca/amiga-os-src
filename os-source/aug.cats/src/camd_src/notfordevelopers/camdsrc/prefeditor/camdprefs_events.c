
/* Events source file */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <libraries/asl.h>

#include "CAMDPrefs.h"

#include <midi/camd.h>
#include <midi/mididefs.h>
#include <midi/camddevices.h>
#include <clib/camd_protos.h>
#include <pragmas/camd_pragmas.h>

BOOL EventFunc_OPEN(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_SAVEAS(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_ABOUT(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_QUIT(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_RESET(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_LASTSAVE(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_RESTORE(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_ICONS(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_DESELECT(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_SELECT(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_DRIVERSI(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_SYSEXQUE(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_MSGQUEUS(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_CANCEL(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_USE(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_SAVE(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_COMMENT(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_OUTPUTNA(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_INPUTNAM(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);
BOOL EventFunc_DRIVER(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);

TMOBJECTDATA tmobjectdata_OPEN = { EventFunc_OPEN };
TMOBJECTDATA tmobjectdata_SAVEAS = { EventFunc_SAVEAS };
TMOBJECTDATA tmobjectdata_ABOUT = { EventFunc_ABOUT };
TMOBJECTDATA tmobjectdata_QUIT = { EventFunc_QUIT };
TMOBJECTDATA tmobjectdata_RESET = { EventFunc_RESET };
TMOBJECTDATA tmobjectdata_LASTSAVE = { EventFunc_LASTSAVE };
TMOBJECTDATA tmobjectdata_RESTORE = { EventFunc_RESTORE };
TMOBJECTDATA tmobjectdata_ICONS = { EventFunc_ICONS };

TMOBJECTDATA tmobjectdata_DESELECT = { EventFunc_DESELECT };
TMOBJECTDATA tmobjectdata_SELECT = { EventFunc_SELECT };
TMOBJECTDATA tmobjectdata_DRIVERSI = { EventFunc_DRIVERSI };
TMOBJECTDATA tmobjectdata_SYSEXQUE = { EventFunc_SYSEXQUE };
TMOBJECTDATA tmobjectdata_MSGQUEUS = { EventFunc_MSGQUEUS };
TMOBJECTDATA tmobjectdata_CANCEL = { EventFunc_CANCEL };
TMOBJECTDATA tmobjectdata_USE = { EventFunc_USE };
TMOBJECTDATA tmobjectdata_SAVE = { EventFunc_SAVE };
TMOBJECTDATA tmobjectdata_COMMENT = { EventFunc_COMMENT };
TMOBJECTDATA tmobjectdata_OUTPUTNA = { EventFunc_OUTPUTNA };
TMOBJECTDATA tmobjectdata_INPUTNAM = { EventFunc_INPUTNAM };
TMOBJECTDATA tmobjectdata_DRIVER = { EventFunc_DRIVER };

/* ================ variables specific to this application ================= */

WORD					last_selected_avail=0,
						last_selected_active=0;

extern struct List		avail_list,
						active_list;

char					fr_filename[256];

/* ========================================================================= */

void DetachLists(struct TMData *TMData)
{
	update_from_strings( TMData );

	GT_SetGadgetAttrs(GadgetInfo_DRIVER.Gadget, WindowInfo_MIDIPREF.Window, NULL,
						GTLV_Labels, NULL,
						TAG_DONE );

	GT_SetGadgetAttrs(GadgetInfo_DRIVERSI.Gadget, WindowInfo_MIDIPREF.Window, NULL,
						GTLV_Labels, NULL,
						TAG_DONE );
}

void ReAttachLists(struct TMData *TMData)
{
	GT_SetGadgetAttrs(GadgetInfo_DRIVER.Gadget, WindowInfo_MIDIPREF.Window, NULL,
						GTLV_Labels, &avail_list,
						GTLV_Selected, last_selected_avail,
						TAG_DONE );

	GT_SetGadgetAttrs(GadgetInfo_DRIVERSI.Gadget, WindowInfo_MIDIPREF.Window, NULL,
						GTLV_Labels, &active_list,
						GTLV_Selected, last_selected_active,
						TAG_DONE );
}

void ResetLists(struct TMData *TMData)
{	struct Node *node;

	DetachLists(TMData);

	while (node = RemHead(&active_list)) AlphaInsertNode(&avail_list, node);

	last_selected_avail = 0;
	last_selected_active = 0;

	ReAttachLists(TMData);
	SetStrings(TMData, NULL);
}

BOOL item_checked(struct TMData *TMData, int menu, int item, int sub)
{	struct MenuItem		*mi = ItemAddress(WindowInfo_MIDIPREF.Menu, FULLMENUNUM( menu, item, sub ));

	return mi ? (mi->Flags & CHECKED) : FALSE;
}

BOOL EventFunc_OPEN(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {

	if (getfilename(TMData, TEXT_LOADPREF,
					fr_filename, sizeof fr_filename,
					WindowInfo_MIDIPREF.Window, FALSE))
	{
		DetachLists(TMData);

		GetMidiPrefs(fr_filename);
		last_selected_avail = last_selected_active = 0;

		ReAttachLists(TMData);
		SetStrings(TMData, NULL);
	}

  return(FALSE);
  }

BOOL EventFunc_SAVEAS(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {
	update_from_strings( TMData );

 	if (getfilename(TMData, TEXT_SAVEPREF,
					fr_filename, sizeof fr_filename,
					WindowInfo_MIDIPREF.Window, TRUE))
	{
		PutMidiPrefs(	fr_filename,
						item_checked(TMData, MENU_SETTINGS, ITEM_ICONS, NOSUB));
	}

  return(FALSE);
  }

BOOL EventFunc_ABOUT(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {
  TM_Request(NULL, (UBYTE *)TEXT_INFO, (UBYTE *)TEXT_ABOUT, (UBYTE *)TEXT_CONTINUE, NULL, NULL);
  return(FALSE);
  }

BOOL EventFunc_QUIT(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {
  return(TRUE);		/* just quit */
  }

BOOL EventFunc_RESET(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {
	 ResetLists(TMData);
  return(FALSE);
  }

BOOL EventFunc_LASTSAVE(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {

	DetachLists(TMData);

	GetMidiPrefs("envarc:sys/" MidiPrefsName );
	last_selected_avail = last_selected_active = 0;

	ReAttachLists(TMData);
	SetStrings(TMData, NULL);

  return(FALSE);
  }

BOOL EventFunc_RESTORE(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {
	DetachLists(TMData);

	GetMidiPrefs("env:sys/" MidiPrefsName );
	last_selected_avail = last_selected_active = 0;

	ReAttachLists(TMData);
	SetStrings(TMData, NULL);

  return(FALSE);
  }

BOOL EventFunc_ICONS(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {
  /* check the flag...setup flag to save icon... */
  return(FALSE);
  }

BOOL EventFunc_DESELECT(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {	struct Node *node;

  	if (node = SelectNode(&active_list, last_selected_active))
  	{
		DetachLists(TMData);

		Remove(node);
		last_selected_avail = AlphaInsertNode(&avail_list, node);
		last_selected_active = 0;

		ReAttachLists(TMData);
	}

		/* draw the data for the newly selected active node */

	SetStrings(TMData,
				(struct DriverNode *)SelectNode(&active_list, last_selected_active));

  return(FALSE);
  }

BOOL EventFunc_SELECT(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {	struct Node 	*node;

  	if (node = SelectNode(&avail_list, last_selected_avail))
  	{
		DetachLists(TMData);

		Remove(node);
		last_selected_active = AlphaInsertNode(&active_list, node);
		last_selected_avail = 0;

		ReAttachLists(TMData);

		SetStrings(TMData, (struct DriverNode *)node);
	}

  return(FALSE);
  }

BOOL EventFunc_DRIVERSI(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {	struct Node 	*node;

	update_from_strings( TMData );

  last_selected_active = imsg->Code;

		/* draw the data for the new node */

  if (node = SelectNode(&active_list, last_selected_active))
  {	SetStrings(TMData, (struct DriverNode *)node);
  }

  return(FALSE);
  }

BOOL EventFunc_CANCEL(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {
  return(TRUE);
  }

BOOL EventFunc_USE(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {
	update_from_strings( TMData );

	PutMidiPrefs("env:sys/" MidiPrefsName, FALSE);
	RethinkCAMD();
  return(TRUE);
  }

BOOL EventFunc_SAVE(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
  {
	update_from_strings( TMData );

	if (PutMidiPrefs("env:sys/" MidiPrefsName, FALSE ))
		PutMidiPrefs("envarc:sys/" MidiPrefsName, FALSE );
	RethinkCAMD();
  return(TRUE);
  }

BOOL EventFunc_COMMENT(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
{
    return(FALSE);
}

BOOL EventFunc_OUTPUTNA(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
{
    return(FALSE);
}

BOOL EventFunc_INPUTNAM(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
{
    return(FALSE);
}

BOOL EventFunc_MSGQUEUS(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
{
    return(FALSE);
}

BOOL EventFunc_SYSEXQUE(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
{
    return(FALSE);
}

BOOL EventFunc_DRIVER(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)
{
    last_selected_avail = imsg->Code;
    return(FALSE);
}
