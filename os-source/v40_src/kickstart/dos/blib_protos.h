/* blib_protos.h */
/* use ANSI token concatenation operator - ## */

#define ASM __asm
#define REG(x)	register __ ## x

LONG ASM fault (REG(d1) LONG code, REG(d2) UBYTE *header);	/* GLOBAL */
void ASM cpymax (REG(a0) UBYTE **dest, REG(a1) UBYTE *src,
		 REG(a2) ULONG *len);
LONG ASM Fault (REG(d1) LONG code, REG(d2) UBYTE *header,
		REG(d3) UBYTE *buffer, REG(d4) LONG len);	/* NEW */
BPTR ASM noreqloadseg (REG(d1) char *name);			/* GLOBAL */
LONG ASM err_report (REG(d1) LONG type,REG(d2) BPTR arg,	/* GLOBAL */
		     REG(d3) struct MsgPort *arg2);

LONG ASM ErrorReport (REG(d1) LONG code, REG(d2) LONG type,	/* NEW */
		      REG(d3) BPTR arg,
		      REG(d4) struct MsgPort *arg2);

void __regargs closedisk (struct MsgPort *task);

void ASM endtask (REG(d1) BPTR seg);				/* GLOBAL */
void ASM dotimer (REG(d1) LONG request,				/* GLOBAL */
		  REG(d2) struct timerequest *iob,
		  REG(d3) LONG secs,
		  REG(d4) LONG micro);
void ASM delay (REG(d1) ULONG ticks);				/* GLOBAL */

/* BCPL io routines, equivalent to stdio */
LONG ASM rdch (void);						/* GLOBAL */
LONG ASM FGetC (REG(d1) BPTR fh);				/* GLOBAL */
LONG ASM UnGetC (REG(d1) BPTR fh, REG(d2) LONG ch);		/* GLOBAL */
LONG ASM waitforchar (REG(d1) BPTR scb, REG(d2) LONG timeout);	/* GLOBAL */
void ASM wrch (REG(d1) LONG ch);				/* GLOBAL */
LONG ASM FPutC (REG(d1) BPTR fh, REG(d2) LONG ch);		/* GLOBAL */
LONG ASM Flush (REG(d1) BPTR fh);				/* NEW */
LONG ASM SetVBuf (REG(d1) BPTR fh, REG(d2) UBYTE *buff,
		  REG(d3) LONG type, REG(d4) LONG size);	/* NEW */
LONG __regargs BufferSize(struct NewFileHandle *scb);
LONG __regargs replenish (LONG end, struct FileHandle *cis);
LONG __regargs deplete (struct FileHandle *cos);

BPTR ASM findinput (REG(d1) char *string);			/* GLOBAL */
BPTR ASM findoutput (REG(d1) char *string);			/* GLOBAL */

/* device and volume routines */
struct MsgPort * ASM deviceproc (REG(d1) char *name);		/* GLOBAL */
struct DevProc * ASM getdevproc (REG(d1) char *name,
				 REG(d2) struct DevProc *old_dp); /* NEW */
void ASM freedevproc (REG(d1) struct DevProc *dp);		  /* NEW */
struct DosList * ASM finddev (REG(d1) char *name,		/* GLOBAL */
			      REG(d2) LONG devonly);
struct MsgPort * ASM loaddevice (REG(d1) struct DosList *node,	/* GLOBAL */
				 REG(d2) BPTR string);

/* BCPL buffering routines */
void ASM init_scb (REG(d1) struct FileHandle *fhb, REG(d2) ULONG type);
BPTR ASM findstream (REG(d1) char *string,REG(d2) LONG act);
char * __regargs actinitcommon (struct NewFileHandle *scb);
extern LONG __regargs actreadinit (struct NewFileHandle *scb);
LONG __regargs actread (struct FileHandle *scb);
LONG __regargs actend (struct FileHandle *scb);
LONG __regargs actendread (struct FileHandle *scb);
LONG __regargs actwriteinit (struct NewFileHandle *scb);
LONG __regargs actwrite (struct FileHandle *scb);
LONG __regargs actendwrite (struct FileHandle *scb);

LONG ASM endread (void);					/* GLOBAL */
LONG ASM endwrite (void);					/* GLOBAL */
LONG ASM endstream (REG(d1) BPTR scb);				/* GLOBAL */

/* BCPL formatted IO routines */
LONG ASM readn (void);						/* GLOBAL */
void ASM newline (void);					/* GLOBAL */
void ASM writed   (REG(d1) LONG n, REG(d2) LONG d);		/* GLOBAL */
void ASM writen   (REG(d1) LONG n);				/* GLOBAL */
void ASM writehex (REG(d1) LONG n, REG(d2) LONG d);		/* GLOBAL */
void ASM writeoct (REG(d1) LONG n, REG(d2) LONG d);		/* GLOBAL */
void ASM bwrites  (REG(d1) BSTR s);				/* GLOBAL */
void ASM bwritet  (REG(d1) BSTR s, REG(d2) LONG n);		/* GLOBAL */
void ASM writes   (REG(d1) UBYTE *s);
void ASM writet   (REG(d1) LONG s, REG(d2) LONG n);
void ASM writeu   (REG(d1) ULONG n,REG(d2) LONG d);
void ASM bwritef  (REG(d1) char *format, REG(d2) LONG *t,	/* GLOBAL */
		   REG(d3) LONG isbcpl);

