/* awrite.c
 *
 * copy files from PC to Amiga
 *
 * rsd	01/09/91	first version
 *
 * known bugs:
 *	- mixes locks and absolute path names during operation, so moving
 *	  stuff around on the amiga side while copying can screw things up.
 *	- when you get a dos abort/retry/ignore/fail requester, doing an
 *	  abort will not close files or dosserv.
 *	- /d:mmddyy option not implemented
 *	- /v option not implemented
 *	- /e option assumed
 *
 * fixed bugs:
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

#define ALLOW_OPT_E

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

struct flist_entry {
	struct list_node node;	/* linkage */
	char *name;		/* name of this file/dir */
	struct list_head *dir;	/* if this is a dir, hang children off this */
	int is_file;		/* TRUE if this is a file, FALSE for dir */
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

/* misc vars */
int n_copied = 0;		/* # of files copied */
int multi_source = 0;		/* true if multiple source files */

/* lock tracking */
struct list_head locklist;

/* dosserv stuff */
struct dslib_struct *ds;


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
"AWRITE 1.0 usage:\n");
	fprintf(stderr,
"\n    AWRITE source destination [control options] [conversion options]\n");
	fprintf(stderr,
"\nSource may include MS-DOS wildcards.\n");
	fprintf(stderr,
"Destination must start with an AmigaDOS volume or device identifier.\n");
	fprintf(stderr,
"Converts MS-DOS CR/LF to AmigaDOS LF and IBM PC character set to Amiga\n");
	fprintf(stderr,
"character set, unless a conversion option is specified.\n");
	fprintf(stderr,
"\nControl Options:\n");
	fprintf(stderr,
"/s - copy subdirectories\n");

#ifdef ALLOW_OPT_E
	fprintf(stderr,
"/e - copy empty subdirs (use with /s)\n");
#endif

#if 0
	fprintf(stderr,
"/v - verify after copy\n");
#endif

#if 0
	fprintf(stderr,
"/d:mm/dd/yy - only copy files modified since this date\n");
#endif

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
"/nc - no PC/Amiga character set conversion\n");
	fprintf(stderr,
"/cr - no PC/Amiga CR/LF conversion\n");
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
	j_UnLock(ds, le->lock);
	free(le->name);
	free(le);
}

struct lock_entry *amiga_lock(char *name, LONG accessmode)
{
struct lock_entry *le;

	le = emalloc(sizeof(struct lock_entry));

