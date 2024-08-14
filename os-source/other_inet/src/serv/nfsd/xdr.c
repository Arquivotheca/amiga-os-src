/*
 * Directory handling XDR.  Original was recursive with call depth a
 * function of directory size.
 */

#include <rpc/rpc.h>
#include <nfs/nfs.h>

bool_t
xdr_my_entry(xdrs, objp)
	XDR *xdrs;
	entry *objp;
{
	if (!xdr_u_int(xdrs, &objp->fileid)) {
		return (FALSE);
	}
	if (!xdr_filename(xdrs, &objp->name)) {
		return (FALSE);
	}
	if (!xdr_nfscookie(xdrs, &objp->cookie)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_my_entry_list(xdrs, objp)
	XDR *xdrs;
	struct entry **objp;
{
	bool_t more_data, freeing;
	struct entry **next;

	freeing = (xdrs->x_op == XDR_FREE);
	
	while(TRUE){
		more_data = (*objp == (struct entry *)NULL);
		if(!xdr_bool(xdrs, &more_data)){
			return FALSE;
		}
		if(!more_data){
			return TRUE;
		}

		if(freeing){
			next = &((*objp)->nextentry);
		}

		if(!xdr_reference(xdrs, objp, sizeof(struct entry), xdr_my_entry)){
			return FALSE;
		}

		objp = (freeing) ? next : &(*objp)->nextentry;
	}

	return (TRUE);
}

bool_t
xdr_my_dirlist(xdrs, objp)
	XDR	*xdrs;
	dirlist *objp;
{
	if (!xdr_my_entry_list(xdrs, (char **)&objp->entries)){
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &objp->eof)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_my_readdirres(xdrs, objp)
	XDR *xdrs;
	readdirres *objp;
{
	if (!xdr_nfsstat(xdrs, &objp->status)) {
		return (FALSE);
	}

	switch (objp->status) {
	case NFS_OK:
		if (!xdr_my_dirlist(xdrs, &objp->readdirres_u.reply)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}

