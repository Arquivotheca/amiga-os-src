#define STITLE	"Installation Utility 2.1"

#define DIRS_NOT_FANCY 1

/* things you need for install.c functions */

/* structures ********************/

struct file_node {
	struct Node node;	/* linkage */
	char *src_name;		/* actual file name (with '+' things) */
	char *dest_name;	/* what the user will see on his dest */
	struct List *ramhead;	/* head of RAM buffer list */
	LONG blocks;		/* number of blocks in file */
	LONG bytes;		/* number of bytes in file */
	LONG idx;		/* which item in list */
	UBYTE type;		/* file or dir? */
	UBYTE selected;		/* whether or not this is selected */
	UBYTE o_read;		/* is it currently open for read? */
	UBYTE o_write;		/* is it currently open for write? */
	UBYTE in_ram;		/* is at least some of it in ram? */
	UBYTE depth;		/* depth of dir */
};

#define FILENODE_TYPE_UNKNOWN	0
#define FILENODE_TYPE_FILE	1
#define FILENODE_TYPE_DIR	2

struct lock_node {
	struct Node node;
	BPTR lock;
};

struct ram_node {
	struct Node node;
	LONG index;
	LONG len;
};

struct open_node {
	struct Node node;
	struct FileHandle *fh;
};

struct mem_node {
	struct Node node;
	LONG size;
};

/* publics ***********************/

extern void *my_malloc(LONG n);
extern void my_free(void *p);

extern void quit(int retval);

#define QUIT_OK				0
#define QUIT_ADDSORTED_EVIL		1
#define QUIT_SCAN_NOMEM			2
#define QUIT_SCAN_EXAMINE		3
#define QUIT_SCAN_NOTDIR		4
#define QUIT_ALLOC_DELETE_FAIL		6
#define QUIT_INIT_FAILED		8
#define QUIT_OPEN_DEFDEL_FAIL		9
#define QUIT_ALLOC_DEFDEL_FAIL		10
#define QUIT_OPEN_INSTALL_FAIL		12
#define QUIT_ALLOC_INSTALL_FAIL		13
#define QUIT_SPLIT_INSTALL_FAIL		14
#define QUIT_LOCK_INSTALL_FAIL		17
#define QUIT_EXAMINE_INSTALL_FAIL	19
#define QUIT_EDIT_INSTALL		20
#define QUIT_GETDEV_NOMEM		21
#define QUIT_GETDELETE_LOCK		22
#define QUIT_EDIT_DELETE		24
#define QUIT_WTHIS_WRITE		25
#define QUIT_DOINST_NOMEM		26
#define QUIT_FLUSH_NOMEM		27
#define QUIT_FLUSH_EXAM			28
#define QUIT_FLUSH_OPEN			29
#define QUIT_FLUSH_BITTER		30
#define QUIT_DOINST_NOSRC		31
#define QUIT_DOINST_NOGENTLE		33
#define QUIT_DOINST_READERR		34
#define QUIT_MY_LOCK_NOMEM		35
#define QUIT_AVAIL_MEM			36
#define QUIT_AVAIL_LOCK			37
#define QUIT_AVAIL_INFO			38
#define QUIT_MAKEDIR_NOMEM		39
#define QUIT_MAKEDIR_EXAMINE_FAIL	40
#define QUIT_MAKEDIR_CANT		41
#define QUIT_PLEASE_DEL_FILE		42
