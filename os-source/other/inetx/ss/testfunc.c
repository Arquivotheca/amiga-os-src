#include <exec/types.h>
#include "sslib.h"

extern struct SSLib *sSBase ;
extern struct Library *iNetBase ;

int
testfunc(void)
{
	return( (int)iNetBase ) ;
}
