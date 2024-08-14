
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
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 1 Oct 86   =RJ Mical=   Created this file
 *
 * *************************************************************************/

#include "zaphod.h"
#include <exec/memory.h>

#define  DBG_OPEN_TEXT_CLIP_ENTER      1
#define  DBG_OPEN_TEXT_CLIP_RETURN     1
#define  DBG_WRITE_TEXT_CLIP_ENTER     1
#define  DBG_WRITE_TEXT_CLIP_RETURN    1
#define  DBG_EXAMINE_TEXT_CLIP_ENTER   1
#define  DBG_EXAMINE_TEXT_CLIP_RETURN  1
#define  DBG_CLOSE_TEXT_CLIP_ENTER     1
#define  DBG_CLOSE_TEXT_CLIP_RETURN    1

struct Remember *ClipKey = NULL;
UBYTE NullString = '\0';
BOOL CBOpened = FALSE;


BOOL OpenTextClip()
/* Returns TRUE if everything was initialized OK.
 * If something went wrong, false is returned.
 */
{
#if (DEBUG1 & DBG_OPEN_TEXT_CLIP_ENTER)
   kprintf("clip.c:       OpenTextClip(VOID)\n");
#endif

   if (CBOpen(0) != 0)
      {
      CloseTextClip();
      return(FALSE);
      }
   CBOpened = TRUE;
   ClipKey = NULL;

#if (DEBUG2 & DBG_OPEN_TEXT_CLIP_RETURN)
   kprintf("clip.c:       OpenTextClip: Returns(???)\n");
#endif

   return(TRUE);
}



void WriteTextClip(text)
UBYTE *text;
/* Writes a text string into the internal buffer, allocating a new
 * buffer every time (use a Key).  Calls CBWriteText.
 */
{
   SHORT length;

#if (DEBUG1 & DBG_WRITE_TEXT_CLIP_ENTER)
   kprintf("clip.c:       WriteTextClip(text=0x%lx)\n",text);
#endif

   FreeRemember(&ClipKey, TRUE);

   length = StringLength(text) + 1; /* plus one for the trailing NULL */

   if (AllocRemember(&ClipKey, length, MEMF_CLEAR))
      {
      CopyString(ClipKey->Memory, text);
      }

   CBWriteText(text, (SHORT)(length - 1));

#if (DEBUG2 & DBG_WRITE_TEXT_CLIP_RETURN)
   kprintf("clip.c:       WriteTextClip: Returns(VOID)\n");
#endif
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

#if (DEBUG1 & DBG_EXAMINE_TEXT_CLIP_ENTER)
   kprintf("clip.c:       ExamineTextClip(VOID)\n");
#endif

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

#if (DEBUG2 & DBG_EXAMINE_TEXT_CLIP_RETURN)
   kprintf("clip.c:       ExamineTextClip: Returns(0x%lx)\n",ptr);
#endif

   return (ptr);
}



void CloseTextClip()
{
#if (DEBUG1 & DBG_CLOSE_TEXT_CLIP_ENTER)
   kprintf("clip.c:       CloseTextClip(VOID)\n");
#endif

   if (CBOpened) CBClose();
   FreeRemember(&ClipKey, TRUE);

#if (DEBUG2 & DBG_CLOSE_TEXT_CLIP_RETURN)
   kprintf("clip.c:       CloseTextClip: Returns(VOID)\n");
#endif
}



