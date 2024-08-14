
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <libraries/expansion.h>
#include <libraries/configvars.h>
#include <workbench/workbench.h>
#include <workbench/icon.h>
#include <string.h>
#include <dos.h>

#include <clib/exec_protos.h>
#include <clib/expansion_protos.h>
#include <clib/icon_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/expansion_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "binddrivers_rev.h"


/*****************************************************************************/


char version[] = VERSTAG;


/*****************************************************************************/


struct Library *ExpansionBase;
struct Library *DOSBase;


/*****************************************************************************/


static struct ConfigDev *TestProduct(struct ConfigDev *cd, STRPTR s, LONG len);
static struct ConfigDev *TestManufacturer(STRPTR s);
static struct Resident *FindRomTag(BPTR seg);
static char *Index(STRPTR s, char c);
static char *Indexn(STRPTR s, char c, LONG len);
static BOOL NonAlpha(STRPTR s, LONG len);
static struct ConfigDev *MarkConfigDev(struct ConfigDev *cd, LONG manufacturer,
                                LONG product);


/*****************************************************************************/


LONG main(VOID)
{
struct Library                 *SysBase = (*((struct Library **) 4));
struct Library                 *IconBase;
struct FileInfoBlock __aligned  fib;
BPTR                            oldCD;
BPTR                            lock;
BPTR                            segment;
struct Resident                *res;
STRPTR                          productstring;
struct ConfigDev               *cd;
struct DiskObject              *diskObj;
LONG                            len;
struct CurrentBinding           curbind;
LONG                            failureLevel = RETURN_FAIL;
struct WBStartup               *WBenchMsg = NULL;
struct Process                 *process;

    geta4();

    process = (struct Process *)FindTask(NULL);
    if (!process->pr_CLI)
    {
        WaitPort(&process->pr_MsgPort);
        WBenchMsg = (struct WBStartup *) GetMsg(&process->pr_MsgPort);
    }

    if (DOSBase = OpenLibrary("dos.library",37))
    {
        if (ExpansionBase = OpenLibrary("expansion.library",37))
        {
            if (IconBase = OpenLibrary("icon.library",37))
            {
                failureLevel = RETURN_WARN;

                if (lock = Lock("SYS:Expansion",SHARED_LOCK))
                {
                    if (Examine(lock,&fib) && (fib.fib_DirEntryType > 0))
                    {
                        ObtainConfigBinding();

                        failureLevel = RETURN_OK;

                        oldCD = CurrentDir(lock);

                        while (ExNext(lock,&fib))
                        {
                            len = strlen(fib.fib_FileName);
                            if ((len >= 5) && (stricmp(&fib.fib_FileName[len-5],".info") == 0))
                            {
                                fib.fib_FileName[len-5] = 0;

                                if (diskObj = GetDiskObject(fib.fib_FileName))
                                {
                                    if (productstring = FindToolType(diskObj->do_ToolTypes,"PRODUCT"))
                                    {
                                        if (cd = TestManufacturer(productstring))
                                        {
                                            if (segment = LoadSeg(fib.fib_FileName))
                                            {
                                                if (res = FindRomTag(segment))
                                                {
                                                    curbind.cb_ProductString = productstring;
                                                    curbind.cb_ToolTypes     = diskObj->do_ToolTypes;
                                                    curbind.cb_FileName      = fib.fib_FileName;
                                                    curbind.cb_ConfigDev     = cd;
                                                    SetCurrentBinding(&curbind,sizeof(struct CurrentBinding));

                                                    if (InitResident(res,segment))
                                                        segment = NULL; /* avoid UnLoadSeg() */
                                                }
                                                UnLoadSeg(segment);
                                            }
                                        }
                                    }
                                    FreeDiskObject(diskObj);
                                }
                            }
                        }
                        CurrentDir(oldCD);

                        ReleaseConfigBinding();
                    }
                    UnLock(lock);
                }
                CloseLibrary(IconBase);
            }
            CloseLibrary(ExpansionBase);
        }
        CloseLibrary(DOSBase);
    }

    if (WBenchMsg)
    {
    	Forbid();
    	ReplyMsg(WBenchMsg);
    }

    return(failureLevel);
}


