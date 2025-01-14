/*** classes.c *****************************************************************
 *
 * classes.c -- Intuition class/object/message scheme
 *
 *  $Id: classes.c,v 38.11 92/08/03 15:52:34 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


#include "intuall.h"
#include <exec/nodes.h>
#include <exec/memory.h>
#include "classusr.h"
#include "classes.h"
#include "gadgetclass.h"
#include "icclass.h"

#define DNO(x)	;	/* debug NewObject() */
#define DN(x)	;	/* debug Notify() */
#define D(x)	;
#define DMC(x)	;

/*** intuition.library/DoGadgetMethodA ***/

ULONG
DoGadgetMethodA( gad, win, req, message )
struct Gadget *gad;
struct Window *win;
struct Requester *req;
Msg message;
{
    lockFree("DoGadgetMethodA", LAYERINFOLOCK );
    lockFree("DoGadgetMethodA", LAYERROMLOCK );
    lockFree("DoGadgetMethodA", IBASELOCK );
    return ( doISM( itGADGETMETHOD, win, gad, message ) );
}


/* We arrive here inside the state machine */
ULONG
IDoGadgetMethodA( gad, win, message )
struct Gadget *gad;
struct Window *win;
Msg message;
{
    ULONG retval;
    struct Requester *req = NULL;
    struct GListEnv genv;
    struct GadgetInfo *ginfo = &genv.ge_GInfo;
    LONG thirdplace = FALSE;
    struct Requester *findGadgetRequest();

    /* Peter 2-Apr-92:  gListDomain() fails if the requester is not
     * found, or if the requester has no layer.  Failing for no layer
     * is correct for most other uses (which are typically rendering),
     * but that's wrong for this operation.  So if we have a layerless
     * requester, skip gListDomain() and proceed with ginfo = NULL
     * instead.
     * If gad is NULL, we return with zero.  That's compatible with
     * V37 behavior of SetGadgetAttrsA().
     */

    if ( !gad )
    {
	return( 0 );
    }

    /* Locking situation is very tricky here.  Clearly, we have
     * to lock the gadgets list to protect re-entrancy of boopsi
     * objects.  We are state-synchronized, which protects the boopsi
     * scrollraster sniffing stuff.  Ideally, we could take
     * the IBASELOCK too, but boopsi gadgets may need to
     * get the LAYERROM lock, which in fact ObtainGIRPort() does.
     */
    LOCKGADGETS();
    /* LOCKIBASE() -- Can't do this! */

    /* We don't have enough field in an InputToken to pass the
     * requester parameter through, so we just work it out
     * again here...
     */
    if ( TESTFLAG( gad->GadgetType, REQGADGET ) )
    {
	req = findGadgetRequest( gad, win );
    }

    if ( ( ! win ) || ( ( req ) && ( !req->ReqLayer ) ) )
    {
	ginfo = NULL;
    }
    else if ( !gListDomain( gad, win, &genv ) )
    {
	return ( 0 );
    }
    else
    {
	gadgetInfo( gad, &genv );
    }

    /* We need to supply GadgetInfo to different methods.
     * Unfortunately, the GadgetInfo is not at a fixed place
     * in the method's message.  For OM_NEW, OM_SET, OM_NOTIFY,
     * and OM_UPDATE, the GadgetInfo is in the third position.
     * All other methods must have the GadgetInfo in the second,
     * by example (and now by decree).
     */
    if ( ( message->MethodID == OM_NEW ) || ( message->MethodID == OM_SET ) ||
	( message->MethodID == OM_NOTIFY ) || ( message->MethodID == OM_UPDATE ) )
    {
	/* opSet is an example structure with GadgetInfo
	 * in the third position, so use that structure
	 * to install the GadgetInfo.
	 */
	((struct opSet *)message)->ops_GInfo = ginfo;
	thirdplace = TRUE;
    }
    else
    {
	/* gpInput is an example structure with GadgetInfo
	 * in the second position, so use that structure
	 * to install the GadgetInfo.
	 */
	((struct gpInput *)message)->gpi_GInfo = ginfo;
    }

    /* Now, let's invoke the method on the gadget! */
    retval = callGadgetHookPkt( gadgetHook(gad), gad, message );

    /* Now NULL out the GInfo pointer, since it's no longer valid,
     * and we'd hate for our caller to steal it!
     */
    if ( thirdplace )
    {
	((struct opSet *)message)->ops_GInfo = ginfo;
    }
    else
    {
	((struct gpInput *)message)->gpi_GInfo = ginfo;
    }

    /* UNLOCKIBASE() -- Can't do this! */
    UNLOCKGADGETS();

    return( retval );
}

