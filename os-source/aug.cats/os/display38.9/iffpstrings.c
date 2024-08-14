/* iffpstrings.c
 * localized message routine for IFFP modules and display
 */

#define SHAREDCAT

/* Ignore this - this is Commodore stuff for combined catalog files */
#ifdef SHAREDCAT
#define CATALOGNAME	"sys/utilities.catalog"
#define INCLUDENAME	<localestr/utilities.h>
#else
#define CATALOGNAME	"display.catalog"
#define INCLUDENAME	"iffp/iffpstrings.h"
#endif

#include <clib/locale_protos.h>
#include <pragmas/locale_pragmas.h>

/* Locale stuff */
#define  DISPLAY
#define  STRINGARRAY
#include INCLUDENAME

extern struct Library *LocaleBase;

extern struct AppString AppStrings[];
static APTR   catalog = NULL;

/* For reference
struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};
*/

#include "iffp/iff.h"
#include <intuition/screens.h>

static UBYTE nullstring[] = {""};

/* OpenStrings - localizes strings
 * Requires open locale.library to work, but safe to call without
 * You may pass nulls as args if you have no localized application strings
 */
void OpenStrings()
   {
   if((LocaleBase)&&(!catalog))
	{
	catalog = OpenCatalogA(NULL,CATALOGNAME,NULL);
	}
    }

/* CloseStrings - release the localized strings 
 * Make sure all error messages are printed BEFORE calling this function
 */
void CloseStrings()
    {
    if((LocaleBase)&&(catalog))
	{
    	CloseCatalog(catalog);
	}
    }


/*
 * IFFerr
 *
 * Returns pointer to IFF Error string or NULL string (no error)
 */
UBYTE *IFFerr(LONG error)
{
	/*
	 * English error messages for possible IFFERR_#? returns from various
	 * IFF routines.  To get the index into this array, take your IFFERR
	 * code, negate it, and subtract one.
	 *  idx = -error - 1;
	 *
	 * To index localized string, then add MSG_IFFP_STDFIRST
	 */

	static UBYTE unknown[48];
	UBYTE  *s = nullstring;

	if((error < 0)&&(error >= -12))
		{
		s=SI(((-error) - 1) + MSG_IFFP_STDFIRST);
		}
	else if(error == CLIENT_ERROR)
		{
		s=SI(MSG_IFFP_CLIENTERR);
		}
	else if(error == NOFILE)
		{
		s=SI(MSG_IFFP_NOFILE);
		}
	else if(error)
		{
		sprintf(unknown,SI(MSG_IFFP_UNKNOWNERR_D),error);
		s=unknown;
		}
	return(s);
}


/* OpenScreen error messages
 */
UBYTE *openScreenErr(ULONG errorcode)
   {
   static UBYTE unknown[48];
   UBYTE *s=NULL;

    	switch ( errorcode )
	{
	case OSERR_NOMEM:
	    s=SI(MSG_IFFP_OSNOMEM);
	    break;
	case OSERR_NOCHIPMEM:
	    s=SI(MSG_IFFP_OSNOCHIPMEM);
	    break;
	case OSERR_NOMONITOR:
	    s=SI(MSG_IFFP_OSNOMONITOR);
	    break;
	case OSERR_NOCHIPS:
	    s=SI(MSG_IFFP_OSNOCHIPS);
	    break;
	case OSERR_PUBNOTUNIQUE:
	    s=SI(MSG_IFFP_OSPUBNOTUNIQUE);
	    break;
	case OSERR_UNKNOWNMODE:
	    s=SI(MSG_IFFP_OSUNKNOWNMODE);
	    break;
	default:
	    sprintf(unknown,SI(MSG_IFFP_OSUNKNOWNERR_D),errorcode);
	    s=unknown;
	}
    return(s);
    }


UBYTE *GetString(ULONG id)
    {
    struct AppString *as;
    UBYTE *s = "";
    int k,l;

    l = sizeof(AppStrings);
    as = AppStrings;

    for(k=0; k<l; k++, as++)
	{
	if(as->as_ID == id)
	    {
	    s = as->as_Str;
	    break;
	    }
	}

    if((LocaleBase)&&(catalog)&&(*s))
	{
	s = GetCatalogStr(catalog,id,s);
	}
    return(s);
    }