	if (le->lock = j_Lock(ds, name, accessmode)) {
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

struct lock_entry *amiga_createdir(char *name)
{
struct lock_entry *le;
ULONG lock;

	if (lock = j_CreateDir(ds, name)) {
		j_UnLock(ds, lock);
		if (lock = j_Lock(ds, name, SHARED_LOCK)) {
			le = emalloc(sizeof(struct lock_entry));
			le->lock = lock;
			le->name = emalloc(strlen(name) + 1);
			strcpy(le->name, name);
			list_add_tail(&locklist, &le->node);
#ifdef PRINTING
printf("** createdir: %s, %ld\n", le->name, le->lock);
#endif
			return le;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

struct lock_entry *amiga_create_dir(char *dirname)
{
struct lock_entry *lock;
struct FileInfoBlock fib;

	if (lock = amiga_lock(dirname, SHARED_LOCK)) {
		if (!j_Examine(ds, lock->lock, &fib)) {
			fprintf(stderr, "Examine(%ld / %s) error\n", lock, dirname);
			quit(1);
		}
		if (fib.fib_DirEntryType < 0) {
			fprintf(stderr,
				"Error: attempt to create directory %s over a file\n", dirname);
			quit(1);
		}
	} else {
		if (!(lock = amiga_createdir(dirname))) {
			fprintf(stderr, "couldn't create directory %s\n", dirname);
			quit(1);
		}
		printf("%s (dir) created\n", dirname);
	}		

	return lock;
}

struct lock_entry *amiga_create_dir_all(char *dirname)
{
char *s, *p;
struct lock_entry *lock;
struct list_head *lh;
struct temp_lock {
	struct list_node node;
	struct lock_entry *le;
} *le, *n;

	lh = emalloc(sizeof(struct list_head));
	list_init(lh);

	s = emalloc(strlen(dirname)+1);
	strcpy(s, dirname);

	p = s;
	
	while (p = strchr(p+1, '/')) {
		*p = 0;
		lock = amiga_create_dir(s);
		le = emalloc(sizeof(struct temp_lock));
		le->le = lock;
		list_add_tail(lh, &le->node);
		*p = '/';
	}

	lock = amiga_create_dir(s);

	/* free all locks we own */
	for (le = (struct temp_lock *)lh->head;
	     n = (struct temp_lock *)le->node.next;
	     le = n) {
		list_remove(&le->node);
		amiga_unlock(le->le);
		free(le);
	}

	free(s);
	free(lh);

	return lock;
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
	
	if (strcmp(name, ".") == 0)
		return TRUE;

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

	if (strcmp(name, ".") == 0)
		return TRUE;

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

struct lock_entry *amiga_exists(char *name)
{
	return amiga_lock(name, SHARED_LOCK);
}

int amiga_is_dir(struct lock_entry *le)
{
struct FileInfoBlock fib;

	if (!j_Examine(ds, le->lock, &fib)) {
		fprintf(stderr, "Examine(%ld) error\n", le->lock);
		quit(1);
	}

	if (fib.fib_DirEntryType < 0)
		return FALSE;
	else
		return TRUE;
}

void set_amiga_file_date(char *name, struct DateStamp *d_s)
{
	j_SetFileDate(ds, name, d_s);
}

int get_pc_file_date(char *name, unsigned *date, unsigned *time)
{
int handle;

	if (!(_dos_open(name, O_RDONLY, &handle))) {
		_dos_getftime(handle, date, time);
		_dos_close(handle);
		return TRUE;
	} else {
		return FALSE;
	}
}

void copy_file(char *source, char *dest)
{
int mode, convert, retval;
char pcmode[3];
unsigned date,time;
int hour, minute, second, year, month, day;
struct DateStamp *d_s;

	printf("copying %s to %s\n", source, dest);

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
		strcpy(pcmode, "rb");
	else
		strcpy(pcmode, "rt");

	/* open pc file */
	if (!(pcfile = fopen(source, pcmode))) {
		fprintf(stderr,
			"Error opening PC file %s\n", source);
		quit(1);
	}

	strcpy(destname, dest);

	/* open the amiga file */
	j_DeleteFile(ds, dest);
	if (!(amfile = j_Open(ds, dest, MODE_NEWFILE))) {
		fprintf(stderr,
			"Error opening Amiga file %s\n", dest);
		quit(1);
	}

	/* copy & convert */
	retval = j_file_xfer(ds, pcfile, amfile, JFT_PC_AMIGA, mode, convert,
			     (unsigned char *) 0);

	/* close files */
	fclose(pcfile);
	pcfile = 0;
	j_Close(ds, amfile);
	amfile = 0;
	
	if (retval) {
		fprintf(stderr,
			"Error copying %s to %s\n", source, dest);
		if (retval == JFT_ERR_AMIGA_WRITE) {
			fprintf(stderr,
				"(deleting %s)\n", dest);
			j_DeleteFile(ds, dest);
		}
		quit(1);
	}

	/* clone date of the pc file */
	d_s = emalloc(sizeof(struct DateStamp));
	if (get_pc_file_date(source, &date, &time)) {
#ifdef PRINTING
printf("msdos date is %04x\n", date);
printf("msdos time is %04x\n", time);
#endif
		date_msdos_to_normal(date, &year, &month, &day);
#ifdef PRINTING
printf("normalized date is year = %d, month = %d, day = %d\n",
	year, month, day);
#endif
		date_normal_to_amiga(year, month, day, d_s);
		time_msdos_to_normal(time, &hour, &minute, &second);
#ifdef PRINTING
printf("normalized time is hour = %d, minute = %d, second = %d\n",
	hour, minute, second);
#endif
		time_normal_to_amiga(hour, minute, second, d_s);
#ifdef PRINTING
printf("amiga file date (ds_Tick = %ld, ds_Minute = %ld, ds_Days = %ld\n",
	d_s->ds_Tick, d_s->ds_Minute, d_s->ds_Days);
#endif
		set_amiga_file_date(dest, d_s);
	}
	free(d_s);

	n_copied ++;
}

/*****************************************************************************
 * directory scanner
 ****************************************************************************/
void build_list(struct list_head **listhead,
	        char *source_path, char *source_name)
{
char *s;
struct find_t *dosbuf;
unsigned result;
struct flist_entry *node;
int doit;

	dosbuf = emalloc(sizeof(struct find_t));
	
	*listhead = emalloc(sizeof(struct list_head));
	list_init(*listhead);
	
	s = emalloc(strlen(source_path)+1 + strlen(source_name)+1 + 1);
	strcpy(s, source_path);
	pc_add_path(s, source_name);

	result = _dos_findfirst(s, _A_NORMAL | _A_RDONLY | _A_SUBDIR, dosbuf);

	while (result == 0) {
		doit = TRUE;
		if (!(strcmp(dosbuf->name, ".") && strcmp(dosbuf->name, "..")))
			doit = FALSE;
		else if ((option_a || option_m) && !(dosbuf->attrib & _A_ARCH))
			doit = FALSE;
			
		if (doit) {
			node = emalloc(sizeof(struct flist_entry));
			node->name = emalloc(strlen(dosbuf->name)+1);
			strcpy(node->name, dosbuf->name);
			if (dosbuf->attrib & _A_SUBDIR)
				node->is_file = FALSE;
			else
				node->is_file = TRUE;
			list_add_tail(*listhead, &node->node);
		}
		result = _dos_findnext(dosbuf);
	}

	free(s);
	free(dosbuf);
}

/*****************************************************************************
 * actual copier - copies a directory and it's subidirectories recursively
 ****************************************************************************/
void copy_stuff(char *source_path, char *source_file,
		char *dest_path, char *dest_file,
		struct list_head **listhead, int parent_exists)
{
char *s, *d;
struct flist_entry *p, *n;
int doit;
int answer;
unsigned attrib;
struct lock_entry *parentlock, *subdirlock;

	s = emalloc(512);
	
	d = emalloc(512);

	build_list(listhead, source_path, source_file);

	parentlock = 0;
	subdirlock = 0;
	
	/* do files */
	for (p = (struct flist_entry *)(*listhead)->head;
	     n = (struct flist_entry *)p->node.next;
	     p = n) {
		if (p->is_file) {
			/* if this dir doesn't exist, make it */
			if (!parent_exists) {
				parentlock = amiga_create_dir_all(dest_path);
				parent_exists = TRUE;
			}
			strcpy(s, source_path);
			pc_add_path(s, p->name);

			strcpy(d, dest_path);
			if (dest_file && dest_file[0] != 0)
				amiga_add_path(d, dest_file);
			else
				amiga_add_path(d, p->name);

			doit = TRUE;
			if (option_p) {
				printf("Copy file %s (Y/N)? ", s);
				fflush(stdout);
				answer = getche();
				printf("\n");
				if (isupper(answer))
					answer = tolower(answer);
				if (answer != 'y')
					doit = FALSE;
			}

			if (doit) {
				copy_file(s, d);

				if (option_m) {
					/* clear archive bit on source */
					if (!_dos_getfileattr(s, &attrib)) {
						attrib &= ~_A_ARCH;
						_dos_setfileattr(s, attrib);
					}
				}
			}
			
			list_remove(&p->node);	/* rip us out */
			free(p->name);		/* dump name on the floor */
			free(p);		/* recycle */
		}
	}

	/* do dirs */
	for (p = (struct flist_entry *)(*listhead)->head;
	     n = (struct flist_entry *)p->node.next;
	     p = n) {
		if (!p->is_file) {
			if (option_s) {
				strcpy(s, source_path);
				pc_add_path(s, p->name);
				
				strcpy(d, dest_path);
				amiga_add_path(d, p->name);

				doit = TRUE;
				if (option_p) {
					printf("Copy directory %s (Y/N)? ", s);
					fflush(stdout);
					answer = getche();
					printf("\n");
					if (isupper(answer))
						answer = tolower(answer);
					if (answer != 'y')
						doit = FALSE;
				}

				if (doit) {
					if (option_e) {
						subdirlock = amiga_create_dir(d);
						copy_stuff(s, source_file,
							   d, dest_file,
							   (struct list_head **)&p->dir,
							   TRUE);
						amiga_unlock(subdirlock);
					} else {
						copy_stuff(s, source_file,
							   d, dest_file,
							   (struct list_head **)&p->dir,
							   FALSE);
					}

					if (option_m) {
						/* clear archive bit on source */
						if (!_dos_getfileattr(s, &attrib)) {
							attrib &= ~_A_ARCH;
							_dos_setfileattr(s, attrib);
						}
					}
				}
			}

			list_remove(&p->node);	/* rip us out */
			free(p->name);		/* dump name on the floor */
			free(p);		/* recycle */
		}
	}

	if (parentlock)
		amiga_unlock(parentlock);

	free(*listhead);
	*listhead = 0;
	free(d);
	free(s);

}


void quit(int retval)
{
struct lock_entry *p, *n;

	/* close PC file if transfer aborted */
	if (pcfile) {
		fclose(pcfile);
		pcfile = 0;
	}

	/* close amiga file if transfer aborted */
	if (amfile) {
		j_Close(ds, amfile);
		j_DeleteFile(ds, destname);
		amfile = 0;
	}

	/* free all locks we own */
	for (p = (struct lock_entry *)locklist.tail;
	     n = (struct lock_entry *)p->node.prev;
	     p = n) {
		amiga_unlock(p);
	}

	if (ds) {
		j_close_dosserv(ds);
		ds = 0;
	}	

	/* back to dos (yuck!) */
	exit(retval);
}

void breakhandler()
{
	quit(1);
}

/*****************************************************************************
 * main: arg parsing etc.
 ****************************************************************************/
void main(int argc, char **argv)
{
int i;
struct list_head *p;
static char source[256], source_path[256], source_name[256];
static char dest[256], dest_path[256], dest_name[256];
static char x[512];
char c, *a;
int answer;
int parent_exists;
struct lock_entry *rootlock;

	if (argc == 0) {
		/* workbench! */
		exit(1);
	}

	pcfile = 0;
	amfile = 0;
	list_init(&locklist);
	ds = 0;

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
	if (!(ds = j_open_dosserv())) {
		fprintf(stderr, "DOSServ service not available\n");
		quit(1);
	}

	/* need at least one argument */
	if (argc < 2) {
		usage();
		quit(1);
	}

	/* 1st arg is source */
	strcpy(source, "");

	/* 2nd arg is dest */
	strcpy(dest, "");
	
	/* process rest of args (if any - will all be options */
	for (i = 1; i < argc; i++) {
		a = &argv[i][0];
		if (a[0] != '/') {
			if (i == 1 && source[0] == 0) {
				strcpy(source, argv[i]);
			} else if (i == 2 && dest[0] == 0) {
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
#if 0
					case 'd':
						/* copy files >= this date */
						/* ... parse date from string to numeric form ... */
						option_d = 0;
						break;
#endif

#ifdef ALLOW_OPT_E
					case 'e':
						/* copy empty subdirs */
						option_e = TRUE;
						break;
#endif

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

#if 0
					case 'v':
						/* verify destination file after copy */
						option_v = TRUE;
						break;
#endif

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

#ifndef ALLOW_OPT_E
	option_e = TRUE;
#endif

	if (source[0] == 0 || dest[0] == 0) {
		fprintf(stderr, "No destination and/or source file specified\n");
		quit(1);
	}

	/* here:
	 *	source = source name (one file, wild file, or dir name)
	 *	dest = destination name (file or dir name)
	 *	option_? = all set for options user specified
	 *
	 * now we validate that what he wants us to do makes sense!
	 */

	if (option_w) {
		printf("Press any key to start: ");
		fflush(stdout);
		answer = getch();
		printf("\n");
	}

/* process source ************************************************************/

	/* split source into source_path and source_name */
	for (i = strlen(source)-1; i >= 0; i--) {
		if (source[i] == ':' || source[i] == '\\') {
			strcpy(source_path, source);
			source_path[i+1] = 0;
			strcpy(source_name, &source[i+1]);
			break;
		}
	}
	if (i < 0) {
		strcpy(source_path, "");
		strcpy(source_name, source);
	}

	/* if the name is '.' or '..', we goofed - put the . or .. in the path */
	if (strcmp(source_name, ".") == 0 || strcmp(source_name, "..") == 0) {
		pc_add_path(source_path, source_name);
		strcpy(source_name, "");
	}

	/* if the name is empty, we want *.* */
	if (source_name[0] == 0) {
		strcat(source_name, "*.*");
	}

	/* if the path has wildcards, that's not good */
	if (strchr(source_path, '?') || strchr(source_path, '*')) {
		fprintf(stderr, "PC source path cannot contain wildcards\n");
		quit(1);
	}

	/* if the name is wild or is a directory, we're doing multi files */
	if (strchr(source_name, '?') || strchr(source_name, '*')) {
		multi_source = TRUE;
	} else {
		/* if source is a dir, we're doing multi files */
		strcpy(x, source_path);
		pc_add_path(x, source_name);
		if (pc_exists(x) && pc_is_dir(x)) {
			/* it's a dir */
			strcpy(source_path, x);
			strcpy(source_name, "*.*");
			multi_source = TRUE;
		} else {
			/* it's a file, or it doesn't exist */
			multi_source = FALSE;
		}
	}


/* process dest ************************************************************/

	/* split dest into dest_path and dest_name */
	for (i = strlen(dest)-1; i > 0; i--) {
		if (dest[i] == ':' || dest[i] == '/') {
			strcpy(dest_path, dest);
			dest_path[i+1] = 0;
			strcpy(dest_name, &dest[i+1]);
			break;
		}
	}
	if (i < 0) {
		strcpy(dest_path, "");
		strcpy(dest_name, dest);
	}

	/* make sure dest has a : in it (ie, has a device/vol name) */
	if (strchr(dest_path, ':') == 0) {
		fprintf(stderr,
		 "Amiga destination path must include device name or volume name\n");
		quit(1);
	}

	parent_exists = FALSE;
	
	/* make sure stuff is correct for multi-source copy */
	if (multi_source) {
		/* dest must either not exist or be a directory */
		strcpy(x, dest_path);
		amiga_add_path(x, dest_name);
		if (rootlock = amiga_exists(x)) {
			if (amiga_is_dir(rootlock)) {
				strcpy(dest_path, x);
				strcpy(dest_name, "");
				parent_exists = TRUE;
			} else {
				fprintf(stderr,
				 "Amiga destination must be a directory for multi-file copies\n");
				 quit(1);
			}
		} else {
			/* doesn't exist - we'll make a dir when we get around to it */
			strcpy(dest_path, x);
			strcpy(dest_name, "");
		}
	} else {
		/* single source - dest can be anything */
		strcpy(x, dest_path);
		amiga_add_path(x, dest_name);
		if (rootlock = amiga_exists(x)) {
			if (amiga_is_dir(rootlock)) {
				/* if dest is a dir, use the source filename */
				strcpy(dest_path, x);
				strcpy(dest_name, "");
				parent_exists = TRUE;
			} else {
				amiga_unlock(rootlock);
			}
		}
	}

/* do proper type of copy ***************************************************/
/*
 * source = original source as specified by the user
 * source_path = directory component of source
 * source_file = file component of source
 * multi_source = TRUE if source_file is wild
 * dest = original destination as specified by the user
 * dest_path = directory component of destination
 * dest_file = destination filename ("" if depends on source)
 * option_s = TRUE if we want subdirectories
 * option_e = TRUE if we want subdirectories even if they're empty
 */

	copy_stuff(source_path, source_name, dest_path, dest_name,
		   &p, parent_exists);

	if (n_copied == 0) {
		fprintf(stderr, "No files found\n");
	}

	quit(0);
}
