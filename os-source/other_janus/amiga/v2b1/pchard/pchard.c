/*********************************************************************/
/*                                                                   */
/*  Program name:  pckey   (c) 1987 Commodore-Amiga Inc.             */
/*                                                                   */
/*  Purpose:  To provide a means to send keypresses to the bridge    */
/*            board.                                                 */
/*                                                                   */
/*  Arguments:  None.                                                */
/*                                                                   */
/*  Programer:  Bill Koester                                         */
/*                                                                   */
/*  Date Released:  22-Oct-87                                        */
/*********************************************************************/
/*
  PCKey, stupid program to cause keyboard ints on pc
 */
 
#include <janus/janus.h>

struct JanusBase *JanusBase;

void main()
{
   UBYTE *JBoard;
   int temp;

   JanusBase = (struct JanusBase *)OpenLibrary(JANUSNAME,0);
   if(JanusBase==0) return;
/*
   printf("JanusBase = %lx\n");
*/

   JBoard = (UBYTE *)GetJanusStart();
/*
   printf("Janus Board Base at %lx\n",JBoard);
*/

   if(JanusBase) {
          temp = (UBYTE)*(JBoard+IoAccessOffset + IoRegOffset+jio_Control);
            /* trun off bit 2 */
          temp &= 0xfb;
            /* write temp back */
          *(JBoard+IoAccessOffset + IoRegOffset+jio_Control)=(UBYTE)temp;
            /* read negate pc reset */

          temp = (UBYTE)*(JBoard+IoAccessOffset + IoRegOffset+jio_ReleasePcReset);
          temp = 0xfe;
          *(JBoard+IoAccessOffset + IoRegOffset+jio_Control)=(UBYTE)temp;

      CloseLibrary(JanusBase);
   }
}
