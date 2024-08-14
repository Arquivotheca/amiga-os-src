
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dosasl.h>
#include <dos/stdio.h>
#include <libraries/iffparse.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/iffparse_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

/* application includes */
#include "texttable.h"
#include "catcomp_rev.h"


/*****************************************************************************/


#define	LIB_VERSION 37


/*****************************************************************************/


#define TEMPLATE        "DESCRIPTOR/A,TRANSLATION,CATALOG/K,CTFILE/K,CFILE/K,ASMFILE/K,M2FILE/K,OBJFILE/K,SYMBOLS/M/K,VB=VERBOSITY/N/K,NOOPTIM/S,NONUMBERS/S,NOSTRINGS/S,NOARRAY/S,NOBLOCK/S,NOCODE/S" VERSTAG
#define OPT_DESC	0
#define OPT_TRANSLATION	1
#define OPT_CATALOG     2
#define OPT_CTFILE      3
#define OPT_CFILE       4
#define OPT_ASMFILE     5
#define OPT_M2FILE      6
#define OPT_OBJFILE     7
#define OPT_SYMBOLS     8
#define OPT_VERBOSITY	9
#define OPT_NOOPTIM     10
#define OPT_NONUMBERS   11
#define OPT_NOSTRINGS   12
#define OPT_NOARRAY     13
#define OPT_NOBLOCK     14
#define OPT_NOCODE      15
#define OPT_COUNT	16


/*****************************************************************************/


#define ID_CTLG	 MAKE_ID('C','T','L','G')
#define ID_STRS	 MAKE_ID('S','T','R','S')
#define ID_CSET	 MAKE_ID('C','S','E','T')
#define ID_LANG	 MAKE_ID('L','A','N','G')
#define ID_FVER	 MAKE_ID('F','V','E','R')


/*****************************************************************************/


struct FormatCmd
{
    struct FormatCmd *fc_Next;
    BOOL	      fc_Long;
    char	      fc_Cmd;
};


/*****************************************************************************/


struct CatEntry
{
    struct MinNode    str_Link;
    STRPTR            str_Name;
    LONG	      str_ID;
    LONG	      str_MinLen;
    LONG	      str_MaxLen;
    STRPTR            str_OriginalLine;
    struct FormatCmd *str_FormatCmds;
    STRPTR	      str_TransStr;
    LONG              str_TransStrLen;
    STRPTR	      str_IfDefSym;
    BOOL	      str_IfDefFree;
    BOOL              str_EntryFound;
    LONG              str_LengthBytes;
};


/*****************************************************************************/


struct CodeSet
{
    LONG  cs_CodeSet;
    ULONG cs_Reserved[7];
};


/*****************************************************************************/


struct Global
{
    struct Library *DOSBase;
    struct Library *UtilityBase;
    struct Library *SysBase;
    struct Library *IFFParseBase;
    struct MinList  Desc;
    char	    HeaderName[200];
    char	    ArrayName[200];
    char            BlockName[200];
    char	    FunctionName[200];
    char            Prototype[200];
    char            CompilerOpt[200];
    char	    Version[200];
    char            CatalogName[200];
    char	    Language[200];
    char            M2Prolog[200];
    char            M2Epilog[200];
    struct MinList  Symbols;     /* symbols defined on the cmd-line */
    struct CodeSet  CodeSet;
    LONG	    IoError;
    ULONG           LineNum;
    ULONG	    Verbosity;
    LONG            LengthBytes;
    STRPTR	    IfDefSymbol;
    ULONG	    IfDefUsers;

    BOOL	    GotVersion;
    BOOL	    GotLanguage;
    BOOL	    GotCodeSet;
    BOOL            GotRCSID;
    BOOL            GotCatalogName;

    BOOL            Optim;
    BOOL            GenNumbers;
    BOOL            GenStrings;
    BOOL            GenArray;
    BOOL            GenBlock;
    BOOL            GenCode;
};

#define DOSBase       global->DOSBase
#define IFFParseBase  global->IFFParseBase
#define UtilityBase   global->UtilityBase

/* verbosity levels */
#define VB_QUIET   0
#define VB_WARN0   1
#define VB_WARN1   2
#define VB_WARN2   3
#define VB_FULL    4


/*****************************************************************************/


VOID kprintf(STRPTR,...);

static BOOL ReadDescriptor(struct Global *global, STRPTR name);
static VOID FreeDescriptor(struct Global *global);
static BOOL ReadTranslation(struct Global *global, STRPTR name);
static VOID FreeTranslation(struct Global *global);
static BOOL WriteCatalog(struct Global *global, STRPTR name);
static BOOL WriteCTFile(struct Global *global, STRPTR name);
static BOOL WriteCFile(struct Global *global, STRPTR name);
static BOOL WriteASMFile(struct Global *global, STRPTR name);
static BOOL WriteM2File(struct Global *global, STRPTR name);
static BOOL WriteObjFile(struct Global *global, STRPTR name);
static VOID PrintF(struct Global *global,enum AppStringsID str, LONG arg1, ... );
static BOOL AddSymbol(struct Global *global, STRPTR symbol);
static VOID NewList(struct List *list);


/*****************************************************************************/


LONG main(VOID)
{
LONG            opts[OPT_COUNT];
struct RDArgs  *rdargs;
LONG            failureLevel = RETURN_FAIL;
struct Global  *global;
struct Library *SysBase;
STRPTR         *symbol;
struct Node    *node;
BOOL            error;

    SysBase = (*((struct Library **) 4));

    if (global = AllocVec(sizeof(struct Global),MEMF_CLEAR))
    {
        global->SysBase    = SysBase;
        global->IoError    = ERROR_INVALID_RESIDENT_LIBRARY;
        global->Verbosity  = VB_FULL;
        strcpy(global->ArrayName,"CatCompArray");
        strcpy(global->BlockName,"CatCompBlock");
        strcpy(global->FunctionName,"GetString");
        strcpy(global->CompilerOpt,"static const");

        if (DOSBase = OpenLibrary("dos.library",LIB_VERSION))
        {
            if (UtilityBase = OpenLibrary("utility.library",LIB_VERSION))
            {
                if (IFFParseBase = OpenLibrary("iffparse.library",LIB_VERSION))
                {
                    global->IoError = 0;

                    memset(opts,0,sizeof(opts));
                    if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
                    {
                        if (opts[OPT_VERBOSITY])
                            global->Verbosity = *(LONG *)opts[OPT_VERBOSITY];

                        global->Optim      = !opts[OPT_NOOPTIM];
                        global->GenNumbers = !opts[OPT_NONUMBERS];
                        global->GenStrings = !opts[OPT_NOSTRINGS];
                        global->GenArray   = !opts[OPT_NOARRAY];
                        global->GenBlock   = !opts[OPT_NOBLOCK];
                        global->GenCode    = !opts[OPT_NOCODE];

                        NewList((struct List *)&global->Symbols);
                        error = FALSE;
                        if (symbol = (STRPTR *)opts[OPT_SYMBOLS])
                        {
                            while (*symbol)
                            {
                                if (!AddSymbol(global,*symbol))
                                {
                                    global->IoError = ERROR_NO_FREE_STORE;
                                    error = TRUE;
                                    break;
                                }
                                symbol++;
                            }
                        }

                        if (!error && ReadDescriptor(global,(STRPTR)opts[OPT_DESC]))
                        {
                            if ((!opts[OPT_TRANSLATION]) || ReadTranslation(global,(STRPTR)opts[OPT_TRANSLATION]))
                            {
                                failureLevel = RETURN_OK;

                                if (CheckSignal(SIGBREAKF_CTRL_C))
                                    failureLevel = RETURN_WARN;

                                if ((failureLevel == RETURN_OK) && (opts[OPT_CATALOG]))
                                {
                                    if (!WriteCatalog(global,(STRPTR)opts[OPT_CATALOG]))
                                    {
                                        PrintF(global,MSG_ERROR_WRITING_CAT,(LONG)opts[OPT_CATALOG]);
                                        failureLevel = RETURN_FAIL;
                                    }
                                }
                                FreeTranslation(global);

                                if (CheckSignal(SIGBREAKF_CTRL_C))
                                    failureLevel = RETURN_WARN;

                                if ((failureLevel == RETURN_OK) && (opts[OPT_CTFILE]))
                                {
                                    if (!WriteCTFile(global,(STRPTR)opts[OPT_CTFILE]))
                                    {
                                        PrintF(global,MSG_ERROR_WRITING_CTFILE,(LONG)opts[OPT_CTFILE]);
                                        failureLevel = RETURN_FAIL;
                                    }
                                }

                                if (CheckSignal(SIGBREAKF_CTRL_C))
                                    failureLevel = RETURN_WARN;

                                if ((failureLevel == RETURN_OK) && (opts[OPT_CFILE]))
                                {
                                    if (!WriteCFile(global,(STRPTR)opts[OPT_CFILE]))
                                    {
                                        PrintF(global,MSG_ERROR_WRITING_CFILE,(LONG)opts[OPT_CFILE]);
                                        failureLevel = RETURN_FAIL;
                                    }
                                }

                                if (CheckSignal(SIGBREAKF_CTRL_C))
                                    failureLevel = RETURN_WARN;

                                if ((failureLevel == RETURN_OK) && (opts[OPT_ASMFILE]))
                                {
                                    if (!WriteASMFile(global,(STRPTR)opts[OPT_ASMFILE]))
                                    {
                                        PrintF(global,MSG_ERROR_WRITING_ASMFILE,(LONG)opts[OPT_ASMFILE]);
                                        failureLevel = RETURN_FAIL;
                                    }
                                }

                                if (CheckSignal(SIGBREAKF_CTRL_C))
                                    failureLevel = RETURN_WARN;

                                if ((failureLevel == RETURN_OK) && (opts[OPT_M2FILE]))
                                {
                                    if (!WriteM2File(global,(STRPTR)opts[OPT_M2FILE]))
                                    {
                                        PrintF(global,MSG_ERROR_WRITING_M2FILE,(LONG)opts[OPT_M2FILE],(LONG)opts[OPT_M2FILE]);
                                        failureLevel = RETURN_FAIL;
                                    }
                                }

                                if (CheckSignal(SIGBREAKF_CTRL_C))
                                    failureLevel = RETURN_WARN;

                                if ((failureLevel == RETURN_OK) && (opts[OPT_OBJFILE]))
                                {
                                    if (!WriteObjFile(global,(STRPTR)opts[OPT_OBJFILE]))
                                    {
                                        PrintF(global,MSG_ERROR_WRITING_OBJFILE,(LONG)opts[OPT_OBJFILE]);
                                        failureLevel = RETURN_FAIL;
                                    }
                                }
                            }

                            if (failureLevel == RETURN_OK)
                            {
                                if (opts[OPT_CATALOG] + opts[OPT_CTFILE] + opts[OPT_CFILE] + opts[OPT_ASMFILE] + opts[OPT_M2FILE] + opts[OPT_OBJFILE] == 0)
                                {
                                    PrintF(global,MSG_VALID_DESCRIPTOR,opts[OPT_DESC]);
                                    if (opts[OPT_TRANSLATION])
                                        PrintF(global,MSG_VALID_TRANSLATION,opts[OPT_TRANSLATION]);
                                }
                            }

                            FreeDescriptor(global);
                        }

                        while (node = RemHead((struct List *)&global->Symbols))
                            FreeVec(node);

                        FreeArgs(rdargs);
                    }
                    else
                    {
                        global->IoError = IoErr();
                    }

                    CloseLibrary(IFFParseBase);
                }
                CloseLibrary(UtilityBase);
            }

            if (global->IoError)
            {
                PrintFault(global->IoError,GetString(MSG_ERROR));
            }
            else if (failureLevel == RETURN_WARN)
            {
                PrintFault(ERROR_BREAK,NULL);
            }

            SetIoErr(global->IoError);

            CloseLibrary(DOSBase);
        }
        FreeVec(global);
    }

    return(failureLevel);
}


