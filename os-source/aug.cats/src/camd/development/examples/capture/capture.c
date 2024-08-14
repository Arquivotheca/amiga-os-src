
/* Notes: Wouldn't it be better to have the internal representation be
	standard MIDI (i.e. no time delays?)
*/

/* Includes */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <dos/dos.h>
#include <string.h>
#include <stdlib.h>
#include <libraries/asl.h>
#include <midi/camd.h>
#include <midi/mididefs.h>
#include <clib/macros.h>

#include "/camdlistreq/listreq.h"

#include "capture.h"	/* User header file */
#include "capture_rev.h"	/* Bumprev header file */

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>
#include <clib/asl_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <clib/camd_protos.h>

/* Pragma includes for register parameters */

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/camd_pragmas.h>

/* Global variables */

char vers[] = VERSTAG;	/* Bumprev version string */
struct Library *IntuitionBase;	/* intuition.library pointer */
struct Library *GadToolsBase;	/* gadtools.library pointer */
struct Library *UtilityBase;	/* utility.library pointer */
struct Library *AslBase;	/* asl.library pointer */
struct Library *CamdBase;		/* camd.library pointer */
struct IntuiText BodyText = {0,1,JAM2,20,8,NULL,(UBYTE *)TEXT_NORELEASE2,NULL};
struct IntuiText NegText  = {0,1,JAM2, 6,4,NULL,(UBYTE *)TEXT_OK,NULL};

struct List				memory_list;

#define BLOCK_SIZE		2048

#define SYSEX_BUFFER_SIZE 2048

struct memory_block {
	struct MinNode		node;
	LONG				filled;
	UBYTE				data[ BLOCK_SIZE ];
};

LONG					buffer_size = 0;
UBYTE					buffer_running_status = 0,
						read_running_status = 0;
LONG					buffer_time = 0;		/* future use				*/
BOOL					has_end_event = FALSE;

struct MidiNode			*midi;
struct MidiLink			*in_link,
						*out_link;

struct ListReq			*lreq;

struct MidiNode *CreateMidi(Tag tag, ...)
{	return CreateMidiA((struct TagItem *)&tag );
}

BOOL SetMidiAttrs(struct MidiNode *mi, Tag tag, ...)
{	return SetMidiAttrsA(mi, (struct TagItem *)&tag );
}

struct MidiLink *AddMidiLink(struct MidiNode *mi, LONG type, Tag tag, ...)
{	return AddMidiLinkA(mi, type, (struct TagItem *)&tag );
}

BOOL SetMidiLinkAttrs(struct MidiLink *mi, Tag tag, ...)
{	return SetMidiLinkAttrsA(mi, (struct TagItem *)&tag );
}

ULONG GetMidiLinkAttrs(struct MidiLink *mi, Tag tag, ...)
{	return GetMidiLinkAttrsA(mi, (struct TagItem *)&tag );
}

/* Entry functions */

#ifdef _DCC	/* for DICE compatibility */
VOID wbmain(VOID *wbmsg)
  {
  main(0, wbmsg);
  }
#endif

