/* protos.h */

/* Prototypes for functions defined in transfer.c */
LONG read(struct lock *lock,
          CPTR buf,
          LONG bsize);
LONG write(struct lock *lock,
           CPTR buf,
           LONG bsize);
struct data *growblock(struct data *p,
		       struct node *node);

/* Prototypes for functions defined in seek.c */
LONG seek(struct lock *lock,
         LONG poscount,
         LONG frompos,
	 LONG truncate);

/* Prototypes for functions defined in renamedisk.c */
LONG renamedisk(struct DosPacket *pkt);

/* Prototypes for functions defined in rename.c */
LONG rename(struct lock *from_dir,
           char *from_str,
           struct lock *to_dir,
           char *to_str);

/* Prototypes for functions defined in parent.c */
struct lock *parentfh(struct lock *lock, LONG action);

/* Prototypes for functions defined in mygetvec.c */
void *mygetvec(ULONG n);
void *getvec(ULONG size);

/* Prototypes for functions defined in lock.c */
struct node *checklock(struct lock *lock);
struct lock *getlock(struct node *ptr,
                     LONG access);
LONG freelock(struct lock *lock);
struct lock * find_lock(struct lock *p,
			struct node *node);
LONG change_lock(struct lock *lock,
		 LONG newmode);

/* Prototypes for functions defined in locate.c */
struct lock *locate(struct lock *lock,
     	            char *str,
		    LONG mode);
struct node *locatenode(struct lock *lock,
			char *str,
			LONG follow_links);
struct node *findnode(struct node *dptr,
		      char *str,
		      LONG follow_links);

/* Prototypes for functions defined in findentry.c */
LONG findentry(struct node *dptr,
               char *name,
	       LONG follow_links);

/* Prototypes for functions defined in finddir.c */
struct node *finddir(struct node *dptr,
                     char *string,
                     char *name);

/* Prototypes for functions defined in file.c */
LONG openmakefile(struct FileHandle *scb,
                 struct lock *dlock,
                 char *string,
                 int type);
LONG openfromlock(struct FileHandle *scb,
		  struct lock *lock);

/* Prototypes for functions defined in exall.c */
LONG exnext(struct DosPacket *pkt,
            LONG object);
LONG exall (struct node *node,
	    struct DosPacket *dp);
LONG fill_data (struct ExAllData *ed,
		ULONG data,
		struct node *node,
		LONG *size);

LONG setinfo(struct FileInfoBlock *ivec,
            struct node *ptr);

/* Prototypes for functions defined in diskinfo.c */
LONG diskinfo(struct DosPacket *pkt);

/* Prototypes for functions defined in deletedir.c */
void deletedir(struct node *node);

/* Prototypes for functions defined in delete.c */
LONG delete(struct node *dptr,
           char *string,
	   long notify);
void remove_node(struct node *node);

/* Prototypes for functions defined in create.c */
struct lock *create(struct lock *lock,
                    char *string,
		    LONG mode,
		    LONG is_dir);

/* Prototypes for functions defined in comment.c */
LONG comment(struct DosPacket *pkt,
	     char *objname);

/* Prototypes for functions defined in checkname.c */
LONG checkname(char *name);

/* Prototypes for functions defined in start.c */
void start(struct DosPacket *dp);

/* Prototypes for functions defined in closefile.c */
LONG closefile(struct lock *lock);
struct data *swapblock(struct data *oldblock,
		       LONG size,
		       LONG xfersize);

/* Prototypes for functions defined in makelink.c */
LONG makelink(struct lock *lock,
	      UBYTE *string,
	      LONG arg,
	      LONG soft);
LONG readlink(struct lock *lock,
	      UBYTE *string,
	      UBYTE *buffer,
	      ULONG size);

/* Prototypes for functions defined in notify.c */
LONG addnotify(struct NotifyRequest *req);
LONG remnotify(struct NotifyRequest *req);
struct node *exists(char *file);
void notify(struct lock *lock);
void notifynode(struct node *node);
void do_notify(struct notify *notify);
void find_notifies(struct node *node,
		  long flag);
struct NotifyMessage *FindMsg(void);
void rem_notifies(struct node *node,
		 long flag);
void AddFreeMsg(struct NotifyMessage *msg);
char *tail(char *str);

/* Prototypes for functions defined in notify.c */
LONG lockrecord (struct lock *lock,struct DosPacket *dp);
LONG find_rlock (struct lock *lock, ULONG offset, ULONG length, ULONG mode);
LONG rlock_wait_add (struct lock *lock, struct DosPacket *dp);
LONG unlockrecord (struct lock *lock, ULONG offset, ULONG length);
void wakeup (struct node *node, ULONG offset, ULONG top);
void kill_rwait (struct timerequest *iob);

/* external stuff */

extern char *BtoC(char *string,BPTR bstr);
extern void CtoBcpy(char *dest,char *src);
extern void xfer(char *from,char *to,ULONG number);
extern LONG mult32(LONG val1,LONG val2);
extern LONG div32(LONG val1,LONG val2);
extern LONG rem32(LONG val1,LONG val2);
extern LONG freelist(struct node *list);	/* was unloadseg */
extern void NewList(struct List *list);
extern void freevec(void *ptr);

/* assembler function in ram_c.a */

LONG do_match(struct ExAllControl *ec, ULONG data, struct ExAllData *ed);
