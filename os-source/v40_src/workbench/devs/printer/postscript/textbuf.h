#ifndef TEXTBUF_H
#define TEXTBUF_H


/*****************************************************************************/


#include <exec/types.h>


/*****************************************************************************/


/* Size of the buffer passed to PSDrain(). This corresponds to the
 * internal buffer size of the printer.device under V39. This is the
 * maximum amount of data that can passed to the printer.device as a
 * result of DoSpecial() or ConvFunc()
 */
#define DRAIN_SIZE 200


/* TRUE if there is any data currently in the buffer. This value will sometimes
 * be TRUE even if there are no bytes in the buffer. It will never be FALSE if
 * there are bytes in the buffer.
 */
extern BOOL bufferHasBytes;


VOID InitTextBuf(VOID);
BOOL PSWrite(const STRPTR buf);
LONG PSDrain(STRPTR buf);
LONG PSFlush(VOID);


/*****************************************************************************/


#endif /* TEXTBUF_H */
