
#include <exec/types.h>
#include <intuition/intuition.h>
#include <dos/dos.h>
#include <string.h>
#include <stdlib.h>
#include <libraries/asl.h>

#include "CAMDPrefs.h"
#include "CAMDPrefs_rev.h"

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>
#include <clib/asl_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include <midi/camd.h>
#include <midi/mididefs.h>
#include <midi/camddevices.h>
#include <clib/camd_protos.h>
#include <pragmas/camd_pragmas.h>

#include <prefs/prefhdr.h>

char vers[] = VERSTAG;
struct Library *IntuitionBase;
struct Library *GadToolsBase;
struct Library *UtilityBase;
struct Library *IFFParseBase;
struct Library *AslBase;
struct Library *CamdBase;
struct Library *IconBase;
struct IntuiText BodyText = {0,1,JAM2,20,8,NULL,(UBYTE *)TEXT_NORELEASE2,NULL};
struct IntuiText NegText  = {0,1,JAM2, 6,4,NULL,(UBYTE *)TEXT_OK,NULL};

#ifdef _DCC
VOID wbmain(VOID *wbmsg)
  {
  main(0, wbmsg);
  }
#endif

/* ================ variables specific to this application ================= */

struct List				avail_list,
						active_list;

struct DriverNode		*selected_driver;

extern WORD				last_selected_avail,
						last_selected_active;

/* ========================================================================= */

VOID main(int argc, char **argv)
  {
  struct TMData *TMData;
  ULONG error;

  NewList(&avail_list);
  NewList(&active_list);

  if(!(IntuitionBase = OpenLibrary((UBYTE *)"intuition.library", 37L)))
    {
    if(IntuitionBase = OpenLibrary((UBYTE *)"intuition.library", 0L))
      {
      AutoRequest(NULL, &BodyText, NULL, &NegText, 0, 0, 320, 80);
      CloseLibrary(IntuitionBase);
      }
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(GadToolsBase = OpenLibrary((UBYTE *)"gadtools.library", 37L)))
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, "gadtools.library V37");
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(UtilityBase = OpenLibrary((UBYTE *)"utility.library", 37L)))
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, "utility.library V37");
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(AslBase = OpenLibrary((UBYTE *)"asl.library", 37L)))
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, "asl.library V37");
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(CamdBase = OpenLibrary((UBYTE *)"camd.library", 0L)))
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, "camd.library");
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(IFFParseBase = OpenLibrary((UBYTE *)"iffparse.library", 0L)))
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, "iffparse.library");
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(IconBase = OpenLibrary((UBYTE *)"icon.library", 37L)))
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, "icon.library V37");
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(TMData = TM_Open(&error)))
    {
    switch(error)
      {
      case TMERR_MEMORY:
        TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOMEMORY, (UBYTE *)TEXT_ABORT, NULL, NULL);
        break;
      case TMERR_MSGPORT:
        TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOMSGPORT, (UBYTE *)TEXT_ABORT, NULL, NULL);
        break;
      }
    cleanexit(NULL, RETURN_FAIL);
    }

	scan_devices(TMData);
	GetMidiPrefs("env:sys/" MidiPrefsName );

  if(!(TMData->FileRequester = AllocAslRequestTags(ASL_FileRequest,
		ASL_Dir, "SYS:Prefs/presets",
  		TAG_DONE)))
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOFILEREQ, (UBYTE *)TEXT_ABORT, NULL, NULL);
    cleanexit(TMData, RETURN_FAIL);
    }

  if(!(OpenScreen_Workbench(TMData)))
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOSCREEN, (UBYTE *)TEXT_ABORT, NULL, NULL);
    cleanexit(TMData, RETURN_FAIL);
    }

  if(!(OpenWindow_MIDIPREF(TMData)))
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOWINDOW, (UBYTE *)TEXT_ABORT, NULL, NULL);
    cleanexit(TMData, RETURN_FAIL);
    }

	GT_SetGadgetAttrs(GadgetInfo_DRIVER.Gadget, WindowInfo_MIDIPREF.Window, NULL,
						GTLV_Labels, &avail_list,
						GTLV_Selected, 0,
						TAG_DONE );

	GT_SetGadgetAttrs(GadgetInfo_DRIVERSI.Gadget, WindowInfo_MIDIPREF.Window, NULL,
						GTLV_Labels, &active_list,
						GTLV_Selected, 0,
						TAG_DONE );

	SetStrings(TMData, (struct DriverNode *)SelectNode(&active_list, last_selected_active));

  TM_EventLoop(TMData);

  cleanexit(TMData, RETURN_OK);
  }

