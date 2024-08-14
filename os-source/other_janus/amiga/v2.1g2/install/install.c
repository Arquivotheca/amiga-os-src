#include <exec/types.h>
#include <exec/exec.h>
#include <libraries/dos.h>

#include <stdio.h>
#include <string.h>

#include <proto/all.h>

#include "init.h"

#include "intui.h"

#include "install.h"

/* statics ***********************/

static struct List my_mem_list;		/* my_malloc/my_malloc_gently/my_free/my_freeall */
static struct List my_lock_list;	/* my_lock/my_unlock/my_unlockall */
#if SELECT_INSTALL_DISK
static struct List disk_list;		/* GetDisks/FreeDisks */
#endif
static struct List delete_list;		/* list of files to delete from dest */
static struct List install_list;	/* list of files to install on dest */
static struct List my_open_list;	/* list of files open */

static struct open_node *fout;		/* file currently being written */

static char default_delete_name[512];	/* name of list of files to delete */
static char install_name[512];		/* name of list of files to install */
static char source_volume[64];		/* name of source disk */
static char dest_volume[64];		/* name of dest disk */

static LONG dest_blocks_free;		/* # of blocks free on dest volume */
static LONG install_blocks;		/* # of blocks we're going to install */
static LONG delete_blocks;		/* # of blocks marked for deletion */

/*********************************/

struct err_desc {
	int num;
	char *str;
} errlist[] = {
	{ QUIT_OK,			"No Error" },
	{ QUIT_ADDSORTED_EVIL,		"Internal Error" },
	{ QUIT_SCAN_NOMEM,		"Out of memory while reading directory" },
	{ QUIT_SCAN_EXAMINE,		"Couldn't Examine() directory entry" },
	{ QUIT_SCAN_NOTDIR,		"Attempt to get a directory of a file" },
	{ QUIT_INIT_FAILED,		"Bad arguments or couldn't initialize" },
	{ QUIT_OPEN_DEFDEL_FAIL,	"Couldn't open default delete list" },
	{ QUIT_ALLOC_DEFDEL_FAIL,	"Out of memory while reading default delete list" },
	{ QUIT_OPEN_INSTALL_FAIL,	"Couldn't open default install list" },
	{ QUIT_ALLOC_INSTALL_FAIL,	"Out of memory while reading default install list" },
	{ QUIT_SPLIT_INSTALL_FAIL,	"Bad line in default install list" },
	{ QUIT_LOCK_INSTALL_FAIL,	"Couldn't find file/dir specified in install list" },
	{ QUIT_EXAMINE_INSTALL_FAIL,	"Couldn't Examine() file/dir specified in install list" },
	{ QUIT_EDIT_INSTALL,		"User Abort from Install List Edit screen" },
	{ QUIT_GETDELETE_LOCK,		"Couldn't open destination volume" },
	{ QUIT_EDIT_DELETE,		"User Abort from Delete List Edit screen" },
	{ QUIT_WTHIS_WRITE,		"Error writing to destination volume" },
	{ QUIT_DOINST_NOMEM,		"Out of memory while reading source to RAM" },
	{ QUIT_FLUSH_NOMEM,		"Out of memory while flushing RAM to destination" },
	{ QUIT_FLUSH_EXAM,		"Couldn't Examine() file/dir while flushing" },
	{ QUIT_FLUSH_OPEN,		"Couldn't create file/dir on destination volume" },
	{ QUIT_FLUSH_BITTER,		"Attempt to overwrite a file with a dir or vice versa" },
	{ QUIT_DOINST_NOSRC,		"Couldn't open file on source disk" },
	{ QUIT_DOINST_NOGENTLE,		"Out of buffer memory" },
	{ QUIT_DOINST_READERR,		"Read error on source disk" },
	{ QUIT_MY_LOCK_NOMEM,		"Out of memory during Lock()" },
	{ QUIT_AVAIL_MEM,		"Out of memory during avail_blocks()" },
	{ QUIT_AVAIL_LOCK,		"Couldn't lock during avail_blocks()" },
	{ QUIT_AVAIL_INFO,		"Couldn't Info() during avail_blocks()" },
	{ QUIT_MAKEDIR_NOMEM,		"Out of memory during Makedir()" },
	{ QUIT_MAKEDIR_EXAMINE_FAIL,	"Couldn't Examine() during Makedir()" },
	{ QUIT_MAKEDIR_CANT,		"Couldn't create directory" },
	{ QUIT_PLEASE_DEL_FILE,		"Attempt to overwrite a file with a dir or vice versa" },
};

