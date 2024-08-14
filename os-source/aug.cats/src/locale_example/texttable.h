#ifndef SAMPLE_TEXTTABLE_H
#define SAMPLE_TEXTTABLE_H


/*****************************************************************************/


#include <exec/types.h>
#include "samplestr.h"


/*****************************************************************************/


struct LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};


/*****************************************************************************/


STRPTR GetString(struct LocaleInfo *li, LONG id);


/*****************************************************************************/


#endif /* SAMPLE_TEXTTABLE_H */