VOID main(int argc, char **argv)
  {
  struct TMData *TMData;	/* data structure pointer */
  ULONG error;

	NewList( &memory_list );

  if(!(IntuitionBase = OpenLibrary((UBYTE *)"intuition.library", 37L)))	/* Open intuition.library V37 */
    {
    if(IntuitionBase = OpenLibrary((UBYTE *)"intuition.library", 0L))
      {
      AutoRequest(NULL, &BodyText, NULL, &NegText, 0, 0, 320, 80);
      CloseLibrary(IntuitionBase);
      }
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(GadToolsBase = OpenLibrary((UBYTE *)"gadtools.library", 37L)))	/* Open gadtools.library V37 */
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, "gadtools.library V37");
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(UtilityBase = OpenLibrary((UBYTE *)"utility.library", 37L)))	/* Open utility.library V37 */
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, "utility.library V37");
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(AslBase = OpenLibrary((UBYTE *)"asl.library", 37L)))	/* Open asl.library V37 */
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, "asl.library V37");
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(CamdBase = OpenLibrary((UBYTE *)"camd.library", 0L)))	/* Open camd.library */
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, "camd.library");
    cleanexit(NULL, RETURN_FAIL);
    }

	if (!(midi = CreateMidi(	MIDI_Name, 		"MIDI Capture",
								MIDI_RecvSignal,SIGBREAKB_CTRL_E,
								MIDI_MsgQueue, 	200,
								MIDI_SysExSize,	SYSEX_BUFFER_SIZE,
								TAG_DONE )))
	{	TM_Request(NULL, (UBYTE *)TEXT_ERROR, "Can't Create the MIDI Node.", (UBYTE *)TEXT_ABORT, NULL, NULL);
	    cleanexit(NULL, RETURN_FAIL);
	}

	if (!(out_link = AddMidiLink (midi, MLTYPE_Sender,
									MLINK_Location, "out.0",
									MLINK_Name, 	"Out",
									TAG_END )))
	{	TM_Request(NULL, (UBYTE *)TEXT_ERROR, "Can't establish output link.", (UBYTE *)TEXT_ABORT, NULL, NULL);
	    cleanexit(NULL, RETURN_FAIL);
	}

	if (!(in_link = AddMidiLink (	midi, MLTYPE_Receiver,
									MLINK_Location, "in.0",
									MLINK_Name,		"In",
									TAG_END )))
	{	TM_Request(NULL, (UBYTE *)TEXT_ERROR, "Can't establish input link.", (UBYTE *)TEXT_ABORT, NULL, NULL );
	    cleanexit(NULL, RETURN_FAIL);
	}

	if (!(lreq = AllocListRequest( TAG_END )))
	{	TM_Request(NULL, (UBYTE *)TEXT_ERROR, "Error creating cluster list requester.", (UBYTE *)TEXT_ABORT, NULL, NULL );
	    cleanexit(NULL, RETURN_FAIL);
	}

  if(!(TMData = TM_Open(&error)))	/* Open Toolmaker data */
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

  if(!(TMData->FileRequester = AllocAslRequestTags(ASL_FileRequest, TAG_DONE)))
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOFILEREQ, (UBYTE *)TEXT_ABORT, NULL, NULL);
    cleanexit(TMData, RETURN_FAIL);
    }

  if(!(OpenScreen_Workbench(TMData)))	/* Open default public screen */
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOSCREEN, (UBYTE *)TEXT_ABORT, NULL, NULL);
    cleanexit(TMData, RETURN_FAIL);
    }

  if(!(OpenWindow_CAPTUREM(TMData)))	/* Open the window */
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOWINDOW, (UBYTE *)TEXT_ABORT, NULL, NULL);
    cleanexit(TMData, RETURN_FAIL);
    }




  TM_EventLoop(TMData);	/* Process events */

  cleanexit(TMData, RETURN_OK);
  }

/* cleanexit function */

VOID cleanexit(struct TMData *TMData, int returnvalue)
{
	if(TMData)
	{
		CloseWindow_CAPTUREM(TMData);
		CloseScreen_Workbench(TMData);
		if(TMData->FileRequester) FreeAslRequest(TMData->FileRequester);
		TM_Close(TMData);
	}

	if (out_link)		RemoveMidiLink ( out_link );
	if (in_link)		RemoveMidiLink ( in_link );
	if (midi)			DeleteMidi( midi );
	if (lreq)			FreeListRequest( lreq );

	if(CamdBase)		CloseLibrary(CamdBase);		/* Close camd.library		*/
	if(AslBase)			CloseLibrary(AslBase);		/* Close asl.library		*/
	if(UtilityBase)		CloseLibrary(UtilityBase);	/* Close utility.library	*/
	if(GadToolsBase)	CloseLibrary(GadToolsBase);	/* Close gadtools.library	*/
	if(IntuitionBase)	CloseLibrary(IntuitionBase);/* Close intuition.library	*/

	exit(returnvalue);
}

/* ASL file requester function */

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
      strcpy(buffer, TMData->FileRequester->rf_Dir);
      if(AddPart(buffer, TMData->FileRequester->rf_File, bufsize))
        {
        return(buffer);
        }
      }
    }

  return(NULL);
  }

/* Window event functions */

