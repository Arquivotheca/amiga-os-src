#ifndef RESOURCES_CARD_H
#include "cdtv:src/cardres/card.h"
#endif

#include "tuples.h"

#define TPLFMTTYPE_CARDMARK     0x91

/* defines for cardprep */

/* attribute memory types - figured out via some memory massage code */

#define ATTR_OVERLAPPED         0
#define ATTR_NONE               1
#define ATTR_ISMEM              2
#define ATTR_ISROM              3
#define ATTR_SMALLMEM           4

/* minimum attribute memory size (bytes) we need for CISTPL_DEVICE info
 * and link in real/unique attribute memory (e.g., HP Palm-Top cards)
 */

#define CHECKATTRCNT            16

/* preparation error returns */

#define PREP_ERROR_NOERROR      0
#define PREP_ERROR_NOCARD       1
#define PREP_ERROR_WP           2
#define PREP_ERROR_TOOSMALL     3
#define PREP_ERROR_NOTWRITABLE  4
#define PREP_ERROR_REMOVED      5
#define PREP_ERROR_MINBLOCKS    6

LONG WriteCISTPL_DEVICE( ULONG *size, UWORD attrmemtype, BOOL isADEVICE, struct DeviceTData *dt, ULONG *targetat );
LONG WriteCISTPL_FORMAT( ULONG size, UBYTE *mem);
BOOL WriteTuple(UBYTE *volatile mem, UBYTE skip, UBYTE len, UBYTE *data );
UBYTE FujiDelay(UBYTE *volatile mem, UBYTE val);

struct CardMemoryMap *GetCardMap(void);

#define CAU_VIRGSTATE 155193616L

/* Status states for MasterF.gstate (card status) */

#define CAU_NOCARD      0
#define CAU_CARDIN      1
#define CAU_BATTERY     2
#define CAU_PROTECT     4
#define CAU_CAUTION     8
#define CAU_STOP        16
#define CAU_GO          32
