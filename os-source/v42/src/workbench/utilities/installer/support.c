/*
 *		support.c
 *	
 * ========================================================================= 
 * Support.c - creates the window for installer utility                      
 * By Talin & Joe Pearce. (c) 1990 Sylvan Technical Arts                     
 * ========================================================================= 
 *
 * Prototypes for functions defined in support.c
 *
 *	struct Gadget * 		FindGadgetB		(struct Gadget * , WORD );
 *	struct Gadget * 		FindGadget		(struct Window * , WORD );
 *	void 					EmptyPort		(struct MsgPort * );
 *	void 					SelectGadget	(struct Gadget * , struct Window * , USHORT );
 *	struct String * 		new_string		(unsigned char * );
 *	struct String * 		empty_string	(LONG );
 *	BOOL 					CheckToolValue	(struct DiskObject * , unsigned char * , unsigned char * );
 *	unsigned char * 		LocateToolType	(struct DiskObject * , unsigned char * );
 *
 *	extern struct Library * ConsoleDevice;
 *	extern struct IOStdReq 	conreq;
 *
 *	struct Library * 		OpenConsoleLib	(void);
 *	void 					CloseConsoleLib	(void);
 *	UWORD 					DoKeyConvert	(struct IntuiMessage * );
 *	
 *
 *	Revision History:
 *
 *	lwilton	07/11/93:
 *		General cleanup and reformatting to work with SAS 6.x and the
 *		standard header files.
 */



#include <intuition/intuition.h>
#include <workbench/workbench.h>
#include <exec/memory.h>

#include "functions.h"

#include <string.h>

#include "xfunctions.h"

#include "installer.h"


extern struct Library *IconBase;

struct Gadget *FindGadgetB(struct Gadget *gad, WORD num)
{
	while (gad)
	{
		if (gad->GadgetID == num) 
			return gad;
		gad = gad->NextGadget;
	}
	return NULL;
}

struct Gadget *FindGadget(struct Window *win, WORD num)
{
	return FindGadgetB(win->FirstGadget,num);
}

void EmptyPort(struct MsgPort *port)
{
	struct Message *msg;

	Forbid();
	while (msg = GetMsg(port)) 	
		ReplyMsg(msg);
	Permit();
}

void SelectGadget(struct Gadget *gad, struct Window *win, USHORT state)
{
	long p;

	if ((gad->Flags & GADGHIGHBITS) == GADGHCOMP)
	{
		p = RemoveGadget(win,gad);
		AddGList(win,gad,p,1,NULL);
		RefreshGList(gad,win,NULL,1);
	}

	p = RemoveGadget(win,gad);
	gad->Flags = (gad->Flags & ~SELECTED) | state;
	AddGList(win,gad,p,1,NULL);
	RefreshGList(gad,win,NULL,1);
}

struct String *new_string(char *text)
{
	struct String	*str;
	WORD			len = 3 + strlen(text);

	if (str = MemAlloc(len,0L))
	{
		str->length = len;
		strcpy((char *)(str + 1),text);
	}

	return str;
}

struct String *empty_string(LONG size)
{
	struct String	*str;
	WORD			len = 3 + size;

	if (str = MemAlloc(len,MEMF_CLEAR)) 
		str->length = len;
	return str;
}

BOOL CheckToolValue(struct DiskObject *dobj, char *type, char *value)
{
	char *str;

	if (dobj->do_ToolTypes == NULL) 
		return FALSE;

	str = FindToolType(dobj->do_ToolTypes,type);
	if (str == NULL) 
		return FALSE;

	return MatchToolValue(str,value);
}

char *LocateToolType(struct DiskObject *dobj,char *type)
{
	char *str;

	if (dobj->do_ToolTypes == NULL) 
		return NULL;

	str = FindToolType(dobj->do_ToolTypes,type);
	if (str == NULL) 
		return NULL;

	return (strlen(str) == 0 ? NULL : str);
}

struct Library	*ConsoleDevice;
struct IOStdReq	conreq;

struct Library *OpenConsoleLib(void)
{
	if (!OpenDevice("console.device",-1,(struct IORequest *)&conreq,0))
	{
		ConsoleDevice = (struct Library *)conreq.io_Device;
	}

	return ConsoleDevice;
}

void CloseConsoleLib(void)
{
	if (ConsoleDevice) 
		CloseDevice((struct IORequest *)&conreq);
}

UWORD DoKeyConvert(struct IntuiMessage *imsg)
{
	static struct InputEvent ievent = { NULL, IECLASS_RAWKEY, 0, 0, 0};
	char	buffer[4];

	if (imsg->Class != RAWKEY) 
		return 0;

	ievent.ie_Code = imsg->Code;
	ievent.ie_Qualifier = imsg->Qualifier;

	/* get previous codes from location pointed to by IAddress
	 *  this pointer is valid until IntuiMessage is replied.
	 */

	ievent.ie_position.ie_addr = *(APTR *)imsg->IAddress;
	if (RawKeyConvert(&ievent, buffer, 2, NULL) == 1) 
		return (UWORD)buffer[0];

	return 0;
}