VOID Window_CAPTUREM_REFRESHWINDOW(struct TMData *TMData, struct IntuiMessage *imsg)
  {
  }

BOOL Window_CAPTUREM_CLOSEWINDOW(struct TMData *TMData, struct IntuiMessage *imsg)
  {
  return(TRUE);
  }

BOOL Window_CAPTUREM_MENUPICK(struct TMData *TMData, struct IntuiMessage *imsg)
  {
  UWORD menucode;
  struct MenuItem *menuitem;
  TMOBJECTDATA *tmobjectdata;
  BOOL (*eventfunc)(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);

  menucode = imsg->Code;
  while(menucode != MENUNULL)
    {
    menuitem = ItemAddress(WindowInfo_CAPTUREM.Menu, menucode);
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

BOOL Window_CAPTUREM_GADGETDOWN(struct TMData *TMData, struct IntuiMessage *imsg)
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

BOOL Window_CAPTUREM_GADGETUP(struct TMData *TMData, struct IntuiMessage *imsg)
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

	/* Buffer management functions */

LONG LengthVarNum( ULONG num )
{	LONG				length = 0;

	do { length++; num >>= 7; } while (num) ;
	return length;
}

void ClearBuffer( void )
{	struct memory_block	*mb;

	while (mb = (struct memory_block *)RemHead( &memory_list ))
	{	FreeMem( mb, sizeof *mb );
	}
	buffer_size = 0;
	buffer_running_status = 0;
	buffer_time = 0;
	has_end_event = FALSE;
}

BOOL AppendBytes( UBYTE *data, LONG length )
{	struct memory_block	*mb;

	while (length > 0)
	{	LONG			fit;

			/*	Get last list node */

		mb = (struct memory_block *)memory_list.lh_TailPred;

			/*	If empty list, or block full, allocate new block */

		if (mb->node.mln_Pred == NULL ||
			mb->filled >= BLOCK_SIZE)
		{
			if (!(mb = AllocMem( sizeof *mb, 0L )))
			{	return FALSE;
			}
			mb->filled = 0;
			AddTail( &memory_list, (struct Node *)mb );
		}

		fit = MIN( length, BLOCK_SIZE - mb->filled );
		CopyMem( data, &mb->data[ mb->filled ], fit );

		data += fit;
		length -= fit;
		mb->filled += fit;
		buffer_size += fit;
	}
	return TRUE;
}

BOOL AppendVarNum( ULONG num )
{	UBYTE				var[ 6 ],
						varindex = 6;

	var[ --varindex ] = num & 0x7f;
	num >>= 7;

	while (num)
	{	var[ --varindex ] = num | 0x80;
		num >>= 7;
	}

	return (AppendBytes( var + varindex, 6 - varindex ));
}

	/*	Set pointer to beginning of buffer */

void BufferStart( struct memory_block **block, LONG *offset )
{	*offset = 0;
	*block = (struct memory_block *)memory_list.lh_Head;
	read_running_status = 0;
}

	/*	Read 1 MIDI message from the buffer */

WORD GetNextBufferByte( struct memory_block **block, LONG *offset )
{	struct memory_block	*mb = *block;

	while (mb->node.mln_Succ != 0)				/* while not end of list		*/
	{
		if (*offset < mb->filled)				/* If more bytes in block		*/
			return mb->data[ (*offset)++ ];		/* Return next byte from buffer	*/

		*offset = 0;							/* Go to next block				*/
		*block = mb = (struct memory_block *)mb->node.mln_Succ;
	}
	return -1;
}

LONG GetVarNum( struct memory_block **block, LONG *offset )
{	LONG				val = 0;
	WORD				byte;

	do
	{	byte = GetNextBufferByte( block, offset );
		if (byte < 0) return -1;
		val = (val << 7) + (byte & 0x7f);
	} while (byte & 0x80) ;

	return val;
}

	/*	Read 1 MIDI message from the buffer */

	/*	REM: This could use some error checks to make sure that the
		data bytes written don't have high bits set.
	*/

LONG GetNextBufferMessage( struct memory_block **block, LONG *offset, UBYTE *buffer, LONG buffersize )
{	WORD				midibyte;
	LONG				midilength = 0,
						delta,
						i;

	for (;;)
	{
			/*	Get status byte */

		delta = GetVarNum( block, offset );		/* skip over delta time			*/
		if (delta < 0) return 0;

		midibyte = GetNextBufferByte( block, offset );
		if (midibyte < 0) return 0;

		if (midibyte == 0xff)					/* meta-event					*/
		{
				/*	Get meta-event subtype (ignored for now) */

			if ((midibyte = GetNextBufferByte( block, offset )) < 0) return 0;

				/*	Get meta-event length */

			if ((midilength = GetNextBufferByte( block, offset )) < 0) return 0;

				/*	Skip over meta-event data */

			while (midilength > 0)
			{	if (GetNextBufferByte( block, offset ) < 0) return 0;
				midilength--;
			}
		}
		else if (midibyte & 0x80)				/* if status byte				*/
		{
			read_running_status = midibyte;		/* set running status			*/

				/*	Complete sysex message */

			if (midibyte == MS_SysEx)			/* check system exclusive		*/
			{									/* get exclusive length			*/
				midilength = GetVarNum( block, offset );
				if (midilength < 0)				/* check error					*/
					return midilength;
												/* clip to buffer length		*/
				if (midilength + 1 >= buffersize) midilength = buffersize - 1;
				*buffer++ = MS_SysEx;			/* write SysEx byte				*/
				for (i=0; i<midilength; i++)	/* write other bytes			*/
				{	if ((*buffer++ = GetNextBufferByte( block, offset )) < 0) return 0;
				}
				return midilength + 1;
			}

				/*	Incomplete sysex message (some MidiFiles have this) */

			if (midibyte == 0xF7)				/* special sysex				*/
			{									/* get exclusive length			*/
				midilength = GetVarNum( block, offset );
				if (midilength < 0)				/* check error					*/
					return midilength;
												/* clip to buffer length		*/
				if (midilength >= buffersize) midilength = buffersize;
				for (i=0; i<midilength; i++)	/* write other bytes			*/
				{	if ((*buffer++ = GetNextBufferByte( block, offset )) < 0) return 0;
				}
				return midilength;
			}

			midilength = MidiMsgLen( midibyte );

			if (midilength > 0)				/* if normal message			*/
			{	*buffer++ = midibyte;		/* put status byte in buffer	*/

				for (i=1; i<midilength; i++) /* copy data bytes to buffer	*/
				{	if ((*buffer++ = GetNextBufferByte( block, offset )) < 0) return 0;
				}
				return midilength;			/* return length				*/
			}
		}
		else	/* no status byte, use running status */
		{
			midilength = MidiMsgLen( read_running_status );
			*buffer++ = read_running_status;
			*buffer++ = midibyte;
			for (i=2; i<midilength; i++)
			{	if ((*buffer++ = GetNextBufferByte( block, offset )) < 0) return 0;
			}
			return midilength;
		}
	}
}

	/* MIDI Input functions */

void HandleMidi( struct TMData *TMData )
{	MidiMsg		mm;
	ULONG		time = 0;

	while (GetMidi( midi, &mm ))
	{
		ULONG	length = MidiMsgLen( mm.mm_Status );

			/*	REM: Append a time delta value for standard MIDI? ? */

		if (mm.mm_Status == MS_SysEx)
		{	UBYTE		temp_ssx[ 1024 ];

			length = GetSysEx( midi, temp_ssx, sizeof temp_ssx );

			if (length >= sizeof temp_ssx)
			{	/* error, message too long for buffer */
			}

				/*	Store into the buffer in standard MIDI style:
					--	Variable length delta time (always zero here).
					--	System Exclusive byte (0xF0)
					--	Variable-length number of bytes in message.
					--	Actual message bytes, including EOX (0xf7);
				*/

			if (	AppendVarNum( time - buffer_time ) == FALSE
				 || AppendBytes( &temp_ssx[ 0 ], 1 ) == FALSE
				 || AppendVarNum( length - 1 ) == FALSE
				 || AppendBytes( &temp_ssx[ 1 ], length - 1 ) == FALSE )
			{	/* error... */
			}
		}
		else if (length)						/* only save messages with length	*/
		{
				/*	First, append the delta-tie to the stream */

			if ( AppendVarNum( time - buffer_time ) == FALSE )
			{	/* error */
			}
			else if (mm.mm_Status < 0xf0 && mm.mm_Status == buffer_running_status)
			{
					/*	If the status byte is the same as the last,
						The don't need to write it...
					*/

				if ( AppendBytes( &mm.mm_Data[1], length-1 ) == FALSE )
				{	/* error */
				}
			}		/*	Otherwise, write out the status byte */
			else if (AppendBytes( mm.mm_Data, length ) == FALSE)
			{	/* error */
			}
		}
		buffer_running_status = mm.mm_Status;
		buffer_time = time;
	}

	GT_SetGadgetAttrs(
		GadgetInfo_BYTESINB.Gadget,
		WindowInfo_CAPTUREM.Window,
		NULL,
		GTNM_Number, buffer_size,
		TAG_END );
}

	/* Send MIDI data out again */

UWORD __chip WaitPData[] =
{   0x0000, 0x0000,
    0x0400, 0x07C0,
    0x0000, 0x07C0,
    0x0100, 0x0380,
    0x0000, 0x07E0,
    0x07C0, 0x1FF8,
    0x1FF0, 0x3FEC,
    0x3FF8, 0x7FDE,
    0x3FF8, 0x7FBE,
    0x7FFC, 0xFF7F,
    0x7EFC, 0xFFFF,
    0x7FFC, 0xFFFF,
    0x3FF8, 0x7FFE,
    0x3FF8, 0x7FFE,
    0x1FF0, 0x3FFC,
    0x07C0, 0x1FF8,
    0x0000, 0x07E0,
    0x0000, 0x0000,             /* reserved, must be NULL */
};

void SendMIDI( struct TMData *TMData )
{	UBYTE				temp_msg[ 1024 ];
	LONG				offset;
	struct memory_block *block;
	LONG				length;

	SetPointer( WindowInfo_CAPTUREM.Window, WaitPData, 16, 16, -7, -7 );
	BufferStart( &block, &offset );

	while (length = GetNextBufferMessage( &block, &offset, temp_msg, sizeof temp_msg ))
	{	MidiMsg			mm;

		if (length < 0) break;

		if (temp_msg[0] == MS_SysEx) PutSysEx ( out_link, temp_msg );
		else
		{	CopyMem( temp_msg, mm.mm_Data, 3 );
			PutMidi( out_link,
					 (temp_msg[0] << 24) | (temp_msg[1] << 16) | (temp_msg[2] << 8) );
		}
	}
	ClearPointer( WindowInfo_CAPTUREM.Window );
}

	/* MIDI Load / Save functions */

struct smfHeader {
	WORD				format;
	WORD				tracks;
	WORD				division;
};


void FileError( struct TMData *TMData, LONG error, char *filename )
{	TM_Request( WindowInfo_CAPTUREM.Window,
				(UBYTE *)TEXT_ERROR,
				"Error writing file '%s', code %ld",
				"Continue", NULL,
				filename, error );
}

	/*	Note that buffer has no running status. Should we compress? */

void SaveMIDI( struct TMData *TMData, char *filename )
{
	LONG				offset;
	struct memory_block *block;
	struct smfHeader	smf;
	LONG				track_size = 0;
	LONG				header_length = sizeof smf;
	BPTR				fh;
	LONG				error = 0;
	static UBYTE		end_event[] = { 0x00, 0xFF, 0x2F, 0x00 };

	SetPointer( WindowInfo_CAPTUREM.Window, WaitPData, 16, 16, -7, -7 );

	smf.format = 0;
	smf.tracks = 1;
	smf.division = 192;

		/*	Start by counting the number of messages and calculating the track size */

	BufferStart( &block, &offset );
	track_size = buffer_size;
	if (!has_end_event) track_size += 4;

	if (fh = Open( filename, MODE_NEWFILE ))
	{	struct memory_block	*mb;

		BufferStart( &block, &offset );

		if (   (FWrite( fh, "MThd",                 4,             1 ) != 1)
		    || (FWrite( fh, (STRPTR)&header_length, sizeof (LONG), 1 ) != 1)
			|| (FWrite( fh, (STRPTR)&smf,			sizeof smf,    1 ) != 1)
			|| (FWrite( fh, "MTrk",					4,             1 ) != 1)
			|| (FWrite( fh, (STRPTR)&track_size,	sizeof (LONG), 1 ) != 1) )
		{	error = IoErr();
		}

		for (mb = (struct memory_block *)memory_list.lh_Head;
			 mb->node.mln_Succ;
			 mb = (struct memory_block *)mb->node.mln_Succ )
		{
			if (FWrite( fh, mb->data, mb->filled, 1 ) != 1)
			{	error = IoErr();
			}
		}
		if (!has_end_event &&
			(FWrite( fh, end_event, sizeof end_event, 1 ) != 1) )
		{	error = IoErr();
		}

		/*	Printf("%02.2lx length=%ld\n", temp_msg[ 0 ], length ); */
		Close( fh );
	}
	ClearPointer( WindowInfo_CAPTUREM.Window );

	if (error) FileError( TMData, error, filename );
}

#if 0
BOOL StuffBytes( struct TMData *TMData, UBYTE *data, LONG length, char *filename )
{
	if (AppendBytes( data, length ) == FALSE)
	{	TM_Request( WindowInfo_CAPTUREM.Window,
					(UBYTE *)TEXT_ERROR,
					"Not enough memory to load file '%s'",
					"Continue", NULL,
					filename );
		break;
	}
}
#endif

void LoadMIDI( struct TMData *TMData, char *filename )
{	BPTR				fh;
	struct smfHeader	smf;
	LONG				length;
	LONG				error = 0;
	struct {
		LONG			chunkID,
						chunkLength;
	} chunkHeader;

	SetPointer( WindowInfo_CAPTUREM.Window, WaitPData, 16, 16, -7, -7 );

	if (fh = Open( filename, MODE_OLDFILE ))
	{

		if (FRead( fh, (STRPTR)&chunkHeader, sizeof chunkHeader, 1 ) != 1)
		{	error = IoErr();
		}
		else if (chunkHeader.chunkID != 'MThd')	/* Read file header	ID		*/
		{	TM_Request( WindowInfo_CAPTUREM.Window,
						(UBYTE *)TEXT_ERROR,
						"This is not a Standard MIDI File",
						"Continue", NULL, NULL );
		}
		else if (chunkHeader.chunkLength != sizeof smf) /* Read file header length	*/
		{	TM_Request( WindowInfo_CAPTUREM.Window,
						(UBYTE *)TEXT_ERROR,
						"Bad size for MIDI file header",
						"Continue", NULL, NULL );
		}										/* Read file header			*/
		else if (FRead( fh, (STRPTR)&smf, sizeof smf, 1 ) != 1)
		{	error = IoErr();
		}
		else
		{	UBYTE		temp_buffer[512];

			for (;;)
			{
					/* Read the track header ID */

				if (FRead( fh, (STRPTR)&chunkHeader, sizeof chunkHeader, 1 ) != 1)
				{	error = IoErr();
					break;
				}

					/* If it's not a track, skip it */

				if (chunkHeader.chunkID != 'MTrk')
				{	Seek( fh, chunkHeader.chunkLength, OFFSET_CURRENT );
					if (error = IoErr()) break;
					continue;
				}

				length = chunkHeader.chunkLength;
				ClearBuffer();					/* clear out the old data		*/

					/* Read entire track */

				while (length > 0)
				{	LONG	block_size = MIN( length, sizeof temp_buffer );

					if (FRead( fh, temp_buffer, block_size, 1 ) != 1)
					{	error = IoErr();
						break;
					}

					if (AppendBytes( temp_buffer, block_size ) == FALSE)
					{	TM_Request( WindowInfo_CAPTUREM.Window,
									(UBYTE *)TEXT_ERROR,
									"Not enough memory to load file '%s'",
									"Continue", NULL,
									filename );
						break;
					}
					length -= block_size;
				}
				break;
			}
			has_end_event = TRUE;
		}
		Close( fh );
	}
	ClearPointer( WindowInfo_CAPTUREM.Window );

	if (error) FileError( TMData, error, filename );
}