/*** intuition.library/SetGadgetAttrsA ***/

/*
 * this one probably will need an actual gadget specific locking
 * or setup.  Keep this entry point separate from OM_SET until
 * you know more than I do now.
 *
 * This is applied only via outer level, to "true" gadget.
 *
 * We're going to let this go without window passed in, but
 * you get a NULL GInfo.
 */
SetGadgetAttrsA( g, window, req, taglist )
struct Gadget	*g;
struct Window *window;
struct Requester *req;
struct TagItem	*taglist;
{
    struct opSet setmsg;
    setmsg.MethodID = OM_SET;
    setmsg.ops_AttrList = taglist;

    return( DoGadgetMethodA( g, window, req, &setmsg ) );
    /* ZZZ: do I care about refresh return code? */
    /* ZZZ: maybe  I do the refresh call right here */
}

/*** intuition.library/SetAttrsA ***/

SetAttrsA( o, tags )
Object	*o;
struct TagItem	*tags;
{
    /* no GInfo in this call	*/
    return ( SendMessage( o, OM_SET, tags, NULL ) );
}

/*** intuition.library/GetAttr ***/

GetAttr( AttrID, o, StoragePtr )
ULONG	AttrID;
Object	*o;
ULONG	*StoragePtr;
{
    return ( SendMessage( o, OM_GET, AttrID, StoragePtr ) );
}

ULONG	initRootClass();
ULONG	initGadgetClass();
ULONG	initGGClass();
ULONG	initPropGadgetClass();
ULONG	initButtonGClass();
ULONG	initStrGadgetClass();
ULONG	initImageClass();
ULONG	initICClass();
ULONG	initModelClass();
ULONG	initFrameIClass();
ULONG	initITextIClass();
ULONG	initFramedButtonClass();
ULONG	initFillRectClass();
ULONG	initSysIClass();
ULONG	initPointerClass();


ULONG	(*classinitfuncs[])()= {
    initRootClass,
    initGadgetClass,
    initGGClass,
    initPropGadgetClass,
    initButtonGClass,
    initStrGadgetClass,
    initImageClass,
    initICClass,
    initModelClass,
    initFrameIClass,
    initITextIClass,
    initFramedButtonClass,
    initFillRectClass,
    initSysIClass,
    initPointerClass,
    NULL
};

initIClasses()
{
    ULONG	(**classfunc)();

    D( printf("initIClasses\n") );
    InitClassList();

    for ( classfunc = classinitfuncs; *classfunc; classfunc++ )
    {
	(**classfunc)();
    }
}

/* This is the central repository of class names, so that there
 * are no more duplicate strings throughout Intuition.
 */

UBYTE *ButtonGClassName = BUTTONGCLASS;
UBYTE *FillRectClassName = FILLRECTCLASS;
UBYTE *FrameIClassName = FRAMEICLASS;
UBYTE *FrButtonClassName = FRBUTTONCLASS;
UBYTE *GadgetClassName = GADGETCLASS;
UBYTE *GroupGClassName = GROUPGCLASS;
UBYTE *ICClassName = ICCLASS;
UBYTE *ImageClassName = IMAGECLASS;
UBYTE *ITextIClassName = ITEXTICLASS;
UBYTE *ModelClassName = MODELCLASS;
UBYTE *PropGClassName = PROPGCLASS;
UBYTE *RootClassName = ROOTCLASS;
UBYTE *StrGClassName = STRGCLASS;
UBYTE *SysIClassName = SYSICLASS;
UBYTE *PointerClassName = POINTERCLASS;


/*** intuition.library/NewObject ***/

Object   *
NewObjectA( cl, classid, tags )
Class	*cl;
ClassID classid;
struct TagItem	*tags;
{
    Class   *FindClass();
    Object  *o = NULL;

    DNO( printf("NewObjectA( %lx, %s, %lx)\n", cl, classid, tags ) );

    /* temporarily bump the object count to keep the
     * class from going away.  I don't want to have to
     * hold the ICLASSLOCK for the duration of
     * OM_NEW.
     */
    if ( cl )
    {
	cl->cl_ObjectCount++;
    }
    else 
    {
	lockClassList();
	if ( cl = FindClass( classid ) )
	{
	    cl->cl_ObjectCount++;
	}
	unlockClassList();
    }