VOID cleanexit(struct TMData *TMData, int returnvalue)
  {

	free_driver_nodes(&avail_list);
	free_driver_nodes(&active_list);

  if(TMData)
    {
    CloseWindow_MIDIPREF(TMData);
    CloseScreen_Workbench(TMData);
    if(TMData->FileRequester) FreeAslRequest(TMData->FileRequester);
    TM_Close(TMData);
    }

  if(IconBase)      CloseLibrary(IconBase);
  if(IFFParseBase)  CloseLibrary(IFFParseBase);
  if(CamdBase)      CloseLibrary(CamdBase);
  if(AslBase)       CloseLibrary(AslBase);
  if(UtilityBase)   CloseLibrary(UtilityBase);
  if(GadToolsBase)  CloseLibrary(GadToolsBase);
  if(IntuitionBase) CloseLibrary(IntuitionBase);

  exit(returnvalue);
  }

UBYTE *getfilename(struct TMData *TMData, UBYTE *title, UBYTE *buffer, ULONG bufsize, struct Window *window, BOOL saveflag)
  {
  ULONG funcflags;

  if(saveflag)
    funcflags = FILF_SAVE;
  else
    funcflags = 0;

  if(buffer && TMData)
    {
    if(AslRequestTags((APTR) TMData->FileRequester,
                      ASL_Hail, title,
                      ASL_Window, window,
                      ASL_FuncFlags, funcflags,
                      TAG_DONE))
      {
      strcpy(buffer, (char *)TMData->FileRequester->rf_Dir);
      if(AddPart(buffer, (char *)TMData->FileRequester->rf_File, bufsize))
        {
        return(buffer);
        }
      }
    }

  return(NULL);
  }

BOOL Window_MIDIPREF_MENUPICK(struct TMData *TMData, struct IntuiMessage *imsg)
  {
  UWORD menucode;
  struct MenuItem *menuitem;
  TMOBJECTDATA *tmobjectdata;
  BOOL (*eventfunc)(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);

  menucode = imsg->Code;
  while(menucode != MENUNULL)
    {
    menuitem = ItemAddress(WindowInfo_MIDIPREF.Menu, menucode);
    tmobjectdata = (TMOBJECTDATA *)(GTMENUITEM_USERDATA(menuitem));
    eventfunc = tmobjectdata->EventFunc;

    if(eventfunc)
      {
      if((*eventfunc)(TMData, imsg, tmobjectdata)) return(TRUE);
      }

    menucode = menuitem->NextSelect;
    }
  return(FALSE);
  }

BOOL Window_MIDIPREF_GADGETDOWN(struct TMData *TMData, struct IntuiMessage *imsg)
  {
  struct Gadget *gadget;
  TMOBJECTDATA *tmobjectdata;
  BOOL (*eventfunc)(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);

  gadget = (struct Gadget *)imsg->IAddress;
  tmobjectdata = (TMOBJECTDATA *)(gadget->UserData);
  eventfunc = tmobjectdata->EventFunc;

  if(eventfunc)
    {
    return((*eventfunc)(TMData, imsg, tmobjectdata));
    }
  return(FALSE);
  }

BOOL Window_MIDIPREF_GADGETUP(struct TMData *TMData, struct IntuiMessage *imsg)
  {
  struct Gadget *gadget;
  TMOBJECTDATA *tmobjectdata;
  BOOL (*eventfunc)(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);

  gadget = (struct Gadget *)imsg->IAddress;
  tmobjectdata = (TMOBJECTDATA *)(gadget->UserData);
  eventfunc = tmobjectdata->EventFunc;

  if(eventfunc)
    {
    return((*eventfunc)(TMData, imsg, tmobjectdata));
    }
  return(FALSE);
  }

/* ================= routines specific to this application ================= */

	/* REM: Move to new library... */