/*****************************************************************************/


/* FindRomTag( seg ) -- search for a romtag in the first hunk
 * of seg
 */

static struct Resident *FindRomTag(BPTR seg)
{
struct Resident *rt;
LONG             size;
ULONG           *ptr;

    /* turn into a memory address */
    ptr = (ULONG *) (seg << 2);

    size = ptr[-1] - 8;

    rt = (struct Resident *) &ptr[1];

    while (size > sizeof(struct Resident))
    {
	if (rt->rt_MatchWord == RTC_MATCHWORD && rt->rt_MatchTag == rt)
        {
	    return(rt);
	}

	/* advance to the next word boundary */
	size -= 2;
	rt = (struct Resident *)(2 + (ULONG)rt);
    }

    return(NULL);
}


/*****************************************************************************/


/* TestManufacturer( s ) -- look for manufacturer string
 * in database.  Format of string is (in yacc format):
 *
 *	productlist =	product |
 *			productlist BAR product ;
 *
 *	product =	manufacturer |
 *			manufacturer SLASH code ;
 *
 *	manufacturer =	number ;
 *	code =		number ;
 *
 *	BAR =		'|' ;
 *	SLASH =		'/' ;
 *
 * This routine returns a pointer to the first configdev that matches.
 */

static struct ConfigDev *TestManufacturer(STRPTR s)
{
STRPTR            end;
LONG              len;
struct ConfigDev *cd = NULL;

    /* parse productlist */
    do
    {
	if (end = Index(s,'|'))
            len = end++ - s;
	else
            len = strlen(s);

	cd = TestProduct(cd,s,len);
    }
    while (s = end);

    return(cd);
}


/*****************************************************************************/


static struct ConfigDev *TestProduct(struct ConfigDev *cd, STRPTR s, LONG len)
{
STRPTR end;
LONG   numlen;
LONG   manufacturer, product;

    /* look for <%ld> or <%ld/%ld> */
    if (end = Indexn(s,'/',len))
        numlen = end++ - s;
    else
        numlen = len;

    if (NonAlpha(s,numlen))
        return(NULL);

    StrToLong(s,&manufacturer);

    if (end)
    {
	numlen = len - numlen - 1;
	if (NonAlpha(end,numlen))
            return(NULL);

	StrToLong(end,&product);
    }
    else
    {
	product = -1;
    }

    return(MarkConfigDev(cd,manufacturer,product));
}


/*****************************************************************************/


/* It is possible to construct a product string such that a ConfigDev
 * structure meets more than one criteria.  This routine prevents it
 * from being linked into the list more than once.
 */
static BOOL CheckLink(struct ConfigDev *cd, struct ConfigDev *newcd)
{
    while (cd)
    {
	if (cd == newcd)
            return(TRUE);

	cd = cd->cd_NextCD;
    }

    return(FALSE);
}


/*****************************************************************************/


static struct ConfigDev *MarkConfigDev(struct ConfigDev *cd, LONG manufacturer,
                                       LONG product)
{
struct ConfigDev *newcd = NULL;

    while (newcd = FindConfigDev(newcd,manufacturer,product))
    {
	if ((newcd->cd_Flags & CDF_CONFIGME) && (!CheckLink(cd,newcd)))
        {
	    newcd->cd_NextCD = cd;
	    cd               = newcd;
	}
    }

    return(cd);
}


/*****************************************************************************/


static BOOL NonAlpha(STRPTR s, LONG len)
{
char c;

    if (len == 0)
        return(TRUE);

    while (len-- > 0)
    {
	c = *s++;
	if (c < '0' || c > '9')
          return(TRUE);
    }
    return(FALSE);
}


/*****************************************************************************/


static char *Index(STRPTR s, char c)
{
char *result;

    result = s;
    while (*result && (*result != c))
    {
        result++;
    }

    if (!*result)
        return(NULL);

    return(result);
}


/*****************************************************************************/


static char *Indexn(STRPTR s, char c, LONG len)
{
char *result;

    result = Index(s,c);
    if (result > s + len)
        result = NULL;

    return (result);
}
