/*
 * $Id: asmproto.h,v 38.5 92/05/14 18:07:56 mks Exp $
 *
 * $Log:	asmproto.h,v $
 * Revision 38.5  92/05/14  18:07:56  mks
 * Oops, don't want the prototype...
 * /.
 * 
 * Revision 38.4  92/05/14  18:04:55  mks
 * Some new prototypes to replace the built-in functions (and thus reduce
 * code size)
 *
 * Revision 38.3  91/11/12  18:43:19  mks
 * Updated prototypes as needed
 *
 * Revision 38.2  91/09/12  10:08:35  mks
 * Added prototypes for the open/close of the WB catalog
 *
 * Revision 38.1  91/06/24  11:33:43  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

/*  Prototypes for functions in abs.asm: */
extern ULONG LoadToolSegment;

/* Prototypes for functions defined in alists.asm: */

struct WBObject *MasterSearch();
struct WBObject *SelectSearch();
struct WBObject *UtilitySearch();
struct WBObject *UtilityRevSearch();
struct WBObject *SiblingSuccSearch();
struct WBObject *SiblingPredSearch();
struct WBObject *NodeToObj(struct Node *);
struct ActiveDisk *ActiveSearch();
void SearchList(struct List *, ...);
struct Node *PeekTail(struct List *);
ULONG SizeList(struct List *);
struct WBObject *LastSelected();

/* Prototypes for functions defined in coroutines.asm: */
LONG cocall(struct CoBase *);
void cowait(long);

/* Prototypes for functions defined in kprintf.asm: */
void kprintf(char *s, ...);

/* Prototypes for functions defined in string.asm: */

long stricmp(char *, char *);
long stricmpn(char *, char *, long);

/* Prototypes for functions defined in syscalls.asm: */

void setResult2(long);
void setWbBase(struct WorkbenchBase *);

/* Prototypes for OpenWBCat() and CloseWBCat() */

void OpenWBCat(void);
void CloseWBCat(void);

/* Replace build-in strlen */
#ifdef	strlen
#undef	strlen
#endif

#define	strlen	strlen

/* Replace built-in strcpy */
#ifdef	strcpy
#undef	strcpy
#endif

#define	strcpy	strcpy
