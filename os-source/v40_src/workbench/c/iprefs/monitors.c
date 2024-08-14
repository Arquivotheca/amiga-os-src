
#include <exec/types.h>
#include <graphics/displayinfo.h>
#include <libraries/locale.h>
#include <utility/tagitem.h>
#include <string.h>
#include "V39:src/kickstart/graphics/displayinfo_internal.h"

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/locale_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/locale_pragmas.h>

#include "monitors.h"


/*****************************************************************************/


extern struct Library *SysBase;
extern struct Library *GfxBase;
extern struct Library *LocaleBase;


/*****************************************************************************/


static char lastLanguage[40];


/*****************************************************************************/


/* private calls in gfx lib */
#pragma libcall GfxBase SetDisplayInfoData 2ee 2109805
#pragma libcall GfxBase AddDisplayInfoDataA 2e8 2109805

ULONG SetDisplayInfoData( DisplayInfoHandle handle, UBYTE * buf, ULONG size, ULONG tagID, ULONG displayID );
ULONG AddDisplayInfoDataA( DisplayInfoHandle handle, UBYTE * buf, ULONG size, ULONG tagID, ULONG displayID );


/*****************************************************************************/


VOID NameMonitors(VOID)
{
struct QueryHeader  header;
struct RawNameInfo  info;
struct DisplayInfo  dispInfo;
ULONG               modeID;
STRPTR              modeName;
struct Catalog     *catalog;
DisplayInfoHandle   dh;
struct TagItem      tags[2];
struct Locale      *locale;
BOOL                load;
STRPTR              lang;

    if (LocaleBase)
    {
        /* guaranteed to work */
        locale = OpenLocale(NULL);

        /* if this function was never called before, assume the strings are in
         * English, which is what the Monitors set them up to be.
         */
        lang = lastLanguage;
        if (!lang[0])
            lang = "english";

        /* Don't bother loading if there is no preferred language, and we
         * were never called before, or the last language used was english
         *
         * Also, don't bother loading if the current preferred language matches
         * the last language we loaded
         */

        if (!locale->loc_PrefLanguages[0])
            load = (strcmp(lang,"english") != 0);
        else
            load = (strcmp(locale->loc_PrefLanguages[0],lang) != 0);

        if (load)
        {
            tags[0].ti_Tag  = OC_BuiltInLanguage;
            tags[0].ti_Data = NULL;
            tags[1].ti_Tag  = TAG_DONE;
            if (catalog = OpenCatalogA(NULL,"sys/monitors.catalog",tags))
            {
                strncpy(lastLanguage,catalog->cat_Language,sizeof(lastLanguage));

                modeID = INVALID_ID;
                while ((modeID = NextDisplayInfo(modeID)) != INVALID_ID)
                {
                    dh = FindDisplayInfo(modeID);
                    if (GetDisplayInfoData(dh,(APTR)&dispInfo,sizeof(struct DisplayInfo),DTAG_DISP,INVALID_ID))
                    {
                        if (!dispInfo.NotAvailable)
                        {
                            if (modeName = GetCatalogStr(catalog,modeID,NULL))
                            {
                                memset(&info,0,sizeof(info));
                                info.Header.StructID  = DTAG_NAME;
                                info.Header.SkipID    = TAG_SKIP;
                                info.Header.Length    = 4;
                                info.Header.DisplayID = modeID;
                                strncpy(info.Name,modeName,sizeof(info.Name));

                                if (GetDisplayInfoData(dh,(APTR)&header,sizeof(header),DTAG_NAME,INVALID_ID))
                                {
                                    SetDisplayInfoData(dh,(APTR)&info,sizeof(info),DTAG_NAME,INVALID_ID);
                                }
                                else
                                {
                                    AddDisplayInfoDataA(dh,(APTR)&info,sizeof(info),DTAG_NAME,INVALID_ID);
                                }
                            }
                        }
                    }
                }
                CloseCatalog(catalog);
            }
            else
            {
                lastLanguage[0] = 0xff; /* invalid language name */
            }
        }

        CloseLocale(locale);
    }
}
