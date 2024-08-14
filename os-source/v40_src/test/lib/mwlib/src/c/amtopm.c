/*
    Copyright (c) 1989 Mitchell/Ware Systems, Inc.
    
    May be used by Commodore-Amiga, Inc. for in-house purposes only.
    No part of this program, or any parts or modifications thereof, either
    in source-code or object-code, or library form, may be used by 
    Commodore, or any of its employees, either indepently, or in conjunction
    with Commordore, as, or as a part of , any public-domain or commercial
    product, program, or any type of software, teaching aid, etc. without
    the prior written permission of Mitchell/Ware Systems.

    This copyright notice must not be removed from the document of which
    it is attached.    
*/
/*****************************************************************************
    amtopm.c
    
    Mitchell/Ware Systems           Version 1.00            10-Dec-88
    
    Translate Amiga / MSDOS wildcards to format expected by stcpm & stcpma
*****************************************************************************/

#include <exec/types.h>

void amtopm(to, from)
UBYTE *to, *from;
{
    do
    {
        switch(*from)
        {
        case '#':   
            *to++ = *++from;
            *to++ = '*';
            break;
            
        case '*':
            *to++ = '?';
            *to++ = *from;
            break;
            
        case '\\':
            *to++ = *from++;
            *to++ = *from;
            break;
            
        case '+':
            *to++ = '\\';
            *to++ = *from;
            break;
            
        default:    
            *to++ = *from;
            break;
        }
    } while (*from++);
}