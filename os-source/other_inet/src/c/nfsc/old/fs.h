/* -----------------------------------------------------------------------
 * fs.h   header file for nfsc (as225) SAS
 *
 * $Locker:  $
 *
 * $Id: fs.h,v 1.4 92/10/07 15:10:27 bj Exp $
 *
 * $Revision: 1.4 $
 *
 * $Log:	fs.h,v $
 * Revision 1.4  92/10/07  15:10:27  bj
 * binary 37.9
 * 
 * added prototypes for new dos packet function calls.
 * 
 *
 * $Header: Hog:Other/inet/src/c/nfsc/RCS/fs.h,v 1.4 92/10/07 15:10:27 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/*
** AmigaNFS defines.
*/

#ifndef FS_H
#define FS_H

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <devices/timer.h>
#include <ss/socket.h>
#include <rpc/rpc.h>
#include <rpc/xdr.h>
#include <nfs/nfs.h>
#include <nfs/perm.h>
#include <nfs/clmsg.h>
#include <errno.h>
#include <action.h>


#ifdef LATTICE
#include <proto/exec.h>
#include <proto/dos.h>
#endif

#define malloc(a)		AllocVec(a,MEMF_PUBLIC)
#define calloc(a,b)		AllocVec(a*b,MEMF_PUBLIC|MEMF_CLEAR)
#define free(a)			FreeVec(a)

#define FD_MAGIC	(('N'<<24)|('F'<<16)|('S'<<8)|'!')
#define	FD_DIR		0	/* for completeness			*/
#define FD_FILE		1	/* if file, then fd_file has meaning	*/
#define FD_READONLY 	1	/* File handle is read only		*/
#define FD_FHANDLE  	2	/* fdata is referred to by Amiga fhandle*/
#define FD_LOCK	    	4	/* fdata is referred to by lock		*/
#define FD_LINK		8	/* file is a soft link			*/

struct fdata {
	struct FileLock fd_lock;/* fdata & lock go together	 	*/
	long fd_magic;		/* check that we allocated this..	*/
	unsigned char fd_type;	/* type of fdata		 	*/
	unsigned char fd_flags;	/* various control bits		 	*/

	/*
	** Directory related
	*/
	long	fd_dir_fid;	/* fileid for dir fhandle	 	*/
	nfs_fh 	fd_dir;		/* directory containing object	 	*/
	nfscookie fd_dirpos;	/* current NFS directory position	*/
	struct entry *fd_head;	/* Head of cached directory chain	*/
	struct entry *fd_next;	/* Next entry to return			*/
	bool_t	eof;		/* end of cookie			*/

	/*
	** File related
	*/
	long	fd_file_fid;	/* fileid for file fhandle	 	*/
	nfs_fh	fd_file;	/* NFS struct fhandle for object 	*/
	long	fd_pos;		/* current position in file	 	*/
	struct fh_cache	*fd_cache;	/* cache for this file handle	 	*/
};

struct mount_pt {
	struct fdata mt_lock;	/* initial filehandle for partition	*/
	struct DosList *mt_vol;		/* pointer to volume node for vol	*/
	struct MsgPort *mt_port;/* request port				*/
	CLIENT *mt_host;	/* handle for remote host		*/
	long	mt_rdsize;	/* read transfer size			*/
	long	mt_wrsize;	/* write transfer size			*/
	long	mt_nfiles;	/* number of open files			*/
	long	mt_nlocks;	/* number of active locks		*/
	struct mount_pt	*mt_next;/* next in list			*/
	char	mt_path[NFS_MAXPATHLEN]; /* retained path for umount	*/
	struct amiga_stats mt_astat;/* amiga handler packet stats	*/
	struct nfs_stats mt_nstat;/* NFS call stats			*/
	u_char	mt_case;	/* do case conversions?			*/
};

#define NOTFOUND 0		/* Failed at some point in path	*/
#define LASTNOT  1		/* Matched all but last comp	*/
#define ALLFOUND 2		/* Matched entire path		*/

