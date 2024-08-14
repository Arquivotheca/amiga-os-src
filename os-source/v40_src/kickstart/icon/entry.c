/*
 * $Id: entry.c,v 38.6 92/07/28 13:47:38 mks Exp $
 *
 * $Log:	entry.c,v $
 * Revision 38.6  92/07/28  13:47:38  mks
 * Now uses SetVBuf to set the buffering mode for icon load/save
 * and set the buffer size.
 * 
 * Revision 38.5  92/07/02  08:34:35  mks
 * More typo fixes
 *
 * Revision 38.4  92/07/02  08:32:19  mks
 * Fixed up autodoc...
 *
 * Revision 38.3  92/02/26  09:33:19  mks
 * Added feature such that you can delete a fake icon too...
 *
 * Revision 38.2  91/11/08  14:08:01  mks
 * autodoc cleanup
 *
 * Revision 38.1  91/06/24  19:01:07  mks
 * Changed to V38 source tree - Trimmed Log
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/stdio.h>
#include <workbench/workbench.h>

#include <string.h>

#include "oldwbinternal.h"
#include "internal.h"
#include "support.h"

char *GetOtherName(char *name)
{
char *buf=NULL;

#ifndef	NO_LENGTH_HACK
	if (strlen(FilePartStub(name)) > 25)
	{
		/* Guess what?  The file name is too long! */
		setResult2(ERROR_INVALID_COMPONENT_NAME);
	}
	else
#endif	/* NO_LENGTH_HACK */

	if (buf=AllocVecStub(strlen(name)+6,MEMF_PUBLIC))
	{
		/*
		 * Ok, we now have the memory needed to make the
		 * new name and the name is not too long, so make
		 * the name and open the file.  We won't check the
		 * result from open as it is just passed back
		 * to the caller and he needs to check it anyway.
		 */
		strcpy(buf,name);
		strcat(buf,".info");
	}
	return(buf);
}

/*
 * This function will open a .info file given the name of the base
 * file in the mode given.  (Just like Open())  It will return
 * an error if the filename is too long or if the Open() failed.
 */
LONG OpenIcon(char *name,LONG mode)
{
LONG file=NULL;
char *buf;

	if (buf=GetOtherName(name))
	{
		DP(("Opening %s\n",buf));
		if (file=OpenStub(buf,mode))
		{
			SetVBufStub(file,NULL,BUF_FULL,2048);
		}
		FreeVecStub(buf);
	}
	return(file);
}

/*
 * Returns FALSE if it did not work, TRUE if it did...
 */