WORD AlphaInsertNode(struct List *l, struct Node *n)
{	struct Node			*search;
	WORD				pos = 0;

	for (	search=l->lh_Head ;				/* insert alphabetically		*/
			search->ln_Succ ;
			search=search->ln_Succ, pos++ )
	{
		if (n->ln_Name[0] == (UBYTE)'«') break;	/* special for this app */
		if (search->ln_Name[0] == (UBYTE)'«') continue; /* special for this app */

		if (Stricmp(search->ln_Name,n->ln_Name) >= 0) break;
	}
	Insert(l,n,search->ln_Pred);
	return pos;
}

void free_driver_nodes(struct List *list)
{	struct DriverNode *dn;

	while (dn = (struct DriverNode *)RemHead(list)) FreeMem(dn, sizeof *dn);
}

struct DriverNode *alloc_driver_node(void)
{	struct DriverNode	*dn;

	if (!(dn = AllocMem(sizeof *dn, MEMF_CLEAR)))
	{   TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOMEMORY, (UBYTE *)TEXT_ABORT, NULL, NULL);
		return NULL;
	}

	dn->node.ln_Name = dn->listname;

	dn->unit.XmitQueueSize = MinXmitQueueSize;
	dn->unit.RecvQueueSize = MinRecvQueueSize;
	dn->unit.Flags = FALSE;

	return dn;
}

void TM_DosRequest(struct TMData *TMData, char *msg)
{   char				fault_buf[128];

	if (!Fault(IoErr(), "", fault_buf, sizeof fault_buf))
 	{	strcpy(fault_buf,"(Exact error unknown)");
 	}

    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)msg, (UBYTE *)TEXT_ABORT, NULL, fault_buf);
}

void scan_devices(struct TMData *TMData)
{ BPTR					devs_lock = NULL;
  struct FileInfoBlock	*fib = NULL;
  WORD					result = FALSE;
  struct DriverNode		*dn;

		/* create node for internal unit */

  if (!(dn = alloc_driver_node()))
  {	  cleanexit(NULL, RETURN_FAIL);
  }
  dn->unit.Flags = MUDF_Internal;
  dn->unit.MidiDevicePort = 0;
  Sprintf(dn->listname,"« internal serial »");
  Sprintf(dn->version_string,"%ld.%ld", CamdBase->lib_Version, CamdBase->lib_Revision);
  strncpy(dn->unit.MidiDeviceComment, "Amiga built-in serial", sizeof dn->unit.MidiDeviceComment);
  AlphaInsertNode(&avail_list, &dn->node);

		/* now, scan for internal units */

  if (!(fib = AllocMem(sizeof *fib, MEMF_CLEAR)))
  {   TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOMEMORY, (UBYTE *)TEXT_ABORT, NULL, NULL);
  }
  else if (!(devs_lock = Lock("DEVS:midi", ACCESS_READ)))
  {   /* Q: Should we create a DEVS:Midi if there is none already? */
      TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NODEVS, (UBYTE *)TEXT_ABORT,  NULL, NULL);
  }
  else if (!(Examine(devs_lock,fib)))
  {
	  TM_DosRequest(TMData, (UBYTE *)TEXT_NODEVS2);
  }
  else
  {   result = TRUE;

      while (result && ExNext(devs_lock,fib))
  	  {
		if (fib->fib_EntryType < 0)
		{	struct MidiDeviceData *mdd;

				/* A file! see if it's a driver... */

			if (mdd = OpenMidiDevice(fib->fib_FileName))
			{	WORD	i;

				for (i=0; i<mdd->NPorts; i++)
				{
					if (!(dn = alloc_driver_node()))
					{	result = FALSE;
						break;
					}

					dn->unit.MidiDevicePort = i;
					strncpy(dn->unit.MidiDeviceName, mdd->Name, sizeof dn->unit.MidiDeviceName);
				    Sprintf(dn->version_string,"%ld.%ld", mdd->Version, mdd->Revision);
				    Sprintf(dn->listname,"%-.13s, unit %ld", mdd->Name, i);
					strncpy(dn->unit.MidiDeviceComment, dn->listname, sizeof dn->unit.MidiDeviceComment);
					AlphaInsertNode(&avail_list, &dn->node);
				}
				CloseMidiDevice (mdd);
			}
		}
  	  }

  	  if (IoErr() != ERROR_NO_MORE_ENTRIES)		/* done scanning? */
  	  {	TM_DosRequest(TMData, (UBYTE *)TEXT_NODEVS3);
  	    result = FALSE;
  	  }
  }

  if (fib) FreeMem(fib, sizeof *fib);
  if (devs_lock) UnLock(devs_lock);

  if (result == FALSE) cleanexit(NULL, RETURN_FAIL);
}

	/* set strings */

