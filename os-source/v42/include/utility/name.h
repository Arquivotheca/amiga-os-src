#ifndef UTILITY_NAME_H
#define	UTILITY_NAME_H
/*
**	$Id: name.h,v 39.3 92/09/04 16:25:21 mks Exp $
**
**	Namespace definitions
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	All Rights Reserved
**/

#include	<exec/types.h>

/* The named object structure */
/* Note how simple this structure is!  You have nothing else that is */
/* defined.  Remember that...  Do not hack at the namespaces!!! */
struct NamedObject
{
	VOID	*no_Object;	/* Your pointer, for whatever you want */
};

/* Tags for AllocNamedObjectTags */
#define	ANO_NameSpace	4000	/* Tag to define namespace	*/
#define	ANO_UserSpace	4001	/* tag to define userspace	*/
#define	ANO_Priority	4002	/* tag to define priority	*/
#define	ANO_Flags	4003	/* tag to define flags		*/

/* Flags for tag ANO_FLAGS */
#define	NSB_NODUPS	0
#define	NSB_CASE	1

#define	NSF_NODUPS	(1L << NSB_NODUPS)	/* Default allow duplicates */
#define	NSF_CASE	(1L << NSB_CASE)	/* Default to caseless... */

#endif
