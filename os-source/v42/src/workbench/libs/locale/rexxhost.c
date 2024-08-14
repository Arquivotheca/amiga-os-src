

/* includes */
#include <exec/types.h>
#include <utility/tagitem.h>
#include <rexx/storage.h>
#include <rexx/errors.h>
#include <string.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/rexxsyslib_protos.h>
#include <clib/utility_protos.h>
#include <clib/dos_protos.h>
#include <clib/locale_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/rexxsyslib_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/locale_pragmas.h>

/* application includes */
#include "localebase.h"
#include "format.h"


/*****************************************************************************/


#define REXX_NO_ERROR          0
#define REXX_PROGRAM_NOT_FOUND ERR10_001  /* program not found            */
#define REXX_NO_MEMORY         ERR10_003  /* no memory available          */
#define REXX_NO_LIBRARY        ERR10_014  /* required library not found   */
#define REXX_WRONG_NUMBER_ARGS ERR10_017  /* wrong number of arguments    */
#define REXX_BAD_ARGS	       ERR10_018  /* invalid argument to function */

#define ARG3(rmp) (rmp->rm_Args[3])    /* third argument */


/*****************************************************************************/


struct RexxCmd
{
    STRPTR rc_Name;
    STRPTR rc_Template;
    UWORD  rc_NumArgs;
};


struct RexxCmd rexxCmds[] =
{
    {"CLOSECATALOG",  "CT/N/A"            ,1},
    {"CONVTOLOWER",   "CH/A"              ,1},
    {"CONVTOUPPER",   "CH/A"              ,1},
    {"GETCATALOGSTR", "CT/A,ST/N/A,DEF/A" ,3},
    {"ISALNUM",       "CH/A"              ,1},
    {"ISALPHA",       "CH/A"              ,1},
    {"ISCNTRL",       "CH/A"              ,1},
    {"ISDIGIT",       "CH/A"              ,1},
    {"ISGRAPH",       "CH/A"              ,1},
    {"ISLOWER",       "CH/A"              ,1},
    {"ISPRINT",       "CH/A"              ,1},
    {"ISPUNCT",       "CH/A"              ,1},
    {"ISSPACE",       "CH/A"              ,1},
    {"ISUPPER",       "CH/A"              ,1},
    {"ISXDIGIT",      "CH/A"              ,1},
    {"OPENCATALOG",   "NM/A,BIL/A,VS/N/A" ,3},
    {"STRNCMP",       "S1/A,S2/A,TP/N/A"  ,3},
};
#define NUM_COMMANDS 17


/*****************************************************************************/


ULONG ASM ARexxHostLVO(REG(a0) struct RexxMsg *parms,
                       REG(a1) STRPTR *result,
                       REG(a6) struct LocaleLib *LocaleBase)
{
struct RexxCmd *rc;
ULONG           resCode;
WORD            numArgs;
struct Library *RexxSysBase;
UWORD           cmd;
STRPTR          str;
struct Catalog *catalog;
char            ch[2];
BOOLEAN         state;
struct Locale  *locale;
ULONG           l;
struct TagItem  tags[3];
char            buffer[14];
LONG            strnum;

    *result = NULL;

    if (!(RexxSysBase = OpenLibrary("rexxsyslib.library",0)))
        return(REXX_NO_LIBRARY);

    resCode = REXX_PROGRAM_NOT_FOUND;
    rc      = NULL;
    cmd     = NUM_COMMANDS;
    while (cmd--)
    {
	if (Stricmp(ARG0(parms),rexxCmds[cmd].rc_Name) == 0)
	{
	    rc = &rexxCmds[cmd];
	    break;
	}
    }

    if (rc)
    {
	numArgs = (parms->rm_Action) & RXARGMASK;

	if (numArgs != rc->rc_NumArgs)
	{
            str     = NULL;
	    resCode = REXX_WRONG_NUMBER_ARGS;
	}
	else
	{
            locale = OpenLocale(NULL);
            str    = (STRPTR)ARG1(parms);
	    l      = (ULONG)str[0];
	    ch[1]  = 0;

	    switch (cmd)
	    {
	        case 0 : StrToLong(str,(LONG *)&catalog);
                         CloseCatalog(catalog);
                         str = "";
                         break;

	        case 1 : ch[0] = ConvToLower(locale,l);
                         str = ch;
                         break;

	        case 2 : ch[0] = ConvToUpper(locale,l);
                         str = ch;
                         break;

	        case 3 : StrToLong(str,(LONG *)&catalog);
	        	 StrToLong((STRPTR)ARG2(parms),&strnum);
                         if (!(str = GetCatalogStr(catalog,strnum,ARG3(parms))))
                             str = "";
                         break;

	        case 4 :
	        case 5 :
	        case 6 :
	        case 7 :
	        case 8 :
	        case 9 :
	        case 10:
	        case 11:
	        case 12:
	        case 13:
                case 14: switch (cmd-4)
                         {
                             case 0 : state = IsAlNum(locale,l);
                                      break;

                             case 1 : state = IsAlpha(locale,l);
                                      break;

                             case 2 : state = IsCntrl(locale,l);
                                      break;

                             case 3 : state = IsDigit(locale,l);
                                      break;

                             case 4 : state = IsGraph(locale,l);
                                      break;

                             case 5 : state = IsLower(locale,l);
                                      break;

                             case 6 : state = IsPrint(locale,l);
                                      break;

                             case 7 : state = IsPunct(locale,l);
                                      break;

                             case 8 : state = IsSpace(locale,l);
                                      break;

                             case 9 : state = IsUpper(locale,l);
                                      break;

                             case 10: state = IsXDigit(locale,l);
                                      break;
                         }

                         if (state)
                             str = "1";
                         else
                             str = "0";

                         break;

	        case 15: StrToLong((STRPTR)ARG3(parms),&strnum);
                         tags[0].ti_Tag  = OC_BuiltInLanguage;
	                 tags[0].ti_Data = (ULONG)ARG2(parms);
	                 tags[1].ti_Tag  = OC_Version;
	                 tags[1].ti_Data = (ULONG)strnum;
	                 tags[2].ti_Tag  = TAG_DONE;
	                 catalog = OpenCatalogA(locale,ARG1(parms),tags);
	                 LongToStr((ULONG)catalog,buffer,12);
	                 str = buffer;
                         break;

	        case 16: StrToLong(ARG3(parms),&strnum);
                         strnum = StrnCmp(locale,(STRPTR)ARG1(parms),
                                                 (STRPTR)ARG2(parms),
                                                 -1,
                                                 strnum);
                         if (strnum < 0)
                             str = "-1";
                         else if (strnum > 0)
                             str = "1";
                         else
                             str = "0";
                         break;
	    }
	    CloseLocale(locale);
        }

        if (str)
        {
            if (*result = CreateArgstring(str,strlen(str)))
                resCode = REXX_NO_ERROR;
            else
                resCode = REXX_NO_MEMORY;
        }
    }

    CloseLibrary(RexxSysBase);
    return(resCode);
}
