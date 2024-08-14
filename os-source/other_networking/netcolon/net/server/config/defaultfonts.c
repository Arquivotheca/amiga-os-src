/*
 * MKSoft Development Amiga ToolKit V1.0
 *
 * Copyright (c) 1985,86,87,88,89,90 by MKSoft Development
 *
 * $Id: defaultfonts.c,v 1.1 91/04/09 16:46:02 dlarson Exp $
 *
 * $Source: HOGNET:src/net/server/config/defaultfonts.c,v $
 *
 * $Date: 91/04/09 16:46:02 $
 *
 * $Revision: 1.1 $
 *
 * $Log:	defaultfonts.c,v $
 * Revision 1.1  91/04/09  16:46:02  dlarson
 * Initial revision
 * 
 * Revision 1.1  90/05/20  12:15:58  mks
 * Initial revision
 * 
 */

/*
 * This file contains the TOPAZ80 default font for
 * global recognition...
 */

#include	<exec/types.h>
#include	<graphics/text.h>

#include	"DefaultFonts.h"

static char fontnam[11]="topaz.font";

struct TextAttr TOPAZ80={fontnam,8,0,FPF_ROMFONT};

struct TextAttr TOPAZ60={fontnam,9,0,FPF_ROMFONT};
