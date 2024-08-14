
/* *** clip.c ***************************************************************
 * 
 * Clipboard management routines 
 * See the document clip.doc
 *
 * Copyright (C) 1986, =Robert J. Mical=
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY    Name         Description
 * ---------  -----------  --------------------------------------------
 * 1 Oct 86   =RJ Mical=   Created this file
 *
 * *************************************************************************/

#include "zaphod.h"
#include <exec/memory.h>

void WriteTextClip();
void CloseTextClip();

struct Remember *ClipKey = NULL;
UBYTE NullString = '\0';
BOOL CBOpened = FALSE;


BOOL OpenTextClip()
/* Returns TRUE if everything was initialized OK.
 * If something went wrong, false is returned.
 */
{
   if (CBOpen(0) != 0)
      {
      CloseTextClip();
      return(FALSE);
      }
   CBOpened = TRUE;
   ClipKey = NULL;
   return(TRUE);
}



void WriteTextClip(text)
UBYTE *text;
/* Writes a text string into the internal buffer, allocating a new
 * buffer every time (use a Key).  Calls CBWriteText.
 */
{
   SHORT length;

   FreeRemember(&ClipKey, TRUE);

   length = StringLength(text) + 1; /* plus one for the trailing NULL */

   if (AllocRemember(&ClipKey, length, MEMF_CLEAR))
      {
      CopyString(ClipKey->Memory, text);
      }

   CBWriteText(text, length - 1);
}



UBYTE *ExamineTextClip()
/* Gets the address of the current internal buffer.  Since you
 * technically don't know anything about the memory, you should probably
 * regard the textclip memory as read-only memory.
 *
 * The return value of this function is guaranteed to be non-null, even
 * though the pointer may point to a null string.
 */
{
   SHORT length;
   UBYTE *ptr;

   if (ClipKey && CBReadEqualsWrite())
      return(ClipKey->Memory);

   FreeRemember(&ClipKey, TRUE);

   ptr = NULL;
   length = CBReadSetup();

   if (length >= 0)
      {
      if (length > 0)
         {
         if (ptr = AllocRemember(&ClipKey, length + 1, 0))
            {
            CBRead(ptr, length);

            *(ptr + length) = '\0';
            }
         }

      CBReadCleanup();
      }

   if (ptr == NULL) ptr = &NullString;
   return (ptr);
}



void CloseTextClip()
{
   if (CBOpened) CBClose();
   FreeRemember(&ClipKey, TRUE);
}



