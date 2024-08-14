#ifndef	IFFI_H
#define	IFFI_H


/*****************************************************************************/


#include <exec/types.h>
#include "iffparsebase.h"
#include "iffprivate.h"


/*****************************************************************************/


#define UserRead(iffp,buf,size) (streamaction(iffp,IFFSCC_READ,IFFERR_READ,buf,size,IFFParseBase))
#define UserWrite(iffp,buf,size) (streamaction(iffp,IFFSCC_WRITE,IFFERR_WRITE,buf,size,IFFParseBase))
LONG UserSeek(struct IFFHandleP *iffp, LONG nbytes, struct IFFParseLib *IFFParseBase);
LONG streamaction(struct IFFHandleP *, LONG, LONG, char *, LONG, struct IFFParseLib *IFFParseBase);


/*****************************************************************************/


#endif /* IFFI_H */
