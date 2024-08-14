/*
** OpenLocaleEnv() - Detect and localize app if possible
**
** This routine "sniffs" the system to see if it supports locale.  If it
** does, AppShell then makes sure that it can open the proper Locale and
** Catalog structures for its stuff.  Finally, it attempts to make a spot
** judgement as to whether the Application has catalogs, etc and opens them
** if they are present.
**
*/


#include <libraries/locale.h>
#include <clib/locale_protos.h>
#include <pragmas/locale_pragmas.h>
#include "appshell_internal.h"
#include "apshattrs.h"

#define DL(x)	;
#define DX(x)   ;

extern void kprintf( char *, ... );

struct AppString
{
	LONG	as_ID;
	STRPTR	as_Str;
};

struct AppLocContext *OpenLocaleEnv( struct TagItem *tl )
{
	struct AppLocContext	*alc = NULL;

	struct Library	*LocaleBase = NULL;
	struct Locale	*loc = NULL;
	struct Catalog	*apsh_cat = NULL,
					*app_cat = NULL;

	STRPTR	cat_nam = NULL,
			lan_nam = NULL;

	struct TagItem	lang[] ={
							{OC_BuiltInLanguage,(ULONG)&"english"},
							{TAG_DONE, 0L}
							};

	/* Say we're here... */
	DL(kprintf("\n** OpenLocaleEnv\n"))

	/* Allocate an AppLocContext */
	if ((alc = AllocVec(sizeof(struct AppLocContext),MEMF_CLEAR))==NULL)
	{
		DX(kprintf("[AS.OpenLocaleEnv] Cant allocate ALC.\n"))
		return(NULL);
	}
	else
	{
	/* Check taglist for App-specific stuff */

	cat_nam = (STRPTR) GetTagData(APSH_AppCatalog,NULL,tl);
	DL(kprintf("**   AppCatalog nametag extracted: %s\n",cat_nam))
	lan_nam = (STRPTR) GetTagData(APSH_AppDefLang,NULL,tl);
	DL(kprintf("**   AppDefLang nametag extracted: %s\n",lan_nam))

	/* Sniff for locale.library */

	if ((LocaleBase = OpenLibrary("locale.library",38L))==NULL)
	{
		DX(kprintf("[AS.OpenLocaleEnv] No Locale Library.\n"))
	}
	else
	{
		DL(kprintf("**   LocaleBase = 0x%lx\n",LocaleBase))

		alc->alc_LocBase = LocaleBase;

		/* Got the lib, try for the Locale.  The way the Locale works, */
		/* the return from OpenLocale() is kind of irrelevant but we   */
		/* need to make the attempt.                                   */

		if (loc = OpenLocale(NULL))
		{
			alc->alc_Loc = loc;
		}
		DL(kprintf("**   Locale = 0x%lx\n",loc))

		/* Basically, loc is either NULL or a locale structure, either */
		/* of which is an acceptable state to attempt to open up the   */
		/* catalogs.                                                   */

		if (apsh_cat = OpenCatalogA(NULL,"appshell.catalog",lang))
		{
			DL(kprintf("**   ApshCatalog = 0x%lx\n",apsh_cat))

			alc->alc_ApshCat = apsh_cat;
		}
		else
		{
			if (!IoErr())
			{
				DL(kprintf("**   ApshCatalog = 0x%lx\n",apsh_cat))
			}
			else
			{
				DX(kprintf("[AS.OpenLocaleEnv] No AppShell catalog.\n"))
			}
		}

		if (cat_nam)
		{
			lang[1].ti_Data = (ULONG) lan_nam;
			DL(kprintf("**   cat_nam = %s\n",cat_nam))
			if (app_cat = OpenCatalogA(	NULL,cat_nam,lang))
			{
				alc->alc_AppCat = app_cat;
			}
			DL(kprintf("**   AppCatalog = 0x%lx\n",app_cat))
		}
		else
		{
			DX(kprintf("[AS.OpenLocaleEnv] No Application catalog\n"))
		}
	}
	}

	DL(kprintf("**   End of Conditionals\n"))

	DL(kprintf("** End of OpenLocaleEnv\n"))
	return( alc );
}


