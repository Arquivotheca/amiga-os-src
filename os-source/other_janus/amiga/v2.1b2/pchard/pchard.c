/*********************************************************************/
/*                                                                   */
/*  Program name:  pchard  (c) 1988 Commodore-Amiga Inc.             */
/*                                                                   */
/*  Purpose:  To provide a means to reset the bridgeboard when ctrl  */
/*            alt del won't work.                                    */
/*                                                                   */
/*  Arguments:  None.                                                */
/*                                                                   */
/*  Programer:  Bill Koester                                         */
/*                                                                   */
/*********************************************************************/
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