#define MSIZE ((sizeof(struct mem_node) + 3) & ~0x03)
void *my_malloc(LONG n)
{
struct mem_node *m;

	n += MSIZE;

	if (m = (struct mem_node *)AllocMem(n, MEMF_PUBLIC | MEMF_CLEAR)) {
		m->size = n;
		AddTail(&my_mem_list, &m->node);
		return (void *)(((char *)m) + MSIZE);
	} else {
		return 0;
	}
}

void my_free(void *p)
{
struct mem_node *m;

	if (p) {
		m = (struct mem_node *)(((char *)p) - MSIZE);
		Remove(&m->node);
		FreeMem(m, m->size);
	}
}

#define MAX_GENTLE	65536	/* allocate in 64K chunks max */
#define GENTLE_QUANTUM	1024	/* decrease allocation in 1K chunks */
#define MIN_AVAIL	32768	/* leave at least 32K free */

void *my_malloc_gently(LONG *len)
{
void *p, *x;
LONG min;

	/* assume failure */
	p = 0;

	/* determine lower limit of block size */
	if (*len < MAX_GENTLE) {
		min = *len;
	} else {
		min = MAX_GENTLE;
	}

	/* allocate a piece we'll give back.  this is the smallest free
	 * chunk we'll leave if we succeed
	 */
	if (x = (void *)AllocMem(MIN_AVAIL, MEMF_CHIP)) {

		/* everybody hold their breath... */
		Forbid();

		/* try, try again... */
		while (!(p = my_malloc(*len))) {

			/* fail.  can we chop it down some more? */
			if (*len > GENTLE_QUANTUM && *len - GENTLE_QUANTUM > min) {

				/* yes. */
				*len -= GENTLE_QUANTUM;

			} else {

				/* no.  bail out */
				break;
			}
		}

		/* give the system back it's chunk */
		FreeMem(x, MIN_AVAIL);

		/* exhale. */
		Permit();
	}

	return p;
}

void free_fnode(struct file_node *n)
{
	Remove(&n->node);

	/* free source name */
	if (n->src_name)
		my_free(n->src_name);

	/* free dest name */
	if (n->dest_name)
		my_free(n->dest_name);

	/* free node itself */
	my_free(n);
}

struct open_node *my_open(char *name, LONG mode)
{
struct open_node *n;

	if (n = (struct open_node *)my_malloc(sizeof(*n))) {
		if (!(n->fh = (struct FileHandle *)Open(name, mode))) {
			my_free(n);
			n = 0;
		} else {
			AddTail(&my_open_list, &n->node);
		}
	}

	return n;
}

void my_close(struct open_node *n)
{
	Remove(&n->node);
	Close(n->fh);
	my_free(n);
}

void fstrcat(char *d, char *s)
{
int l;

	l = strlen(d);

	if (l > 0) {
		if (d[l-1] != ':' && d[l-1] != '/')
			strcat(d, "/");
	}

	strcat(d, s);
}

/***********************************************************************
 * my_lock
 *
 * resource-tracking version of Lock.  see my_unlock below.
 *
 * stores each lock in a linked list so that when we quit we can get
 * rid of all the locks easily.
 *
 **********************************************************************/
struct lock_node *my_lock(char *name, LONG mode)
{
struct lock_node *node;

	if (!(node = my_malloc(sizeof(*node))))
		quit(QUIT_MY_LOCK_NOMEM);

	if (node->lock = Lock(name, mode)) {
		AddTail(&my_lock_list, &node->node);
	} else {
		my_free(node);
		node = 0;
	}

	return node;
}

/***********************************************************************
 * my_unlock
 *
 * resource-tracking version of UnLock.  see my_lock above.
 *
 **********************************************************************/
void my_unlock(struct lock_node *lock)
{
	Remove(&lock->node);
	UnLock(lock->lock);
	my_free(lock);
}

LONG avail_blocks(char *volume)
{
struct lock_node *lock;
struct InfoData *info;
LONG retval;
char *p, c;

	if (p = strchr(volume, ':')) {
		c = *(p+1);
		*(p+1) = 0;
	}

	if (!(info = my_malloc(sizeof(*info))))
		quit(QUIT_AVAIL_MEM);

	if (!(lock = my_lock(volume, SHARED_LOCK)))
		quit(QUIT_AVAIL_LOCK);

	if (!(Info(lock->lock, info)))
		quit(QUIT_AVAIL_INFO);

	retval = info->id_NumBlocks - info->id_NumBlocksUsed;

	my_unlock(lock);

	my_free(info);

	*(p+1) = c;

	return retval;
}

