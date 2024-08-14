#ifndef IESRC_H
#define IESRC_H


/*****************************************************************************/


#include <exec/types.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <workbench/workbench.h>


/*****************************************************************************/


BOOL IconToC(STRPTR name, STRPTR oname, struct DiskObject * dob, BOOL icon);
VOID ImageToC(BPTR fh, STRPTR name, struct Image * i);
VOID GadgetToC(BPTR fh, STRPTR stnm, struct Gadget * g);
VOID NewWindowToC(BPTR fh, STRPTR stnm, struct NewWindow * n);


/*****************************************************************************/


#endif /* IESRC_H */
