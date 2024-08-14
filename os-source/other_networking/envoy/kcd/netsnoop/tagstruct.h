

#ifndef TAGSTRUCT_H
#define TAGSTRUCT_H

/* needed by struct globals and by netsnoop.c */
struct Opts {
    STRPTR *query;
    ULONG  *maxTime;     /* max time inquiry can take, in seconds */
    ULONG  *maxResp;     /* max number of responses to process    */
    STRPTR ipaddr;       /* need to convert... */
    STRPTR realm;
    STRPTR hostName;
    STRPTR service;
    STRPTR entity;
    STRPTR owner;        /* currently ignored; not documented */
    STRPTR machDesc;     /* undocumented */
    STRPTR attnFlags;    /* hex; cpu types, see execbase.AttnFlags */
    STRPTR libVersion;   /* currently ignored; not documented */
    STRPTR chipRevBits;  /* most likely hex */
    ULONG *maxFastMem;
    ULONG *availFastMem;
    ULONG *maxChipMem;
    ULONG *availChipMem;
    STRPTR kickVersion;
    STRPTR wbVersion;
    STRPTR nipcVersion;
    LONG  noAutoQuit;
    LONG  help;
    LONG  examineInput;  /* DEBUG... prints arguments to NIPCInquiryA() */
};

/* needed by printTags.c and nethook.c */
struct Globals {
    struct Hook *myHook;
    struct Opts *opts;
    ULONG  maxTime;
    ULONG  maxResp;
    struct TagItem *inqTags;
    struct RDArgs *rdargs;
    struct Library *UtilityBase;
    struct Library *NIPCBase;
    ULONG  ipaddr_real;        /* string addr (209.132.5.234) to ULONG, converted                   */
    int    rc;                 /* return code for any MATCH_ things                                 */
    struct TagItem *doneTags;  /* hook will signal when this is ready                               */
    ULONG  hookDone;           /* continue hook processing?                                         */
    struct List foundList;     /* added to by the hook                                              */
    struct List *l;            /* shorthand for most funcs which need it :-)                        */
    ULONG  addr_tag;           /* tag to expect back---query or match ipaddress                     */
    ULONG  pr_ipaddr;          /* whether to really print the ipaddress, since we always ask for it */
    ULONG  err2;               /* further error info for debugging */
    ULONG  clonedTags;         /* debugging */
    ULONG  hostLen;            /* tmp debug */
    ULONG  svcLen;             /* tmp debug */
    ULONG  sigMask;
    BYTE   sigBit;
};

/* defines for globals.err2 */
#define NS_NO_ERROR         0
#define NS_ERR_TAG_MISMATCH 1
#define NS_ERR_NO_MEM       2
#define NS_ERR_CLONEFAIL    3
#define NS_ERR_NULL_STRING  4
#define NS_ERR_EMPTY_STRING 5
#define NS_ERR_NULL_ARG     6

/* Tags for all-encompassing NIPCInquiry() calls */
#define Q_IPADDR		0
#define M_IPADDR		1
#define S_IPADDR        "ADDRS"
#define Q_REALMS		2
#define M_REALM		    3
#define S_REALMS        "REALMS"
#define Q_HOSTNAME		4
#define M_HOSTNAME		5
#define S_HOSTS         "HOSTS"
#define Q_SERVICE		6
#define M_SERVICE		7
#define S_SERVICE       "SERVICES"
#define Q_ENTITY		8
#define M_ENTITY		9
#define S_ENTITY        "ENTITIES"
#define Q_OWNER		    10
#define M_OWNER		    11
#define S_OWNER         "OWNERS"
#define Q_MACHDESC		12
#define M_MACHDESC		13
#define S_MACHDESC      "MACHDESCS"
#define Q_ATTNFLAGS		14
#define M_ATTNFLAGS		15
#define S_ATTNFLAGS     "CPUFLAGS"
#define Q_LIBVERSION	16
#define M_LIBVERSION	17
#define S_LIBVERSION    "LIBVERS"
#define Q_CHIPREVBITS	18
#define M_CHIPREVBITS	19
#define S_CHIPREVBITS   "CHIPREVS"
#define Q_MAXFASTMEM	20
#define M_MAXFASTMEM	21
#define S_MAXFASTMEM    "MAXFAST"
#define Q_AVAILFASTMEM	22
#define M_AVAILFASTMEM	23
#define S_AVAILFASTMEM  "AVAILFAST"
#define Q_MAXCHIPMEM	24
#define M_MAXCHIPMEM	25
#define S_MAXCHIPMEM    "MAXCHIP"
#define Q_AVAILCHIPMEM	26
#define M_AVAILCHIPMEM	27
#define S_AVAILCHIPMEM  "AVAILCHIP"
#define Q_KICKVERSION	28
#define M_KICKVERSION	29
#define S_KICKVERSION   "KICKVERS"
#define Q_WBVERSION		30
#define M_WBVERSION		31
#define S_WBVERSION     "WBVERS"
#define Q_NIPCVERSION	32
#define M_NIPCVERSION	33
#define S_NIPCVERSION   "NIPCVERS"
#define Q_ENDSTRUCT     34       /* for TAG_DONE */
#define NUM_TAGITEMS    35


#endif /* TAGSTRUCT_H */
