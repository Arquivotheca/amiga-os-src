#ifndef CAMDLISTS_H
#define CAMDLISTS_H

/* ======================================================================== *
      Prototypes and structures for camd and realtime list requesters
                                Written by Talin
 * ======================================================================== *
                        $VER: camdlists.h 0.1 (19.2.93)
 * ======================================================================== */

#ifndef LISTREQ_H
#include "listreq.h"
#endif

LONG SelectConductor( struct ListRequester *, char *, int, Tag, ... );
LONG SelectCluster( struct ListRequester *, char *, int, Tag, ... );

#endif
