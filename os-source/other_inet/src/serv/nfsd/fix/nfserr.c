/*
 * Map Amiga error number to NFSERR
 */

#include <libraries/dos.h>

#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <string.h>

#include "nfsd.h"

/*
 * map table between NFS error conditions and error strings
 */
static struct errtbl {
	short	nfserrcode;
	char 	*errmsg;
} et[] = {
	{NFS_OK,	"operation completed ok"},
	{NFSERR_PERM,	"not owner, permission denied"},
	{NFSERR_NOENT,	"no such file or directory"},
	{NFSERR_IO,	"input/output error"},
	{NFSERR_NXIO,	"no such device"},
	{NFSERR_ACCES,	"permission denied"},
	{NFSERR_EXIST,	"file or directory already exists"},
	{NFSERR_NODEV,	"no such device"},
	{NFSERR_NOTDIR,	"not a directory"},
	{NFSERR_ISDIR,	"is a directory"},
	{NFSERR_FBIG,	"file too big"},
	{NFSERR_NOSPC,	"no space left on device"},
	{NFSERR_ROFS,	"read only filesystem"},
	{NFSERR_NAMETOOLONG, "name is too long"},
	{NFSERR_NOTEMPTY,"directory not empty"},
	{NFSERR_DQUOT,	"disk quoat exceeded"},
	{NFSERR_STALE,	"invalid nfs file or directory reference"},
	{NFSERR_WFLUSH,	"write flushed to disk"}
};
#define NUMERR (sizeof(et)/sizeof(et[0]))

/*
 * translate NFS error code into ascii string
 */

int nfserrtxt(struct RexxMsg *rm)
{
	register struct errtbl *etp;
	register int error;
	char *ns, buf[32];

	error = atoi(rm->rm_Args[1]);
	sprintf(buf, "error %ld is unknown", error);
	ns = buf;

	for(etp = et; etp < (et + NUMERR); etp++){
		if(etp->nfserrcode == error){
			ns = etp->errmsg;
			break;
		}
	}

	if(rm->rm_Action & RXFF_RESULT){
		rm->rm_Result2 = (long)CreateArgstring(ns, strlen(ns));
	}

	return NFS_OK;
}

/*
 * Map an AmigaDOS error code from IoErr() into an equivalant NFS error code
 */
nfsstat io_to_nfserr()
{
	int error;

	error = IoErr();

	switch(error){
	case ERROR_DIR_NOT_FOUND:
	case ERROR_OBJECT_NOT_FOUND:
		return NFSERR_NOENT;

	case ERROR_OBJECT_EXISTS:
		return NFSERR_EXIST;

	case ERROR_SEEK_ERROR:
	case ERROR_INVALID_LOCK:
	case ERROR_OBJECT_IN_USE:
		return NFSERR_IO;

	case ERROR_DELETE_PROTECTED:
	case ERROR_WRITE_PROTECTED:
	case ERROR_READ_PROTECTED:
		return NFSERR_PERM;

	case ERROR_DISK_WRITE_PROTECTED:
		return NFSERR_ROFS;

	case ERROR_DISK_FULL:
		return NFSERR_NOSPC;

	case ERROR_INVALID_COMPONENT_NAME:
		return NFSERR_NAMETOOLONG;

	case ERROR_OBJECT_TOO_LARGE:
	case ERROR_NO_FREE_STORE:
		return NFSERR_FBIG;

	case ERROR_DIRECTORY_NOT_EMPTY:
		return NFSERR_NOTEMPTY;
	}

	return NFSERR_IO;
}

/*
 * set the NFSderror variable to reflect error condition.  If rexx is asking
 * for a result string, return "-1", else just set Result1 to code.
 */
void
seterror(rm, code)
	struct RexxMsg *rm;
	nfsstat code;
{
	if(rm->rm_Action & RXFF_RESULT){
		rm->rm_Result1 = 0;
		rm->rm_Result2 = CVi2arg(-1, 8);
	} else {
		rm->rm_Result1 = (int)code;
		rm->rm_Result2 = 0;
	}

	if(CheckRexxMsg(rm)){
		char buf[32];

		CVi2a(buf, code, 8);
		SetRexxVar(rm, "NFSDERROR", buf, strlen(buf));
	}
}
