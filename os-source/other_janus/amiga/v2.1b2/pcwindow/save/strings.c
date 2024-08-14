
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
 * 6 Sep 86     RJ              Burst this file out of misc.c
 * 25 Jul 85    =RJ Mical=      Created this file.
 *
 * *********************************************************************** */


#include "zaphod.h"

void CopyString();
void CopyStringLength();

void CopyString(tostring, fromstring)
UBYTE *tostring, *fromstring;
{
      while (*tostring++ = *fromstring++) ;
}



void CopyStringLength(tostring, fromstring, length)
UBYTE *tostring, *fromstring;
{
   SHORT i;
   for (i = 0; i < length; i++)
      *tostring++ = *fromstring++;
}



SHORT StringLength(text)
UBYTE *text;
{
      SHORT i;

      i = 0;
      while (*text++) i++;
      return(i);
}



BOOL StringsEqual(text1, text2)
UBYTE *text1, *text2;
{
      while (*text1 == *text2)
            {
            if (*text1 == '\0') return(TRUE);
            text1++;
            text2++;
            }

      return(FALSE);
}


