/*---------------------------------------------------------------------*/
/*                                                                     */
/*   The fpinit.c and fpterm.c modules are called by the C startup     */
/*   code and vary based upon the type of math being used.  To compile */
/*   these routines for your own use, use the following commands:      */
/*                                                                     */
/*      68881/2:   lc -DM881 fpinit.c                                  */
/*      ieee:      lc -DIEEE fpinit.c                                  */
/*      ffp:       lc -DFFP  fpinit.c                                  */
/*                                                                     */
/*---------------------------------------------------------------------*/


#ifdef M881
#include <dos.h>
#endif
#include <stdio.h>
#include <proto/exec.h>

void __stdargs __fpinit(void);
void __stdargs __fpterm(void);

#ifdef FFP
extern struct Library *MathBase;
extern struct Library *MathTransBase;
#endif

#ifdef IEEE
extern struct Library *MathIeeeDoubBasBase;
extern struct Library *MathIeeeDoubTransBase;
#endif



/*************************************************/
/**  The following defines are used with the    **/
/**  68881/2 coprocessors.  Set the PRECISION   **/
/**  and ROUNDING defines to the desired        **/
/**  values.                                    **/
/*************************************************/

#define SINGLE      0x40         /**  Single precision    **/
#define DOUBLE      0x80         /**  Double precision    **/
#define EXTENDED    0x00         /**  Extended precision  **/

#define PRECISION   EXTENDED         /**  Set to desired precision  **/


#define TONEAREST   0x00         /**  Round towards nearest    **/
#define TOZERO      0x10         /**  Round towards zero       **/
#define TONINF      0x20         /**  Round towards -infinity  **/
#define TOINF       0x30         /**  Round towards infinity   **/

#define ROUNDING    TONEAREST        /**  Set to desired rounding   **/



void __stdargs __fpinit()
{
#ifdef M881
    __emit(0xf23c);              /*  Generate the FMOVE.L #$90,FPCR    */
    __emit(0x9000);              /*  to set the 68881/2 precision and  */
    __emit(0x0000);              /*  rounding.                         */
    __emit(PRECISION+ROUNDING);
#endif

#ifdef FFP
    if ((MathBase      = OpenLibrary("mathffp.library", 0)) == NULL)
        XCEXIT(1L);

    if ((MathTransBase = OpenLibrary("mathtrans.library", 0)) == NULL)
        XCEXIT(2L);
#endif

#ifdef IEEE
    if ((MathIeeeDoubBasBase = OpenLibrary("mathieeedoubbas.library", 0)) == NULL)
        XCEXIT(1L);

    if ((MathIeeeDoubTransBase = OpenLibrary("mathieeedoubtrans.library", 0)) == NULL)
        XCEXIT(1L);
#endif
}



void __stdargs __fpterm()
{
#ifdef FFP
    if (MathBase)
        CloseLibrary(MathBase);

    if (MathTransBase)
        CloseLibrary(MathTransBase);
#endif

#ifdef IEEE
    if (MathIeeeDoubBasBase)
        CloseLibrary(MathIeeeDoubBasBase);

    if (MathIeeeDoubTransBase)
        CloseLibrary(MathIeeeDoubTransBase);
#endif
}
