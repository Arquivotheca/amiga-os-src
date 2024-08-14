#ifndef PE_IFF_H
#define PE_IFF_H


/*****************************************************************************/


#include <exec/types.h>
#include <libraries/iffparse.h>
#include "pe_custom.h"


/*****************************************************************************/


typedef EdStatus (*IFFFunc)(EdDataPtr,struct IFFHandle *, struct ContextNode *);

EdStatus ReadIFF(EdDataPtr ed, STRPTR name, ULONG *stopChunks, ULONG chunkCnt,
                 IFFFunc readFunc);

EdStatus WriteIFF(EdDataPtr ed, STRPTR name, APTR hdr, IFFFunc writeFunc);


/*****************************************************************************/


#endif /* PE_IFF_H */
