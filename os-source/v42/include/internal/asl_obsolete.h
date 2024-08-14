#ifndef	LIBRARIES_ASL_H
#define	LIBRARIES_ASL_H
/*
**
**	$Id: asl.h,v 1.3 90/07/19 18:15:26 andy Exp $
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
*/

#ifndef	EXEC_TYPES_H
#include	<exec/types.h>
#endif

#ifndef	EXEC_LISTS_H
#include	<exec/lists.h>
#endif

#ifndef	EXEC_ALERTS_H
#include	<exec/alerts.h>
#endif

#ifndef	EXEC_LIBRARIES_H
#include	<exec/libraries.h>
#endif


#ifndef UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif


#ifndef WBArg
#include <workbench/startup.h>
#endif

#ifndef GRAPHICS_TEXT_H
#include <graphics/text.h>
#endif

#ifndef    INTUITION_INTUITION_H
#include   <intuition/intuition.h>
#endif

/*
 ************************************************************************
 *	Standard definitions for asl library information		*
 ************************************************************************
 */
#define	ASLNAME		"asl.library"   /* Name of library    */
#define	AslName		ASLNAME         /* For compatibility  */
#define	AslVersion	36L		/* Current version... */
#define	AslRevision	1038L

/*
 ************************************************************************
 *	The ASL file requester data structure...		        *
 * You can only obtain this structure through AllocFileRequest.         *
 * The mapping presented below is the public portion of the structure   *
 * AllocFileRequest can allocate more storage than is presented here    *
 * hence, you must NEVER allocate this structure yourself because       *
 * the other ASL routines will expect this extra hidden storage to have *
 * been allocated and properly initialized.                             *
 ************************************************************************
 */
struct FileRequester	{
	BYTE	*rf_Hail;		/* Hailing text			*/
	BYTE	*rf_File;		/* Filename array (FCHARS + 1)	*/
	BYTE	*rf_Dir;		/* Directory array (DSIZE + 1)	*/
	struct	Window	*rf_Window;	/* Window requesting or NULL	*/
	UBYTE	rf_FuncFlags;		/* Set bitdef's below		*/
	UBYTE	rf_Flags2;		/* New flags...			*/
	VOID	(*rf_Function)();	/* Your function, see bitdef's	*/
        LONG    rf_NumArgs;     /* A-la WB Args, for multiselects */
        struct WBArg *rf_ArgList;
        APTR    rf_UserData;    /* Applihandle (you may write!!) */
        };

/*
 ************************************************************************
 * The following are the defines for rf_FuncFlags.  These bits tell	*
 * FileRequest() what your rf_UserFunc is expecting, and what		*
 * FileRequest() should call it for.					*
 *						        		*
 * You are called like so:						*
 * rf_Function(Mask, Object)						*
 * ULONG	Mask;							*
 * CPTR		*Object;						*
 *							        	*
 * The Mask is a copy of the flag value that caused FileRequest() to	*
 * call your function. You can use this to determine what action you	*
 * need to perform, and exactly what Object is, so you know what to do	*
 * and what to return.							*
 ************************************************************************
 */


#define	RFB_DOWILDFUNC	7L /* Call me with a FIB and a name, ZERO return accepts */
#define	RFB_DOMSGFUNC	6L /* You get all IDCMP messages not for FileRequest() */
#define	RFB_DOCOLOR	5L /* Set this bit for that new and different look */
#define	RFB_SAVE	5L /* For a SAVE operation, set this bit */
#define	RFB_NEWIDCMP	4L /* Force a new IDCMP (only if rf_Window != NULL) */
#define	RFB_NEWWINDFUNC	3L /* You get to modify the newwindow structure. */
#define	RFB_ADDGADFUNC	2L /* You get to add gadgets.			*/
#define	RFB_GEVENTFUNC	1L /* Function to call if one of your gadgets is selected.	*/
#define	RFB_LISTFUNC	0L /* Not implemented yet.		*/

#define	RFF_DOWILDFUNC	(1L << RFB_DOWILDFUNC)
#define	RFF_DOMSGFUNC	(1L << RFB_DOMSGFUNC)
#define	RFF_DOCOLOR	(1L << RFB_DOCOLOR)
#define	RFF_SAVE	(1L << RFB_SAVE)
#define	RFF_NEWIDCMP	(1L << RFB_NEWIDCMP)
#define	RFF_NEWWINDFUNC	(1L << RFB_NEWWINDFUNC)
#define	RFF_ADDGADFUNC	(1L << RFB_ADDGADFUNC)
#define	RFF_GEVENTFUNC	(1L << RFB_GEVENTFUNC)
#define	RFF_LISTFUNC	(1L << RFB_LISTFUNC)

