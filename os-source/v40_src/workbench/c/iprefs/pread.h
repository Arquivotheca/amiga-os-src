#ifndef PREAD_H
#define PREAD_H


/*****************************************************************************/


#include <exec/types.h>
#include <dos.h>

#include "texttable.h"


/*****************************************************************************/


VOID ReadPrefsFile(STRPTR name);

VOID __asm ReplaceBackdrop(register __a0 struct BitMap *bm,
                           register __d0 UWORD width,
                           register __d1 UWORD height,
                           register __a1 Object *dto,
                           register __a2 UWORD backdropType);

VOID UserProblems(AppStringsID string, STRPTR extra);


/*****************************************************************************/


#endif /* PREAD_H */
