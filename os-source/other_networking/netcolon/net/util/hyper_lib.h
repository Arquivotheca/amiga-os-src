#ifndef LIBRARIES_HYPER_LIB_H
#define LIBRARIES_HYPER_LIB_H
/* hyper_lib.h
 * structures needed for the hyper.library system
 * written by David N. Junod
 *
 * $Id
 *
 * $Log
 *
 */
#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* EXEC_TYPES_H */

#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif /* EXEC_LISTS_H */

#ifndef EXEC_NODES_H
#include <exec/nodes.h>
#endif /* EXEC_NODES_H */

#ifndef EXEC_SEMAPHORES_H
#include <exec/semaphores.h>
#endif

#ifndef INTUITION_INTUITION_H
#include <intuition/intuition.h>
#endif

#ifndef INTUITION_SCREENS_H
#include <intuition/screens.h>
#endif

#ifndef LIBRARIES_DOS_H
#include <libraries/dos.h>
#endif

#ifndef MEMORY_FOLLOWING
/* Usually allocate one large block of memory for a group of items and then
 * divy out to the appropriate pointers.  Use with caution---must be
 * consistent with field types! */
#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))
#endif

#ifndef V
/* Used to cast an pointer to a void pointer */
#define V(x) ((void *)x)
#endif

#define	ALTED	(IEQUALIFIER_RALT | IEQUALIFIER_LALT)
#define	SHIFTED	(IEQUALIFIER_RSHIFT | IEQUALIFIER_LSHIFT)

#ifndef APSH_TOOL_ID
#define	APSH_TOOL_ID 11000L
#define	StartupMsgID	(APSH_TOOL_ID+1L)	/* Startup message */
#define	LoginToolID	(APSH_TOOL_ID+2L)	/* Login a tool SIPC port */
#define	LogoutToolID	(APSH_TOOL_ID+3L)	/* Logout a tool SIPC port */
#define	ShutdownMsgID	(APSH_TOOL_ID+4L)	/* Shutdown message */
#define	ActivateToolID	(APSH_TOOL_ID+5L)	/* Activate tool */
#define	DeactivateToolID (APSH_TOOL_ID+6L)	/* Deactivate tool */
#define	ActiveToolID	(APSH_TOOL_ID+7L)	/* Tool Active */
#define	InactiveToolID	(APSH_TOOL_ID+8L)	/* Tool Inactive */
#define	ToolStatusID	(APSH_TOOL_ID+9L)	/* Status message */
#define	ToolCmdID	(APSH_TOOL_ID+10L)	/* Tool command message */
#define	ToolCmdReplyID	(APSH_TOOL_ID+11L)	/* Reply to tool command */
#define	ShutdownToolID	(APSH_TOOL_ID+12L)	/* Shutdown tool */
#endif

#ifndef	HYPERCONTEXT
typedef void *HYPERCONTEXT;
#endif

struct HyperMsg
{
    struct Message hmsg_Msg;	/* Embedded Exec message structure */
    ULONG hmsg_Type;		/* Type of message */
    APTR hmsg_Data;		/* Pointer to message data */
    ULONG hmsg_DSize;		/* Size of message data */
    ULONG hmsg_DType;		/* Type of message data */
    ULONG hmsg_Pri_Ret;		/* Primary return value */
    ULONG hmsg_Sec_Ret;		/* Secondary return value */
    APTR hmsg_System1;
    APTR hmsg_System2;
};

/* Allocation description structure */
struct NewHyper
{
    STRPTR nh_Name;		/* Complete name of document file */
    struct Screen *nh_Screen;	/* Screen to place windows within */
    STRPTR nh_PubScreen;	/* Public screen name to open on */
    STRPTR nh_HostPort;		/* Application's ARexx port name */
    STRPTR nh_ClientPort;	/* Name to assign to the clients ARexx port */
    STRPTR nh_BaseName;		/* Base name of the application */
    ULONG nh_Flags;		/* Flags */
    STRPTR *nh_Context;		/* NULL terminated context table */
    VOID *nh_Client;		/* Private! MUST be NULL */
};

/* public flags */
#define	HTF_LOAD_INDEX	(1L<<0)	/* Force load the index at init time */
#define	HTF_LOAD_ALL	(1L<<1)	/* Force load the entire database at init */
#define	HTF_CACHE_NODE	(1L<<2)	/* Cache each node as visited */
#define	HTF_CACHE_DB	(1L<<3)	/* Keep the buffers around until expunge */

#define	HTERR_NOT_ENOUGH_MEMORY		100L
#define	HTERR_CANT_OPEN_DATABASE	101L
#define	HTERR_CANT_FIND_NODE		102L
#define	HTERR_CANT_OPEN_NODE		103L
#define	HTERR_CANT_OPEN_WINDOW		104L
#define	HTERR_INVALID_COMMAND		105L
#define	HTERR_CANT_COMPLETE		106L
#define	HTERR_PORT_CLOSED		107L
#define	HTERR_CANT_CREATE_PORT		108L

#endif /* LIBRARIES_HYPER_LIB_H */
