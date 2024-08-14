/* aread.c
 *
 * copy files from Amiga to PC
 *
 * rsd	01/21/91	first version
 *
 * known bugs:
 *	- needs to map amiga filenames to pc names (for instance, fails to
 *	  open pc files with amiga names such as "dosserv.ld.strip".
 *	- doesn't handle lots of wildcards yet (ie, work:h#?s/#?/#?.c will get
 *	  everything in the "work:hacks" directory and lower.  idea was that
 *	  it should get "work:hacks/anything/only.c".
 *	- /e doesn't work right (need to split scanning into scan/copy)
 *	- when you get a dos abort/retry/ignore/fail requester, doing an
 *	  abort will not close files or dosserv.
 *	- /d:mmddyy option not implemented (silently does nothing)
 *	- /v option not implemented (silently does nothing)
 *
 * fixed bugs:
 *	- destinations like A: by itself fail (tries to make a dir)
 *	- when there's a write error, attempts to delete the wrong file
 *	- should preserve source file datestamp, but doesn't even try yet.
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

/* _dos_....() */
#include <dos.h>
#include <fcntl.h>

/* getch(), getche() */
#include <conio.h>

/* int86 & friends */
#include <bios.h>

/* misc types */
#include <exec/types.h>

/* file transfer */
#include <janus/jftrans.h>

/* dosserv stuff */
#include "..\dslib\dslib.h"

/* time/date stuff */
#include "dates.h"


/*
#define PRINTING
*/

#define FALSE 0
#define TRUE (!FALSE)

struct list_head {
	struct list_node *head;
	struct list_node *zero;
	struct list_node *tail;
};

struct list_node {
	struct list_node *next;
	struct list_node *prev;
};

struct lock_entry {
	struct list_node node;	/* linkage */
	ULONG lock;
	char *name;
};


/* msdos 4.00 xcopy options */
/* implemented so far: /e, /p, /s, /a, /m, /w */
/*            missing: /d, /v */
int	option_a = 0;		/* /a - only copy files with archive bit set */
unsigned int option_d = 0;	/* /d:mm/dd/yy - date code or 0 if no /d opt */
int	option_e = 0;		/* /e - copy empty subdirs */
int	option_m = 0;		/* /m - only copy files with archive bit set,
				 * clearing archive bit on source files */
int	option_p = 0;		/* /p - prompt (copy file.ext (y/n)) for each
				 * file or directory */
int	option_s = 0;		/* /s - copy subdirectories */
int	option_v = 0;		/* /v - verify after copy */
int	option_w = 0;		/* /w - wait for keystroke before starting copy
				 * operation (allows for changing disk after
				 * loading EXE on a single drive machine) */

/* awrite options */
int	option_b = 0;		/* /b - straight byte-by-byte copy (/nc /cr) */
int	option_nc = 0;		/* /nc - no char set conversion */
int	option_cr = 0;		/* /cr - no cr-lf conversion */

/* lock tracking */
struct list_head locklist;

/* dosserv stuff */
struct dslib_struct ds;

/* misc */
int n_copied = 0;		/* # of files copied */


/* proto for quit() */
void quit(int retval);
void breakhandler();

/* file copy filehandles in case of abort */
FILE *pcfile;
ULONG amfile;
char destname[512];

void usage()
{
	fprintf(stderr,
"AREAD 1.0 usage:\n");
	fprintf(stderr,
"\n    AREAD source destination [control options] [conversion options]\n");
	fprintf(stderr,
"\nSource may include AmigaDOS wildcards.\n");
	fprintf(stderr,
"Source must start with an AmigaDOS volume or device identifier.\n");
	fprintf(stderr,
"Converts AmigaDOS LF to MS-DOS CR/LF and Amiga character set to IBM PC\n");
	fprintf(stderr,
"character set, unless a conversion option is specified.\n");
	fprintf(stderr,
"\nControl Options:\n");
	fprintf(stderr,
"/s - copy subdirectories\n");
	fprintf(stderr,
"/e - copy empty subdirs (use with /s)\n");
/*	fprintf(stderr,
"/v - verify after copy\n");
*/
/*	fprintf(stderr,
"/d:mm/dd/yy - only copy files modified since this date\n");
*/
	fprintf(stderr,
"/a - only copy files with archive bit set\n");
	fprintf(stderr,
"/m - like /a, but clears the source's archive bit after copying\n");
	fprintf(stderr,
"/p - prompt for each file or directory before copying\n");
	fprintf(stderr,
"/w - wait for keystroke before starting copy operation (allows for changing\n");
	fprintf(stderr,
"     disk after loading EXE on a single drive machine)\n");
	fprintf(stderr,
"\nConversion Options:\n");
	fprintf(stderr,
"/b - binary file copy (same as /nc /cr)\n");
	fprintf(stderr,
"/nc - no Amiga/PC character set conversion\n");
	fprintf(stderr,
"/cr - no Amiga/PC CR/LF conversion\n");
}

