#ifndef RENDER_H
#define RENDER_H


/*****************************************************************************/


#include <exec/types.h>


/*****************************************************************************/


LONG __saveds __stdargs DriverRender(struct IODRPReq *request, LONG x, LONG y,
                                     UBYTE status);


/*****************************************************************************/


#endif /* RENDER_H */