/***********************************************************************
 * quit
 *
 * clean up and exit.
 *
 **********************************************************************/
void quit(int retval)
{
struct file_node *f, *f1;
struct open_node *o, *o1;
struct lock_node *l, *l1;
struct mem_node *m, *m1;
int i;

	if (w && retval) {
		for (i = 0; i < sizeof(errlist) / sizeof(struct err_desc); i++) {
			if (errlist[i].num == retval) {
				Notice(w, "Installation error:", errlist[i].str);
				goto skipit;
			}
		}
		Notice(w, "Installation error:", "Unknown error code");
skipit:
	}

	/* close down intuition things */
	Bye();

	/* close open files */
	for (o = (struct open_node *)my_open_list.lh_TailPred;
	     o1 = (struct open_node *)o->node.ln_Pred;
	     o = o1)
		my_close(o);

	/* empty my_lock_list */
	for (l = (struct lock_node *)my_lock_list.lh_TailPred;
	     l1 = (struct lock_node *)l->node.ln_Pred;
	     l = l1)
		my_unlock(l);

#if SELECT_INSTALL_DISK
	/* empty disk_list */
	FreeDisks(&disk_list);
#endif

	/* empty delete_list */
	for (f = (struct file_node *)delete_list.lh_TailPred;
	     f1 = (struct file_node *)f->node.ln_Pred;
	     f = f1)
		free_fnode(f);

	/* empty install_list */
	for (f = (struct file_node *)install_list.lh_TailPred;
	     f1 = (struct file_node *)f->node.ln_Pred;
	     f = f1)
		free_fnode(f);

	/* empty my_mem_list */
	for (m = (struct mem_node *)my_mem_list.lh_TailPred;
	     m1 = (struct mem_node *)m->node.ln_Pred;
	     m = m1)
		my_free((void *)(((char *)m) + MSIZE));

	exit(retval);
}

/***********************************************************************
 * add_sorted_by_dest
 *
 * given a list of file_nodes and a file_node, insert the file_node
 * in the list according to alphabetical order of the dest_name field
 * of the node.
 *
 * if we find an identical node, we make sure it's TYPE_UNKNOWN and
 * replace it with the new node.  if it's not TYPE_UNKNOWN, we panic
 * and die horribly.
 *
 **********************************************************************/
void add_sorted_by_dest(struct List *list, struct file_node *node)
{
struct file_node *n, *p;
int diff;

	for (n = (struct file_node *)list->lh_Head;
	     p = (struct file_node *)n->node.ln_Succ;
	     n = p) {
		diff = stricmp(n->dest_name, node->dest_name);
		if (diff == 0) {
			/* we be same.  hopefully superceding unknown */
			if (n->type == FILENODE_TYPE_UNKNOWN) {
				/* update his selected field */
				node->selected = n->selected;

				/* now install new one in it's place */				
				Insert(list, &node->node, &n->node);
				
				/* wango tango.  nuke the unknown */
				free_fnode(n);

				/* done. */
				return;
			} else {
				/* something EVIL has happened */
				quit(QUIT_ADDSORTED_EVIL);
			}
		} else if (diff > 0) {
			/* insert us before this one */
			Insert(list, &node->node, n->node.ln_Pred);
			return;
		}
	}

	/* hmm.  it's either empty or we're the biggest */
	AddTail(list, &node->node);
}

/***********************************************************************
 * scan_dir
 *
 * walk a directory tree, adding dirs & files to the given list via
 * add_sorted_by_dest().
 *
 **********************************************************************/
void scan_dir(struct lock_node *lock, char *sourcepath,
	      struct List *list, int mark, int depth)
{
struct lock_node *newlock;
BPTR prev;
struct FileInfoBlock *fib;
char *source, *p;
struct file_node *node;

	if (!(fib = my_malloc(sizeof(*fib))))
		quit(QUIT_SCAN_NOMEM);

	if (!Examine(lock->lock, fib))
		quit(QUIT_SCAN_EXAMINE);

	if (fib->fib_DirEntryType < 0)
		quit(QUIT_SCAN_NOTDIR);

	if (!(source = my_malloc(strlen(sourcepath) + 32)))
		quit(QUIT_SCAN_NOMEM);

	prev = CurrentDir(lock->lock);