/*****************************************************************************/


#define SysBase	global->SysBase


/*****************************************************************************/


static BOOL AddSymbol(struct Global *global, STRPTR symbol)
{
struct Node *node;

    if (node = AllocVec(sizeof(struct Node)+strlen(symbol)+1,MEMF_ANY))
    {
        node->ln_Name = (STRPTR)((ULONG)node + sizeof(struct Node));
        strcpy(node->ln_Name,symbol);
        AddTail((struct List *)&global->Symbols,node);
        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


static struct Node *FindNameNC(struct Global *global,
                               struct List *list, STRPTR name)
{
struct Node *node;
WORD         result;

    node = list->lh_Head;
    while (node->ln_Succ)
    {
        result = Stricmp(name,node->ln_Name);
        if (result == 0)
            return(node);

	node = node->ln_Succ;
    }

    return(NULL);
}


/*****************************************************************************/


static BOOL FindSymbol(struct Global *global, STRPTR symbol)
{
    if (FindNameNC(global,(struct List *)&global->Symbols,symbol))
        return(TRUE);

    return(FALSE);
}


/*****************************************************************************/


static VOID PrintF(struct Global *global, enum AppStringsID str, LONG arg1, ... )
{
    if (global->Verbosity > VB_QUIET)
        VPrintf(GetString(str),(LONG *)&arg1);
}


/*****************************************************************************/


static VOID FPrintF(struct Global *global, BPTR file, STRPTR format, LONG arg1, ... )
{
    VFPrintf(file,format,(LONG *)&arg1);
}


/*****************************************************************************/


static VOID NewList(struct List *list)
{
    list->lh_Head     = (struct Node *)&list->lh_Tail;
    list->lh_Tail     = NULL;
    list->lh_TailPred = (struct Node *)&list->lh_Head;
}


/*****************************************************************************/


#define LINESIZE 16384


/*****************************************************************************/


static BOOL GetLine(struct Global *global, BPTR file, STRPTR buf, ULONG bufSize,
                    BOOL doContinuation)
{
LONG len;

    while (FGets(file,buf,bufSize))
    {
        global->LineNum++;
        if (len = strlen(buf))
        {
            if (buf[len-1] == '\n')
                buf[--len] = 0;
        }

        if (len)
        {
            if ((buf[len-1] == '\\') && doContinuation)
            {
                buf[--len] = 0;

                buf      = &buf[len];
                bufSize -= len;
                continue;         /* keep filling the buffer */
            }
        }
        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


#define GetCh {c = line[sour++]; C = ToUpper(c);}

static UWORD EscapeStr(struct Global *global, STRPTR line, BOOL write, BOOL *trailingNULL)
{
char  c,C;
UWORD value,count;
UWORD digit;
UWORD sour,dest;
WORD  lastNull;

    sour = 0;
    dest = 0;
    lastNull = -1;

    do
    {
        GetCh;
        if (c == '\\')
        {
            GetCh;
            value = 0;
            count = 0;
            switch (c)
            {
                case 'x': GetCh;
                          while ((count<2) && (((C>='0') && (C<='9')) || ((C>='A') && (C<='F'))))
                          {
                              digit = (UWORD)C;
                              if (digit >= (UWORD)'A')
                                  digit -= 7;
			      digit -= (UWORD)'0';

			      value = value*16+digit;
                              count++;
                              GetCh;
                          }

                          c = (char)value;
                          sour--;
                          break;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7': do
                          {
                              value = value*8+(UWORD)c-(UWORD)'0';
                              count++;
                              GetCh;
                          } while ((count < 3) && (c >= '0') && (c <= '7'));

                          c = (char)value;
                          sour--;
                          break;

		case 'a': c = '\a';
		          break;

                case 'b': c = '\b';
                          break;

                case 't': c = '\t';
                          break;

                case 'n': c = '\n';
                          break;

                case 'v': c = '\v';
                          break;

                case 'f': c = '\f';
                          break;

                case 'r': c = '\r';
                          break;

                case 'e': c = '\x1e';
                          break;

                case 'c': c = '\x9b';
                	  break;

            }

            if (c == '\0')
               lastNull = dest;
        }

        if (write)
            line[dest] = c;

        dest++;

    } while (line[sour-1]);

    if (trailingNULL)
    {
        *trailingNULL = (dest-1 == lastNull);
    }

    return((UWORD)(dest-1));
}


/*****************************************************************************/


static BOOL ParseStringLine(struct Global *global, STRPTR line,
                            struct CatEntry *entry, STRPTR name)
{
struct FormatCmd *fc;
struct FormatCmd *last;
UWORD		  i,len;

    if (!(entry->str_OriginalLine = AllocVec(strlen(line)+1,MEMF_PUBLIC)))
    {
        global->IoError = ERROR_NO_FREE_STORE;
        return(FALSE);
    }
    strcpy(entry->str_OriginalLine,line);

    len = EscapeStr(global,line,TRUE,NULL);

    last = (struct FormatCmd *)&entry->str_FormatCmds;
    i    = 0;

    while (i < len)
    {
        if (line[i] == '%')
        {
            if ((line[i+1] == '%') || (line[i+1] == ' '))
            {
                i++;
            }
            else
            {
                if (!(fc = AllocVec(sizeof(struct FormatCmd),MEMF_PUBLIC|MEMF_CLEAR)))
                {
                    global->IoError = ERROR_NO_FREE_STORE;
                    return(FALSE);
                }

                last->fc_Next = fc;
                last = fc;

                while (line[i] && ((ToUpper(line[i]) <= 'A') || (ToUpper(line[i]) >= 'Z')))
                    i++;

                if (line[i] == 'l')
                {
                    fc->fc_Long = TRUE;
                    i++;
                }
                else if (line[i] == 'h')
                {
                    fc->fc_Long = FALSE;
                    i++;
                }

                if ((line[i] != 'b')
                 && (line[i] != 'c')
                 && (line[i] != 'd')
                 && (line[i] != 'D')
                 && (line[i] != 'e')
                 && (line[i] != 'E')
                 && (line[i] != 'f')
                 && (line[i] != 'g')
                 && (line[i] != 'G')
                 && (line[i] != 'i')
                 && (line[i] != 'o')
                 && (line[i] != 'p')
                 && (line[i] != 's')
                 && (line[i] != 'u')
                 && (line[i] != 'U')
                 && (line[i] != 'x')
                 && (line[i] != 'X')
                 && (line[i] != 'N')    /* for BCPL */
                 && (line[i] != 'T'))   /* for BCPL */
                {
                    if (global->Verbosity > VB_WARN0)
                        PrintF(global,MSG_WARN_BAD_FORMAT_CMD,(LONG)line[i],name,global->LineNum,i+1);
                }

                fc->fc_Cmd = line[i];
            }
        }
        i++;
    }

    if (i < entry->str_MinLen)
    {
        PrintF(global,MSG_ERROR_STRING_TOO_SHORT,(LONG)entry->str_Name,name,global->LineNum);
        return(FALSE);
    }
    else if (i > entry->str_MaxLen)
    {
        PrintF(global,MSG_ERROR_STRING_TOO_LONG,(LONG)entry->str_Name,name,global->LineNum);
        return(FALSE);
    }

    return(TRUE);
}


/*****************************************************************************/


#define AssertChar(test,err) {if (line[i] != test) { error = err; goto FailAssertion;}}
#define AssertPos(err) {if (num < 0) {error = err; goto FailAssertion;}}


static ULONG ParseNum(struct Global *global, STRPTR line, LONG *value,
                      LONG defaultNum, BOOL issigned)
{
ULONG j;
ULONG num;
BOOL  addMode;
BOOL  negate;

    num     = 0;
    j       = 0;
    negate  = FALSE;
    addMode = FALSE;

    while ((line[j] == ' ') || (line[j] == '\t'))
        j++;

    if (line[j] == '+')
    {
        addMode = TRUE;
        j++;
    }
    else if (line[j] == '-')
    {
        if (!issigned)
            return(j);

        negate = TRUE;
        j++;
    }

    if (line[j] == '$')
    {
        j++;
        while (((ToUpper(line[j]) >= 'A') && (ToUpper(line[j]) <= 'F')) ||
              ((line[j] >= '0') && (line[j] <= '9')))
        {
            if (ToUpper(line[j]) >= 'A')
                num = num * 16 + ToUpper(line[j]) - 'A' + 10;
            else
                num = num * 16 + line[j] - '0';
            j++;
        }

        if (addMode)
            *value = defaultNum + num;
        else
            *value = num;
    }
    else if ((line[j] < '0') || (line[j] > '9'))
    {
        *value = defaultNum;
    }
    else
    {
        while ((line[j] >= '0') && (line[j] <= '9'))
        {
            num = num * 10 + line[j] - '0';
            j++;
        }

        if (addMode)
            *value = defaultNum + num;
        else
            *value = num;
    }

    if (negate)
        *value = -(*value);

    while ((line[j] == ' ') || (line[j] == '\t'))
        j++;

    return(j);
}


static struct CatEntry *ParseHeaderLine(struct Global *global, STRPTR line,
	                                LONG tokenNum, STRPTR name)
{
struct CatEntry  *entry,*node;
UWORD		  i,j;
char		  c;
enum AppStringsID error;
LONG		  num;

    i = 0;
    c = ToUpper(line[0]);
    if ((c >= 'A') && (c <= 'Z') || (c == '_'))
    {
        while (TRUE)
        {
            c = ToUpper(line[i]);
            if (!(((c >= 'A') && (c <= 'Z')) || (c == '_') || ((c >= '0') && (c <= '9'))))
                break;
            i++;
        }
    }

    if (i == 0)
    {
        PrintF(global,MSG_ERROR_NO_TOKEN,(LONG)name,global->LineNum);
        return(NULL);
    }

    if (!(entry = AllocVec(sizeof(struct CatEntry)+i+1,MEMF_PUBLIC|MEMF_CLEAR)))
    {
        global->IoError = ERROR_NO_FREE_STORE;
        return(NULL);
    }
    entry->str_Name = (STRPTR)((ULONG)entry+sizeof(struct CatEntry));
    CopyMem(line,entry->str_Name,i);

    node = (struct CatEntry *)global->Desc.mlh_Head;
    while (node->str_Link.mln_Succ)
    {
        if (strcmp(node->str_Name,entry->str_Name) == 0)
        {
            PrintF(global,MSG_ERROR_TOKEN_DEFINED_TWICE,(LONG)entry->str_Name,(LONG)name,global->LineNum);
            FreeVec(entry);
            return(NULL);
        }

        node = (struct CatEntry *)node->str_Link.mln_Succ;
    }

    while ((line[i] == ' ') || (line[i] == '\t'))
        i++;

    j = i;
    AssertChar('(',MSG_ERROR_NO_LPAREN);

    j  = ++i;
    i += ParseNum(global,&line[j],&entry->str_ID,tokenNum,TRUE);
    AssertChar('/',MSG_ERROR_NO_SLASH);

    node = (struct CatEntry *)global->Desc.mlh_Head;
    while (node->str_Link.mln_Succ)
    {
        if (entry->str_ID == node->str_ID)
        {
            PrintF(global,MSG_ERROR_ID_DEFINED_TWICE,entry->str_ID,(LONG)entry->str_Name,(LONG)name,global->LineNum);
            FreeVec(entry);
            return(NULL);
        }

        node = (struct CatEntry *)node->str_Link.mln_Succ;
    }

    j   = ++i;
    num = ParseNum(global,&line[j],&entry->str_MinLen,0,FALSE);
    AssertPos(MSG_ERROR_NEGATIVE_MINLEN);
    i += num;
    AssertChar('/',MSG_ERROR_NO_SLASH);

    j   = ++i;
    num = ParseNum(global,&line[j],&entry->str_MaxLen,65535,FALSE);
    AssertPos(MSG_ERROR_NEGATIVE_MAXLEN);
    i += num;
    AssertChar(')',MSG_ERROR_NO_RPAREN);

    entry->str_IfDefSym    = global->IfDefSymbol;
    entry->str_IfDefFree   = (global->IfDefUsers++ == 0);
    entry->str_LengthBytes = global->LengthBytes;

    return(entry);

FailAssertion:
    PrintF(global,error,(LONG)name,global->LineNum,j+1);
    FreeVec(entry);
    return(NULL);
}


/*****************************************************************************/


static BOOL ParseDescriptorCmd(struct Global *global, STRPTR line, STRPTR name)
{
UWORD i,j,start;
char  c;

    start = 0;
    while ((line[start] == ' ') || (line[start] == '\t') || (line[start] == '#'))
        start++;

    i = start;
    c = ToUpper(line[start]);
    if ((c >= 'A') && (c <= 'Z') || (c == '_'))
    {
        while (TRUE)
        {
            c = ToUpper(line[i]);
            if (!(((c >= 'A') && (c <= 'Z')) || (c == '_') || ((c >= '0') && (c <= '9'))))
                break;
            i++;
        }
    }

    if (i == start)
    {
        PrintF(global,MSG_ERROR_NO_COMMAND,(LONG)name,global->LineNum);
        return(FALSE);
    }

    j = i;
    while ((line[i] == ' ') || (line[i] == '\t'))
        i++;

    line[j] = 0;
    if (Stricmp(&line[start],"IFDEF") == 0)
    {
        if (!global->IfDefUsers)
            FreeVec(global->IfDefSymbol);

        start = i;
        while ((line[i] != ' ') && (line[i] != '\t') && line[i])
            i++;

        if (!(global->IfDefSymbol = AllocVec(i-start+1,MEMF_PUBLIC|MEMF_CLEAR)))
        {
            global->IoError = ERROR_NO_FREE_STORE;
            return(FALSE);
        }
        CopyMem(&line[start],global->IfDefSymbol,i-start);
        global->IfDefUsers = 0;
    }
    else if (Stricmp(&line[start],"ENDIF") == 0)
    {
        if (!global->IfDefUsers)
            FreeVec(global->IfDefSymbol);

        global->IfDefSymbol = NULL;
        global->IfDefUsers  = 0;
    }
    else if (Stricmp(&line[start],"HEADER") == 0)
    {
        j = 0;
        while (line[i] && (line[i] != ' ') && (line[i] != '\t'))
        {
            global->HeaderName[j++] = line[i++];
        }
        global->HeaderName[j] = 0;
    }
    else if (Stricmp(&line[start],"ARRAY") == 0)
    {
        j = 0;
        while (line[i] && (line[i] != ' ') && (line[i] != '\t'))
        {
            global->ArrayName[j++] = line[i++];
        }
        global->ArrayName[j] = 0;
    }
    else if (Stricmp(&line[start],"BLOCK") == 0)
    {
        j = 0;
        while (line[i] && (line[i] != ' ') && (line[i] != '\t'))
        {
            global->BlockName[j++] = line[i++];
        }
        global->BlockName[j] = 0;
    }
    else if (Stricmp(&line[start],"FUNCTION") == 0)
    {
        j = 0;
        while (line[i] && (line[i] != ' ') && (line[i] != '\t'))
        {
            global->FunctionName[j++] = line[i++];
        }
        global->FunctionName[j] = 0;
    }
    else if (Stricmp(&line[start],"PROTOTYPE") == 0)
    {
        j = 0;
        while (line[i])
        {
            global->Prototype[j++] = line[i++];
        }
        global->Prototype[j] = 0;
        EscapeStr(global,global->Prototype,TRUE,NULL);
    }
    else if (Stricmp(&line[start],"M2PROLOG") == 0)
    {
        j = 0;
        while (line[i])
        {
            global->M2Prolog[j++] = line[i++];
        }
        global->M2Prolog[j] = 0;
        EscapeStr(global,global->M2Prolog,TRUE,NULL);
    }
    else if (Stricmp(&line[start],"M2EPILOG") == 0)
    {
        j = 0;
        while (line[i])
        {
            global->M2Epilog[j++] = line[i++];
        }
        global->M2Epilog[j] = 0;
        EscapeStr(global,global->M2Epilog,TRUE,NULL);
    }
    else if (Stricmp(&line[start],"ARRAYOPTS") == 0)
    {
        j = 0;
        while (line[i])
        {
            global->CompilerOpt[j++] = line[i++];
        }
        global->CompilerOpt[j] = 0;
    }
    else if (Stricmp(&line[start],"LENGTHBYTES") == 0)
    {
        if ((StrToLong(&line[i],&global->LengthBytes) < 0) || (global->LengthBytes > 4))
        {
            PrintF(global,MSG_ERROR_BAD_LENGTHBYTES,(LONG)&line[i],(LONG)name,global->LineNum);
            return(FALSE);
        }
    }
    else
    {
        PrintF(global,MSG_ERROR_UNKNOWN_COMMAND,(LONG)&line[start],(LONG)name,global->LineNum);
        return(FALSE);
    }

    return(TRUE);
}


/*****************************************************************************/


static BOOL ReadDescriptor(struct Global *global, STRPTR name)
{
STRPTR line;
BPTR   file;
BOOL   state;
BOOL   ok = FALSE;
LONG   tokenNum;
struct CatEntry *entry;

    NewList((struct List *)&global->Desc);

    if (line = AllocVec(LINESIZE+1,MEMF_PUBLIC)) /* +1 to compensate for a bug in V37 DOS */
    {
        if (file = Open(name,MODE_OLDFILE))
        {
            SetVBuf(file,NULL,BUF_FULL,4096);

	    state           = TRUE;
	    ok              = TRUE;
	    global->LineNum = 0;
	    tokenNum        = 0;
            while (ok && GetLine(global,file,line,LINESIZE,(BOOL)!state))
            {
                if (state)
                {
                    if (line[0] == '#')
                    {
                        ok = ParseDescriptorCmd(global,line,name);
                    }
                    else if (line[0] != ';')
                    {
                        if (entry = ParseHeaderLine(global,line,tokenNum,name))
                        {
                            AddTail((struct List *)&global->Desc,(struct Node *)entry);
                            tokenNum = entry->str_ID+1;
                        }
                        else
                        {
                            ok = FALSE;
                        }
                        state = !state;
                    }
                }
                else
                {
                    if (!ParseStringLine(global,line,entry,name))
                        ok = FALSE;
                    state = !state;
                }
            }

            if (!global->IfDefUsers)
            {
                FreeVec(global->IfDefSymbol);
                global->IfDefSymbol = NULL;
            }

	    if (ok)
	    {
                if (!(global->IoError = IoErr()))
                {
                    if (!state)
                    {
                        PrintF(global,MSG_ERROR_NO_STRING_LINE,(LONG)entry->str_Name);
                        ok = FALSE;
                    }
                }
            }
            Close(file);
        }
        else
        {
            global->IoError = IoErr();
        }
        FreeVec(line);
    }
    else
    {
        global->IoError = ERROR_NO_FREE_STORE;
    }

    if ((global->IoError == 0) && ok)
        return(TRUE);

    FreeDescriptor(global);
    return(FALSE);
}


/*****************************************************************************/


static VOID FreeDescriptor(struct Global *global)
{
struct CatEntry  *node;
struct FormatCmd *fc;
struct FormatCmd *next;

    while (node = (struct CatEntry *)RemHead((struct List *)&global->Desc))
    {
        next = node->str_FormatCmds;
        while (fc = next)
        {
            next = fc->fc_Next;
            FreeVec(fc);
        }
        FreeVec(node->str_OriginalLine);

        if (node->str_IfDefFree)
            FreeVec(node->str_IfDefSym);

        FreeVec(node);
    }
}


/*****************************************************************************/


#undef GetCh
#define GetCh {c = line[sour++]; C = ToUpper(c);}


static BOOL ParseTranslationStr(struct Global *global, STRPTR line,
                                struct CatEntry *entry, STRPTR name)
{
struct FormatCmd *fc;
UWORD             dest;
UWORD		  i,j;
UWORD             arg;
BOOL		  longmode;
LONG		  num;
LONG              dummy;

    entry->str_EntryFound = TRUE;

    if (global->Optim && (strcmp(line,entry->str_OriginalLine) == 0))
    {
        entry->str_TransStrLen = -1;
        if ((strlen(line) > 1) && (global->Verbosity > VB_WARN2))
            PrintF(global,MSG_WARN_SAME_STRINGS,(LONG)entry->str_Name,name,global->LineNum);
        return(TRUE);
    }

    if (global->Verbosity > VB_WARN1)
        if (strlen(entry->str_OriginalLine) > 4)
            if (strncmp(&entry->str_OriginalLine[strlen(entry->str_OriginalLine)-4],"...",3) == 0)
                if ((strlen(line) < 4) || (strncmp(&line[strlen(line)-4],"...",3) != 0))
                    PrintF(global,MSG_WARN_NO_ELLIPSIS,(LONG)entry->str_Name,name,global->LineNum);

    dest = EscapeStr(global,line,TRUE,NULL);

    if (dest != 0)
    {
        if (!(entry->str_TransStr = AllocVec(dest+1,MEMF_PUBLIC|MEMF_CLEAR)))
        {
            global->IoError = ERROR_NO_FREE_STORE;
            return(FALSE);
        }

        CopyMem(line,entry->str_TransStr,dest);
    }
    entry->str_TransStrLen = dest;
    line[dest] = 0;

    if (dest < entry->str_MinLen)
    {
        PrintF(global,MSG_ERROR_STRING_TOO_SHORT,(LONG)entry->str_Name,name,global->LineNum);
        return(FALSE);
    }
    else if (dest > entry->str_MaxLen)
    {
        PrintF(global,MSG_ERROR_STRING_TOO_LONG,(LONG)entry->str_Name,name,global->LineNum);
        return(FALSE);
    }

    i   = 0;
    arg = 0;
    while (i < dest)
    {
        if (line[i] == '%')
        {
            i++;
            if ((line[i] == '%') || (line[i] == ' '))
            {
                i++;
            }
            else
            {
                dummy = StrToLong(&line[i],&num);

		if (dummy >= 0)
                    i += dummy;

		if (line[i] == '$')
		{
		    num--;
		    if (num < 0)
		    {
                        PrintF(global,MSG_ERROR_NEGATIVE_ORDER,(LONG)name,global->LineNum,i+1);
		        return(FALSE);
		    }
		    i++;
		}
		else
		{
		    num = arg;
		}

                while (line[i] && ((ToUpper(line[i]) <= 'A') || (ToUpper(line[i]) >= 'Z')))
                    i++;

		j        = i;
                longmode = FALSE;
                if (line[i] == 'l')
                {
                    longmode = TRUE;
                    i++;
                }

                fc = entry->str_FormatCmds;
                while ((num > 0) && fc)
                {
                    fc = fc->fc_Next;
                    num--;
                }

                if (!fc)
                {
                    PrintF(global,MSG_ERROR_ORDER_TOO_LARGE,(LONG)name,global->LineNum,i+1);
                    return(FALSE);
                }

                if (ToUpper(fc->fc_Cmd) != ToUpper(line[i]))
                {
                    PrintF(global,MSG_ERROR_BAD_CMD,(LONG)name,global->LineNum,i+1);
                    return(FALSE);
                }

                if (fc->fc_Long != longmode)
                {
                    PrintF(global,MSG_ERROR_BAD_CMD_SIZE,(LONG)name,global->LineNum,j+1);
                    return(FALSE);
                }

                arg++;
            }
        }
        i++;
    }

    return(TRUE);
}


/*****************************************************************************/


static struct CatEntry *ParseSymbolicName(struct Global *global, STRPTR line,
                                          LONG tokenNum, STRPTR name)
{
struct CatEntry  *entry;
UWORD		  i,j,start;
char		  c;

    start = 0;
    while ((line[start] == ' ') || (line[start] == '\t'))
        start++;

    i = start;
    c = ToUpper(line[start]);
    if ((c >= 'A') && (c <= 'Z') || (c == '_'))
    {
        while (TRUE)
        {
            c = ToUpper(line[i]);
            if (!(((c >= 'A') && (c <= 'Z')) || (c == '_') || ((c >= '0') && (c <= '9'))))
                break;
            i++;
        }
    }

    if (i == start)
    {
        PrintF(global,MSG_ERROR_NO_TOKEN,(LONG)name,global->LineNum);
        return(NULL);
    }

    j = i;
    while ((line[i] == ' ') || (line[i] == '\t'))
        i++;

    if (line[i])
    {
        line[j] = 0;
        PrintF(global,MSG_ERROR_JUNK_AFTER_TOKEN,(LONG)line,(LONG)name,global->LineNum,i+1);
        return(NULL);
    }

    line[j] = 0;
    entry = (struct CatEntry *)global->Desc.mlh_Head;
    while (entry->str_Link.mln_Succ)
    {
        if (strcmp(&line[start],entry->str_Name) == 0)
        {
            if (entry->str_EntryFound)
            {
                PrintF(global,MSG_ERROR_TOKEN_DEFINED_TWICE,(LONG)line,(LONG)name,global->LineNum);
                return(NULL);
            }
            return(entry);
        }

        entry = (struct CatEntry *)entry->str_Link.mln_Succ;
    }

    PrintF(global,MSG_ERROR_INVALID_TOKEN,(LONG)line,(LONG)name,global->LineNum);
    return(NULL);
}


/*****************************************************************************/


static BOOL ParseTranslationCmd(struct Global *global, STRPTR line, STRPTR name)
{
UWORD i,j,start;
char  c;
LONG  version,revision,day,month,year;
LONG  skip;

    start = 0;
    while ((line[start] == ' ') || (line[start] == '\t') || (line[start] == '#'))
        start++;

    i = start;
    c = ToUpper(line[start]);
    if ((c >= 'A') && (c <= 'Z') || (c == '_'))
    {
        while (TRUE)
        {
            c = ToUpper(line[i]);
            if (!(((c >= 'A') && (c <= 'Z')) || (c == '_') || ((c >= '0') && (c <= '9'))))
                break;
            i++;
        }
    }

    if (i == start)
    {
        PrintF(global,MSG_ERROR_NO_COMMAND,(LONG)name,global->LineNum);
        return(FALSE);
    }

    j = i;
    while ((line[i] == ' ') || (line[i] == '\t'))
        i++;

    line[j] = 0;
    if (Stricmp(&line[start],"VERSION") == 0)
    {
        j = 0;
        while (line[i])
            global->Version[j++] = line[i++];

        global->Version[j]     = 0;
        global->GotVersion     = TRUE;
        global->GotCatalogName = FALSE;
        global->GotRCSID       = FALSE;
    }
    else if (Stricmp(&line[start],"RCSID") == 0)
    {
        if (Strnicmp(&line[i],"$Date: ",7) != 0)
        {
            PrintF(global,MSG_ERROR_BAD_RCSID,(LONG)&line[i],(LONG)name,global->LineNum);
            return(FALSE);
        }
        i += 7;   /* skip over "$Date: " */

        skip = StrToLong(&line[i],&year);
        if (skip > 0)
            i += skip;

        if (line[i] != '/')
        {
            PrintF(global,MSG_ERROR_BAD_RCSID,(LONG)&line[i],(LONG)name,global->LineNum);
            return(FALSE);
        }
        i++;

        skip = StrToLong(&line[i],&month);
        if (skip > 0)
            i += skip;

        if (line[i] != '/')
        {
            PrintF(global,MSG_ERROR_BAD_RCSID,(LONG)&line[i],(LONG)name,global->LineNum);
            return(FALSE);
        }
        i++;

        StrToLong(&line[i],&day);

        while ((line[i] != '$') && line[i])
            i++;

        if (line[i] != '$')
        {
            PrintF(global,MSG_ERROR_BAD_RCSID,(LONG)&line[i],(LONG)name,global->LineNum);
            return(FALSE);
        }
        i++;

        while ((line[i] != '$') && line[i])
            i++;

        if (Strnicmp(&line[i],"$Revision: ",11) != 0)
        {
            PrintF(global,MSG_ERROR_BAD_RCSID,(LONG)&line[i],(LONG)name,global->LineNum);
            return(FALSE);
        }
        i += 11;      /* skip over "$Revision: " */

        skip = StrToLong(&line[i],&version);
        if (skip > 0)
            i += skip;

        if (line[i] != '.')
        {
            PrintF(global,MSG_ERROR_BAD_RCSID,(LONG)&line[i],(LONG)name,global->LineNum);
            return(FALSE);
        }
        i++;

        StrToLong(&line[i],&revision);
        sprintf(global->Version,"%ld.%ld (%ld.%ld.%ld)",version,revision,day,month,year);

        global->GotRCSID   = TRUE;
        global->GotVersion = FALSE;
    }
    else if (Stricmp(&line[start],"NAME") == 0)
    {
        j = 0;
        while (line[i] && (line[i] != ' ') && (line[i] != '\t'))
        {
            global->CatalogName[j++] = line[i++];
        }
        global->CatalogName[j] = 0;

        global->GotCatalogName = TRUE;
        global->GotVersion     = FALSE;
    }
    else if (Stricmp(&line[start],"CODESET") == 0)
    {
        if (StrToLong(&line[i],&global->CodeSet.cs_CodeSet) < 0)
        {
            PrintF(global,MSG_ERROR_BAD_CODESET,(LONG)&line[i],(LONG)name,global->LineNum);
            return(FALSE);
        }
        global->GotCodeSet = TRUE;
    }
    else if (Stricmp(&line[start],"LANGUAGE") == 0)
    {
        j = 0;
        while (line[i] && (line[i] != ' ') && (line[i] != '\t'))
        {
            global->Language[j++] = line[i++];
        }
        global->Language[j] = 0;
        global->GotLanguage = TRUE;
    }
    else
    {
        PrintF(global,MSG_ERROR_UNKNOWN_COMMAND,(LONG)&line[start],(LONG)name,global->LineNum);
        return(FALSE);
    }

    return(TRUE);
}


/*****************************************************************************/


static BOOL ReadTranslation(struct Global *global, STRPTR name)
{
STRPTR line;
BPTR   file;
BOOL   state;
BOOL   ok = FALSE;
LONG   tokenNum;
struct CatEntry *entry;

    memset(&global->CodeSet,0,sizeof(struct CodeSet));
    global->Version[0]     = 0;
    global->Language[0]    = 0;
    global->GotVersion     = FALSE;
    global->GotRCSID       = FALSE;
    global->GotCatalogName = FALSE;
    global->GotLanguage    = FALSE;
    global->GotCodeSet     = FALSE;

    if (line = AllocVec(LINESIZE+1,MEMF_PUBLIC)) /* +1 to compensate for a bug in V37 DOS */
    {
        if (file = Open(name,MODE_OLDFILE))
        {
            SetVBuf(file,NULL,BUF_FULL,4096);

	    state           = TRUE;
	    ok              = TRUE;
	    global->LineNum = 0;
	    tokenNum        = 0;
            while (ok && GetLine(global,file,line,LINESIZE,(BOOL)!state))
            {
                if (state)
                {
                    if (line[0] == '#')
                    {
                        ok = ParseTranslationCmd(global,line,name);
                    }
                    else if (line[0] != ';')
                    {
                        if (!(entry = ParseSymbolicName(global,line,tokenNum,name)))
                        {
                            ok = FALSE;
                        }
                        state = !state;
                    }
                }
                else
                {
                    if (!ParseTranslationStr(global,line,entry,name))
                        ok = FALSE;
                    state = !state;
                }
            }

	    if (ok)
	    {
                if (!(global->IoError = IoErr()))
                {
                    if (!state)
                    {
                        PrintF(global,MSG_ERROR_NO_STRING_LINE,(LONG)entry->str_Name);
                        ok = FALSE;
                    }
                    else
                    {
                        entry = (struct CatEntry *)global->Desc.mlh_Head;
                        while (entry->str_Link.mln_Succ)
                        {
                            if (!entry->str_EntryFound)
                            {
                                PrintF(global,MSG_ERROR_TOKEN_ABSENT,(LONG)entry->str_Name,(LONG)name);
                                ok = FALSE;
                            }
                            entry = (struct CatEntry *)entry->str_Link.mln_Succ;
                        }
                    }
                }
            }
            Close(file);
        }
        else
        {
            global->IoError = IoErr();
        }
        FreeVec(line);
    }
    else
    {
        global->IoError = ERROR_NO_FREE_STORE;
    }

    if ((global->IoError == 0) && ok)
        return(TRUE);

    FreeTranslation(global);
    return(FALSE);
}


/*****************************************************************************/


static VOID FreeTranslation(struct Global *global)
{
struct CatEntry *node;

    node = (struct CatEntry *)global->Desc.mlh_Head;
    while (node->str_Link.mln_Succ)
    {
        FreeVec(node->str_TransStr);
        node->str_TransStr    = NULL;
        node->str_TransStrLen = 0;
        node->str_EntryFound  = FALSE;
        node = (struct CatEntry *)node->str_Link.mln_Succ;
    }
}


/*****************************************************************************/


static BOOL WriteCatalog(struct Global *global, STRPTR name)
{
struct IFFHandle *iff;
BOOL              result,ok;
struct CatEntry  *entry;
ULONG		  len,zero,pad;

    result = FALSE;
    if (iff = AllocIFF())
    {
        if (iff->iff_Stream = (ULONG)Open(name,MODE_NEWFILE))
        {
            InitIFFasDOS(iff);

            if (!OpenIFF(iff,IFFF_WRITE))
            {
                if (!PushChunk(iff,ID_CTLG,ID_FORM,IFFSIZE_UNKNOWN))
                {
		    ok = TRUE;
                    if (global->GotVersion)
                    {
                        ok = FALSE;
                        if (!PushChunk(iff,0,ID_FVER,IFFSIZE_UNKNOWN))
                            if (WriteChunkBytes(iff,global->Version,strlen(global->Version)+1) == strlen(global->Version)+1)
                                if (!PopChunk(iff))
                                    ok = TRUE;
                    }
                    else if (global->GotRCSID)
                    {
                        ok = FALSE;
                        if (!PushChunk(iff,0,ID_FVER,IFFSIZE_UNKNOWN))
                        {
			    /* I don't check for errors in the next few calls, so sue me! */

                            WriteChunkBytes(iff,"$VER: ",6);

                            if (global->GotCatalogName)
                                WriteChunkBytes(iff,global->CatalogName,strlen(global->CatalogName));
                            else
                                WriteChunkBytes(iff,FilePart(name),strlen(FilePart(name)));

                            WriteChunkBytes(iff," ",1);

                            if (WriteChunkBytes(iff,global->Version,strlen(global->Version)+1) == strlen(global->Version)+1)
                            {
                                if (!PopChunk(iff))
                                {
                                    ok = TRUE;
                                }
                            }
                        }
                    }

                    if (ok && global->GotLanguage)
                    {
                        ok = FALSE;
                        if (!PushChunk(iff,0,ID_LANG,IFFSIZE_UNKNOWN))
                            if (WriteChunkBytes(iff,global->Language,strlen(global->Language)+1) == strlen(global->Language)+1)
                                if (!PopChunk(iff))
                                    ok = TRUE;
                    }

                    if (ok && global->GotCodeSet)
                    {
                        ok = FALSE;
                        if (!PushChunk(iff,0,ID_CSET,IFFSIZE_UNKNOWN))
                            if (WriteChunkBytes(iff,&global->CodeSet,sizeof(struct CodeSet)) == sizeof(struct CodeSet))
                                if (!PopChunk(iff))
                                    ok = TRUE;
                    }

                    if (ok && !PushChunk(iff,0,ID_STRS,IFFSIZE_UNKNOWN))
                    {
                        ok    = TRUE;
                        entry = (struct CatEntry *)global->Desc.mlh_Head;
                        zero  = 0;
                        while ((entry->str_Link.mln_Succ) && ok)
                        {
                            if (entry->str_TransStrLen >= 0)
                            {
                                if (len = entry->str_TransStrLen + entry->str_LengthBytes)
                                {
                                    switch (len % 4)
                                    {
                                        case 0: len++;
                                                pad = 3;
                                                break;

                                        case 1: pad = 3;
                                                break;

                                        case 2: pad = 2;
                                                break;

                                        case 3: pad = 1;
                                                break;
                                    }

                                    if ((WriteChunkBytes(iff,&entry->str_ID,4) != 4)
                                     || (WriteChunkBytes(iff,&len,4) != 4)
                                     || (WriteChunkBytes(iff,(APTR)((ULONG)&entry->str_TransStrLen + (4-entry->str_LengthBytes)),entry->str_LengthBytes) != entry->str_LengthBytes)
                                     || (WriteChunkBytes(iff,entry->str_TransStr,len-entry->str_LengthBytes) != (len-entry->str_LengthBytes))
                                     || (WriteChunkBytes(iff,&zero,pad) != pad))
                                    {
                                        ok = FALSE;
                                    }
                                }
                                else
                                {
                                    len = 1;
                                    if ((WriteChunkBytes(iff,&entry->str_ID,4) != 4)
                                     || (WriteChunkBytes(iff,&len,4) != 4)
                                     || (WriteChunkBytes(iff,&zero,4) != 4))
                                    {
                                        ok = FALSE;
                                    }
                                }
                            }

                            entry = (struct CatEntry *)entry->str_Link.mln_Succ;
                        }

                        if (ok)
                        {
                            if (!PopChunk(iff))
                            {
                                if (!PopChunk(iff))
                                {
                                    result = TRUE;
                                }
                            }
                        }
                    }
                }
                CloseIFF(iff);
	    }
            Close((BPTR)iff->iff_Stream);
            SetProtection(name,FIBF_EXECUTE);
        }
        FreeIFF(iff);
    }

    return(result);
}


/*****************************************************************************/


static BOOL WriteCTFile(struct Global *global, STRPTR name)
{
BPTR             file;
LONG             error = 0;
struct CatEntry *entry;

    if (file = Open(name,MODE_NEWFILE))
    {
        SetVBuf(file,NULL,BUF_FULL,4096);

        FPuts(file,GetString(MSG_CTHEADER));

        entry = (struct CatEntry *)global->Desc.mlh_Head;
        while (entry->str_Link.mln_Succ)
        {
            FPuts(file,entry->str_Name);

            FPuts(file,"\n\n; ");
            FPuts(file,entry->str_OriginalLine);

            FPuts(file,"\n;\n");
            entry = (struct CatEntry *)entry->str_Link.mln_Succ;
        }

        Close(file);
    }
    else
    {
        error = IoErr();
    }

    global->IoError = error;
    return((BOOL)(error == 0));
}


/*****************************************************************************/


static BOOL WriteCFile(struct Global *global, STRPTR name)
{
BPTR             file;
LONG             error = 0;
struct CatEntry *entry;
UWORD            len;
STRPTR		 sym;
char             headName[256];
UWORD            i;
char             lengthBytes[80];
ULONG            escLen;
UWORD            zeros;
BOOL             zeropad;
BOOL             gen;

    if (global->HeaderName[0])
    {
        strcpy(headName,global->HeaderName);
    }
    else
    {
        strcpy(headName,FilePart(name));
        i = 0;
        while (headName[i] && (headName[i] != '.'))
        {
            headName[i] = ToUpper(headName[i]);
            i++;
        }
        headName[i] = 0;
    }
    strcat(headName,"_H");

    if (file = Open(name,MODE_NEWFILE))
    {
        SetVBuf(file,NULL,BUF_FULL,4096);

        FPuts(file,"#ifndef ");
        FPuts(file,headName);
        FPuts(file,"\n#define ");
        FPuts(file,headName);
        FPuts(file,"\n\n\n/****************************************************************************/\n\n");
        FPuts(file,GetString(MSG_CCOMMENT));

        if (global->GenNumbers)
        {
            FPuts(file,"#ifdef CATCOMP_NUMBERS\n\n");

            sym   = NULL;
            entry = (struct CatEntry *)global->Desc.mlh_Head;
            gen   = TRUE;
            while (entry->str_Link.mln_Succ)
            {
                if (entry->str_IfDefSym != sym)
                {
                    if (sym && gen)
                    {
                        FPuts(file,"#endif /* ");
                        FPuts(file,sym);
                        FPuts(file," */\n");
                    }

                    gen = TRUE;
                    if (sym = entry->str_IfDefSym)
                    {
                        if (gen = FindSymbol(global,entry->str_IfDefSym))
                        {
                            FPuts(file,"\n#ifdef ");
                            FPuts(file,entry->str_IfDefSym);
                            FPuts(file,"\n");
                        }
                    }
                }

                if (gen || !sym)
                {
                    FPuts(file,"#define ");
                    FPuts(file,entry->str_Name);
                    FPuts(file," ");
                    FPrintF(global,file,"%ld",entry->str_ID);
                    FPuts(file,"\n");
                }

                entry = (struct CatEntry *)entry->str_Link.mln_Succ;
            }

            if (sym && gen)
            {
                FPuts(file,"#endif /* ");
                FPuts(file,sym);
                FPuts(file," */\n");
            }

            FPuts(file,"\n#endif /* CATCOMP_NUMBERS */\n");
            FPuts(file,"\n\n/****************************************************************************/\n\n\n");
        }

        if (global->GenStrings)
        {
            FPuts(file,"#ifdef CATCOMP_STRINGS\n\n");

            sym   = NULL;
            entry = (struct CatEntry *)global->Desc.mlh_Head;
            gen   = TRUE;
            while (entry->str_Link.mln_Succ)
            {
                if (entry->str_IfDefSym != sym)
                {
                    if (sym && gen)
                    {
                        FPuts(file,"#endif /* ");
                        FPuts(file,sym);
                        FPuts(file," */\n");
                    }

                    gen = TRUE;
                    if (sym = entry->str_IfDefSym)
                    {
                        if (gen = FindSymbol(global,entry->str_IfDefSym))
                        {
                            FPuts(file,"\n#ifdef ");
                            FPuts(file,entry->str_IfDefSym);
                            FPuts(file,"\n");
                        }
                    }
                }

                if (gen || !sym)
                {
                    FPuts(file,"#define ");
                    FPuts(file,entry->str_Name);
                    FPuts(file,"_STR \"");

                    len = strlen(entry->str_OriginalLine);

                    if (entry->str_LengthBytes)
                    {
                        escLen = EscapeStr(global,entry->str_OriginalLine,FALSE,NULL);

                        switch (entry->str_LengthBytes)
                        {
                            case 1: sprintf(lengthBytes,"\\x%02lx",escLen % 256);
                                    break;

                            case 2: sprintf(lengthBytes,"\\x%02lx\\x%02lx",(escLen / 256) % 256, escLen % 256);
                                    break;

                            case 3: sprintf(lengthBytes,"\\x%02lx\\x%02lx\\x%02lx",(escLen / 65536) % 256, (escLen / 256) % 256, escLen % 256);
                                    break;

                            case 4: sprintf(lengthBytes,"\\x%02lx\\x%02lx\\x%02lx\\x%02lx",(escLen / (65536*256)) % 256, (escLen / 65536) % 256, (escLen / 256) % 256, escLen % 256);
                                    break;
                        }
                        FWrite(file,lengthBytes,strlen(lengthBytes),1);
                    }

                    if (len)
                        FWrite(file,entry->str_OriginalLine,len,1);

                    FPuts(file,"\"\n");
                }

                entry = (struct CatEntry *)entry->str_Link.mln_Succ;
            }

            if (sym && gen)
            {
                FPuts(file,"#endif /* ");
                FPuts(file,sym);
                FPuts(file," */\n");
            }
            FPuts(file,"\n#endif /* CATCOMP_STRINGS */\n");

            FPuts(file,"\n\n/****************************************************************************/\n\n\n");
        }

        if (global->GenArray)
        {
            FPuts(file,"#ifdef CATCOMP_ARRAY\n\n");
            FPuts(file,"struct ");
            FPuts(file,global->ArrayName);
            FPuts(file,"Type\n{\n    LONG   cca_ID;\n    STRPTR cca_Str;\n};\n\n");
            FPuts(file,global->CompilerOpt);
            FPuts(file," struct ");
            FPuts(file,global->ArrayName);
            FPuts(file,"Type ");
            FPuts(file,global->ArrayName);
            FPuts(file,"[] =\n{\n");

            sym     = NULL;
            entry   = (struct CatEntry *)global->Desc.mlh_Head;
            gen     = TRUE;
            while (entry->str_Link.mln_Succ)
            {
                if (entry->str_IfDefSym != sym)
                {
                    if (sym && gen)
                    {
                        FPuts(file,"#endif /* ");
                        FPuts(file,sym);
                        FPuts(file," */\n");
                    }

                    gen = TRUE;
                    if (sym = entry->str_IfDefSym)
                    {
                        if (gen = FindSymbol(global,entry->str_IfDefSym))
                        {
                            FPuts(file,"\n#ifdef ");
                            FPuts(file,entry->str_IfDefSym);
                            FPuts(file,"\n");
                        }
                    }
                }

                if (gen || !sym)
                {
                    FPuts(file,"    {");
                    FPuts(file,entry->str_Name);
                    FPuts(file,",(STRPTR)");
                    FPuts(file,entry->str_Name);
                    FPuts(file,"_STR},\n");
                }

                entry = (struct CatEntry *)entry->str_Link.mln_Succ;
            }

            if (sym && gen)
            {
                FPuts(file,"#endif /* ");
                FPuts(file,sym);
                FPuts(file," */\n");
            }

            FPuts(file,"};\n\n#endif /* CATCOMP_ARRAY */\n");

            FPuts(file,"\n\n/****************************************************************************/\n\n\n");
        }

        if (global->GenBlock)
        {
            FPuts(file,"#ifdef CATCOMP_BLOCK\n\n");
            FPuts(file,global->CompilerOpt);
            FPuts(file," char ");
            FPuts(file,global->BlockName);
            FPuts(file,"[] =\n{\n");

            sym   = NULL;
            entry = (struct CatEntry *)global->Desc.mlh_Head;
            gen   = TRUE;
            while (entry->str_Link.mln_Succ)
            {
                if (entry->str_IfDefSym != sym)
                {
                    if (sym && gen)
                    {
                        FPuts(file,"#endif /* ");
                        FPuts(file,sym);
                        FPuts(file," */\n");
                    }

                    gen = TRUE;
                    if (sym = entry->str_IfDefSym)
                    {
                        if (gen = FindSymbol(global,entry->str_IfDefSym))
                        {
                            FPuts(file,"\n#ifdef ");
                            FPuts(file,entry->str_IfDefSym);
                            FPuts(file,"\n");
                        }
                    }
                }

                if (gen || !sym)
                {
                    escLen = EscapeStr(global,entry->str_OriginalLine,FALSE,&zeropad) + entry->str_LengthBytes;
                    zeros  = 0;

                    if (!zeropad)
                    {
                        zeros++;
                        escLen++;
                    }

                    if (escLen & 1)
                    {
                        zeros++;
                        escLen++;
                    }

                    sprintf(lengthBytes,"    \"\\x%02lx\\x%02lx\\x%02lx\\x%02lx",entry->str_ID / (256*256*256),
                                                                                 (entry->str_ID / (256*256)) % 256,
                                                                                 (entry->str_ID / 256) % 256,
                                                                                 entry->str_ID % 256);
                    FWrite(file,lengthBytes,strlen(lengthBytes),1);

                    sprintf(lengthBytes,"\\x%02lx\\x%02lx\"\n    ",(escLen / 256) % 256, escLen % 256);
                    FWrite(file,lengthBytes,strlen(lengthBytes),1);

                    FPuts(file,entry->str_Name);
                    FPuts(file,"_STR");

                    if (zeros == 1)
                        FPuts(file," \"\\x00\"\n");
                    else if (zeros == 2)
                        FPuts(file," \"\\x00\\x00\"\n");
                    else
                        FPuts(file,"\n");
                }

                entry = (struct CatEntry *)entry->str_Link.mln_Succ;
            }

            if (sym && gen)
            {
                FPuts(file,"#endif /* ");
                FPuts(file,sym);
                FPuts(file," */\n");
            }

            FPuts(file,"};\n\n#endif /* CATCOMP_BLOCK */\n");
            FPuts(file,"\n\n/****************************************************************************/\n\n\n");
        }

	FPuts(file,"struct LocaleInfo\n");
	FPuts(file,"{\n");
	FPuts(file,"    APTR li_LocaleBase;\n");
	FPuts(file,"    APTR li_Catalog;\n");
	FPuts(file,"};\n\n\n");

	if (global->Prototype[0])
	{
	    FPuts(file,global->Prototype);
	    FPuts(file,"\n\n\n");
	}

        if (global->GenCode)
        {
            FPuts(file,"#ifdef CATCOMP_CODE\n\n");

            FPuts(file,"STRPTR ");
            FPuts(file,global->FunctionName);
            FPuts(file,"(struct LocaleInfo *li, LONG stringNum)\n");
            FPuts(file,"{\n");
            FPuts(file,"LONG   *l;\n");
            FPuts(file,"UWORD  *w;\n");
            FPuts(file,"STRPTR  builtIn;\n");
            FPuts(file,"\n");
            FPuts(file,"    l = (LONG *)");
            FPuts(file,global->BlockName);
            FPuts(file,";\n");
            FPuts(file,"\n");
            FPuts(file,"    while (*l != stringNum)\n");
            FPuts(file,"    {\n");
            FPuts(file,"        w = (UWORD *)((ULONG)l + 4);\n");
            FPuts(file,"        l = (LONG *)((ULONG)l + (ULONG)*w + 6);\n");
            FPuts(file,"    }\n");
            FPuts(file,"    builtIn = (STRPTR)((ULONG)l + 6);\n");
            FPuts(file,"\n");
            FPuts(file,"#define XLocaleBase LocaleBase\n");
            FPuts(file,"#define LocaleBase li->li_LocaleBase\n");
            FPuts(file,"    \n");
            FPuts(file,"    if (LocaleBase)\n");
            FPuts(file,"        return(GetCatalogStr(li->li_Catalog,stringNum,builtIn));\n");
            FPuts(file,"#define LocaleBase XLocaleBase\n");
            FPuts(file,"#undef XLocaleBase\n");
            FPuts(file,"\n");
            FPuts(file,"    return(builtIn);\n");
            FPuts(file,"}\n");

            FPuts(file,"\n\n#endif /* CATCOMP_CODE */\n");
            FPuts(file,"\n\n/****************************************************************************/\n\n");
        }

        FPuts(file,"\n#endif /* ");
        FPuts(file,headName);
        FPuts(file," */\n");

        Close(file);
    }
    else
    {
        error = IoErr();
    }

    global->IoError = error;
    return((BOOL)(error == 0));
}


/*****************************************************************************/


static UWORD startM2Code[] =
{
    0x43FA, 0x0026, 0x6004, 0x3219, 0xD2C1, 0xB099, 0x66F8, 0x5489,
    0x2218, 0x6604, 0x2009, 0x4E75, 0x2F0E, 0x2C41, 0x2050, 0x4EAE,
    0xFFB8, 0x2C5F, 0x4E75, 0x0000
};

static BOOL WriteM2File(struct Global *global, STRPTR name)
{
BPTR             file;
LONG             error = 0;
struct CatEntry *entry;
UWORD            len;
STRPTR		 sym;
char             headName[256];
UWORD            i;
char             lengthBytes[80];
ULONG            escLen;
UWORD            zeros;
BOOL             zeropad;
BOOL             gen;
char             fileName[256];
APTR             buffer;

    strcpy(fileName,name);
    strcat(fileName,".def");
    if (file = Open(fileName,MODE_NEWFILE))
    {
        SetVBuf(file,NULL,BUF_FULL,4096);

        if (global->HeaderName[0])
        {
            strcpy(headName,global->HeaderName);
        }
        else
        {
            strcpy(headName,FilePart(name));
            i = 0;
            while (headName[i] && (headName[i] != '.'))
                i++;
            headName[i] = 0;
        }

        FPuts(file,"DEFINITION MODULE ");
        FPuts(file,headName);
        FPuts(file,";\n\nFROM SYSTEM IMPORT ADDRESS;\n\n");
        FPuts(file,GetString(MSG_M2COMMENT));
        FPuts(file,"\n");

        if (global->GenNumbers)
        {
            FPuts(file,"CONST\n");

            sym   = NULL;
            entry = (struct CatEntry *)global->Desc.mlh_Head;
            while (entry->str_Link.mln_Succ)
            {
                if (entry->str_IfDefSym != sym)
                {
                    gen = TRUE;
                    if (sym = entry->str_IfDefSym)
                        gen = FindSymbol(global,entry->str_IfDefSym);
                }

                if (gen || !sym)
                {
                    FPuts(file,"  ");
                    FPuts(file,entry->str_Name);
                    FPrintF(global,file," = 0%lxH;\n",entry->str_ID);
                }

                entry = (struct CatEntry *)entry->str_Link.mln_Succ;
            }
            FPuts(file,"\n");
        }

        if (global->GenStrings)
        {
            FPuts(file,"CONST\n");

            sym   = NULL;
            entry = (struct CatEntry *)global->Desc.mlh_Head;
            gen   = TRUE;
            while (entry->str_Link.mln_Succ)
            {
                if (entry->str_IfDefSym != sym)
                {
                    gen = TRUE;
                    if (sym = entry->str_IfDefSym)
                        gen = FindSymbol(global,entry->str_IfDefSym);
                }

                if (gen || !sym)
                {
                    FPuts(file,"  ");
                    FPuts(file,entry->str_Name);
                    FPuts(file,"STR = \"");

                    len = strlen(entry->str_OriginalLine);

                    if (entry->str_LengthBytes)
                    {
                        escLen = EscapeStr(global,entry->str_OriginalLine,FALSE,NULL);

                        switch (entry->str_LengthBytes)
                        {
                            case 1: sprintf(lengthBytes,"\\x%02lx",escLen % 256);
                                    break;

                            case 2: sprintf(lengthBytes,"\\x%02lx\\x%02lx",(escLen / 256) % 256, escLen % 256);
                                    break;

                            case 3: sprintf(lengthBytes,"\\x%02lx\\x%02lx\\x%02lx",(escLen / 65536) % 256, (escLen / 256) % 256, escLen % 256);
                                    break;

                            case 4: sprintf(lengthBytes,"\\x%02lx\\x%02lx\\x%02lx\\x%02lx",(escLen / (65536*256)) % 256, (escLen / 65536) % 256, (escLen / 256) % 256, escLen % 256);
                                    break;
                        }
                        FWrite(file,lengthBytes,strlen(lengthBytes),1);
                    }

                    if (len)
                        FWrite(file,entry->str_OriginalLine,len,1);

                    FPuts(file,"\";\n");
                }

                entry = (struct CatEntry *)entry->str_Link.mln_Succ;
            }

            FPuts(file,"\n");
        }

	FPuts(file,"TYPE\n");
	FPuts(file,"  LocaleInfoPtr = POINTER TO LocaleInfo;\n");
        FPuts(file,"  LocaleInfo = RECORD\n");
	FPuts(file,"                 liLocaleBase : ADDRESS;\n");
	FPuts(file,"                 liCatalog    : ADDRESS;\n");
	FPuts(file,"               END;\n\n\n");

	if (global->Prototype[0])
	{
	    FPuts(file,global->Prototype);
	    FPuts(file,"\n\n\n");
	}

        FPuts(file,"END ");
        FPuts(file,headName);
        FPuts(file,".\n");

        Close(file);

        if (global->GenCode)
        {
            strcpy(fileName,name);
            strcat(fileName,".mod");
            if (file = Open(fileName,MODE_NEWFILE))
            {
                SetVBuf(file,NULL,BUF_FULL,4096);

                FPuts(file,"IMPLEMENTATION MODULE ");
                FPuts(file,headName);
                FPuts(file,";\n\n");
                FPuts(file,global->M2Prolog);
                FPuts(file,"(");

                i = 0;
                while (i < 20)
                {
                    sprintf(lengthBytes,"0%04lxH, ",startM2Code[i]);
                    FPuts(file,lengthBytes);

                    i++;

                    if (i % 9 == 0)
                        FPuts(file,"\n    ");
                }
                FPuts(file,"\n");

                sym   = NULL;
                entry = (struct CatEntry *)global->Desc.mlh_Head;
                gen   = TRUE;
                while (entry->str_Link.mln_Succ)
                {
                    if (entry->str_IfDefSym != sym)
                    {
                        gen = TRUE;
                        if (sym = entry->str_IfDefSym)
                            gen = FindSymbol(global,entry->str_IfDefSym);
                    }

                    if (gen || !sym)
                    {
                        len = strlen(entry->str_OriginalLine);

                        if (!(buffer = AllocVec(len+1,MEMF_ANY)))
                        {
                            Close(file);
                            return(FALSE);
                        }

                        CopyMem(entry->str_OriginalLine,buffer,len+1);

                        escLen = EscapeStr(global,buffer,TRUE,&zeropad) + entry->str_LengthBytes;
                        zeros  = 0;

                        if (!zeropad)
                        {
                            zeros++;
                            escLen++;
                        }

                        if (escLen & 1)
                        {
                            zeros++;
                            escLen++;
                        }

                        sprintf(lengthBytes,"    0%02lx%02lxH, 0%02lx%02lxH, ",entry->str_ID / (256*256*256),
                                                                              (entry->str_ID / (256*256)) % 256,
                                                                              (entry->str_ID / 256) % 256,
                                                                               entry->str_ID % 256);
                        FWrite(file,lengthBytes,strlen(lengthBytes),1);

                        sprintf(lengthBytes,"0%04lxH,\n    ",escLen);
                        FWrite(file,lengthBytes,strlen(lengthBytes),1);

                        i = 0;
                        while (entry->str_OriginalLine[i])
                        {
                            if (i & 1)
                                sprintf(lengthBytes,"%02lxH, ",entry->str_OriginalLine[i]);
                            else
                                sprintf(lengthBytes,"0%02lx",entry->str_OriginalLine[i]);

                            FPuts(file,lengthBytes);

                            i++;

                            if (i % 18 == 0)
                                FPuts(file,"\n    ");
                        }

                        if (zeros == 1)
                            FPuts(file,"00H,\n");
                        else if (zeros == 2)
                            FPuts(file,"00000H,\n");
                        else
                            FPuts(file,"\n");
                    }

                    entry = (struct CatEntry *)entry->str_Link.mln_Succ;
                }


                FPuts(file,"  0);\n");
                FPuts(file,global->M2Epilog);
                FPuts(file,"\n\nEND ");
                FPuts(file,headName);
                FPuts(file,".\n");

                FreeVec(buffer);

                Close(file);
            }
            else
            {
                error = IoErr();
            }
        }
    }
    else
    {
        error = IoErr();
    }

    global->IoError = error;
    return((BOOL)(error == 0));
}


/*****************************************************************************/


static VOID PutASMString(struct Global *global, struct CatEntry *entry,
                         BPTR file, BOOL blockMode)
{
UWORD  escLen;
UWORD  padLen;
UWORD  i;
char   lengthBytes[80];
BOOL   quoted;
UWORD  cnt;
char   ch;
STRPTR buffer;
BOOL   zeropad;
UWORD  zeros;
UWORD  len;

    len = strlen(entry->str_OriginalLine);

    if (!(buffer = AllocVec(len+1,MEMF_ANY)))
    {
        /*** !!!! NO ERROR RETURN !!!! ***/
        return;
    }

    CopyMem(entry->str_OriginalLine,buffer,len+1);

    escLen = EscapeStr(global,buffer,TRUE,&zeropad) + entry->str_LengthBytes;
    padLen = 0;
    zeros  = 0;

    if (!zeropad)
    {
        zeros++;
        padLen++;
    }

    if (blockMode)
    {
        if ((escLen+padLen) & 1)
        {
            zeros++;
            padLen++;
        }
    }

    escLen += padLen;

    if (blockMode)
    {
        sprintf(lengthBytes,"\tDC.L $%lx\n\tDC.W $%lx\n\tDC.B ",entry->str_ID,escLen);
        FWrite(file,lengthBytes,strlen(lengthBytes),1);
    }

    if (entry->str_LengthBytes)
    {
        switch (entry->str_LengthBytes)
        {
            case 1: sprintf(lengthBytes,"%lu,",escLen % 256);
                    break;

            case 2: sprintf(lengthBytes,"%lu,%lu,",(escLen / 256) % 256, escLen % 256);
                    break;

            case 3: sprintf(lengthBytes,"%lu,%lu,%lu,",(escLen / 65536) % 256, (escLen / 256) % 256, escLen % 256);
                    break;

            case 4: sprintf(lengthBytes,"%lu,%lu,%lu,%lu,",(escLen / (65536*256)) % 256, (escLen / 65536) % 256, (escLen / 256) % 256, escLen % 256);
                    break;
        }
        FWrite(file,lengthBytes,strlen(lengthBytes),1);
    }

    escLen = escLen - entry->str_LengthBytes - padLen;

    i = 0;
    while (i < escLen)
    {
        if (i > 0)
            FPuts(file,"\n\tDC.B ");

        FPuts(file,"'");

        quoted = TRUE;
        cnt    = 0;
        while ((i < escLen) && (cnt < 70))
        {
            ch = buffer[i];
            if ((ch == '\n') || (ch == 0) || (ch == '\r') || (ch == '\t') || (ch == '\''))
            {
                if (quoted)
                {
                    FPuts(file,"'");
                    quoted = FALSE;
                }
                FPrintF(global,file,",%ld",(ULONG)ch);
            }
            else
            {
                if (!quoted)
                {
                    FPuts(file,",'");
                    quoted = TRUE;
                }
                FPutC(file,ch);
            }
            i++;
            cnt++;
        }

        if (quoted)
            FPuts(file,"'");
    }

    if (zeros == 1)
    {
        if (escLen != 0)
            FPuts(file,",");

        FPuts(file,"$00\n");
    }
    else if (zeros == 2)
    {
        if (escLen != 0)
            FPuts(file,",");

        FPuts(file,"$00,$00\n");
    }
    else
    {
        FPuts(file,"\n");
    }

    FreeVec(buffer);
}


/*****************************************************************************/


static BOOL WriteASMFile(struct Global *global, STRPTR name)
{
BPTR             file;
LONG             error = 0;
struct CatEntry *entry;
ULONG		 i;
STRPTR           sym;
char             headName[256];
BOOL             gen;

    if (global->HeaderName[0])
    {
        strcpy(headName,global->HeaderName);
    }
    else
    {
        strcpy(headName,FilePart(name));
        i = 0;
        while (headName[i] && (headName[i] != '.'))
        {
            headName[i] = ToUpper(headName[i]);
            i++;
        }
        headName[i] = 0;
    }
    strcat(headName,"_I");

    if (file = Open(name,MODE_NEWFILE))
    {
        SetVBuf(file,NULL,BUF_FULL,4096);

        FPuts(file,"\tIFND ");
        FPuts(file,headName);
        FPuts(file,"\n");
        FPuts(file,headName);
        FPuts(file,"\tSET\t1\n\n\n;-----------------------------------------------------------------------------\n\n");
        FPuts(file,GetString(MSG_ASMCOMMENT));

        if (global->GenNumbers)
        {
            FPuts(file,"\tIFD CATCOMP_NUMBERS\n\n");

            sym   = NULL;
            entry = (struct CatEntry *)global->Desc.mlh_Head;
            gen   = TRUE;
            while (entry->str_Link.mln_Succ)
            {
                if (entry->str_IfDefSym != sym)
                {
                    if (sym && gen)
                    {
                        FPuts(file,"\tENDC ; ");
                        FPuts(file,sym);
                        FPuts(file,"\n");
                    }

                    gen = TRUE;
                    if (sym = entry->str_IfDefSym)
                    {
                        if (gen = FindSymbol(global,entry->str_IfDefSym))
                        {
                            FPuts(file,"\n\tIFD ");
                            FPuts(file,entry->str_IfDefSym);
                            FPuts(file,"\n");
                        }
                    }
                }

                if (gen || !sym)
                {
                    FPuts(file,entry->str_Name);
                    FPuts(file," EQU ");
                    FPrintF(global,file,"%ld",entry->str_ID);
                    FPuts(file,"\n");
                }

                entry = (struct CatEntry *)entry->str_Link.mln_Succ;
            }

            if (sym && gen)
            {
                FPuts(file,"\tENDC ; ");
                FPuts(file,sym);
                FPuts(file,"\n");
            }

            FPuts(file,"\n\tENDC ; CATCOMP_NUMBERS\n");
            FPuts(file,"\n\n;-----------------------------------------------------------------------------\n\n\n");
        }

        if (global->GenStrings)
        {
            FPuts(file,"\tIFD CATCOMP_STRINGS\n\n");

            sym   = NULL;
            entry = (struct CatEntry *)global->Desc.mlh_Head;
            gen   = TRUE;
            while (entry->str_Link.mln_Succ)
            {
                if (entry->str_IfDefSym != sym)
                {
                    if (sym && gen)
                    {
                        FPuts(file,"\tENDC ; ");
                        FPuts(file,sym);
                        FPuts(file,"\n");
                    }

                    gen = TRUE;
                    if (sym = entry->str_IfDefSym)
                    {
                        if (gen = FindSymbol(global,entry->str_IfDefSym))
                        {
                            FPuts(file,"\n\tIFD ");
                            FPuts(file,entry->str_IfDefSym);
                            FPuts(file,"\n");
                        }
                    }
                }

                if (gen || !sym)
                {
                    FPuts(file,entry->str_Name);
                    FPuts(file,"_STR: DC.B ");

                    PutASMString(global,entry,file,FALSE);
                }

                entry = (struct CatEntry *)entry->str_Link.mln_Succ;
            }

            if (sym && gen)
            {
                FPuts(file,"\tENDC ; ");
                FPuts(file,sym);
                FPuts(file,"\n");
            }

            FPuts(file,"\n\tENDC ; CATCOMP_STRINGS\n");
            FPuts(file,"\n\n;-----------------------------------------------------------------------------\n\n\n");
        }

        if (global->GenArray)
        {
            FPuts(file,"\tIFD CATCOMP_ARRAY\n\n");

            FPuts(file,"   STRUCTURE ");
            FPuts(file,global->ArrayName);
            FPuts(file,"Type,0\n\tLONG cca_ID\n\tAPTR cca_Str\n   LABEL ");
            FPuts(file,global->ArrayName);
            FPuts(file,"Type_SIZEOF\n");
            FPuts(file,"\n\tCNOP 0,4\n\n");
            FPuts(file,global->ArrayName);
            FPuts(file,":\n");

            sym   = NULL;
            i     = 0;
            entry = (struct CatEntry *)global->Desc.mlh_Head;
            gen   = TRUE;
            while (entry->str_Link.mln_Succ)
            {
                if (entry->str_IfDefSym != sym)
                {
                    if (sym && gen)
                    {
                        FPuts(file,"\tENDC ; ");
                        FPuts(file,sym);
                        FPuts(file,"\n");
                    }

                    gen = TRUE;
                    if (sym = entry->str_IfDefSym)
                    {
                        if (gen = FindSymbol(global,entry->str_IfDefSym))
                        {
                            FPuts(file,"\n\tIFD ");
                            FPuts(file,entry->str_IfDefSym);
                            FPuts(file,"\n");
                        }
                    }
                }

                if (gen || !sym)
                {
                    FPrintF(global,file,"AS%ld:\tDC.L ",i);
                    FPuts(file,entry->str_Name);
                    FPuts(file,",");
                    FPuts(file,entry->str_Name);
                    FPuts(file,"_STR\n");
                }

                i++;
                entry = (struct CatEntry *)entry->str_Link.mln_Succ;
            }

            if (sym && gen)
            {
                FPuts(file,"\tENDC ; ");
                FPuts(file,sym);
                FPuts(file,"\n");
            }

            FPuts(file,"\n\tENDC ; CATCOMP_ARRAY\n");
            FPuts(file,"\n\n;-----------------------------------------------------------------------------\n\n\n");
        }

        if (global->GenBlock)
        {
            FPuts(file,"\tIFD CATCOMP_BLOCK\n\n");
            FPuts(file,global->BlockName);
            FPuts(file,":\n");

            sym   = NULL;
            entry = (struct CatEntry *)global->Desc.mlh_Head;
            gen   = TRUE;
            while (entry->str_Link.mln_Succ)
            {
                if (entry->str_IfDefSym != sym)
                {
                    if (sym && gen)
                    {
                        FPuts(file,"\tENDC ; ");
                        FPuts(file,sym);
                        FPuts(file,"\n");
                    }

                    gen = TRUE;
                    if (sym = entry->str_IfDefSym)
                    {
                        if (gen = FindSymbol(global,entry->str_IfDefSym))
                        {
                            FPuts(file,"\n\tIFD ");
                            FPuts(file,entry->str_IfDefSym);
                            FPuts(file,"\n");
                        }
                    }
                }

                if (gen || !sym)
                {
                    PutASMString(global,entry,file,TRUE);
                }

                entry = (struct CatEntry *)entry->str_Link.mln_Succ;
            }

            if (sym && gen)
            {
                FPuts(file,"\tENDC ; ");
                FPuts(file,sym);
                FPuts(file,"\n");
            }

            FPuts(file,"\n\tENDC ; CATCOMP_BLOCK\n");
            FPuts(file,"\n\n;-----------------------------------------------------------------------------\n\n\n");
        }

	FPuts(file,"   STRUCTURE LocaleInfo,0\n");
	FPuts(file,"\tAPTR li_LocaleBase\n");
	FPuts(file,"\tAPTR li_Catalog\n");
	FPuts(file,"   LABEL LocaleInfo_SIZEOF\n\n");

        if (global->GenCode)
        {
            FPuts(file,"\tIFD CATCOMP_CODE\n\n");

            FPuts(file,"\tXREF      _LVOGetCatalogStr\n");
            FPuts(file,"\tXDEF      _");
            FPuts(file,global->FunctionName);
            FPuts(file,"\n");
            FPuts(file,"\tXDEF      ");
            FPuts(file,global->FunctionName);
            FPuts(file,"\n");
            FPuts(file,global->FunctionName);
            FPuts(file,":\n_");
            FPuts(file,global->FunctionName);
            FPuts(file,":\n");
            FPuts(file,"\tlea       ");
            FPuts(file,global->BlockName);
            FPuts(file,"(pc),a1\n");
            FPuts(file,"\tbra.s     2$\n");
            FPuts(file,"1$: move.w  (a1)+,d1\n");
            FPuts(file,"\tadd.w     d1,a1\n");
            FPuts(file,"2$: cmp.l   (a1)+,d0\n");
            FPuts(file,"\tbne.s     1$\n");
            FPuts(file,"\taddq.l    #2,a1\n");
            FPuts(file,"\tmove.l    (a0)+,d1\n");
            FPuts(file,"\tbne.s     3$\n");
            FPuts(file,"\tmove.l    a1,d0\n");
            FPuts(file,"\trts\n");
            FPuts(file,"3$: move.l  a6,-(sp)\n");
            FPuts(file,"\tmove.l    d1,a6\n");
            FPuts(file,"\tmove.l    (a0),a0\n");
            FPuts(file,"\tjsr       _LVOGetCatalogStr(a6)\n");
            FPuts(file,"\tmove.l    (sp)+,a6\n");
            FPuts(file,"\trts\n");
            FPuts(file,"\tEND\n");

            FPuts(file,"\n\tENDC ; CATCOMP_CODE\n");
            FPuts(file,"\n\n;-----------------------------------------------------------------------------\n\n");
        }

        FPuts(file,"\n\tENDC ; ");
        FPuts(file,headName);
        FPuts(file,"\n");

        Close(file);
    }
    else
    {
        error = IoErr();
    }

    global->IoError = error;
    return((BOOL)(error == 0));
}


/*****************************************************************************/


static ULONG startObj[] =
{
    0x000003E7, 0x00000000, 0x000003E9, 0x00000000,
    0x43FA0026, 0x60043219, 0xD2C1B099, 0x66F85489,
    0x22186604, 0x20094E75, 0x2F0E2C41, 0x20504EAE,
    0xFFB82C5F, 0x4E750000
};


static ULONG endObj[] =
{
    0x000003EF, 0x01000003, 0x5F476574, 0x53747269, 0x6E670000,
    0x00000000, 0x01000003, 0x47657453, 0x7472696E, 0x67000000,
    0x00000000, 0x00000000, 0x000003F2
};

static BOOL WriteObjFile(struct Global *global, STRPTR name)
{
BPTR             file;
LONG             error = 0;
struct CatEntry *entry;
ULONG            length;
ULONG            len;
STRPTR           buffer;
UWORD            escLen;
UWORD            padLen;
UWORD            zeros;
BOOL             zeropad;
BOOL             gen;
STRPTR           sym;

    if (file = Open(name,MODE_NEWFILE))
    {
        SetVBuf(file,NULL,BUF_FULL,4096);

        /* length of startObj's code section */
        length = 40;

        FWrite(file,startObj,sizeof(startObj),1);

        sym   = NULL;
        entry = (struct CatEntry *)global->Desc.mlh_Head;
        gen   = TRUE;
        while (entry->str_Link.mln_Succ)
        {
            if (entry->str_IfDefSym != sym)
            {
                gen = TRUE;

                if (entry->str_IfDefSym)
                    gen = FindSymbol(global,entry->str_IfDefSym);

                sym = entry->str_IfDefSym;
            }

            if (gen)
            {
                len = strlen(entry->str_OriginalLine);

                if (!(buffer = AllocVec(len+1,MEMF_ANY)))
                {
                    Close(file);
                    return(FALSE);
                }

                CopyMem(entry->str_OriginalLine,buffer,len+1);

                escLen = EscapeStr(global,buffer,TRUE,&zeropad) + entry->str_LengthBytes;
                padLen = 0;
                zeros  = 0;

                if (!zeropad)
                {
                    zeros++;
                    padLen++;
                }

                if ((escLen+padLen) & 1)
                {
                    zeros++;
                    padLen++;
                }

                escLen += padLen;

                FWrite(file,&entry->str_ID,4,1);
                FWrite(file,&escLen,2,1);
                length += 6;

                if (entry->str_LengthBytes)
                {
                    switch (entry->str_LengthBytes)
                    {
                        case 1: FWrite(file,(APTR)((ULONG)&entry->str_LengthBytes + 3),1,1);
                                length += 1;
                                break;

                        case 2: FWrite(file,(APTR)((ULONG)&entry->str_LengthBytes + 2),2,1);
                                length += 2;
                                break;

                        case 3: FWrite(file,(APTR)((ULONG)&entry->str_LengthBytes + 1),3,1);
                                length += 3;
                                break;

                        case 4: FWrite(file,&entry->str_LengthBytes,4,1);
                                length += 4;
                                break;
                    }
                }

                escLen = escLen - entry->str_LengthBytes - padLen;

                FWrite(file,buffer,escLen,1);
                length += escLen;

                if (zeros == 1)
                {
                    FWrite(file,"\0",1,1);
                    length += 1;
                }
                else if (zeros == 2)
                {
                    FWrite(file,"\0\0",2,1);
                    length += 2;
                }

                FreeVec(buffer);
            }

            entry = (struct CatEntry *)entry->str_Link.mln_Succ;
        }

        if (length % 4)
        {
            FWrite(file,"\0\0\0",4 - length % 4,1);
            length += (4 - length % 4);
        }

        FWrite(file,endObj,sizeof(endObj),1);
        Flush(file);

        Seek(file,12,OFFSET_BEGINING);
        length = length / 4;
        Write(file,&length,4);

        Close(file);
    }
    else
    {
        error = IoErr();
    }

    global->IoError = error;
    return((BOOL)(error == 0));
}
