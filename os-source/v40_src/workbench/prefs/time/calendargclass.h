

#include <exec/types.h>
#include <utility/tagitem.h>
#include <intuition/gadgetclass.h>
#include "pe_custom.h"


/*****************************************************************************/


/* Tags that the calendar boopsi object understands */
#define	BOA_Day		 (TAG_USER + 1)
#define	BOA_Month	 (TAG_USER + 2)
#define	BOA_Year	 (TAG_USER + 3)
#define	BOA_ClockData	 (TAG_USER + 4)
#define BOA_FirstWeekday (TAG_USER + 5)


/*****************************************************************************/


UWORD GetDaysInMonth(EdDataPtr ed, UWORD n_month, UWORD n_year);
Class *InitCalendarGClass(EdDataPtr ed);
VOID FreeCalendarGClass(Class *class);