#define CHECKLOCK(fd, dp)\
	fd = dp->dp_Arg1==0 ? &mt->mt_lock: btod(dp->dp_Arg1, struct fdata *);\
	if(fd->fd_magic != FD_MAGIC){\
		dp->dp_Res2 = ERROR_INVALID_LOCK;\
		return;\
	}

#define CHECKDIR(fd, dp)\
	CHECKLOCK(fd, dp)\
	if(fd->fd_type != FD_DIR){\
		dp->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;\
		return;\
	}

/* clnt_generic.c */
CLIENT *clnt_create(char *, u_int, u_int, char *);

/* lk.c */
bool_t get_parent  (struct mount_pt *mt, struct DosPacket *dp, nfs_fh *dirp, struct diropres *dorp);
void nfs_parentdir (struct mount_pt *mt, struct DosPacket *dp);
void nfs_delete    (struct mount_pt *mt, struct DosPacket *dp);
void nfs_createdir (struct mount_pt *mt, struct DosPacket *dp);
void nfs_locate    (struct mount_pt *mt, struct DosPacket *dp);
void nfs_copydir   (struct mount_pt *mt, struct DosPacket *dp);
void nfs_freelock  (struct mount_pt *mt, struct DosPacket *dp);
void nfs_setprotect(struct mount_pt *mt, struct DosPacket *dp);
void nfs_rename    (struct mount_pt *mt, struct DosPacket *dp);
void nfs_readlink  (struct mount_pt *mt, struct DosPacket *dp);
void nfs_makelink  (struct mount_pt *mt, struct DosPacket *dp);
void nfs_fh_from_lk(struct mount_pt *mt, struct DosPacket *dp);
void nfs_same_lock (struct mount_pt *mt, struct DosPacket *dp);
enum clnt_stat nfs_call();

/* examine.c */
void nfs_examine(struct mount_pt *mt, struct DosPacket *dp);
void nfs_examine_next(struct mount_pt *mt, register struct DosPacket *dp);
void nfs_setdate(struct mount_pt *mt, register struct DosPacket *dp);

/* fhcache.c */
void nfs_open (struct mount_pt *mt, register struct DosPacket *dp);
void nfs_close (struct mount_pt *mt, register struct DosPacket *dp);
void nfs_rd (struct mount_pt *mt, register struct DosPacket *dp);
void nfs_wr (struct mount_pt *mt, register struct DosPacket *dp);
void nfs_seek (struct mount_pt *mt, register struct DosPacket *dp);
void nfs_dkinfo (register struct mount_pt *mt, register struct DosPacket *dp);
void nfs_setsize(struct mount_pt *mt, register struct DosPacket *dp);

/* mount.c */
void nfs_command(struct nfs_req *req);
int free_mt(register struct mount_pt *mt, struct nfs_umount_req *req);

/* perm.c */
int perm_amiga_to_unix(int);
int perm_unix_to_amiga(int,int,int);

/* ptofh.c */
int ptofh(struct mount_pt *mt, register struct DosPacket *dp, struct fdata
	*fd, u_char *path, register struct fdata *nfd, char *name);
int fh_status(enum clnt_stat rpc_error, int nfs_error);

/* requester.c */
BOOL requester(UBYTE *positive, UBYTE *negative, char *fmt, ...);

/* utils.c */
void bst_to_comm (char *bst, char *str);
struct MsgPort *CreateMyPort (void);
void DeleteMyPort (struct MsgPort *port);
// void fprintf();

/* volume.c */
struct mount_pt *voltomt(char *volume);
void deletevol(struct DosList *dl);
int validvol(char *volname);
struct mount_pt *insert_vname(char *volname);
void  nfs_rename_dsk (struct mount_pt *mt, register struct DosPacket *dp);

/* xdr.c */
bool_t xdr_my_entry_list(XDR *xdrs, struct entry **objp);
bool_t xdr_my_dirlist(XDR *xdrs, dirlist *objp);
bool_t xdr_my_readdirres(XDR *xdrs, readdirres *objp);

#endif /* FS_H */