	/* scan yonder directory */
	while (ExNext(lock->lock, fib)) {

		if (newlock = my_lock(fib->fib_FileName, SHARED_LOCK)) {
			strcpy(source, sourcepath);
			if (source[strlen(source) - 1] != ':')
				strcat(source, "/");
			strcat(source, fib->fib_FileName);

			/* alloc mem for the node */
			if (!(node = my_malloc(sizeof(*node))))
				quit(QUIT_SCAN_NOMEM);

			/* alloc mem for the dest string */
			p = strchr(source, ':') + 1;
			if (!(node->dest_name = my_malloc(strlen(p) + 1)))
				quit(QUIT_SCAN_NOMEM);

			/* copy name into node */
			strcpy(node->dest_name, p);

			/* copy the mark */
			node->selected = mark;

			/* the direntry type */
			if (fib->fib_DirEntryType < 0)
				node->type = FILENODE_TYPE_FILE;
			else
				node->type = FILENODE_TYPE_DIR;

			/* blocks occupied by file/dir */
			node->blocks = fib->fib_NumBlocks + 1;
			node->bytes = fib->fib_Size;

			/* depth */
			node->depth = depth;

			/* attach to list */
			add_sorted_by_dest(list, node);

			/* if this is a dir, recurse */
			if (node->type == FILENODE_TYPE_DIR)
				scan_dir(newlock, source, list, mark, depth + 1);
		}

		my_unlock(newlock);
	}

	CurrentDir(prev);

	my_free(fib);
	my_free(source);
}

void init(int argc, char **argv)
{
	NewList(&my_mem_list);
	NewList(&my_lock_list);
#if SELECT_INSTALL_DISK
	NewList(&disk_list);
#endif
	NewList(&delete_list);
	NewList(&install_list);
	NewList(&my_open_list);

	fout = 0;

	strcpy(default_delete_name, "");
	strcpy(install_name, "");

	strcpy(source_volume, "");
	strcpy(dest_volume, "");

	/* intuition and user interface stuff */
	if (!Init(argc, argv, install_name, default_delete_name, source_volume, dest_volume))
		quit(QUIT_INIT_FAILED);
}

void get_disk_list()
{
#if SELECT_INSTALL_DISK
struct Node *n, *n1;

	/* Get system disk configuration */
	GetDisks(&disk_list);

	/* omit RAM: */
	for (n = disk_list.lh_Head; n1 = n->ln_Succ; n = n1) {
		if (stricmp(n->ln_Name, "ram") == 0) {
			Remove(n);
			FreeNode(n);
		}
	}
#endif
}

/***********************************************************************
 * get_default_delete_list
 *
 * the default delete list is in a file of the form:
 *	dir1/name1
 *	dir1/name2
 *	dir2/name3
 *	...
 *
 * lines beginning with ';' are comments.
 *
 * the volume prefix must be left off - it will be supplied based on
 * the user's choice of an installation volume.
 *
 * each line must be terminated by a single \n.
 *
 **********************************************************************/
void get_default_delete_list()
{
FILE *f;
int len;
char buf[512];
struct file_node *node;

	PrintMsg("Reading default list of files to delete...");

	if (!default_delete_name[0])
		return;

	if (!(f = fopen(default_delete_name, "r")))
		quit(QUIT_OPEN_DEFDEL_FAIL);

	/* insurance - fgets() might not return a null terminated string */
	buf[sizeof(buf) - 1] = 0;

	/* suck the file dry */
	while (fgets(buf, sizeof(buf) - 1, f)) {

		/* check it out */
		len = strlen(buf);
		if (len == 0)
			continue;

		/* skip comments */
		if (buf[0] == ';')
			continue;

		/* delete \n on end */
		if (buf[len - 1] == '\n') {
			buf[len - 1] = 0;
			len--;
			if (len == 0)
				continue;
		}

		/* make len include null */
		len++;

		/* alloc mem for the node */
		if (!(node = my_malloc(sizeof(*node))))
			quit(QUIT_ALLOC_DEFDEL_FAIL);

		/* alloc mem for the string */
		if (!(node->dest_name = my_malloc(len)))
			quit(QUIT_ALLOC_DEFDEL_FAIL);

		/* copy name into node */
		strcpy(node->dest_name, buf);

		/* it's marked for deletion */
		node->selected = 1;

		/* we don't know it's length or type (yet) */
		node->blocks = -1;
		node->bytes = -1;
		node->type = FILENODE_TYPE_UNKNOWN;

		/* attach to list */
		add_sorted_by_dest(&delete_list, node);
	}

	fclose(f);
}

