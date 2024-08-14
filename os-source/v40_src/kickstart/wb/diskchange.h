/*
 * $Id: diskchange.h,v 38.1 91/06/24 11:34:45 mks Exp $
 *
 * $Log:	diskchange.h,v $
 * Revision 38.1  91/06/24  11:34:45  mks
 * Initial V38 tree checkin - Log file stripped
 * 
 */

#define ACTION_RENAME_DISK	9

/* this structure MUST be alligned on a longword.  In addition,
 * it depends on a Message structure being 14 bytes long.
 */

/* magic word offsets for the packet data */
#define	PKT_LINK	0	/* pointer to message */
#define PKT_ID		1	/* msg port to send reply to */
#define PKT_TYPE	2	/* the request number */
#define PKT_RES1	3	/* the primary result */
#define PKT_RES2	4	/* the secondary result */
#define PKT_ARG1	5	/* the first argument */


/* internal volume states */
#define	VOL_GONE	0	/* dos says it has gone away */
#define	VOL_NOTPRESENT	1	/* has locks, but not in drive */
#define	VOL_NEW		2	/* we have no knowledge of this one */
#define VOL_PRESENT	3	/* this one is in the drive */
#define VOL_DEVICE	4	/* this is actually a device, not a volume */
#define VOL_IGNORE	5	/* ignore, cannot get a lock on it */
#define VOL_RELABEL	6	/* volume has been relabeled via CLI cmd */