LONG ASM FRead (REG(d1) BPTR fh, REG(d2) UBYTE *block, REG(d3) LONG blocklen,
		REG(d4) LONG number);
LONG ASM WriteChars (REG(d1) UBYTE *buffer, REG(d2) LONG buflen);
LONG ASM FWrite (REG(d1) BPTR fh, REG(d2) UBYTE *block, REG(d3) LONG blocklen,
		 REG(d4) LONG number);
LONG ASM FPuts (REG(d1) BPTR fh, REG(d2) UBYTE *str);		/* NEW */
LONG ASM PutStr (REG(d1) UBYTE *str);				/* NEW */
UBYTE * ASM FGets (REG(d1) BPTR fh, REG(d2) UBYTE *buf, REG(d3) ULONG len);
void ASM VFWritef (REG(d1) BPTR fh, REG(d2) UBYTE *format,	/* NEW */
		   REG(d3) LONG *argv);
void __stdargs writef (char *format,LONG a, ...);		/* GLOBAL */
LONG ASM VPrintf (REG(d1) UBYTE *fmt,REG(d2) LONG *argv);	/* NEW */
LONG ASM VFPrintf (REG(d1) BPTR fh, REG(d2) UBYTE *fmt,		/* NEW */
		   REG(d3) LONG *argv);
void ASM myputch (REG(d0) LONG ch, REG(a3) LONG *numwritten);

/* general library routines, mostly packet sending */
LONG * ASM makeglob (REG(d1) LONG *segl);			/* GLOBAL */
LONG ASM isinteractive (REG(d1) BPTR s);			/* GLOBAL */
struct DateStamp * ASM datstamp (REG(d1) struct DateStamp *v);	/* GLOBAL */
void ASM setdosdate (REG(d1) struct DateStamp *v);		/* GLOBAL */

/* these are NOT in the global vector (see dir.b for example)!! */
LONG ASM do_lock (REG(d1) BPTR lock, REG(a0) LONG *args,
		  REG(d0) LONG action);
LONG ASM examine (REG(d1) BPTR lock,
		  REG(d2) struct FileInfoBlock *fib);
LONG ASM exnext  (REG(d1) BPTR lock,
		  REG(d2) struct FileInfoBlock *fib);
LONG ASM info    (REG(d1) BPTR lock,
		  REG(d2) struct FileInfoBlock *fib);
LONG ASM changemode (REG(d1) LONG type, REG(d2) BPTR fh,
		     REG(d3) LONG newmode);			/* NEW */
LONG ASM ExamineFH (REG(d1) BPTR lock,
		    REG(d2) struct FileInfoBlock *fib);		/* NEW */

LONG ASM parent  (REG(d1) BPTR lock);				/* GLOBAL */
BPTR ASM ParentOfFH (REG(d1) BPTR fh);				/* NEW */
LONG ASM copydir (REG(d1) BPTR dir);				/* GLOBAL */

/* all the objact packet-senders */
LONG ASM setcomment (REG(d1) char *name,			/* GLOBAL */
		     REG(d2) char *comment);
LONG ASM setprotection (REG(d1) char *name, REG(d2) LONG mask);	/* GLOBAL */
LONG ASM setdate (REG(d1) char *name,
		  REG(d2) struct DateStamp *date);		/* NEW */
LONG ASM deletefile (REG(d1) char *name);			/* GLOBAL */
LONG ASM createdir (REG(d1) char *name);			/* GLOBAL */
LONG ASM lock (REG(d1) char *name,REG(d2) LONG mode);		/* GLOBAL */
LONG ASM locateobj (REG(d1) char *name);			/* GLOBAL */
LONG ASM objact    (REG(d1) char *name, REG(d2) LONG action,
		    REG(d3) LONG *arg);

LONG ASM renameobj (REG(d1) char *fromname,			/* GLOBAL */
		    REG(d2) char *toname);
void ASM freeobj (REG(d1) BPTR lock);				/* GLOBAL */

LONG ASM CompareDates(REG(d1) struct DateStamp *date1,
		      REG(d2) struct DateStamp *date2);		/* GLOBAL */

LONG ASM read (REG(d1) BPTR scb,				/* GLOBAL */
	       REG(d2) char *v,
	       REG(d3) LONG n);