/***********************************************************************
 * get_install_list
 *
 * the install list is in a file of the form:
 *	dir1/name1:destdir/name1
 *	dir1/name2:destdir/name2
 *	dir2/name3:destdir/name3
 *	...
 *
 * the name before the : is the source filename.  the volume prefix is
 * provided by the PCInstall disk's name.
 *
 * lines beginning with ';' are comments.
 *
 * the name after the : is the destination filename.  the volume prefix
 * is provided by the user's choice of install destination.
 *
 * each line must be terminated by a single \n.
 *
 **********************************************************************/
void get_install_list()
{
FILE *f;
int len;
char buf[512];
struct file_node *node;
char *p;
struct lock_node *lock;
struct FileInfoBlock *fib;

	PrintMsg("Reading default list of files to install...");

	/* zero the install count */
	install_blocks = 0;

	if (!install_name[0])
		return;

	if (!(f = fopen(install_name, "r")))
		quit(QUIT_OPEN_INSTALL_FAIL);

	/* insurance - fgets() might not return a null terminated string */
	buf[sizeof(buf) - 1] = 0;

	/* suck the file dry */
	while (fgets(buf, sizeof(buf) - 1, f)) {

		/* check it out */
		len = strlen(buf);
		if (len == 0)
			continue;

		/* skip comments */
		if (buf[0] == ';')
			continue;

		/* delete \n on end */
		if (buf[len - 1] == '\n') {
			buf[len - 1] = 0;
			len--;
			if (len == 0)
				continue;
		}

		/* make len include null */
		len++;

		/* alloc mem for the node */
		if (!(node = my_malloc(sizeof(*node))))
			quit(QUIT_ALLOC_INSTALL_FAIL);

		/* split it at the : */
		if (!(p = strchr(buf, ':')))
			quit(QUIT_SPLIT_INSTALL_FAIL);
		*p++ = 0;

		/* alloc mem for the src string */
		if (!(node->src_name = my_malloc(strlen(buf) + 1)))
			quit(QUIT_ALLOC_INSTALL_FAIL);

		/* alloc mem for the dest string */
		if (!(node->dest_name = my_malloc(strlen(p) + 1)))
			quit(QUIT_ALLOC_INSTALL_FAIL);

		/* copy name into node */
		strcpy(node->src_name, buf);

		/* copy name into node */
		strcpy(node->dest_name, p);

		/* it's marked for installation */
		node->selected = 1;

		/* build filename */
		strcpy(buf, source_volume);
		strcat(buf, node->src_name);

		/* get a lock on it */
		if (!(lock = my_lock(buf, SHARED_LOCK))) {
			quit(QUIT_LOCK_INSTALL_FAIL);
		}

		/* grasp a fib firmly by the ears */
		if (!(fib = my_malloc(sizeof(*fib))))
			quit(QUIT_ALLOC_INSTALL_FAIL);

		/* examine it */
		if (!Examine(lock->lock, fib))
			quit(QUIT_EXAMINE_INSTALL_FAIL);

		/* the direntry type */
		if (fib->fib_DirEntryType < 0)
			node->type = FILENODE_TYPE_FILE;
		else
			node->type = FILENODE_TYPE_DIR;

		/* blocks occupied by file/dir */
		node->blocks = fib->fib_NumBlocks + 1;
		node->bytes = fib->fib_Size;

		/* add up blocks to install */
		install_blocks += node->blocks;

		/* bye bye fib */
		my_free(fib);

		/* unlock */
		my_unlock(lock);

		/* attach to list */
		add_sorted_by_dest(&install_list, node);
	}

	fclose(f);
}

void edit_install_list()
{
	/* Prompt user with above list and allow them to make changes */
	PrintMsg(
 "Select the files you want installed.   Default: ALL  (Hit OK when ready)");
	if (mark_file_list(&install_list, &install_blocks, &install_blocks, &dest_blocks_free, 0) == -1)
		quit(QUIT_EDIT_INSTALL);
}

/***********************************************************************
 * get_delete_list
 *
 * read the delete list (ie, a dir of the source volume), merging
 * with anything already in the list (from get_default_delete_list()).
 *
 **********************************************************************/