/*
** CloseLocaleEnv() - Shuts down all locale-related elements
**
** Pair routine to OpenLocaleEnv(), this routine shuts down and cleans up
** all locale-related pointers, files, etc.
**
*/

VOID CloseLocaleEnv( struct AppLocContext *alc )
{
	struct Library	*LocaleBase = NULL;
	struct Locale	*loc = NULL;
	struct Catalog	*app_cat = NULL,
					*apsh_cat = NULL;

	DL(kprintf("\n** Start CloseLocaleEnv\n"))

	if (alc)
	{

	LocaleBase = alc->alc_LocBase;
	DL(kprintf("**   Fetch LocaleBase:\t0x%lx\n",LocaleBase))
	loc = alc->alc_Loc;
	DL(kprintf("**   Fetch Locale:\t0x%lx\n",loc))
	app_cat = alc->alc_AppCat;
	DL(kprintf("**   Fetch AppCatalog:\t0x%lx\n",app_cat))
	apsh_cat = alc->alc_ApshCat;
	DL(kprintf("**   Fetch ApshCatalog:\t0x%lx\n"))

	if (LocaleBase)
	{
		if (app_cat)
		{
			DL(kprintf("**   Close AppCat\n"))

			CloseCatalog(app_cat);
			alc->alc_AppCat = NULL;
		}

		if (apsh_cat)
		{
			DL(kprintf("**   Close ApshCat\n"))

			CloseCatalog(apsh_cat);
			alc->alc_ApshCat = NULL;
		}

		if (loc)
		{
			DL(kprintf("**   Close Locale\n"))

			CloseLocale(loc);
			alc->alc_Loc = NULL;
		}

		DL(kprintf("**   Close Locale library\n"))

		CloseLibrary(LocaleBase);
		alc->alc_LocBase = NULL;
	}
	else
	{
		DL(kprintf("**   AppShell/Application not localized.\n"))
	}

	/* Deallocate the ALC itself */
	FreeVec(alc);

	}

	DL(kprintf("** End CloseLocaleEnv\n"))
}

STRPTR __regargs GT_Loc
(
struct AppInfo *ai,
ULONG baseid,
LONG tid
)
{
	register struct AppLocContext	*alc;

	struct Library		*LocaleBase;
	struct AppString	*table;
	struct Catalog		*cat = NULL;
	STRPTR				txt = NULL;
	LONG				index = 0L;

	DL(kprintf("**   ** Start GT_Loc\n"))

	alc = ai->ai_ALC;
	DL(kprintf("**   **   AppLocContext: 0x%lx\n",alc))
	if (alc)
	{

	LocaleBase = alc->alc_LocBase;
	DL(kprintf("**   **   LocaleBase: 0x%lx\n",LocaleBase))

	switch (baseid)
	{
		case APSH_USER_ID:	table = alc->alc_AppDef;
							cat = alc->alc_AppCat;
							DL(kprintf("**   **   Using App catalog\n"))
							break;
		case APSH_MAIN_ID:
		default:			table = alc->alc_ApshDef;
							cat = alc->alc_ApshCat;
							DL(kprintf("**   **   Using Apsh catalog\n"))
	}
	DL(kprintf("**   **   (table = 0x%lx)\n",table))
	DL(kprintf("**   **   (cat   = 0x%lx)\n",cat))

    if (table)
    {
		while ((table[++index].as_ID)>=0)
		{
			DL(kprintf("[%ld:%ld][%ld]\n",tid,table[index].as_ID,table[index+1].as_ID))
			if (tid==(table[index].as_ID))
			{
				txt = table[index].as_Str;
				alc->alc_nID = table[index+1].as_ID;
				break;
			}
		}
		DL(kprintf("**   **   Text: %s\n",txt))

		if ((LocaleBase)&&(txt))
		{
			txt = GetCatalogStr(cat,tid,txt);
		}
		else
		{
			alc->alc_nID = 0L;
			DL(kprintf("**   **   Couldn't find txt!\n"))
		}
		DL(kprintf("**   **   Text: %s\n",txt))
	}
    else
    {
		DL(kprintf("**   **   No text table!\n"));
    }

    } /* endif alc */

	DL(kprintf("**   ** End GT_Loc\n"))
	return(txt);
}