int msdos_version()
{
union REGS r;

	r.h.ah = 0x30;
	int86(0x21, &r, &r);
	return r.h.al*100 + r.h.ah;
}

/*****************************************************************************
 * list manipulation functions
 ****************************************************************************/
void list_init(struct list_head *list)
{
	list->head = (struct list_node *)&list->zero;
	list->zero = (struct list_node *)0;
	list->tail = (struct list_node *)&list->head;
}

void list_add_head(struct list_head *list, struct list_node *node)
{
	node->prev = (struct list_node *)&list->head;
	node->next = list->head;
	list->head = node;
	node->next->prev = node;
}

void list_add_tail(struct list_head *list, struct list_node *node)
{
	node->prev = list->tail;
	node->next = (struct list_node *)&list->zero;
	list->tail = node;
	node->prev->next = node;
}

void list_insert(struct list_node *ref, struct list_node *new)
{
	new->prev = ref;
	new->next = ref->next;
	ref->next->prev = new;
	ref->next = new;
}

void list_remove(struct list_node *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

struct list_node *list_rem_tail(struct list_head *list)
{
struct list_node *node;

	node = list->tail;
	if (node->prev)
		list_remove(node);
	else
		node = 0;

	return node;
}

struct list_node *list_rem_head(struct list_head *list)
{
struct list_node *node;

	node = list->head;
	if (node->next)
		list_remove(node);
	else
		node = 0;

	return node;
}


/*****************************************************************************
 * error-detecting malloc
 ****************************************************************************/
void *emalloc(unsigned int size)
{
void *p;

	if (!(p = malloc(size))) {
		fprintf(stderr, "Out of memory\n");
		quit(1);
	}

	return p;
}

/*
 * "" + name -> name
 * a: + name -> a:name
 * \ + name -> \name
 * a:\ + name -> a:\name
 * a:test + name -> a:test\name
 * a:test\ + name -> a:test\name
 */
void pc_add_path(char *source_path, char *source_name)
{
int last;

	last = strlen(source_path) - 1;
	
	if (last == -1) {
		/* source_path is empty */
		strcpy(source_path, source_name);
	} else if (source_path[last] == ':') {
		/* source path is of the a: form */
		strcat(source_path, source_name);
	} else if (source_path[last] == '\\') {
		/* source path is of the a:???\ form */
		strcat(source_path, source_name);
	} else {
		/* source path is of the a:xyz form */
		strcat(source_path, "\\");
		strcat(source_path, source_name);
	}
}

void amiga_add_path(char *source_path, char *source_name)
{
int last;

	last = strlen(source_path) - 1;
	
	if (last == -1) {
		/* source_path is empty */
		strcpy(source_path, source_name);
	} else if (source_path[last] == ':') {
		/* source path is of the a: form */
		strcat(source_path, source_name);
	} else if (source_path[last] == '/') {
		/* source path is of the a:???/ form */
		strcat(source_path, source_name);
	} else {
		/* source path is of the a:xyz form */
		strcat(source_path, "/");
		strcat(source_path, source_name);
	}
}

void amiga_unlock(struct lock_entry *le)
{
	list_remove(&le->node);
#ifdef PRINTING
printf("** unlock: %s, %ld\n", le->name, le->lock);
#endif
	j_UnLock(&ds, le->lock);
	free(le->name);
	free(le);
}

struct lock_entry *amiga_lock(char *name, LONG accessmode)
{
struct lock_entry *le;

	le = emalloc(sizeof(struct lock_entry));

	if (le->lock = j_Lock(&ds, name, accessmode)) {
		le->name = emalloc(strlen(name)+1);
		strcpy(le->name, name);
		list_add_tail(&locklist, &le->node);
#ifdef PRINTING
printf("** lock: %s, %ld\n", le->name, le->lock);
#endif
		return le;
	} else {
		free(le);
		return 0;
	}
}

int pc_exists(char *name)
{
struct find_t dosbuf;
int retval;
char *p;

	if (strlen(name) >= 2) {
		p = &name[strlen(name) - 2];
		if (!(strcmp(p, ":.") && strcmp(p, "..") && strcmp(p, "\.")))
			return TRUE;
	}

	if (strlen(name) >= 1) {
		p = &name[strlen(name) - 1];
		if (*p == ':' || *p == '\\')
			return TRUE;
	}
	
	if (!_dos_findfirst(name, _A_NORMAL | _A_RDONLY | _A_SUBDIR, &dosbuf)) {
		retval = TRUE;
	} else {
		retval = FALSE;
	}

	return retval;
}

int pc_is_dir(char *name)
{
struct find_t dosbuf;
int retval;
char *p;

	if (strlen(name) >= 2) {
		p = &name[strlen(name) - 2];
		if (!(strcmp(p, ":.") && strcmp(p, "..") && strcmp(p, "\.")))
			return TRUE;
	}

	if (strlen(name) >= 1) {
		p = &name[strlen(name) - 1];
		if (*p == '\\')
			return TRUE;

		if (*p == ':')
			return TRUE;
	}

	if (!_dos_findfirst(name, _A_NORMAL | _A_RDONLY | _A_SUBDIR, &dosbuf)) {
		if (dosbuf.attrib & _A_SUBDIR)
			retval = TRUE;
		else
			retval = FALSE;
	} else {
		retval = FALSE;
	}

	return retval;
}


int amiga_is_wild(char *name)
{
char *comp, *n;
int complen, retval;

	complen = 3 * strlen(name);
	comp = emalloc(complen);
	retval = j_ParsePattern(&ds, name, comp, complen);
	if (retval != 1)
		retval = FALSE;
	else
		retval = TRUE;
	free(comp);
	return retval;
}

int pc_is_wild(char *name)
{
	if (strchr(name, '*'))
		return 1;
	if (strchr(name, '?'))
		return 1;

	return 0;
}

void pc_create_dir(char *name)
{
	if (pc_exists(name)) {
		if (!pc_is_dir(name)) {
			fprintf(stderr,
					"Attempt to create directory %s over file\n", name);
			quit(1);
		}
	} else {
		if (mkdir(name)) {
			fprintf(stderr,
					"Couldn't create directory %s\n", name);
			quit(1);
		}
		printf("%s (dir) created\n", name);
	}
}

void pc_create_dir_all(char *name)
{
char *s, *p;

	s = emalloc(strlen(name) + 1);
	strcpy(s, name);
	p = s;
	while (p = strchr(p+1, '\\')) {
		*p = 0;
		pc_create_dir(s);
		*p = '\\';
	}
	pc_create_dir(s);
	free(s);
}

int ask(char *type, char *name)
{
char str[80], c;

	printf("copy %s %s (Y/N)? ", type, name);
	fgets(str, sizeof(str)-1, stdin);
	c = str[0];
	if (isupper(c))
		c = tolower(c);
	if (c == 'y')
		return 1;
	else
		return 0;
}

void quit(int retval)
{
struct lock_entry *p, *n;

	/* close PC file if transfer aborted */
	if (pcfile) {
		fclose(pcfile);
		unlink(destname);
		pcfile = 0;
	}

	/* close amiga file if transfer aborted */
	if (amfile) {
		j_Close(&ds, amfile);
		amfile = 0;
	}

	/* free all locks we own */
	for (p = (struct lock_entry *)locklist.tail;
	     n = (struct lock_entry *)p->node.prev;
	     p = n) {
		amiga_unlock(p);
	}

	if (ds.sd_ds) {
		j_close_dosserv(&ds);
	}	

	exit(retval);
}

void breakhandler()
{
	quit(1);
}

int get_amiga_file_date(char *name, struct DateStamp *d_s)
{
struct lock_entry *lock;
struct FileInfoBlock *fib;
int retval;

	fib = emalloc(sizeof(struct FileInfoBlock));
	
	memset(d_s, 0, sizeof(struct DateStamp));

	if (lock = amiga_lock(name, SHARED_LOCK)) {
		if (j_Examine(&ds, lock->lock, fib)) {
			memcpy(d_s, &fib->fib_Date, sizeof(struct DateStamp));
			retval = TRUE;
		} else {
			retval = FALSE;
		}
		
		amiga_unlock(lock);
	} else {
		retval = FALSE;
	}

	free(fib);
	return retval;
}

void set_pc_file_date(char *name, unsigned date, unsigned time)
{
int handle;

	if (!(_dos_open(name, O_RDWR, &handle))) {
		_dos_setftime(handle, date, time);
		_dos_close(handle);
	}
}

void copy_file(char *source, char *dest)
{
int mode, convert, retval;
char pcmode[3];
unsigned date, time;
int year, month, day, hour, minute, second;
struct DateStamp *d_s;

	if (option_cr || option_b)
		mode = JFT_BINARY;
	else
		mode = JFT_CR_LF;

	if (option_nc || option_b)
		convert = JFT_NO_CONVERT;
	else
		convert = JFT_CONVERT;

	/* build mode string for pc file */
	if (mode == JFT_BINARY)
		strcpy(pcmode, "wb");
	else
		strcpy(pcmode, "wt");

	strcpy(destname, dest);

	/* open pc file */
	if (!(pcfile = fopen(dest, pcmode))) {
		fprintf(stderr,
			"Error opening PC file %s\n", source);
		quit(1);
	}

	/* open the amiga file */
	if (!(amfile = j_Open(&ds, source, MODE_OLDFILE))) {
		fprintf(stderr,
			"Error opening Amiga file %s\n", dest);
		quit(1);
	}

	/* copy & convert */
	retval = j_file_xfer(&ds, pcfile, amfile, JFT_AMIGA_PC, mode, convert,
			     (unsigned char *) 0);

	/* close files */
	fclose(pcfile);
	pcfile = 0;
	j_Close(&ds, amfile);
	amfile = 0;
	
	if (retval) {
		fprintf(stderr,
			"Error copying %s to %s\n", source, dest);
		if (retval == JFT_ERR_PC_WRITE) {
			fprintf(stderr,
				"(deleting %s)\n", dest);
			unlink(dest);
		}
		quit(1);
	}

	/* clone date of the amiga file */
	d_s = emalloc(sizeof(struct DateStamp));
	if (get_amiga_file_date(source, d_s)) {
		date_amiga_to_normal(d_s, &year, &month, &day);
		date_normal_to_msdos(year, month, day, &date);
		time_amiga_to_normal(d_s, &hour, &minute, &second);
		time_normal_to_msdos(hour, minute, second, &time);
		set_pc_file_date(dest, date, time);
	}
	free(d_s);

	n_copied ++;
}

void scan_dir(struct lock_entry *lock, char *sourcepath, char *wild,
			  char *destpath, int parent_exists)
{
struct lock_entry *newlock;
ULONG prev;
struct FileInfoBlock *fib;
char *dest, *source, *comp, *p;
int complen, doit;
char *thiswild, *newwild, *n;
LONG wc;
int i;

	fib = emalloc(sizeof(*fib));
	if (!j_Examine(&ds, lock->lock, fib)) {
		fprintf(stderr, "Couldn't examine %s\n", sourcepath);
		quit(1);
	}

	if (fib->fib_DirEntryType < 0) {
		/* yo!  we got a file instead of a dir! */
		fprintf(stderr, "Internal error - %s is a file\n", sourcepath);
		free(fib);
		quit(1);
	}

	dest = emalloc(strlen(destpath) + 32);
	source = emalloc(strlen(sourcepath) + 32);

	prev = j_CurrentDir(&ds, lock->lock);

	if (wild) {
		if (p = strchr(wild, '/')) {
			thiswild = emalloc(p - wild + 1);
			memcpy(thiswild, wild, p - wild);
			thiswild[p - wild] = 0;
			newwild = emalloc(strlen(p+1) + 1);
			strcpy(newwild, p+1);
		} else {
			thiswild = emalloc(strlen(wild) + 1);
			strcpy(thiswild, wild);
			newwild = 0;
		}
	} else {
		thiswild = 0;
		newwild = 0;
	}
		
	if (thiswild) {
		complen = 3 * strlen(thiswild);
		comp = emalloc(complen);
		wc = j_ParsePattern(&ds, thiswild, comp, complen);
		if (wc == -1) {
			/* fubar */
			fprintf(stderr, "ParsePattern error on %s\n", thiswild);
			quit(1);
		} else if (wc == 0) {
			/* tame */
			free(comp);
			comp = 0;
		} else if (wc == 1) {
			/* wild */
			for (i = 0; comp[i]; i++)
				if (islower(comp[i]))
					comp[i] = toupper(comp[i]);
		}
	} else {
		comp = 0;
	}

	/* scan yonder directory */
	while (j_ExNext(&ds, lock->lock, fib)) {
		n = emalloc(strlen(fib->fib_FileName)+1);
		strcpy(n, fib->fib_FileName);
		for (i = 0; n[i]; i++)
			if (islower(n[i]))
				n[i] = toupper(n[i]);
		if (comp == 0 || j_MatchPattern(&ds, comp, n)) {
			if (newlock = amiga_lock(fib->fib_FileName, SHARED_LOCK)) {
				strcpy(dest, destpath);
				pc_add_path(dest, fib->fib_FileName);

				strcpy(source, sourcepath);
				amiga_add_path(source, fib->fib_FileName);

				doit = TRUE;

				if (option_a || option_m)
					if (fib->fib_Protection & FIBF_ARCHIVE)
						doit = FALSE;

				if (fib->fib_DirEntryType < 0) {
					if (doit && option_p) {
						if (!ask("file", source))
							doit = FALSE;
					}

					if (doit) {
						if (pc_exists(dest) && pc_is_dir(dest)) {
							fprintf(stderr,
									"Attempt to create directory %s over file\n", dest);
							quit(1);
						}
						if (!parent_exists) {
							pc_create_dir_all(destpath);
							parent_exists = TRUE;
						}
						printf("copying %s to %s\n", source, dest);
						copy_file(fib->fib_FileName, dest);
					}
				} else {
					if (doit && option_p) {
						if (!ask("directory", source))
							doit = FALSE;
					}

					if (doit) {
						if (option_s) {
							if (option_e) {
								pc_create_dir_all(dest);
								scan_dir(newlock, source, newwild, dest, 1);
							} else {
								scan_dir(newlock, source, newwild, dest, 0);
							}
						}
					}
				}

				if (doit && option_m) {
					/* set FIBF_ARCHIVE */
					j_SetProtection(&ds, fib->fib_FileName,
								    fib->fib_Protection | FIBF_ARCHIVE);
				}

				amiga_unlock(newlock);
			}
		}
		free(n);
	}

	if (comp)
		free(comp);

	if (newwild)
		free(newwild);

	if (thiswild)
		free(thiswild);

	j_CurrentDir(&ds, prev);

	free(fib);
	free(source);
	free(dest);
}

void main(int argc, char **argv)
{
struct lock_entry *lock;
struct FileInfoBlock *fib;
char sbase[512], swild[512], dbase[512];
char *source, *dest;

char c, *a, *p, *p1, *p2;
int i;

	pcfile = 0;
	amfile = 0;
	list_init(&locklist);
	ds.sd_ds = 0;

	signal(SIGINT, breakhandler);		/* ctrl-c */
	signal(SIGABRT, breakhandler);		/* A)bort */

	/* make sure dos 3.0 or higher */
	if (msdos_version() < 300) {
		fprintf(stderr, "Need DOS 3.0 or higher\n");
		quit(1);
	}

	/* make sure janus present */
	if (j_tickle_janus()) {
		fprintf(stderr, "Janus handler not loaded\n");
		quit(1);
	}

	/* make sure dosserv present */
	if (j_open_dosserv(&ds)) {
		fprintf(stderr, "DOSServ service not available\n");
		quit(1);
	}

	/* need at least one argument */
	if (argc < 2) {
		usage();
		quit(1);
	}

	/* 1st arg is source */
	source = 0;

	/* 2nd arg is dest */
	dest = 0;
	
	/* process rest of args (if any - will all be options */
	for (i = 1; i < argc; i++) {
		a = &argv[i][0];
		if (a[0] != '/') {
			if (i == 1 && source == 0) {
				source = emalloc(256);
				strcpy(source, argv[i]);
			} else if (i == 2 && dest == 0) {
				dest = emalloc(256);
				strcpy(dest, argv[i]);
			} else {
				fprintf(stderr,
					"Too many filenames\n");
				quit(1);
			}
		} else {
			do {
				if (*a++ == '/') {
					c = *a++;
					if (isupper(c))
						c = tolower(c);
					switch (c) {
					case 'a':
						/* only copy files with archive bit set */
						option_a = TRUE;
						break;
					case 'd':
						/* copy files >= this date */
						/* ... parse date from string to numeric form ... */
						option_d = FALSE;
						break;
					case 'e':
						/* copy empty subdirs */
						option_e = TRUE;
						break;
					case 'm':
						/* only copy files with archive bit set, then clear the
						 * source's archive bit after copy */
						 option_m = TRUE;
						break;
					case 'p':
						/* prompt before each file */
						option_p = TRUE;
						break;
					case 's':
						/* copy subdirs */
						option_s = TRUE;
						break;
					case 'v':
						/* verify destination file after copy */
						option_v = TRUE;
						break;
					case 'w':
						/* wait for a keystroke before starting operation */
						option_w = TRUE;
						break;
					case 'b':
						/* straight binary copy */
						option_b = TRUE;
						break;
					default:
						if (stricmp(a-1, "nc") == 0) {
							/* no char set conversion */
							option_nc = TRUE;
							a++;
						} else if (stricmp(a-1, "cr") == 0) {
							/* no cr-lf conversion */
							option_cr = TRUE;
							a++;
						} else {
							fprintf(stderr, "Unknown option '/%s'\n", p-1);
							quit(1);
						}
						break;
					}
				} else {
					fprintf(stderr, "Illegal parameter '%s'\n", a-1);
					quit(1);
				}
			} while (*a);
		}
	}	

	if (source == 0 || dest == 0) {
		fprintf(stderr, "No destination and/or source file specified\n");
		quit(1);
	}

	p1 = strchr(source, ':');
	p2 = strchr(source, '/');
	if (!p1 || (p2 && p2 < p1)) {
		fprintf(stderr, "Amiga source name must include volume/device id\n");
		quit(1);
	}

	if (amiga_is_wild(source)) {
		p1++;
		c = *p1;
		*p1 = 0;
		strcpy(sbase, source);
		*p1 = c;

		while (p2 = strchr(p1, '/')) {
			c = *p2;
			*p2 = 0;
			if (!amiga_is_wild(p1)) {
				if (sbase[strlen(sbase)-1] != ':')
					strcat(sbase, "/");
				strcat(sbase, p1);
				*p2 = c;
				p1 = p2 + 1;
			} else {
				break;
			}
		}
		strcpy(swild, p1);
	} else {
		strcpy(sbase, source);
		strcpy(swild, "");
	}

	if (!(lock = amiga_lock(sbase, SHARED_LOCK))) {
		fprintf(stderr, "Couldn't lock %s\n", sbase);
		quit(1);
	}

	fib = emalloc(sizeof(struct FileInfoBlock));
	if (!(j_Examine(&ds, lock->lock, fib))) {
		fprintf(stderr, "Couldn't examine %s\n", sbase);
		quit(1);
	}

	if (fib->fib_DirEntryType < 0) {
		/* single file case */
		if (swild[0]) {
			fprintf(stderr, "%s is a file, not a directory!\n", sbase);
			quit(1);
		}

		/* if dest name is a dir, append filename to end */
		if (pc_exists(dest) && pc_is_dir(dest)) {
			if (!(p1 = strrchr(sbase, '/'))) {
				if (!(p1 = strrchr(sbase, ':'))) {
					p1 = sbase;
				} else {
					p1++;
				}
			} else {
				p1++;
			}

			pc_add_path(dest, p1);
		}

		/* copy one file! */
		copy_file(sbase, dest);
	} else {
		/* multiple files */
		if (pc_exists(dest) && !pc_is_dir(dest)) {
			/* multi files onto one?  bad idea. */
			fprintf(stderr, "Cannot copy multiple files into one file\n");
			quit(1);
		}

		if (swild[0]) {
			scan_dir(lock, sbase, swild, dest, 0);
		} else {
			scan_dir(lock, sbase, 0, dest, 0);
		}
	}

	free(fib);
	amiga_unlock(lock);
	free(dest);
	free(source);

	if (n_copied == 0) {
		fprintf(stderr, "No files found\n");
	}

	quit(0);
}