void get_delete_list()
{
struct lock_node *lock;
char *p, c;

	p = strchr(dest_volume, ':');
	c = *(p+1);
	*(p+1) = 0;

	PrintMsg("Reading directory of destination volume...");

	if (!(lock = my_lock(dest_volume, SHARED_LOCK)))
		quit(QUIT_GETDELETE_LOCK);

	scan_dir(lock, source_volume, &delete_list, 0, 0);

	my_unlock(lock);

	*(p+1) = c;

	dest_blocks_free = avail_blocks(dest_volume);
}

/***********************************************************************
 * clean_delete_list
 *
 * get rid of any UNKNOWN files in the delete list
 *
 **********************************************************************/
void clean_delete_list()
{
struct file_node *node, *n;

	for (node = (struct file_node *)delete_list.lh_Head;
	     n = (struct file_node *)node->node.ln_Succ;
	     node = n) {
		if (node->type == FILENODE_TYPE_UNKNOWN) {

			/* aha!  unlink it */
			free_fnode(node);
		}
	}
}

/***********************************************************************
 * premark_delete_list
 *
 * mark as TBD all files in the install list
 *
 **********************************************************************/
void premark_delete_list()
{
struct file_node *inode, *inn, *dnode, *dnn;

	for (inode = (struct file_node *)install_list.lh_Head;
	     inn = (struct file_node *)inode->node.ln_Succ;
	     inode = inn) {
		if (inode->selected) {
			for (dnode = (struct file_node *)delete_list.lh_Head;
			     dnn = (struct file_node *)dnode->node.ln_Succ;
			     dnode = dnn) {
				if (stricmp(inode->dest_name, dnode->dest_name) == 0) {
					dnode->selected = 1;
					break;
				}
			}
		}
	}
}

void edit_delete_list()
{
struct file_node *n;
LONG blocks;

	/* figure # of blocks to delete */
	delete_blocks = 0;
	for (n = (struct file_node *)delete_list.lh_Head;
	     n->node.ln_Succ;
	     n = (struct file_node *)n->node.ln_Succ) {
		if (n->selected)
			delete_blocks += n->blocks;
	}

	blocks = dest_blocks_free - install_blocks;

	PrintMsg("Select the files you want deleted.  (Hit OK when ready)");

	do {
		if (mark_file_list(&delete_list, &delete_blocks, &blocks, &delete_blocks, 1) == -1)
			quit(QUIT_EDIT_DELETE);

		if (dest_blocks_free + delete_blocks < install_blocks) {
			Notice(w, "You must select more files to delete.",
				  "There isn't enough room on the destination disk yet.");
		}
	} while (dest_blocks_free + delete_blocks < install_blocks);
}

/***********************************************************************
 * do_delete_list
 *
 * delete marked files in the delete list.  goes backwards to get
 * contents of entire dirs before the dir itself.  removes/frees the
 * node from the list when a delete is successful.
 *
 **********************************************************************/
void do_delete_list()
{
struct file_node *node, *n;
char name[512];

	PrintMsg("Deleting marked files from destination volume...");

	for (node = (struct file_node *)delete_list.lh_TailPred;
	     n = (struct file_node *)node->node.ln_Pred;
	     node = n) {
		if (node->selected) {
			strcpy(name, dest_volume);
			fstrcat(name, node->dest_name);

			if (DeleteFile(name)) {

				/* unlink & delete */
				free_fnode(node);
			}
		}
	}

	dest_blocks_free = avail_blocks(dest_volume);
}

int write_this(struct file_node *fnode)
{
struct ram_node *rnode, *rn;
LONG len, lenw;
char *p;

	/* walk the list of buffers */
	for (rnode = (struct ram_node *)fnode->ramhead->lh_Head;
	     rn = (struct ram_node *)rnode->node.ln_Succ;
	     rnode = rn) {

		/* get length remaining in this buf and where to start */
		len = rnode->len - rnode->index;
		p = ((char *)rnode) + sizeof(*rnode);

		/* try to spit on the disk */
		if ((lenw = Write(fout->fh, &p[rnode->index], len)) > 0) {

			/* did something... */
			if (lenw == len) {

				/* wrote the whole thing.  kill it */
				Remove(&rnode->node);
				my_free(rnode);

			} else {

				/* wrote some, but not all */
				rnode->index += lenw;
				return 1;
			}
		} else {

			/* dos error of some kind */
			if (IoErr() == ERROR_DISK_FULL) {
				return 1;
			} else {
				quit(QUIT_WTHIS_WRITE);
			}
		}
	}

	/* if it's not still being read, we're done with it */
	if (!fnode->o_read) {
		fnode->o_write = 0;
		my_free(fnode->ramhead);
	}

	return 0;
}

