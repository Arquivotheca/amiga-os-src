#include <graphics/monitor.h>
#include <graphics/modeid.h>
#include "monitorstuff.h"

#define MONITOR_NUM   2
#define MONITOR_NAME  "pal.monitor"
#define ICON_NAME "PROGDIR:PAL"
#define MSFLAGS REQUEST_PAL
#define BEAMCON0 STANDARD_PAL_BEAMCON
#define TOTROWS STANDARD_PAL_ROWS
#define TOTCLKS STANDARD_COLORCLOCKS
#define MINROW MIN_PAL_ROW

#define PREFERRED_MODEID (NTSC_MONITOR_ID | HIRES_KEY)
#define COMPAT MCOMPAT_MIXED

#undef MAX_DENISE
#undef MIN_DENISE
#define MAX_DENISE STANDARD_DENISE_MAX
#define MIN_DENISE STANDARD_DENISE_MIN
