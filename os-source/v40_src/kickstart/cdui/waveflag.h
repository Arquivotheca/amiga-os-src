#ifndef WAVEFLAG_H
#define WAVEFLAG_H


/*****************************************************************************/


#include <exec/types.h>
#include <dos/dos.h>


/*****************************************************************************/


struct Column
{
    WORD srcx;
    WORD dsty;
};


/*****************************************************************************/


void __asm ColumnCopyBM(register __a0 struct BitMap *srcbm,
                register __a1 struct BitMap *dstbm,
            register __a2 struct Column *coltable,
            register __d0 WORD DstStartCol,
            register __d1 WORD SrcHeight);


/*****************************************************************************/


#endif  /* WAVEFLAG_H */