int create_dirs(char *dn)
{
char *p, *s;
struct lock_node *lock;
struct FileInfoBlock *fib;
BPTR doslock;

	if (doslock = CreateDir(dn)) {
		UnLock(doslock);
		return 1;
	}

	if (!(fib = my_malloc(sizeof(*fib))))
		quit(QUIT_MAKEDIR_NOMEM);

	if (!(s = my_malloc(strlen(dn) + 2)))
		quit(QUIT_MAKEDIR_NOMEM);

	strcpy(s, dn);
	if (s[strlen(s) - 1] != '/')
		strcat(s, "/");

	p = s;

	while (p = strchr(p, '/')) {
		*p = 0;

		if (lock = my_lock(s, SHARED_LOCK)) {
			/* examine it */
			if (!Examine(lock->lock, fib))
				quit(QUIT_MAKEDIR_EXAMINE_FAIL);

			my_unlock(lock);

			/* the direntry type */
			if (fib->fib_DirEntryType < 0) {
				/* ick.  it's a file */
				*p = '/';

				Notice(w, "Please delete or rename the following file:", s);

				quit(QUIT_PLEASE_DEL_FILE);
			} else {
				/* it's a dir.  next, please. */
				*p++ = '/';
			}
		} else {
			if (!(doslock = CreateDir(s))) {
				quit(QUIT_MAKEDIR_CANT);
			}

			UnLock(doslock);

			/* ahhh. */
			*p++ = '/';
		}
	}

	return 1;
}

struct open_node *my_open_create_dirs(char *name, LONG mode)
{
char *p;

	if (p = strrchr(name, '/')) {
		*p = 0;
		if (!create_dirs(name)) {
			*p = '/';
			return 0;
		}

		*p = '/';
	}

	return my_open(name, mode);
}

int flush_install_ram()
{
struct file_node *fnode, *fn;
struct lock_node *lock;
struct FileInfoBlock *fib;
char *dn;
int type;

	PrintMsg("Writing files from ram to destination volume...");

	/* if we've got a file open now, finish it up */
	if (fout) {
		/* go through list */
		for (fnode = (struct file_node *)install_list.lh_Head;
		     fn = (struct file_node *)fnode->node.ln_Succ;
		     fnode = fn) {
			if (fnode->selected && fnode->o_write) {
				if (write_this(fnode))
					return 1;

				/* if not still being read, we're done with it */
				if (!fnode->o_read && !fnode->o_write) {
					my_close(fout);
					fout = 0;
					fnode->in_ram = 0;
				} else {
					return 0;
				}
			}
		}
	}

	/* go through list */
	for (fnode = (struct file_node *)install_list.lh_Head;
	     fn = (struct file_node *)fnode->node.ln_Succ;
	     fnode = fn) {
		if (fnode->selected && fnode->in_ram) {
			/* allocate & build full source path */
			if (!(dn = my_malloc(strlen(dest_volume) + 1 + strlen(fnode->dest_name) + 1)))
				quit(QUIT_DOINST_NOMEM);
			strcpy(dn, dest_volume);
			fstrcat(dn, fnode->dest_name);

			/* see if the thing is already there */
			if (lock = my_lock(dn, SHARED_LOCK)) {

				/* hmm.  what *is* it? */
				if (!(fib = my_malloc(sizeof(*fib))))
					quit(QUIT_FLUSH_NOMEM);

				if (!(Examine(lock->lock, fib)))
					quit(QUIT_FLUSH_EXAM);

				my_unlock(lock);

				if (fib->fib_DirEntryType < 0)
					type = FILENODE_TYPE_FILE;
				else
					type = FILENODE_TYPE_DIR;

				my_free(fib);
			} else {
				type = fnode->type;
			}

			if (type == fnode->type) {
				/* if it's a dir, create it */
				if (fnode->type == FILENODE_TYPE_DIR) {
					if (!create_dirs(dn)) {
						if (IoErr() == ERROR_DISK_FULL) {
							return 1;
						} else {
							quit(QUIT_FLUSH_OPEN);
						}
					}
				} else {
					if (!(fout = my_open_create_dirs(dn, MODE_NEWFILE))) {
						if (IoErr() == ERROR_DISK_FULL) {
							return 1;
						} else {
							quit(QUIT_FLUSH_OPEN);
						}
					}

					/* we be open */
					fnode->o_write = 1;

					/* try to write */
					if (write_this(fnode))
						return 1;

					/* if not still being read, we're done with it */
					if (!fnode->o_read && !fnode->o_write) {
						my_close(fout);
						fout = 0;
						fnode->in_ram = 0;
					}
				}
			} else {
				/* ... complain bitterly ... */
				quit(QUIT_FLUSH_BITTER);
			}
		}
	}

	return 0;
}

