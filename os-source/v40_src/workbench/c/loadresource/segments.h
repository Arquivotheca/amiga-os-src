#ifndef SEGMENTS_H
#define SEGMENTS_H


/*****************************************************************************/


#include <exec/types.h>


/*****************************************************************************/


enum SegmentType
{
    ST_UNKNOWN,
    ST_LIBRARY,
    ST_DEVICE,
    ST_FONT
};


/*****************************************************************************/


VOID InitSegments(VOID);
BOOL TerminateSegments(VOID);

BOOL AddSegmentNode(STRPTR name, BPTR segment);
VOID RemSegmentNode(BPTR segment);

enum SegmentType FindSegmentType(BPTR segment);


/*****************************************************************************/


#endif /* SEGMENTS_H */
