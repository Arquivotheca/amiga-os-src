#ifndef ENVOY_SERVICES_H
#define ENVOY_SERVICES_H
/*
** $Id: services.h,v 37.2 92/06/04 19:02:42 kcd Exp Locker: kcd $
**
** (C) Copyright 1992, Commodore-Amiga, Inc.
**
** Public Structures and definitions for services.library
**
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

/*--------------------------------------------------------------------------*/
/*
** Defined tags for FindService()
*/

#define FSVC_Dummy   (TAG_USER + 2048)

#define FSVC_Error    (FSVC_Dummy + 0x02)
#define FSVC_UserName (FSVC_Dummy + 0x03)
#define FSVC_PassWord (FSVC_Dummy + 0x04)

/*--------------------------------------------------------------------------*/
/*
** Defined tags for Get/SetServiceAttrsA()
*/

#define SVCAttrs_Dummy  (TAG_USER + 4096)

#define SVCAttrs_Name   (SVCAttrs_Dummy + 0x01) /* Your Service Name */

/*--------------------------------------------------------------------------*/
/*
** Defined tags for StartServiceA()
*/

#define SSVC_Dummy	(TAG_USER + 8192)

#define SSVC_UserName	(SSVC_Dummy + 0x01)
#define SSVC_Password	(SSVC_Dummy + 0x02)
#define SSVC_EntityName (SSVC_Dummy + 0x03)
#define SSVC_HostName	(SSVC_Dummy + 0x04)

/*--------------------------------------------------------------------------*/

#endif /* ENVOY_SERVICES_H */