LONG ASM readwrite (REG(d1) BPTR scb,REG(a0) LONG *args,REG(d0) LONG code);
LONG ASM readbytes (REG(d1) char *v,REG(d2) LONG n);		/* GLOBAL */
LONG ASM readwords (REG(d1) BPTR v,REG(d2) LONG n);		/* GLOBAL */
LONG ASM write (REG(d1) BPTR scb,				/* GLOBAL */
		REG(d2) char *v,
		REG(d3) LONG n);
LONG ASM writebytes (REG(d1) char *v,REG(d2) LONG n);		/* GLOBAL */
LONG ASM writewords (REG(d1) BPTR v,REG(d2) LONG n);		/* GLOBAL */

LONG ASM seek (REG(d1) BPTR scb,				/* GLOBAL */
	       REG(d2) LONG pos,
	       REG(d3) LONG mode);
LONG ASM noflushseek (REG(d1) BPTR scb,				/* NEW */
		      REG(d2) LONG pos,
		      REG(d3) LONG mode);
LONG ASM SetFileSize (REG(d1) BPTR scb,				/* NEW */
	              REG(d2) LONG pos,
	              REG(d3) LONG mode);
LONG ASM setmode (REG(d1) BPTR fh, REG(d2) LONG mode);		/* NEW */
BPTR ASM DupLockFromFH (REG(d1) BPTR fh);			/* NEW */
BPTR ASM ParentFH (REG(d1) BPTR fh);				/* NEW */
BPTR ASM OpenFromLock (REG(d1) BPTR lock);			/* NEW */
LONG ASM inhibit (REG(d1) char *name, REG(d2) LONG onoff);	/* NEW */
LONG ASM addbuffers (REG(d1) char *name, REG(d2) LONG number);	/* NEW */
LONG ASM isfilesystem (REG(d1) char *name);			/* NEW */
LONG ASM relabel (REG(d1) char *drive,REG(d2) char *newname);	/* NEW */
LONG ASM Format (REG(d1) char *drive,REG(d2) char *volumename,
		 REG(d3) ULONG private);			/* NEW */
LONG ASM devact_str (REG(d1) char *drive, REG(d2) char *string,
		     REG(d3) ULONG arg, REG(d0) LONG action);
LONG ASM devact (REG(d1) char *name, REG(d2) LONG arg1,		/* NEW */
		 REG(d3) LONG arg2,  REG(d0) LONG action);
LONG ASM CurrentDir (REG(d1) BPTR dir);				/* GLOBAL */
LONG ASM SameLock (REG(d1) BPTR l1, REG(d2) BPTR l2);		/* NEW */
LONG ASM ReadLink (REG(d1) struct MsgPort *port, REG(d2) BPTR lock,
		   REG(d3) UBYTE *path,REG(d0) UBYTE *buffer,
		   REG(a0) ULONG size);				/* NEW */
LONG ASM MakeLink (REG(d1) char *name,REG(d2) LONG dest,
		   REG(d3) LONG soft);				/* NEW */

UBYTE * ASM FilePart (REG(d1) UBYTE *path);			/* NEW */
UBYTE * ASM PathPart (REG(d1) UBYTE *path);			/* NEW */
LONG ASM AddPart (REG(d1) UBYTE *dirname, REG(d2) UBYTE *filename,
		  REG(d3) ULONG size);				/* NEW */
void * ASM AllocDosObject (REG(d1) ULONG type, REG(d2) struct TagItem *tags);
void ASM FreeDosObject (REG(d1) ULONG type, REG(d2) void *ptr);

/* exall.c */
LONG __stdargs fake_exall (BPTR lock, struct ExAllData *ed, LONG size,
			   LONG data, struct ExAllControl *ec);	/* NEW */
LONG __regargs fill_data (struct ExAllData *ed, ULONG data,	/* NEW */
			  struct FileInfoBlock *fib, LONG *size);

/* support2.asm */
LONG __regargs do_match (struct ExAllControl *ec, ULONG data,	/* NEW */
			 struct ExAllData *ed);
void __regargs getSysTime (struct timeval *val, struct Device *dev);

/* readargs.asm */
LONG ASM MatchPattern (REG(d1) UBYTE *pat, REG(d2) UBYTE *str); /* NEW */
LONG ASM MatchPatternNoCase (REG(d1) UBYTE *pat, REG(d2) UBYTE *str); /* NEW */
LONG ASM FindFirst (REG(d1) UBYTE *pat, REG(d2) struct AnchorPath *anchor);


char * __regargs mystrchr(char *ptr, int sep);
char * __regargs mystrrchr(char *ptr, int sep);

/* definitions */

#define finddevice(x)	finddev((x),TRUE)
#define locatedir(name) locateobj(name)
#define IoErr		getresult2
#define deleteobj(x)	deletefile(x)

#define strchr(x,y)	mystrchr((x),(y))
#define strrchr(x,y)	mystrrchr((x),(y))
