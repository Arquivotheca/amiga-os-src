
/* *** strings.c ************************************************************
 *
 * Miscellaneous String Routines
 * All of this stuff was originally in support/strings.c long ago.
 *
 * Copyright (C) 1986, =Robert J. Mical=
 * All Rights Reserved
 *
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY      NAME            DESCRIPTION
 * -----------  --------------  --------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 6 Sep 86     RJ              Burst this file out of misc.c
 * 25 Jul 85    =RJ Mical=      Created this file.
 *
 * *********************************************************************** */


#include "zaphod.h"

#define  DBG_COPY_STRING_ENTER         1
#define  DBG_COPY_STRING_LENGTH_ENTER  1
#define  DBG_STRING_LENGTH_ENTER       0
#define  DBG_STRINGS_EQUAL_ENTER       1

void CopyString(tostring, fromstring)
UBYTE *tostring, *fromstring;
{
#if (DEBUG1 & DBG_COPY_STRING_ENTER)
   kprintf("strings.c:    CopyString(tostring=0x%lx,fromstring=0x%lx)\n",tostring,fromstring);
#endif

      while (*tostring++ = *fromstring++) ;
}



void CopyStringLength(tostring, fromstring, length)
UBYTE *tostring, *fromstring;
LONG length;
{
   SHORT i;

#if (DEBUG1 & DBG_COPY_STRING_LENGTH_ENTER)
   kprintf("strings.c:    CopyStringLength(tostring=0x%lx,fromstring=0x%lx,length=0x%lx)\n",tostring,fromstring,length);
#endif

   for (i = 0; i < length; i++)
      *tostring++ = *fromstring++;
}



SHORT StringLength(text)
UBYTE *text;
{
      SHORT i;

#if (DEBUG1 & DBG_STRING_LENGTH_ENTER)
   kprintf("strings.c:    StringLength(text=0x%lx)\n",text);
#endif

      i = 0;
      while (*text++) i++;
      return(i);
}



BOOL StringsEqual(text1, text2)
UBYTE *text1, *text2;
{

#if (DEBUG1 & DBG_STRINGS_EQUAL_ENTER)
   kprintf("strings.c:    StringsEqual(text1=0x%lx,text2=0x%lx)\n",text1,text2);
#endif

      while (*text1 == *text2)
            {
            if (*text1 == '\0') return(TRUE);
            text1++;
            text2++;
            }

      return(FALSE);
}


