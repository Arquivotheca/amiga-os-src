
#ifndef NIPCEXTERNS
#define NIPCEXTERNS

#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <sana2.h>
#include "nipcbase.h"
#include "nipcinternal.h"
#include <envoy/errors.h>
#include <devices/timer.h>
#include <exec/io.h>
#include <utility/tagitem.h>
#include <dos.h>

#undef IsListEmpty(x)
#define IsListEmpty(x) (!((x)->lh_Head->ln_Succ))

 #define NIPCBASE ((struct NBase *)getreg(REG_A6))

 #define gb       ((struct NBase *)getreg(REG_A6))
 #define SysBase        (NIPCBASE->nb_SysBase)
 #define DOSBase        (NIPCBASE->nb_DOSBase)
 #define UtilityBase    (NIPCBASE->nb_UtilityBase)
 #define IFFParseBase   (NIPCBASE->nb_IFFParseBase)

#endif
