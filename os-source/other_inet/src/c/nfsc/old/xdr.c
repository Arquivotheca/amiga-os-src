/* -----------------------------------------------------------------------
 * XDR.c   NFSC  SAS
 *
 * $Locker:  $
 *
 * $Id: xdr.c,v 1.3 91/08/06 16:10:07 martin Exp $
 *
 * $Revision: 1.3 $
 *
 * $Header: inet2:src/c/nfsc/RCS/xdr.c,v 1.3 91/08/06 16:10:07 martin Exp $
 *
 *------------------------------------------------------------------------
 */



/*
 * Directory handling XDR.
 */

#include "fs.h"

/*
 * Build list of dir entries.  This proc can only decode/free - it is
 * never called to encode
 */
bool_t
xdr_my_entry_list(xdrs, objp)
	XDR *xdrs;
	struct entry **objp;
{
	register struct entry *next, *n;
	unsigned int len, fileid;
	bool_t more;

	/*
	 * Handle freeing explicitly -
	 */
	if(xdrs->x_op == XDR_FREE){
		for(next = *objp; next != NULL; next = n){
			n = next->nextentry;
			free(next);
		}
		return TRUE;
	}

	/*
	 * decode list -
	 */
	for(;;){
		if(!xdr_bool(xdrs, &more)){
			return FALSE;
		}
		if(!more){
			return TRUE;
		}

		/*
		 * unrolled xdr to get entry record from stream.
		 */
		if(!xdr_u_int(xdrs, &fileid)) {
			return FALSE;
		}
		if(!xdr_u_int(xdrs, &len)) {
			return FALSE;
		}
		if(len > NFS_MAXNAMLEN){
			return FALSE;
		}

		/*
		 * Now that we have filename len, allocate enough storage to
		 * hold entry record and the returned filename
		 */
		n = (struct entry *)malloc(sizeof(*n) + len + 1);
		if(n == NULL){
			return FALSE;
		}
		n->fileid = fileid;
		n->name = (filename)(n + 1);
		n->nextentry = NULL;

		if(!xdr_opaque(xdrs, n->name, len)) {
			free(n);
			return FALSE;
		}

		n->name[len] = '\0';
		if(!xdr_opaque(xdrs, n->cookie.data, NFS_COOKIESIZE)) {
			free(n);
			return FALSE;
		}

		/*
		 * Entry is now completely formatted, put it on linked list
		 */
		*objp = n;
		objp = &(*objp)->nextentry;
	}

	return TRUE;
}

bool_t
xdr_my_dirlist(xdrs, objp)
	XDR	*xdrs;
	dirlist *objp;
{

	if (!xdr_my_entry_list(xdrs, &objp->entries)){
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