/*
 ************************************************************************
 * The FR2B_ bits are for rf_Flags2 in the file requester structure	*
 ************************************************************************
 */
#define	FR2B_LONGPATH	0L /* Specify the rf_Dir buffer is 256 bytes long */

#define	FR2F_LONGPATH	(1L << FR2B_LONGPATH)

/*
 ************************************************************************
 *	The sizes of the different buffers... these are allocated by	*
 * the system, and are advisory for applications writers; they may	*
 * grow between releases.						*
 ************************************************************************
 */

#define	LONG_DSIZE	254L
#define	LONG_FSIZE	126L

#define	FR_FIRST_GADGET	0x7680L	/* User gadgetID's must be less than this value	*/

/*
 ************************************************************************
 *	The ASL font requester data structure...		        *
 * You can only obtain this structure through AllocFontRequest.         *
 * The mapping presented below is the public portion of the structure   *
 * Like the FileRequester, you must allocate this only through the ASL  *
 * interfaces.                                                          *
 ************************************************************************
 */
struct FontRequester	{
	BYTE	*fo_Hail;		/* Hailing text for title	*/
	struct Window *fo_Window;	/* Window requesting or NULL	*/

	BYTE	*fo_Name;		/* Returned values for fontname	*/
	USHORT	fo_YSize;
	UBYTE	fo_Style;
	UBYTE	fo_Flags;

	UBYTE	fo_FrontPen;
	UBYTE	fo_BackPen;
	SHORT	fo_FuncFlags;
	SHORT	fo_LeftEdge;
	SHORT	fo_TopEdge;
	};

/* Bit defines for fo_FuncFlags, for FONT requester			*/
#define	FONB_FrontColor	0	/* Display Front color selector?	*/
#define	FONB_BackColor	1	/* Display Back color selector?		*/
#define	FONB_Styles	2	/* Display Styles checkboxes?		*/
#define	FONB_DrawMode	3	/* Display DrawMode NWay?		*/
#define	FONB_FixedWidth	4	/* Only allow fixed-width fonts?	*/
#define	FONB_NewIDCMP	5	/* Create a new IDCMP port, not shared	*/
#define	FONB_DoMsgFunc	6	/* Call with IntuiMessages for other windows in shared port	*/
#define	FONB_DoWildFunc	7	/* Call with each TextAttr to approve	*/

#define	FONF_FrontColor	1
#define	FONF_BackColor	2
#define	FONF_Styles	4
#define	FONF_DrawMode	8
#define	FONF_FixedWidth	16
#define	FONF_NewIDCMP	32
#define	FONF_DoMsgFunc	64
#define	FONF_DoWildFunc	128

/************************************************************************/
/* Arguments to AllocAslRequest()                                       */
/* Types of requester structures which may be allocated:                */
/************************************************************************/
#define ASL_FileRequest 0
#define ASL_FontRequest 1
    
/************************************************************************/
/* Tags for AllocAslRequest() and AslRequest()                          */
/************************************************************************/
enum {
        ASL_Dummy = TAG_USER + 0x80000,
        ASL_Hail,               /* Hailing text follows                 */
        ASL_Window,             /* Parent window for IDCMP and screen   */
        ASL_LeftEdge,           /* Initialize LeftEdge                  */
        ASL_TopEdge,            /* Initialize TopEdge                   */
        ASL_Width,
        ASL_Height,
        ASL_HookFunc,           /* Hook function pointer                */

/* Tags specific to file request                                        */
        ASL_File,               /* Initial name of file follows         */
        ASL_Dir,                /* Initial string for filerequest dir   */

/* Tags specific to font request                                        */
        ASL_FontName,           /* Initial font name                    */
        ASL_FontHeight,         /* Initial font height                  */
        ASL_FontStyles,         /* Initial font styles                  */
        ASL_FontFlags,          /* Initial font flags for textattr      */
        ASL_FrontPen,           /* Initial frontpen color               */
        ASL_BackPen,            /* Initial backpen color                */
        ASL_MinHeight,          /* Minimum font height to display       */
        ASL_MaxHeight,          /* Max font height to display           */

        ASL_OKText,             /* Text displayed in OK gadget          */
        ASL_CancelText,         /* Text displayed in CANCEL gadget      */
        ASL_FuncFlags,          /* Function flags, depend on request    */
	
        ASL_ModeList,           /* Substitute list for font drawmodes   */    \
	
       };
	
#define ASL_Pattern     ASL_FontName

#endif /* LIBRARIES_ASL_H */