    if ( cl )
    {
	DNO( printf("NewObjectA: class %s located at %lx\n", classid, cl ) );

	/* pass in opSet message, with NULL GInfo */
	o =  (Object *) CoerceMessage( cl, cl, OM_NEW, tags, NULL );

	/* ok, OM_NEW has incremented cl_ObjectCount, if appropriate	*/
	cl->cl_ObjectCount--;
    }
    return ( o );	/* might be NULL	*/
}

/*** intuition.library/DisposeObject ***/

DisposeObject( o )
Object	*o;
{
    /* SendMessage() stub in classface.asm handles o==NULL */
    SendMessage( o, OM_DISPOSE );
}

/*** intuition.library/NextObject ***/

/* 'optr' is a "black handle" that bears no resemblence to
 * the object handles added to the list.
 * Initialize before first call to (void *) List.lh_Head.
 */
Object	*
NextObject( lnptr )
struct Node	**lnptr;
{
    struct Node	*ln;
    struct Node	*succ;

    if ( (succ = (*lnptr)->ln_Succ) != NULL )
    {
	ln = *lnptr;
	*lnptr = succ;
	return ( BASEOBJECT( ln ) );
    }
    return ( NULL );
}


/** special deal for sending notify messages via IDCMP **/
sendNotifyIDCMP( msg )
struct opUpdate	*msg;
{
    struct IntuiMessage	*initIMsg();
    struct TagItem	*CloneTagItems();
    struct TagItem	*FindTagItem();

    struct Window	*w = NULL;
    struct IntuiMessage	*imsg;
    struct TagItem	*taglist;
    struct TagItem	*tagitem;
    ULONG		qualifier = 0;

    /* Unfortunately, I need this lock and I don't always have
     * it.  I need it because we're walking the window list
     * looking for free messages down in initIMsg().
     *
     * I can't obtain it here, because OM_NOTIFY might have
     * been sent while ObtainGIRPort() is held, which holds
     * the RPLOCK (which is bigger than IBASELOCK).
     *
     * I can't have gotten it in callGadgetHook() since
     * some methods need to lock layers.
     *
     * The callGadgetHook() function does hold the ISTATELOCK,
     * so we're not totally dead...
     *
     * ICK.
     */

    assertLock( "sendNotifyIDCMP", IBASELOCK );
    DN( printf( "sNIDCMP ginfo %lx\n",  msg->opu_GInfo ));

    if ( msg->opu_GInfo ) w = msg->opu_GInfo->gi_Window;

    DN( printf( "sNIDCMP window %lx\n",  msg->opu_GInfo->gi_Window ));

    DN( printf("test things: w %lx, port %lx, fl %lx\n",
	w, w->UserPort, w->IDCMPFlags ));

    /* check for window IDCMP port */
    if ( w && w->UserPort && TESTFLAG(w->IDCMPFlags, IDCMPUPDATE) )
    {
	/* set repeat qualifier and respect repeat queue
	 * for "interim" messages 
	 */
	if ( TESTFLAG( msg->opu_Flags, OPUF_INTERIM ) )
	{
	    qualifier = IEQUALIFIER_REPEAT;

	    if ( XWINDOW(w)->RptPending  < XWINDOW(w)->RptQueueLimit )
	    {
		XWINDOW(w)->RptPending++;
	    }
	    else
	    {
		DN( printf("repeat limit hit\n"));
		goto OUT;
	    }
	}

	if ( taglist = CloneTagItems( msg->opu_AttrList) )
	{

	    if ( imsg = initIMsg( IDCMPUPDATE, 0, NULL, w, NULL ) )
	    {
		/* fill in some fields	*/
		imsg->IAddress = (APTR) taglist;
		imsg->Qualifier |= qualifier;

		/* special way to set the Code field	*/
		if ( tagitem = FindTagItem( ICSPECIAL_CODE, taglist ) )
		{
		    imsg->Code = (USHORT) tagitem->ti_Data;
		}
	    
		DN( printf("putmsg %lx\n", imsg ));
		/* Peter 16-Oct-90:
		 * As a result of CloseWindowSafely(), there is a possibility
		 * of encountering a window with IDCMPFlags != NULL which
		 * nevertheless has a UserPort of NULL.  This closes that problem.
		 */
		PutMsgSafely( w->UserPort, imsg );
	    }
	    else
	    {
		/* dump the clone */
		DN( printf("no IDCMP free?\n"));
		FreeTagItems( taglist );
	    }
	}
    }
    DN( else printf("failed test\n"));
OUT:
}
