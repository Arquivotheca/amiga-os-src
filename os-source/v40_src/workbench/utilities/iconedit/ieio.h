#ifndef IEIO_H
#define IEIO_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <workbench/workbench.h>
#include <libraries/asl.h>

#include "dynamicimages.h"
#include "iemain.h"


/*****************************************************************************/


VOID UpdateIconInfo(WindowInfoPtr wi, struct DiskObject *dob);
struct DiskObject *LoadIcon(STRPTR name, BOOL real);
BOOL SaveIcon(WindowInfoPtr wi, STRPTR name, struct DiskObject *dob, WORD);
VOID SPUpdateColors(WindowInfoPtr wi);
void RefreshCustomImagery(WindowInfoPtr wi);
VOID FixFileAndPath(struct FileRequester *rf, STRPTR name);


/*****************************************************************************/


#endif /* IEIO_H */