void SetStrings(struct TMData *TMData, struct DriverNode *dr)
{
	if (dr)
	{
		/* see if need unique cluster names */

		if (*dr->unit.MidiClusterInName == 0 || *dr->unit.MidiClusterOutName == 0)
		{	int	i;

			/* at least one bad name */

			for (i=0;i<(1<<30);i++)
			{	char				inName[MaxMidiInOutName],
									outName[MaxMidiInOutName];
				struct DriverNode	*adr;
				BOOL				match = FALSE;

				Sprintf(inName,"in.%ld",i);
				Sprintf(outName,"out.%ld",i);

				for (adr = (struct DriverNode *)active_list.lh_Head;
					 adr->node.ln_Succ;
					 adr = (struct DriverNode *)adr->node.ln_Succ)
				{
					if (!Stricmp(inName,adr->unit.MidiClusterInName) ||
						!Stricmp(outName,adr->unit.MidiClusterOutName))
					{
						match = TRUE;
						break;
					}
				}

				if (!match)
				{
					strcpy(dr->unit.MidiClusterInName,inName);
					strcpy(dr->unit.MidiClusterOutName,outName);
					break;
				}
			}
		}

		GT_SetGadgetAttrs(GadgetInfo_INPUTNAM.Gadget, WindowInfo_MIDIPREF.Window, NULL,
							GTST_String, dr->unit.MidiClusterInName,
							GA_Disabled, FALSE,
							TAG_DONE );

		GT_SetGadgetAttrs(GadgetInfo_OUTPUTNA.Gadget, WindowInfo_MIDIPREF.Window, NULL,
							GTST_String, dr->unit.MidiClusterOutName,
							GA_Disabled, FALSE,
							TAG_DONE );

		GT_SetGadgetAttrs(GadgetInfo_COMMENT.Gadget, WindowInfo_MIDIPREF.Window, NULL,
							GTST_String, dr->unit.MidiDeviceComment,
							GA_Disabled, FALSE,
							TAG_DONE );

		GT_SetGadgetAttrs(GadgetInfo_MSGQUEUS.Gadget, WindowInfo_MIDIPREF.Window, NULL,
							GTIN_Number, dr->unit.RecvQueueSize,
							GA_Disabled, FALSE,
							TAG_DONE );

		GT_SetGadgetAttrs(GadgetInfo_SYSEXQUE.Gadget, WindowInfo_MIDIPREF.Window, NULL,
							GTIN_Number, dr->unit.XmitQueueSize,
							GA_Disabled, FALSE,
							TAG_DONE );

		GT_SetGadgetAttrs(GadgetInfo_IDSTRING.Gadget, WindowInfo_MIDIPREF.Window, NULL,
							GTTX_Text, dr->version_string,
							TAG_DONE );
	}
	else
	{
		GT_SetGadgetAttrs(GadgetInfo_INPUTNAM.Gadget, WindowInfo_MIDIPREF.Window, NULL,
							GTST_String, "",
							GA_Disabled, TRUE,
							TAG_DONE );

		GT_SetGadgetAttrs(GadgetInfo_OUTPUTNA.Gadget, WindowInfo_MIDIPREF.Window, NULL,
							GTST_String, "",
							GA_Disabled, TRUE,
							TAG_DONE );

		GT_SetGadgetAttrs(GadgetInfo_COMMENT.Gadget, WindowInfo_MIDIPREF.Window, NULL,
							GTST_String, "",
							GA_Disabled, TRUE,
							TAG_DONE );

		GT_SetGadgetAttrs(GadgetInfo_SYSEXQUE.Gadget, WindowInfo_MIDIPREF.Window, NULL,
							GA_Disabled, TRUE,
							TAG_DONE );

		GT_SetGadgetAttrs(GadgetInfo_MSGQUEUS.Gadget, WindowInfo_MIDIPREF.Window, NULL,
							GA_Disabled, TRUE,
							TAG_DONE );

		GT_SetGadgetAttrs(GadgetInfo_IDSTRING.Gadget, WindowInfo_MIDIPREF.Window, NULL,
							GTTX_Text, "",
							TAG_DONE );
	}
}

	/* Load preferences file */

