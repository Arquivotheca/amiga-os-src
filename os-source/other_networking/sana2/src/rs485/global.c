/*
 * global data here.  Any other data contained in driver modules
 * should be private!
 */

#ifndef RS485_H
#include "rs485.h"
#endif

struct List rs485Units;	/* list of configured units in system 	*/
struct Library *ExpansionBase;	/* used by expansion.library calls	*/
struct Library *UtilityBase;	/* used for processing tags	*/
