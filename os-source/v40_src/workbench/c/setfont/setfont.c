
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <devices/console.h>
#include <devices/keymap.h>
#include <devices/conunit.h>
#include <libraries/diskfont.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>
#include <internal/commands.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/diskfont_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "setfont_rev.h"


/****************************************************************************/


#define TEMPLATE        "NAME/A,SIZE/N/A,SCALE/S,PROP/S,ITALIC/S,BOLD/S,UNDERLINE/S" VERSTAG
#define OPT_NAME    	0
#define OPT_SIZE        1
#define OPT_SCALE	2
#define OPT_PROP	3
#define OPT_ITALIC	4
#define OPT_BOLD	5
#define OPT_UNDERLINE	6
#define OPT_COUNT	7


/****************************************************************************/


#define D_S(type,name) char a_##name[sizeof(type)+3]; \
		       type *name = (type *)((LONG)(a_##name+3) & ~3);


/****************************************************************************/


LONG main(VOID)
{
struct InfoData __aligned  infoData;
struct Library            *DOSBase;
struct Library            *SysBase = (*((struct Library **) 4));
struct Library            *DiskfontBase;
struct Library            *GfxBase;
struct Library            *UtilityBase;
LONG                       failureCode = ERROR_INVALID_RESIDENT_LIBRARY;
LONG                       failureLevel = RETURN_FAIL;
struct RDArgs             *rdargs;
LONG                       opts[OPT_COUNT];
char                       fontName[80];
struct MsgPort            *con;
struct TextFont           *font = NULL;
struct Window             *window;
struct TextAttr            ta;
UWORD                      i;

    DOSBase = OpenLibrary("dos.library",37);
    GfxBase = OpenLibrary("graphics.library",37);
    DiskfontBase = OpenLibrary("diskfont.library",37);
    UtilityBase = OpenLibrary("utility.library",37);

    if (DOSBase && GfxBase && UtilityBase && DiskfontBase)
    {
        memset(opts,0,sizeof(opts));
        if (rdargs = ReadArgs(TEMPLATE, opts, NULL))
        {
            ta.ta_YSize = *(long *)opts[OPT_SIZE];
            if (ta.ta_YSize > 4)
            {
                ta.ta_Style = FS_NORMAL;
                if (opts[OPT_ITALIC])    ta.ta_Style |= FSF_ITALIC;
                if (opts[OPT_BOLD])      ta.ta_Style |= FSF_BOLD;
                if (opts[OPT_UNDERLINE]) ta.ta_Style |= FSF_UNDERLINED;

                ta.ta_Flags = FPF_ROMFONT|FPF_DISKFONT|FPF_DESIGNED;
                if (opts[OPT_PROP])  ta.ta_Flags |= FPF_PROPORTIONAL;
                if (opts[OPT_SCALE]) ta.ta_Flags &= ~FPF_DESIGNED;

                ta.ta_Name = fontName;
                stccpy(fontName,(char *)opts[OPT_NAME],sizeof(fontName));
                i = strlen(fontName);
                if ((i < 5) || Stricmp(&fontName[i-5],".font"))
                    strncat(fontName,".font",sizeof(fontName));

                font = OpenDiskFont(&ta);

                failureCode = ERROR_OBJECT_NOT_FOUND;

                if (font && !opts[OPT_PROP] && (font->tf_Flags & FPF_PROPORTIONAL))
                {
                    CloseFont(font);
                    font = NULL;
                    failureCode = ERROR_OBJECT_WRONG_TYPE;
                }

                if (font && (con = (struct MsgPort *)THISPROC->pr_ConsoleTask))
                {
                    DoPkt1(con,ACTION_DISK_INFO,MKBADDR(&infoData));
                    if (window = (struct Window *)infoData.id_VolumeNode)
                    {
                        Forbid();
                        SetFont(window->RPort,font);

                        /* close the current font for the window */
                        if (window->IFont)
                            CloseFont(window->IFont);

                        /* let intuition know about the new font */
                        window->IFont = font;

                        PutStr("\033c");
                        Flush(Output());

                        Permit();

                        if (opts[OPT_ITALIC] && !(font->tf_Style & FSF_ITALIC))
                            PutStr("\033[3m");

                        if (opts[OPT_BOLD] && !(font->tf_Style & FSF_BOLD))
                            PutStr("\033[1m");

                        if (opts[OPT_UNDERLINE] && !(font->tf_Style & FSF_UNDERLINED))
                            PutStr("\033[4m");

                        Flush(Output());
                    }

                    failureLevel = RETURN_OK;
                    failureCode  = 0;
                }
            }
            else
            {
                failureCode = ERROR_BAD_NUMBER;
            }
            FreeArgs(rdargs);
        }
        else
        {
            failureCode = IoErr();
        }
    }

    if (DOSBase)
    {
        CloseLibrary(UtilityBase);
        CloseLibrary(DiskfontBase);
        CloseLibrary(GfxBase);

        if (!font)
        {
            PrintFault(failureCode,NULL);
            SetIoErr(failureCode);
        }

        CloseLibrary(DOSBase);
    }

    return(failureLevel);
}