#if 0
"env:sys/" MidiPrefsName
#endif

extern struct List			avail_list,
							active_list;

struct DriverNode *find_unit(struct List *l, char *name, UWORD port)
{	struct DriverNode		*dr;

	for (dr = (struct DriverNode *)l->lh_Head;
		 dr->node.ln_Succ;
		 dr = (struct DriverNode *)dr->node.ln_Succ)
	{	if (!Stricmp(name,dr->unit.MidiDeviceName) &&
			dr->unit.MidiDevicePort == port)
				return dr;
	}
	return NULL;
}

BOOL GetMidiPrefs(char *filename)
{
	struct Node *node;
    struct MidiPrefs *prefs;
    struct IFFHandle *iff;
    struct ContextNode *cn;
    int err = CME_BadPrefs;
    WORD				i;

    if (iff = AllocIFF())
    {
		if (iff->iff_Stream = (ULONG) Open (filename, MODE_OLDFILE))
		{
		    InitIFFasDOS (iff);

				/* make all nodes inactive */

			while (node = RemHead(&active_list)) AlphaInsertNode(&avail_list, node);

		    if (!OpenIFF (iff, IFFF_READ))
		    {

				StopChunk(iff,'PREF','MIDI');

				if (!ParseIFF(iff,IFFPARSE_SCAN))
				{
				    cn = CurrentChunk(iff);

				    if (prefs = AllocMem(cn->cn_Size,MEMF_PUBLIC))
				    {
						if (ReadChunkBytes(iff,prefs,cn->cn_Size) == cn->cn_Size)
						{
							for (i=0; i<prefs->NUnits; i++)
							{	struct MidiUnitDef *mud = &prefs->UnitDef[i];
								struct DriverNode *dr;

								dr = find_unit(&avail_list, mud->MidiDeviceName, mud->MidiDevicePort);
								if (dr == NULL)
									dr = find_unit(&active_list, mud->MidiDeviceName, mud->MidiDevicePort);
								if (dr == NULL) continue;

									/* copy the prefs data over from the file */

								dr->unit = *mud;

								if (!(dr->unit.Flags & MUDF_Ignore))
								{	Remove(&dr->node);
									AlphaInsertNode(&active_list, &dr->node);
								}
							}
						    err = 0;
						}
						FreeMem(prefs,cn->cn_Size);
				    }
				    else
		    		{   TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOMEMORY, (UBYTE *)TEXT_ABORT, NULL, NULL);
		    		}
	            }
	            else
	            {   TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_LOADERR, (UBYTE *)TEXT_ABORT, NULL, NULL);
	           	}

				CloseIFF(iff);
		    }
		    else
	   		{   TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_LOADERR, (UBYTE *)TEXT_ABORT, NULL, NULL);
	   		}

            Close((BPTR)iff->iff_Stream);
		}
		else err = 0;	/* must not be there (?) */

		FreeIFF(iff);
    }
    else
	{   TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOMEMORY, (UBYTE *)TEXT_ABORT, NULL, NULL);
	}

    return !err;
}

long CountNodes(struct List *l)
{	long		count = 0;
	struct Node	*n;

	for (n = l->lh_Head; n->ln_Succ; n = n->ln_Succ) count++;
	return count;
}

	/* write the preferences file */