void install_it(struct file_node *node)
{
struct ram_node *ramnode;
char *sn, *p;
LONG len, bytesleft;
struct open_node *fin;

	PrintMsg("Reading files from source volume to ram...");

	/* allocate & build full source path */
	if (!(sn = my_malloc(strlen(source_volume) + strlen(node->src_name) + 1)))
		quit(QUIT_DOINST_NOMEM);
	strcpy(sn, source_volume);
	strcat(sn, node->src_name);

	/* is it a dir or a file? */
	if (node->type == FILENODE_TYPE_FILE) {

		/* open the sucker */
		if (!(fin = my_open(sn, MODE_OLDFILE)))
			quit(QUIT_DOINST_NOSRC);

		/* we're reading it - this also makes it in-ram */
		node->o_read = 1;
		node->in_ram = 1;

		/* allocate a ram head */
		if (!(node->ramhead = my_malloc(sizeof(*node->ramhead))))
			quit(QUIT_DOINST_NOMEM);

		/* init it */
		NewList(node->ramhead);

		/* gotta move this many bytes for this file */
		bytesleft = node->bytes;

		/* until it's done */
		while (bytesleft) {

			/* allocate a ramnode with data tacked on the end */
			len = bytesleft + sizeof(*ramnode);
			if (!(ramnode = my_malloc_gently(&len))) {

				/* failed!  we filled up ram, so drain it */
				while (flush_install_ram()) {

					/* geez, the dest disk is full.  let him
					 * pick more stuff to delete
					 */
					edit_delete_list();
					do_delete_list();
				}

				PrintMsg("Reading files from source volume to ram...");

				/* okay, can we have some memory already? */
				len = bytesleft + sizeof(*ramnode);
				if (!(ramnode = my_malloc_gently(&len)))
					quit(QUIT_DOINST_NOGENTLE);
			}

			/* init node */
			memset(ramnode, 0, sizeof(*ramnode));
			ramnode->len = len - sizeof(*ramnode);
			ramnode->index = 0;
			p = ((char *)ramnode) + sizeof(*ramnode);

			/* read data into node */
			if (Read(fin->fh, p, ramnode->len) != ramnode->len)
				quit(QUIT_DOINST_READERR);

			/* less stuff to do now */
			bytesleft -= ramnode->len;

			/* link it in */
			AddTail(node->ramhead, &ramnode->node);
		}

		/* finished this file! */
		my_close(fin);

		/* ain't open for read no more */
		node->o_read = 0;
	}

	my_free(sn);
}

/***********************************************************************
 * do_install_list
 *
 * copy files in the install_list from the src_name to the dest_name.
 * tries to read as much into RAM as possible, then spits it out to
 * the dest.  if we run out of room on the dest, we return a nonzero
 * value, which makes the install loop present the delete list again.
 * as files are copied, their entries are removed/freed from the list.
 *
 **********************************************************************/
void do_install_list()
{
struct file_node *node, *n;

	/* go through list */
	for (node = (struct file_node *)install_list.lh_Head;
	     n = (struct file_node *)node->node.ln_Succ;
	     node = n) {
		if (node->selected)
			install_it(node);
	}

	while (flush_install_ram()) {
		edit_delete_list();
		do_delete_list();
	}
}

void main(int argc, char **argv)
{
	init(argc, argv);		/* process cmd line/wb args */

	get_disk_list();		/* get list of devices in system */

	get_default_delete_list();	/* read DefDel file */

	get_install_list();		/* get dirs of install src dirs */

	get_delete_list();		/* get dir of install dest */

	edit_install_list();		/* let user pick & choose */

	clean_delete_list();		/* only files & dirs, please */

	premark_delete_list();		/* mark all files being installed
					 * as being deleted as well.
					 */

	edit_delete_list();		/* let user pick & choose */

	do_delete_list();		/* delete stuff */

	do_install_list();		/* install stuff - if runs out of
					 * space, calls edit/do_delete_list
					 * until it can finish
					 */

	quit(QUIT_OK);			/* done. */
}
