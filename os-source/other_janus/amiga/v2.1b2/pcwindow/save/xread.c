
/* *** xread.c **************************************************************
 * 
 * XRead File Buffering Routines 
 *
 * Copyright (C) 1987, =Robert J. Mical=
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY    Name         Description
 * ---------  -----------  --------------------------------------------
 * 20 Jan 87  RJ           Ported this into Zaphod
 * 14 Oct 86  =RJ Mical=   Created this file
 *
 * *************************************************************************/

#include "zaphod.h"

void InitXRead();
void XReadFill();
void XReadCopy();

#define XREAD_BUFFERSIZE 256


LONG XReadFile;
SHORT XReadLength, XReadOffset;
UBYTE XReadBuffer[XREAD_BUFFERSIZE];



void InitXRead(filenumber)
LONG filenumber;
{
   XReadFile = filenumber;
   XReadLength = 0;
   XReadOffset = 0;
}



void XReadFill()
{
   XReadLength = Read(XReadFile, &XReadBuffer[0], XREAD_BUFFERSIZE);
   XReadOffset = 0;
}



void XReadCopy(ptr, length)
UBYTE *ptr;
SHORT length;
{
   CopyStringLength(ptr, &XReadBuffer[XReadOffset], length);
   XReadOffset += length;
}



SHORT XRead(buffer, length)
UBYTE *buffer;
SHORT length;
/* Returns actual length, -1 if none more to read */
{
   SHORT actual, i;

   actual = 0;

   while (length)
      {
      if (XReadOffset >= XReadLength) XReadFill();
      if (XReadOffset >= XReadLength)
         {
         if (actual == 0) return(-1);
         else return(actual);
         }
      if (length <= XReadLength) i = length;
      else i = XReadLength;
      XReadCopy(buffer, i);
      length -= i;
      actual += i;
      buffer += i;
      }

   return(actual);
}



SHORT XReadLine(buffer, maxlength)
UBYTE *buffer;
SHORT maxlength;
/* maxlength should include one for the terminating NULL.
 * This routine fills the buffer, ends the buffer with a NULL
 * and returns the length of the string (sans the NULL).
 */
{
   SHORT length;
   BYTE inkey;

   maxlength--; /* minus one for the trailing NULL */
   length = 0;

READ:
   while ((XReadOffset < XReadLength) && (length < maxlength))
      {
      inkey = XReadBuffer[XReadOffset++];

      if ((inkey == '\n') || (inkey == '\j'))
         {
         *buffer = '\0';
         return(length);
         }

      *buffer++ = inkey;
      length++;
      }

   if (length >= maxlength)
      {
      /* Well, we reached the end of the user's buffer, so return what
       * we've got so far.
       */
      *buffer = '\0';
      return(length);
      }

   XReadFill();
   if (XReadOffset >= XReadLength)
      {
      *buffer = '\0';
      if (length == 0) return(-1);
      else return(length);
      }
   goto READ;
}