BOOL PutMidiPrefs(char *filename, BOOL do_icon)
{	struct MidiUnitDef *mud;

    struct MidiPrefs 	*prefs;
    struct IFFHandle 	*iff;
    struct ContextNode	*cn;
	WORD				nodes,
						pref_size;
    int					i = 0;
    struct DriverNode	*dr;
    int					result = FALSE;

   	nodes = CountNodes(&avail_list) + CountNodes(&active_list);

		/* build prefs structure */

   	pref_size = (nodes * sizeof *mud) + 4;
   	unless (prefs = AllocMem(pref_size,MEMF_CLEAR))
   	{   TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOMEMORY, (UBYTE *)TEXT_ABORT, NULL, NULL);
		return;
	}

	prefs->NUnits = nodes;
	for (dr = (struct DriverNode *)avail_list.lh_Head;
		 dr->node.ln_Succ;
		 dr = (struct DriverNode *)dr->node.ln_Succ)
	{	dr->unit.Flags |= MUDF_Ignore;
		prefs->UnitDef[i++] = dr->unit;
	}

	for (dr = (struct DriverNode *)active_list.lh_Head;
		 dr->node.ln_Succ;
		 dr = (struct DriverNode *)dr->node.ln_Succ)
	{	prefs->UnitDef[i++] = dr->unit;
	}

    if (iff = AllocIFF())
    {
		if (iff->iff_Stream = (ULONG) Open (filename, MODE_NEWFILE))
		{
		    InitIFFasDOS (iff);

			result = TRUE;

		    if (!OpenIFF (iff, IFFF_WRITE))
		    {	struct PrefHeader	ph;

				zero(&ph, sizeof ph);

				if (PushChunk (iff, ID_PREF, 'FORM', IFFSIZE_UNKNOWN)) result = FALSE;

				if (result == TRUE &&
					PushChunk (iff, ID_PREF, ID_PRHD, sizeof ph)) result = FALSE;
				if (result == TRUE &&
					WriteChunkBytes (iff, (APTR) &ph, sizeof ph) < 0) result = FALSE;
				if (result == TRUE && PopChunk(iff)) result = FALSE;

				if (result == TRUE &&
					PushChunk (iff, ID_PREF, ID_MIDI, pref_size)) result = FALSE;
				if (result == TRUE &&
					WriteChunkBytes (iff, (APTR) prefs, pref_size) < 0) result = FALSE;
				if (result == TRUE && PopChunk(iff)) result = FALSE;

				if (result == TRUE && PopChunk(iff)) result = FALSE;

				CloseIFF(iff);
		    }

			if (!result)
				TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_SAVEERR, (UBYTE *)TEXT_ABORT, NULL, NULL);

            Close((BPTR)iff->iff_Stream);

				/* save out the icon */

			if (result && do_icon)
			{	struct DiskObject *dobj;

				if (dobj = GetDefDiskObject(WBPROJECT))
				{	PutDiskObject(filename, dobj);
					FreeDiskObject(dobj);
				}
			}
		}
		else
		{   TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_SAVEERR, (UBYTE *)TEXT_ABORT, NULL, NULL);
		}

		FreeIFF(iff);
    }
   	else
   	{	TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOMEMORY, (UBYTE *)TEXT_ABORT, NULL, NULL);
	}

    if (prefs) FreeMem(prefs, pref_size);

	return result;
}

#define STR_INFO(gad)	((struct StringInfo *)gad->SpecialInfo)

void update_from_strings( struct TMData *TMData )
{   struct DriverNode *dr;

    if (dr = (struct DriverNode *)SelectNode(&active_list, last_selected_active))
    {
	    strncpy(dr->unit.MidiDeviceComment,
			 STR_INFO( GadgetInfo_COMMENT.Gadget )->Buffer,
			 sizeof dr->unit.MidiDeviceComment );

	    strncpy(dr->unit.MidiClusterOutName,
			STR_INFO( GadgetInfo_OUTPUTNA.Gadget )->Buffer,
			sizeof dr->unit.MidiClusterOutName );

	    strncpy(dr->unit.MidiClusterInName,
			STR_INFO( GadgetInfo_INPUTNAM.Gadget )->Buffer,
			sizeof dr->unit.MidiClusterInName );

        dr->unit.RecvQueueSize = STR_INFO( GadgetInfo_MSGQUEUS.Gadget )->LongInt;

 	    dr->unit.XmitQueueSize = STR_INFO( GadgetInfo_SYSEXQUE.Gadget )->LongInt;
   }
}