IGetIconCommon(char *name,struct DiskObject *icon,struct FreeList *free,struct DiskObject *srcdobj,long version)
{
struct Gadget *gad;
struct NewDD *dd = NULL;
APTR srcgadgetrender;
APTR srcselectrender;
LONG len;
LONG file=NULL;
long retval=FALSE;
long revision;

#ifdef	DEBUGGING
struct Gadget *srcgad;
#endif	/* DEBUGGING */

    DP(("IGetIconCom: enter, name=$%lx (%s), icon=$%lx, free=$%lx, srcdobj=$%lx, ver=$%lx\n",name,name,icon,free,srcdobj,version));

    gad = &icon->do_Gadget;

    /* David Berezowski - Jun/90 */
    if (srcdobj)
    {
	DS((srcgad = &srcdobj->do_Gadget))
	srcgadgetrender = (&srcdobj->do_Gadget)->GadgetRender;
	srcselectrender = (&srcdobj->do_Gadget)->SelectRender;
    }
    else
    {
	DS((srcgad = NULL))
	srcgadgetrender = NULL;
	srcselectrender = NULL;
    }
    DP(("srcgad=$%lx, sgr=$%lx, ssr=$%lx\n",srcgad,srcgadgetrender,srcselectrender));

    /* David Berezowski - Jun/90 */
    if (srcdobj)
    {
	DP(("GIC: copying %ld bytes from $%lx to $%lx\n",sizeof(struct DiskObject),srcdobj,icon));
	*icon=*srcdobj;
	retval=TRUE;
    }
    else
    {
        DP(("Trying to open file...\n"));

#ifdef	DEBUGGING
	if (!(file=OpenIcon(name,MODE_OLDFILE)))
	{
	    DP(("GetIcon: error, could not open %s\n", name));
	}
	else
#else
	if (file=OpenIcon(name,MODE_OLDFILE))
#endif	/* DEBUGGING */
	{
            DP(("GetIcon: File opened...\n"));
            if (IRead(file,icon,sizeof(struct DiskObject)))
            {
            	if ((icon->do_Magic==WB_DISKMAGIC) && (icon->do_Version==WB_DISKVERSION))
            	{
                    gad->NextGadget = NULL;
		    retval=TRUE;
		}

#ifdef	DEBUGGING
            	else
            	{
                    DP(("GetIcon: (%s) do_Magic=%lx, WB_DISKMAGIC=%lx, Error!\n",name,icon->do_Magic,WB_DISKMAGIC));
                    DP(("GetIcon: (%s) do_Version=%ld, WB_DISKVERSION=%ld, Error!\n",name,icon->do_Version,WB_DISKVERSION));
                }
#endif	/* DEBUGGING */

	    }

#ifdef	DEBUGGING
            else
            {
                DP(("GetIcon: error, could not read DiskObject\n"));
            }
#endif	/* DEBUGGING */

	}
    }

    if (retval)
    {
        if (icon->do_DrawerData)
        {
            if (!(dd=TFLAlloc(free,sizeof(struct NewDD),MEMF_CLEAR|MEMF_PUBLIC,struct NewDD *)))
            {
                DP(("GetIcon: (%s) no mem for NewDD struct\n", name));
                retval=FALSE;
            }
            else
            {
            	icon->do_DrawerData=dd;

                if (srcdobj)
                {
                    DP(("GIC: copying %ld bytes from $%lx to $%lx\n",OLDDRAWERDATAFILESIZE,&srcdobj->do_DrawerData->dd_NewWindow,&dd->dd_NewWindow));
                    memcpy(&dd->dd_NewWindow,&srcdobj->do_DrawerData->dd_NewWindow,OLDDRAWERDATAFILESIZE);
                }
                else if (!IRead(file,&dd->dd_NewWindow,OLDDRAWERDATAFILESIZE))
                {
                    DP(("GetIcon: (%s) could not read %ld bytes\n",name,OLDDRAWERDATAFILESIZE));
                    retval=FALSE;
                }
                else
                {
                    icon->do_DrawerData=dd;
                    if (version < 36)
                    {       /* pre V1.4 */
                        NewList(&((struct OldNewDD *)dd)->dd_Children);
                    }
                    else
                    {       /* V1.4 or greater */
                        NewList(&dd->dd_Children);
                    }
                }
            }
        }

        /* deal with images */
        if (retval)
        {
        APTR ptmp;

            if (gad->Flags & GADGIMAGE)
            {

                ptmp=getImage(free,file,gad->GadgetRender,srcgadgetrender);
                if (ptmp==(APTR)-1)
                {
                    DP(("GetIcon: (%s) could not getImage\n", name));
                    retval=FALSE;
                }
                else
                {
                    gad->GadgetRender=ptmp;
                    ptmp=getImage(free,file,gad->SelectRender,srcselectrender);
                    if (ptmp==(APTR)-1)
                    {
                        DP(("GetIcon: (%s) could not getImage\n", name));
                        retval=FALSE;
                    }
                    else gad->SelectRender=ptmp;
                }
            }
            else
            {
            	setResult2(ERROR_OBJECT_WRONG_TYPE);
            	retval=FALSE;
            }
        }

        if (gad->GadgetText)
        {
            gad->GadgetText=NULL; /* no input text for now */
        }

        if (retval)
        {
            if (icon->do_DefaultTool)
            {
                DP(("GIC: icon has a default tool\n"));
                if (srcdobj) len=strlen(srcdobj->do_DefaultTool)+1;
                else if (!IRead(file,&len,sizeof(LONG)))
                {
                    DP(("GetIcon: (%s) could not read defaulttool\n", name));
                    retval=FALSE;
                }

                if (retval)
                {
                    if (icon->do_DefaultTool=TFLAlloc(free,len,MEMF_PUBLIC,char *))
                    {
                        if (srcdobj)
                        {
                            DP(("GIC: copying %ld bytes from $%lx to $%lx\n",len,srcdobj->do_DefaultTool,icon->do_DefaultTool));
                            memcpy(icon->do_DefaultTool,srcdobj->do_DefaultTool,len);
                        }
                        else if (!IRead(file,icon->do_DefaultTool,len))
                        {
                            DP(("GetIcon: (%s) could not read defaulttool\n", name));
                            retval=FALSE;
                        }
                    }
                    else
                    {
                        DP(("GetIcon: (%s) could not read defaulttool\n", name));
                        retval=FALSE;
                    }
                }
            }
        }

        if (retval)
        {
            if (icon->do_ToolTypes)
            {
            char **sp;
            char **srcsp;
            LONG count;

                DP(("GIC: icon has tool types\n"));
                if (srcdobj)
                {
                    for (srcsp=srcdobj->do_ToolTypes,count=sizeof(char *);*srcsp;srcsp++,count+=sizeof(char *));
                }
                else if (!IRead(file,&count,sizeof(LONG)))
                {
                    DP(("GetIcon: (%s) could not read tooltypes\n", name));
                    retval=FALSE;
                }

                if (retval)
                {
                    if (!(sp=TFLAlloc(free,count,MEMF_CLEAR,char **)))
                    {
                        DP(("GetIcon: (%s) could not read tooltypes\n", name));
                        retval=FALSE;
                    }
                    else
                    {
                        icon->do_ToolTypes = sp;

                        if (srcdobj)
                        {
                            for(srcsp=srcdobj->do_ToolTypes;count>sizeof(char *);count-=sizeof(char *),sp++,srcsp++)
                            {
                                if (retval)
                                {
                                    len=strlen(*srcsp)+1;

                                    if (!(*sp=TFLAlloc(free,len,MEMF_PUBLIC,char *)))
                                    {
                                        DP(("GetIcon: (%s) could not read tooltypes\n", name));
                                        retval=FALSE;
                                    }
                                    else
                                    {
                                        DP(("GIC: copying %ld bytes from $%lx to $%lx\n",len,*srcsp,*sp));
                                        memcpy(*sp,*srcsp,len);
                                    }
                                }
                            }
                        }
                        else
                        {
                            for(;count>sizeof(char *);count-=sizeof(char *),sp++)
                            {
                                if (retval)
                                {
                                    if (!IRead(file,&len,sizeof(LONG)))
                                    {
                                        DP(("GetIcon: (%s) could not read tooltypes\n", name));
                                        retval=FALSE;
                                    }
                                    else if (!(*sp=TFLAlloc(free,len,MEMF_PUBLIC,char *)))
                                    {
                                        DP(("GetIcon: (%s) could not read tooltypes\n", name));
                                        retval=FALSE;
                                    }
                                    else if (!IRead(file,*sp,len))
                                    {
                                        DP(("GetIcon: (%s) could not read tooltypes\n", name));
                                        retval=FALSE;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (retval)
        {
            if (icon->do_ToolWindow)
            {
                DP(("GIC: icon has a tool window\n"));
                if (srcdobj) len=strlen(srcdobj->do_ToolWindow)+1;
                else if (!IRead(file,&len,sizeof(LONG)))
                {
                    DP(("GetIcon: (%s) could not read toolwindow string length\n",name));
                    retval=FALSE;
                }
                else
                {
                    if (!(icon->do_ToolWindow=TFLAlloc(free,len,MEMF_PUBLIC,char *)))
                    {
                        DP(("GetIcon: (%s) could not alloc toolwindow ptr\n", name));
                        retval=FALSE;
                    }
                    else if (srcdobj)
                    {
                        DP(("GIC: copying %ld bytes from $%lx to $%lx\n",len,srcdobj->do_ToolWindow,icon->do_ToolWindow));
                        memcpy(icon->do_ToolWindow,srcdobj->do_ToolWindow,len);
                    }
                    else if (!IRead(file,icon->do_ToolWindow,len))
                    {
                        DP(("GetIcon: (%s) could not read toolwindow string\n", name));
                        retval=FALSE;
                    }
                }
            }
        }

        if (retval)
        {
            /*
                New for V1.4.  Although I've extended the size of a DrawerData struct,
                I cannot read in the entire structure in one go else new .info
                files will not be able to be read by pre V1.4 icon.libraries.  Instead
                I use the lower 8 bits of the UserData field in the Gadget struct
                of a DiskObject to to specify the revision # of the .info file.
                0 - pre V1.4, 1 - V1.4.  Now that I've read in all of pre V1.4
                info I can read in any additional info in a compatible way.  Note:
                I cannot use the Version # of a .info file to set the version #
                since pre V1.4 icon.librarys will not load any .info file that
                does not have a version # of 1.  Bummer! :^(
            */
            revision = ((int)icon->do_Gadget.UserData) & WB_DISKREVISIONMASK;
            if (revision && (revision <= WB_DISKREVISION) && dd)
            {
                DP(("GIC: V2.0 drawerdata\n"));
                if (srcdobj)
                {
                    DP(("GIC: copying %ld bytes from $%lx to $%lx\n",
                        DRAWERDATAFILESIZE - OLDDRAWERDATAFILESIZE,
                        &srcdobj->do_DrawerData->dd_Flags, &dd->dd_Flags));
                    memcpy(&dd->dd_Flags,&srcdobj->do_DrawerData->dd_Flags,DRAWERDATAFILESIZE - OLDDRAWERDATAFILESIZE);
                }
                else
                {
                    if (!IRead(file,&dd->dd_Flags,DRAWERDATAFILESIZE-OLDDRAWERDATAFILESIZE))
                    {
                    UBYTE *ptr=(UBYTE *)&dd->dd_Flags;

                        DP(("GetIcon: (%s) could not read %ld new drawerdata bytes\n",name,DRAWERDATAFILESIZE-OLDDRAWERDATAFILESIZE));

                        /* could not read new data, clear fields for safety */
                        for (len=0;len<(DRAWERDATAFILESIZE-OLDDRAWERDATAFILESIZE);ptr[len++]=0);
                    }
                }
            }
        }
    }
    /*
     * Save this error as the following may clear it
     */
    len=IoErrStub();

    if (file) CloseStub(file);

    /*
     * Restore the error that happened...
     */
    setResult2(len);

    DP(("IGetIconCom: exit, returning $%lx\n", retval));
    return(retval);
}

/*
******i icon.library/GetWBObject *********************************************
*
*   NAME
*	GetWBObject - read in a Workbench object from disk.
*
*   SYNOPSIS
*	wbobject = GetWBObject(name)
*         D0                   A0
*
*	struct OldWBObject * = char *;
*
*   FUNCTION
*       This routine reads in a Workbench object in from disk.  The
*	name parameter will have a ".info" postpended to it, and the
*	info file of that name will be read.  If the call fails,
*	it will return zero.  The reason for the failure may be obtained
*	via IoErr().
*
*	This routine is intended only for internal users that can
*	track changes to the Workbench.
*
*   INPUTS
*	name -- name of the object (pointer to a character string).
*
*   RESULTS
*	wbobject -- the Workbench object in question
*
*   SEE ALSO
*
*   BUGS
*	None
*
******************************************************************************
*/
struct OldWBObject *IGetWBObject( char *name )
{
struct OldWBObject *obj;
struct DiskObject dobj;

    MP(("IGetWBObject: enter, name=$%lx (%s)\n", name, name));

    if (obj=AllocWBObjectStub())
    {
        if (IGetIconCommon(name,&dobj,&obj->wo_FreeList,NULL,35))
        {
            obj->wo_Gadget	= dobj.do_Gadget;
            obj->wo_Type	= dobj.do_Type;
            obj->wo_DefaultTool	= dobj.do_DefaultTool;
            obj->wo_ToolTypes	= dobj.do_ToolTypes;
            obj->wo_CurrentX	= dobj.do_CurrentX;
            obj->wo_CurrentY	= dobj.do_CurrentY;
            obj->wo_DrawerData	= (void *)(dobj.do_DrawerData);
            obj->wo_ToolWindow	= dobj.do_ToolWindow;
            obj->wo_StackSize	= dobj.do_StackSize;

            if (obj->wo_DrawerData)
            {
                ((struct OldNewDD *)(obj->wo_DrawerData))->dd_Object = (void *)obj;
            }
        }
        else
        {
            MP(("IGetWBObject: error, could not IOldGetIcon\n"));
            FreeWBObjectStub(obj);
            obj=NULL;
        }
    }

#ifdef	DEBUGGING
    else
    {
	MP(("IGetWBObject: error, could not IAllocWBObject\n"));
    }
#endif	/* DEBUGGING */

    MP(("IGetWBObject: exit, obj=$%lx (%s)\n",obj, obj));
    return(obj);
}


/*
******i icon.library/PutWBObject *********************************************
*
*   NAME
*	PutWBObject - write out a Workbench object to disk.
*
*   SYNOPSIS
*	status = PutWBObject( name, object )
*         D0                   A0     A1
*
*	BOOL = char *, struct OldWBObject *;
*
*   FUNCTION
*       This routine writes a Workbench object out to disk.  The
*	name parameter will have a ".info" postpended to it, and
*	that file name will have the disk-resident information
*	written into it.  If the call fails, it will return a zero.
*	The reason for the failure may be obtained via IoErr().
*
*	This routine is intended only for internal users that can
*	track changes to the Workbench.
*
*   INPUTS
*	name -- name of the object (pointer to a character string)
*	object -- the Workbench object to be written out
*
*   RESULTS
*	status -- TRUE if call succeeded, else FALSE.
*
*   SEE ALSO
*
*   BUGS
*	None
*
******************************************************************************
*/
BOOL IPutWBObject( char *name, struct OldWBObject *object )
{
struct DiskObject dobj;
BOOL result;

    MP(("IPutWBObject: enter, name=$%lx (%s), obj=$%lx (%s)\n",name, name, object, object));

    dobj.do_Magic = WB_DISKMAGIC;
    dobj.do_Version = WB_DISKVERSION;
    dobj.do_Gadget = object->wo_Gadget;	/* structure assignment */
    dobj.do_Gadget.UserData = (APTR)((((int)dobj.do_Gadget.UserData) & ~WB_DISKREVISIONMASK) | WB_DISKREVISION);
    dobj.do_ToolTypes = object->wo_ToolTypes;
    dobj.do_Type = object->wo_Type;
    dobj.do_CurrentX = object->wo_CurrentX;
    dobj.do_CurrentY = object->wo_CurrentY;
    dobj.do_DrawerData = object->wo_DrawerData;
    dobj.do_DefaultTool = object->wo_DefaultTool;
    dobj.do_ToolWindow = object->wo_ToolWindow;
    dobj.do_StackSize = object->wo_StackSize;

    result = PutIconStub(name, &dobj);

    MP(("IPutWBObject: exit, returning $%lx\n", result));
    return(result);
}

/*
******i icon.library/GetIcon *************************************************
*
*   NAME
*       GetIcon - read in a DiskObject structure from disk.
*
*   SYNOPSIS
*       status = GetIcon( name, icon, free )
*         D0               A0    A1    A2
*
*	long = char *, struct DiskObject *, struct FreeList *;
*
*   FUNCTION
*       This routine reads in a DiskObject structure, and its
*	associated information.  All memory will be automatically
*	allocated, and stored in the specified FreeList.  The file
*	name of the info file will be the name parameter with a
*	".info" postpended to it.  If the call fails, a zero will
*	be returned.  The reason for the failure may be obtained
*	via IoErr().
*
*	Users are encouraged to use GetDiskObject instead of this
*	routine.  This routine will fail if the icon is not a
*	"version one" icon.
*
*   INPUTS
*	name -- name of the object (pointer to a character string)
*	icon -- a pointer to a DiskObject
*	free -- a pointer to a FreeList
*
*   RESULTS
*	status -- non-zero if the call succeeded.
*
*   SEE ALSO
*
*   BUGS
*	None
*
******************************************************************************
*/
LONG IGetIcon( char *name, struct DiskObject *icon, struct FreeList *free )
{
int result;

    DP(("IGetIcon: enter, name=$%lx (%s), icon=$%lx, free=$%lx\n",name, name, icon, free));
    /* V1.4 or later */
    result = IGetIconCommon(name, icon, free, NULL, 36);
    DP(("IGetIcon: exit, returning $%lx\n", result));
    return(result);
}

LONG IGetIconFromDobj( char *name, struct DiskObject *icon, struct FreeList *free, struct DiskObject *srcdobj )
{
int result;

    DP(("IGetIconFromDobj: enter, name=$%lx (%s), icon=$%lx, free=$%lx, srcdobj=$%lx\n", name, name, icon, free, srcdobj));
    /* V1.4 or later */
    result = IGetIconCommon(name, icon, free, srcdobj, 36);
    DP(("IGetIconFromDobj: exit, returning $%lx\n", result));
    return(result);
}

/*
******i icon.library/PutIcon *************************************************
*
*   NAME
*       PutIcon - write out a DiskObject to disk.
*
*   SYNOPSIS
*       status = PutIcon( name, icon )
*         D0               A0    A1
*
*	BOOL PutIcon(char *, struct DiskObject *);
*
*   FUNCTION
*       This routine writes out a DiskObject structure, and its
*	associated information.  The file name of the info file
*	will be the name parameter with a ".info" postpended to it.
*	If the call fails, a zero will be returned.  The reason for
*	the failure may be obtained via IoErr().
*
*	PutDiskObject and PutIcon are functionally identical.
*	They are both provided so there is a Put/Get/Free triple
*	for disk objects.
*
*	Users are encouraged to use PutDiskObject instead of this
*	routine.  This routine assumes that the icon is a "version
*	one" icon.
*
*   INPUTS
*	name -- name of the object
*	icon -- a pointer to a DiskObject
*
*   RESULTS
*	status -- TRUE if the call succeeded else FALSE
*
*   SEE ALSO
*
*   BUGS
*	None
*
******************************************************************************
*/
BOOL IPutIcon(char *name, struct DiskObject *romicon)
{
struct DiskObject *icon;
BOOL result=FALSE;
LONG file;
LONG len;
LONG olderr;

    MP(("IPutIcon: enter, name=$%lx (%s), icon=$%lx\n",name, name, romicon));

    if (icon=AllocVecStub(sizeof(struct DiskObject),MEMF_PUBLIC))
    {
        *icon = *romicon;	/* structure copy */

        if (file=OpenIcon(name,MODE_NEWFILE))
        {
            MP(("PutIcon: '%s', Magic=%lx, Version=%ld, UserData=%ld\n",name,icon->do_Magic,icon->do_Version,((int)icon->do_Gadget.UserData) & WB_DISKREVISIONMASK));

            if ((icon->do_Gadget.Flags & GADGHIGHBITS)==GADGBACKFILL)
            {   /* this is a phony image */
                icon->do_Gadget.SelectRender=NULL;
            }

            /* write out the drawer data */
            if (!IWrite(file,icon,sizeof(struct DiskObject)))
            {
                MP(("IPutIcon: error, could not write DiskObject\n"));
            }
	    else
	    {
	        result=TRUE;	/* Now assume we worked... */

                if (icon->do_DrawerData)
                {
                    if (!IWrite(file,icon->do_DrawerData,OLDDRAWERDATAFILESIZE))
                    {
                        MP(("IPutIcon: error, could not write olddrawerdata\n"));
                        result=FALSE;
                    }
                }

		if (result)
		{
                    /* deal with the imagery */
                    if( icon->do_Gadget.Flags & GADGIMAGE )
                    {
                        if (!putImage(file,icon->do_Gadget.GadgetRender))
                        {
                            MP(("IPutIcon: error, could not putImage(GadgetRender)\n"));
                            result=FALSE;
                        }
                        else if (!putImage(file,icon->do_Gadget.SelectRender))
                        {
                            MP(("IPutIcon: error, could not putImage(SelectRender)\n"));
			    result=FALSE;
                        }
                    }
                    else
                    {
	            	setResult2(ERROR_OBJECT_WRONG_TYPE);
	            	result=FALSE;
	            }
		}

		/* We should check for icon text... */

		if (result)
		{
                    if (icon->do_DefaultTool)
                    {
                        len = strlen(icon->do_DefaultTool)+1;
                        if (!IWrite(file,&len,sizeof(LONG)))
                        {
                            MP(("IPutIcon: error, could not write length of D.T.\n"));
                            result=FALSE;
                        }
                        else if (!IWrite(file,icon->do_DefaultTool,len))
                        {
                            MP(("IPutIcon: error, could not write D.T.\n"));
                            result=FALSE;
                        }
                    }
		}

		if (result)
		{
                    if (icon->do_ToolTypes)
                    {
                    char **sp;

                        /* write the number of strings */
                        for (sp=icon->do_ToolTypes,len=sizeof(char *);*sp;sp++,len+=sizeof(char *));
                        if (!IWrite(file,&len,sizeof(LONG)))
                        {
                            MP(("IPutIcon: error, could not write length of TT\n"));
                            result=FALSE;
                        }
                        else
                        {
                            /* write each string */
                            for (sp=icon->do_ToolTypes;*sp;sp++) if (result)
                            {
                                len=strlen(*sp)+1;
                                if (!IWrite(file,&len,sizeof(LONG))) result=FALSE;
                                else if (!IWrite( file, *sp, len )) result=FALSE;
			    }

#ifdef	DEBUGGING
                            if (!result) MP(("IPutIcon: error, could not write TT\n"));
#endif	/* DEBUGGING */

                        }
                    }
		}

		if (result)
		{
                    if (icon->do_ToolWindow)
                    {
                        len=strlen(icon->do_ToolWindow)+1;
                        if (!Write(file,&len,sizeof(LONG)))
                        {
                            MP(("IPutIcon: error, could not write length of TW\n"));
                            result=FALSE;
                        }
                        else if (!IWrite(file,icon->do_ToolWindow,len))
                        {
                            MP(("IPutIcon: error, could not write TW\n"));
                            result=FALSE;
                        }
                    }
		}

		if (result)
		{
                    /*
                     *  New for V1.4.  Although I've extended the size of a DrawerData struct,
                     *  I cannot write out the entire structure in one go else new .info
                     *  files will not be able to be read by pre V1.4 icon.libraries.  Instead
                     *  I use the lower 8 bits of the UserData field in the Gadget struct
                     *  of a DiskObject to to specify the revision # of the .info file.
                     *  0 - pre V1.4, 1 - V1.4.  Now that I've written out all of pre V1.4
                     *  info I can write out any additional info in a compatible way.  Note:
                     *  I cannot use the Version # of a .info file to set the version #
                     *  since pre V1.4 icon.librarys will not load any .info file that
                     *  does not have a version # of 1.  Bummer! :^(
                     */
                    if (icon->do_DrawerData)
                    {

#ifdef DEBUGGING
#define TMP_revision (((int)icon->do_Gadget.UserData) & WB_DISKREVISIONMASK)

                        if (TMP_revision != WB_DISKREVISION)
                        {
                            DP(("PutIcon: Warning: revision=%ld, WB_DISKREVISION=%ld\n",TMP_revision,WB_DISKREVISION));
                        }

                        if (!IWrite(file,&icon->do_DrawerData->dd_Flags,DRAWERDATAFILESIZE-OLDDRAWERDATAFILESIZE))
                        {
                            DP(("IPutIcon: could not write %ld new drawerdata bytes\n",DRAWERDATAFILESIZE-OLDDRAWERDATAFILESIZE));
                        }
#else
			IWrite(file,&icon->do_DrawerData->dd_Flags,DRAWERDATAFILESIZE-OLDDRAWERDATAFILESIZE);
#endif	/* DEBUGGING */

                    }
                }
	    }

	    /*
	     * Save this error as the following may clear it
	     */
	    olderr=IoErrStub();

            CloseStub(file);

	    /*
	     * Check if there was an error, and if so, delete the icon...
	     */
	    {
	    char *buf;


		if (buf=GetOtherName(name))
		{
		    DP((" %s\n",buf));
		    if (result) SetProtectionStub(buf,FIBF_EXECUTE);
		    else
		    {
		    	/*
		    	 * There was an error, so delete the incorrect .info file
		    	 *
		    	 * Mote that there is no length checking hack here since
		    	 * the icon was checked before in OpenIcon()...
		    	 */
			DP(("IPutIcon: error during write, deleting"));
			DeleteFileStub(buf);
		    }
		    FreeVecStub(buf);
		}

#ifdef	DEBUGGING
		else DP(("\nIPutIcon: no memory to setprotection/delete invalid icon\n"));
#endif	/* DEBUGGING */
	    }

	    /*
	     * Restore the error that happened...
	     */
	    setResult2(olderr);
        }

#ifdef	DEBUGGING
        else MP(("IPutIcon: error, could not open %s\n", name));
#endif	/* DEBUGGING */

        FreeVecStub(icon);
    }

#ifdef	DEBUGGING
    else MP(("PI: no mem for disk object\n"));
#endif	/* DEBUGGING */

    MP(("IPutIcon: exit, returning $%lx\n",result));

    return(result);

}

/*
******* icon.library/FreeFreeList ********************************************
*
*   NAME
*       FreeFreeList - free all memory in a free list.
*
*   SYNOPSIS
*       FreeFreeList(free)
*                     A0
*
*	void FreeFreeList(struct FreeList *);
*
*   FUNCTION
*	This routine frees all memory in a free list, and the
*	free list itself.  It is useful for easily getting
*	rid of all memory in a series of structures.  There is
*	a free list in a Workbench object, and this contains
*	all the memory associated with that object.
*
*	A FreeList is a list of MemList structures.  See the
*	MemList and MemEntry documentation for more information.
*
*	If the FreeList itself is in the free list, it must be
*	in the first MemList in the FreeList.
*
*   INPUTS
*	free -- a pointer to a FreeList structure
*
*   RESULTS
*	None
*
*   SEE ALSO
*	AllocEntry(), FreeEntry(), AddFreeList()
*
*   BUGS
*	None
*
******************************************************************************
*/
void IFreeFreeList( struct FreeList *freelist )
{
    struct Node *ln, *pred;

    DP(("IFreeFreeList: enter, freelist=$%lx\n", freelist));

    if (freelist) if (ln=freelist->fl_MemList.lh_TailPred)
    {
        while (pred=ln->ln_Pred)
        {
            DP(("Freeing %lx    Next=%lx\n",ln,pred));
            FreeEntry((void *)ln);
            ln=pred;
        }
    }
    MP(("IFreeFreeList: exit\n"));
}

/*
******i icon.library/FreeWBObject ********************************************
*
*   NAME
*       FreeWBObject - free all memory in a Workbench object.
*
*   SYNOPSIS
*       FreeWBObject( obj )
*                     A0
*
*	void = struct OldWBObject *;
*
*   FUNCTION
*       This routine frees all memory in a Workbench object, and the
*	object itself.  It is implemented via FreeFreeList().
*
*	AllocWBObject() takes care of all the initialization required
*	to set up the objects free list.
*
*	This routine is intended only for internal users that can
*	track changes to the Workbench.
*
*   INPUTS
*	free -- a pointer to a FreeList structure
*
*   RESULTS
*	None
*
*   SEE ALSO
*	AllocEntry(), FreeEntry(), AllocWBObject(), FreeFreeList()
*
*   BUGS
*	None
*
******************************************************************************
*/
void IFreeWBObject( struct OldWBObject *object )
{
    MP(("IFreeWBObject: enter, obj=$%lx (%s)\n",object, object));
    FreeFreeListStub(&object->wo_FreeList);
    FreeMemStub(object,sizeof(struct OldWBObject));
    MP(("IFreeWBObject: exit\n"));
}

/*
******i icon.library/AllocWBObject *******************************************
*
*   NAME
*       AllocWBObject - allocate a Workbench object.
*
*   SYNOPSIS
*       object = AllocWBObject().
*         D0
*
*	struct OldWBObject *;
*
*   FUNCTION
*       This routine allocates a Workbench object, and initializes
*	its free list.  A subsequent call to FreeWBObject will
*	free all of its memory.
*
*	If memory cannot be obtained, a NULL is returned.
*
*	This routine is intended only for internal users that can
*	track changes to the Workbench.
*
*   INPUTS
*	None
*
*   RESULTS
*	object - a pointer to the WBObject (if memory is available) else NULL
*
*   SEE ALSO
*	AllocEntry(), FreeEntry(), FreeWBObject()
*
*   BUGS
*	None
*
******************************************************************************
*/
struct OldWBObject *IAllocWBObject(VOID)
{
struct OldWBObject *obj;

    MP(("IAllocWBObject: enter\n"));

    if (obj=(struct OldWBObject *)AllocMemStub(sizeof(struct OldWBObject),MEMF_CLEAR))
    {
        NewList(&obj->wo_FreeList.fl_MemList);

        if (!replenishMem(&obj->wo_FreeList))
        {
            FreeMemStub(obj,sizeof(struct OldWBObject));
            obj=NULL;
        }
    }

    MP(("IAllocWBObject: exit, returning $%lx\n", obj));
    return(obj);
}

/*
******* icon.library/AddFreeList *********************************************
*
*   NAME
*       AddFreeList - add memory to a free list.
*
*   SYNOPSIS
*       status = AddFreeList(free, mem, len)
*         D0                  A0    A1   A2
*
*	BOOL AddFreeList(struct FreeList *, APTR, ULONG);
*
*   FUNCTION
*	This routine adds the specified memory to the free list.
*	The free list will be extended (if required).  If there
*	is not enough memory to complete the call, a null is returned.
*
*	Note that AddFreeList does NOT allocate the requested memory.
*	It only records the memory in the free list.
*
*   INPUTS
*	free -- a pointer to a FreeList structure
*	mem -- the base of the memory to be recorded
*	len -- the length of the memory to be recorded
*
*   RESULTS
*	status -- TRUE if the call succeeded else FALSE;
*
*   SEE ALSO
*	AllocEntry(), FreeEntry(), FreeFreeList()
*
*   BUGS
*	None
*
******************************************************************************
*/
BOOL IAddFreeList( struct FreeList *free, APTR mem, ULONG len )
{
struct MemList *tail;
BOOL result=TRUE;

    MP(("IAddFreeList: enter, free=$%lx, mem=$%lx, len=$%lx\n",free,mem,len));
    if( --free->fl_NumFree <= 0 )
    {
	if (!replenishMem(free)) result=FALSE;
	--free->fl_NumFree;
    }

    if (result)
    {
        tail=(struct MemList *)free->fl_MemList.lh_TailPred;

        tail->ml_ME[free->fl_NumFree].me_Un.meu_Addr=(APTR)mem;
        tail->ml_ME[free->fl_NumFree].me_Length=len;
    }
    MP(("IAddFreeList: exit %ld\n",(LONG)result));
    return(result);
}

struct DiskObject *IAllocateDiskObject(VOID)
{
    struct DiskObject *dobj;
    struct FreeList *fl;

    /*
	FreeDiskObject ASSUMES that a 'struct DiskObject' and a
	'struct FreeList' were allocated together.
    */

    DP(("IAllocateDiskObject: enter\n"));
    if (dobj = AllocVecStub(sizeof(struct DiskObject) + sizeof(struct FreeList), MEMF_CLEAR|MEMF_PUBLIC))
    {
	fl = (struct FreeList *)(&dobj[1]);
	NewList(&fl->fl_MemList);

	if (!replenishMem(fl))
	{
	    FreeVecStub(dobj);
	    dobj=NULL;
	}
    }

    DP(("IAllocateDiskObject: exit, returning $%lx\n", dobj));
    return(dobj);
}

/*
******* icon.library/DeleteDiskObject ****************************************
*
*   NAME
*       DeleteDiskObject - Delete a Workbench disk object from disk.     (V37)
*
*   SYNOPSIS
*       result = DeleteDiskObject(name)
*         D0                      A0
*
*	BOOL DeleteDiskObject(char *);
*
*   FUNCTION
*	This routine will try to delete a Workbench disk object from disk.
*	The name parameter will have a ".info" postpended to it, and the
*	info file of that name will be deleted.  If the call fails,
*	it will return zero.  The reason for the failure may be obtained
*	via IoErr().
*
*	This call also updates the Workbench screen if needed.
*
*	Using this routine protects you from any future changes to
*	the way icons are stored within the system.
*
*   INPUTS
*	name -- name of the object (char *)
*
*   RESULTS
*	result -- TRUE if it worked, false if not.
*
*   EXAMPLE
*
*	error=NULL;
*
*	\*Check if you have the right library version*\
*
*	if (((struct Library *)IconBase)->lib_Version > 36)
*	{
*		if (!DeleteDiskObject(name)) error=IoErr();
*	}
*	else
*	{
*		\* Delete name plus ".info" *\
*	}
*
*	if (error)
*	{
*		\* Do error routine...*\
*	}
*
*   SEE ALSO
*	PutDiskObject(), GetDiskObject(), FreeDiskObject()
*
*   BUGS
*	None
*
******************************************************************************
*/
BOOL IDeleteDiskObject(char *name)
{
BOOL result=FALSE;
char *tmp;

    if (tmp=GetOtherName(name))
    {
    BPTR parentlock;
    LONG error=NULL;
    char *p;
    char c;


	/* Make a lock onto the directory of the file given */
	p=PathPartStub(tmp);
	c=*p;
	*p='\0';	/* Make tmp be the path... */
	parentlock=LockStub(tmp,ACCESS_READ);
	*p=c;

	if (!(result=DeleteFileStub(tmp))) error=IoErrStub(); /* Save the error */

	/*
	 * Update the workbench if it worked or the file was not found.
	 */
	if ((result) || (error==ERROR_OBJECT_NOT_FOUND))
	{
	    MyUpdateWorkbench(name,parentlock,FALSE);
	}

	UnLockStub(parentlock);
	FreeVecStub(tmp);
	setResult2(error);	/* Restore the error */
    }
    return(result);
}

/*
******* icon.library/GetDiskObject *******************************************
*
*   NAME
*       GetDiskObject - read in a Workbench disk object from disk.
*
*   SYNOPSIS
*       diskobj = GetDiskObject(name)
*         D0                      A0
*
*	struct DiskObject *GetDiskObject(char *);
*
*   FUNCTION
*	This routine reads in a Workbench disk object in from disk.  The
*	name parameter will have a ".info" postpended to it, and the
*	info file of that name will be read.  If the call fails,
*	it will return zero.  The reason for the failure may be obtained
*	via IoErr().
*
*	Using this routine protects you from any future changes to
*	the way icons are stored within the system.
*
*	A FreeList structure is allocated just after the DiskObject
*	structure; FreeDiskObject makes use of this to get rid of the
*	memory that was allocated.
*
*   INPUTS
*	name -- name of the object (char *) or NULL if you just want a
*		DiskObject structure allocated for you (useful when
*		calling AddAppIcon in workbench.library).
*
*   RESULTS
*	diskobj -- the Workbench disk object in question
*
*   SEE ALSO
*	GetDiskObjectNew(), PutDiskObject(), DeleteDiskObject(),
*	FreeDiskObject()
*
*   BUGS
*	None
*
******************************************************************************
*/
struct DiskObject *IGetDiskObject( char *name )
{
    void IFreeDiskObject();
    struct DiskObject *dobj;

    DP(("IGetDiskObject: enter, name=%s\n", name));
    if (dobj=IAllocateDiskObject())
    {
	if (name)
	{
	    if (!GetIconStub(name, dobj, (struct FreeList *)(&dobj[1])))
	    {
		DP(("Freeing disk object\n"));
		FreeDiskObjectStub(dobj);
		dobj=NULL;
	    }
	}
    }
    DP(("IGetDiskObject: exit, returning $%lx\n", dobj));
    return(dobj);
}


/*
******* icon.library/PutDiskObject *******************************************
*
*   NAME
*       PutDiskObject - write out a DiskObject to disk.
*
*   SYNOPSIS
*       status = PutDiskObject(name, diskobj)
*         D0                    A0      A1
*
*	BOOL PutDiskObject(char *, struct DiskObject *);
*
*   FUNCTION
*	This routine writes out a DiskObject structure, and its
*	associated information.  The file name of the info
*	file will be the name parameter with a
*	".info" postpended to it.  If the call fails, a zero will
*	be returned.  The reason for the failure may be obtained
*	via IoErr().
*
*	As of release V2.0, PutDiskObject (if successful) notifies workbench
*	han an icon has been created/modified.
*
*	Using this routine protects you from any future changes to
*	the way icons are stored within the system.
*
*   INPUTS
*	name -- name of the object (pointer to a character string)
*	diskobj -- a pointer to a DiskObject
*
*   RESULTS
*	status -- TRUE if the call succeeded else FALSE
*
*   NOTES
*	It is recommended that if you wish to copy an icon from one place
*	to another than you use GetDiskObject() and PutDiskObject()
*	and do not copy them directly.
*
*   SEE ALSO
*	GetDiskObject(), FreeDiskObject(), DeleteDiskObject()
*
*   BUGS
*	None
*
******************************************************************************
*/
BOOL IPutDiskObject( char *name, struct DiskObject *diskobj )
{
BOOL result;

    MP(("IPutDiskObject: enter, name=$%lx (%s), diskobj=$%lx\n",name, name, diskobj));

    if (result=PutIconStub(name,diskobj))
    {
    char *tmp;

	if (tmp=GetOtherName(name))
	{
	BPTR lock;

	    if (lock=LockStub(tmp,ACCESS_READ))
	    {
	    BPTR parentlock;

		parentlock=ParentDirStub(lock);
		UnLockStub(lock);

		MyUpdateWorkbench(name,parentlock,TRUE);

		UnLockStub(parentlock);
	    }
	}
	FreeVecStub(tmp);
    }

    return(result);
}

/*
******* icon.library/FreeDiskObject ******************************************
*
*   NAME
*       FreeDiskObject - free all memory in a Workbench disk object.
*
*   SYNOPSIS
*       FreeDiskObject(diskobj)
*                        A0
*
*	void FreeDiskObject(struct DiskObject *);
*
*   FUNCTION
*	This routine frees all memory in a Workbench disk object, and the
*	object itself.  It is implemented via FreeFreeList().
*
*	GetDiskObject() takes care of all the initialization required
*	to set up the object's free list.  This procedure may ONLY
*	be called on a DiskObject allocated via GetDiskObject().
*
*   INPUTS
*	diskobj -- a pointer to a DiskObject structure
*
*   RESULTS
*	None
*
*   SEE ALSO
*	GetDiskObject(), PutDiskObject(), DeleteDiskObject(), FreeFreeList()
*
*   BUGS
*	None
*
******************************************************************************
*/
void IFreeDiskObject(struct DiskObject *diskobj)
{
    DP(("IFreeDiskObject: enter, diskobj=$%lx\n", diskobj));
    /* the free list is right after the disk object structure */
    FreeFreeListStub((struct FreeList *)(&diskobj[1]));

    /*
     * It was allocated in AllocDiskObject...
     */
    DP(("IFreeDiskObject: Calling FreeVec\n"));
    FreeVecStub(diskobj);
    MP(("IFreeDiskObject: exit\n"));
}

/*
******* icon.library/FindToolType ********************************************
*
*   NAME
*       FindToolType - find the value of a ToolType variable.
*
*   SYNOPSIS
*       value = FindToolType(toolTypeArray, typeName)
*         D0                      A0           A1
*
*	char *FindToolType(char **, char *);
*
*   FUNCTION
*	This function searches a tool type array for a given entry,
*	and returns a pointer to that entry.  This is useful for
*	finding standard tool type variables.  The returned
*	value is not a new copy of the string but is only
*	a pointer to the part of the string after typeName.
*
*   INPUTS
*	toolTypeArray - an array of strings (char **).
*	typeName - the name of the tooltype entry (char *).
*
*   RESULTS
*	value - a pointer to a string that is the value bound to typeName,
*		or NULL if typeName is not in the toolTypeArray.
*
*   EXAMPLE
*	Assume the tool type array has two strings in it:
*	    "FILETYPE=text"
*	    "TEMPDIR=:t"
*
*	FindToolType( toolTypeArray, "FILETYPE" ) returns "text"
*	FindToolType( toolTypeArray, "filetype" ) returns "text"
*	FindToolType( toolTypeArray, "TEMPDIR" )  returns ":t"
*	FindToolType( toolTypeArray, "MAXSIZE" )  returns NULL
*
*   SEE ALSO
*	MatchToolValue()
*
*   BUGS
*	None
*
******************************************************************************
*/
char *IFindToolType( char **array, char *name )
{
int len;
char *string=NULL;

    MP(("IFindToolType: enter, array=$%lx, name=$%lx (%s)\n",array,name,name));

    /* spackling paste for sloppy programmers */
    if (array && name)
    {
    char *tmp;

        len=strlen(name);

        /* search all the strings in the array */
        while ((tmp=*array++) && (!string))
        {
            if (!stricmpn(tmp,name,len))
            {
                tmp=&tmp[len];
                if (*tmp)
                {
                    if (*tmp=='=') tmp++;
                    else tmp=NULL;
                }
                string=tmp;
            }
        }
    }

    MP(("IFindToolType: exit, returning %ld\n",string));
    return(string);
}

char *index(char *from,char value)
{
char	*ret=NULL;

	while ((!ret)&&(*from))
	{
		if (*from==value) ret=from;
		from++;
	}
	return(ret);
}

/*
******* icon.library/MatchToolValue ******************************************
*
*   NAME
*       MatchToolValue - check a tool type variable for a particular value.
*
*   SYNOPSIS
*       result = MatchToolValue(typeString, value)
*         D0                        A0        A1
*
*	BOOL MatchToolValue(char *, char *);
*
*   FUNCTION
*	MatchToolValue is useful for parsing a tool type value for
*	a known value.  It knows how to parse the syntax for a tool
*	type value (in particular, it knows that '|' separates
*	alternate values).  Note that the parsing is case insensitive.
*
*   INPUTS
*	typeString - a ToolType value (as returned by FindToolType)
*	value - you are interested if value appears in typeString
*
*   RESULTS
*	result - TRUE if the value was in typeString else FALSE.
*
*   EXAMPLE
*	Assume there are two type strings:
*	    type1 = "text"
*	    type2 = "a|b|c"
*
*	MatchToolValue( type1, "text" ) returns TRUE
*	MatchToolValue( type1, "TEXT" ) returns TRUE
*	MatchToolValue( type1, "data" ) returns FALSE
*	MatchToolValue( type2, "a" ) returns TRUE
*	MatchToolValue( type2, "b" ) returns TRUE
*	MatchToolValue( type2, "d" ) returns FALSE
*	MatchToolValue( type2, "a|b" ) returns FALSE
*
*   SEE ALSO
*	FindToolType()
*
*   BUGS
*	None
*
******************************************************************************
*/
BOOL IMatchToolValue(char *type, char *value)
{
int vallen = strlen(value);
int typelen;
char *bar;
BOOL result=FALSE;

    MP(("IMatchToolValue: enter, type=$%lx (%s), value=$%lx (%s)\n",type,type,value,value));

    while (type)
    {
	bar = (char *)index(type, '|');
	if (bar) typelen = bar - type;
	else typelen = strlen(type);

	if (typelen == vallen)
	{
	    if (!stricmpn(type, value, vallen))
	    {
		result=TRUE;
		bar=NULL;
	    }
	}
	if (bar) type = bar + 1; /* advance past the bar */
	else type = 0;
    }

    MP(("IMatchToolValue: exit, returning %ld\n",result));
    return(result);
}

int getnextnum(int c)
{
    MP(("getnextnum: c=%ld, ", c));
    if (c >= '0' && c <= '9') c-='0';
    else c=-1;

    MP(("returning %ld\n",c));
    return(c);
}

/*
******* icon.library/BumpRevision ********************************************
*
*   NAME
*       BumpRevision - reformat a name for a second copy.
*
*   SYNOPSIS
*       result = BumpRevision(newbuf, oldname)
*         D0                    A0      A1
*
*	char *BumpRevision(char *, char *);
*
*   FUNCTION
*	BumpRevision takes a name and turns it into a "copy_of_name".
*	It knows how to deal with copies of copies.  The routine
*	will truncate the new name to the maximum dos name size
*	(currently 30 characters).
*
*   INPUTS
*	newbuf - the new buffer that will receive the name
*		 (it must be at least 31 characters long).
*	oldname - the original name
*
*   RESULTS
*	result - a pointer to newbuf
*
*   EXAMPLE
*	oldname				 newbuf
*	-------				 ------
*	"foo"				 "copy_of_foo"
*	"copy_of_foo"			 "copy_2_of_foo"
*	"copy_2_of_foo"			 "copy_3_of_foo"
*	"copy_199_of_foo"		 "copy_200_of_foo"
*	"copy foo"			 "copy_of_copy foo"
*	"copy_0_of_foo"			 "copy_1_of_foo"
*	"012345678901234567890123456789" "copy_of_0123456789012345678901"
*
*   SEE ALSO
*
*   BUGS
*	None
*
******************************************************************************
*/
void sprintf(char *,char *, ...);
char *IBumpRevision(char *newname, char *name)
{
char *tmp=name;
long rev=0;
long extra=1;

    /* Check if the name starts with COPY */
    if ((!stricmpn(name,"copy ",5)) || (!stricmpn(name,"Copy_",5)))
    {
    long x;

        tmp=&name[5];

	while (!((x=getnextnum(*tmp))<0))
	{
	    extra=0;
	    rev=rev*10+x;
	    tmp++;
	}

	/* Skip past the second space/underscore */
	if (tmp!=&name[5]) tmp++;

	/* Check if we have an "of " in the string... */
        if (stricmpn(tmp,"of ",3) && stricmpn(tmp,"of_",3))
        {
            /* No of, so this is a first copy... */
            tmp=name;
        }
        else
	{
	    /* There is an "of " so, we pull out the real name... */
	    tmp=&tmp[3];
	}
    }

    strcpy(newname,"Copy_");

    rev+=1+extra;	/* Bump the revision */

    /* Check if this is the first copy... */
    if (name!=tmp) sprintf(&newname[5],"%ld_",rev);

    strcat(newname,"of_");
    for (rev=strlen(newname);rev<30;newname[rev++]=*tmp++);
    newname[30]='\0';

    return(newname);
}

/*
******* icon.library/GetDiskObjectNew ****************************************
*
*   NAME
*       GetDiskObjectNew - read in a Workbench disk object from disk.    (V36)
*
*   SYNOPSIS
*       diskobj = GetDiskObjectNew(name)
*         D0                      A0
*
*	struct DiskObject *GetDiskObjectNew(char *);
*
*   FUNCTION
*	This routine reads in a Workbench disk object in from disk.  The
*	name parameter will have a ".info" postpended to it, and the
*	info file of that name will be read.  If the call fails,
*	it will return zero.  The reason for the failure may be obtained
*	via IoErr().
*
*	Using this routine protects you from any future changes to
*	the way icons are stored within the system.
*
*	A FreeList structure is allocated just after the DiskObject
*	structure; FreeDiskObject makes use of this to get rid of the
*	memory that was allocated.
*
*	This call is functionally identical to GetDiskObject with one exception.
*	If its call to GetDiskObject fails, this function calls GetDefDiskObject.
*	This is useful when there is no .info file for the icon you are trying
*	to get a disk object for.  Applications that use workbench application
*	windows MUST use this call if they want to handle the user dropping an
*	icon (that doesn't have a .info file) on their window.  The V2.0
*	icon editor program is an example of a workbench application window
*	that uses this call.
*
*   INPUTS
*	name -- name of the object (char *) or NULL if you just want a
*		DiskObject structure allocated for you (useful when
*		calling AddAppIcon in workbench.library).
*
*   RESULTS
*	diskobj -- the Workbench disk object in question
*
*   SEE ALSO
*	FreeDiskObject(), GetDiskObject(), PutDiskObject(), DeleteDiskObject()
*
*   BUGS
*	None
*
******************************************************************************
*/
struct DiskObject *IGetDiskObjectNew(char *name)
{
struct DiskObject *dobj;

    if (!(dobj = GetDiskObjectStub(name)))
    {
    BPTR lock;

	/* David Berezowski - May/90 */
	if (lock = LockStub(name, ACCESS_READ))
	{
        struct FileInfoBlock *fib;

	    MP(("IGDO: trying to allocate mem for fib\n"));
	    if (fib = AllocVecStub(sizeof(struct FileInfoBlock),MEMF_PUBLIC))
	    {
		MP(("IGDO: trying to examine lock $%lx\n"));
		if (Examine(lock, fib))
		{
		LONG type;

		    if (fib->fib_DirEntryType > 0)
		    {
		    BPTR parentlock;

			if (parentlock = ParentDirStub(lock))
			{
			    MP(("IGDO: WBDRAWER\n"));
			    type = WBDRAWER;
			    UnLockStub(parentlock);
			}
			else
			{
			    MP(("IGDO: WBDISK\n"));
			    type = WBDISK;
			}
		    }
		    else
		    {
			if (fib->fib_Protection & FIBF_EXECUTE)
			{
			    MP(("IGDO: WBPROJECT\n"));
			    type = WBPROJECT;
			}
			else
			{
			    MP(("IGDO: WBTOOL\n"));
			    type = WBTOOL;
			}
		    }
		    MP(("IGDO: calling IGetDefDiskObject...\n"));
		    dobj = GetDefDiskObjectStub(type);
		}
		FreeVecStub(fib);
	    }
	    UnLockStub(lock);
	}
	/* kludge for missing 'disk.info' file */
	else if (!stricmpn(name, "disk", 4)) dobj=GetDefDiskObjectStub(WBDISK);
    }

    MP(("IGDO: returned $%lx\n", dobj));
    return(dobj);
}
