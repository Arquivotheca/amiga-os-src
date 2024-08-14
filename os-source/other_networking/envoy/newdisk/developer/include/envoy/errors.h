#ifndef ENVOY_ERRORS_H
#define ENVOY_ERRORS_H
/*
**      $Id: errors.h,v 1.13 92/05/29 10:13:49 dlarson Exp Locker: dlarson $
**
**      Error codes for all Envoy libraries and etc.
**
**      (c) Copyright 1992 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
#define ENVOYERR_NOERROR 0
/*
**  General to all libraries (500-519)
*/
#define ENVOYERR_NORESOURCES     501UL    /* can't allocate something */
#define ENVOYERR_CMDUNKNOWN      502UL    /* trans_Command unknown to server program  */
#define ENVOYERR_NOTINPLACE      503UL    /* Server can't do this command in-place */
#define ENVOYERR_SMALLRESPBUFF   504UL    /* insufficient response buffer size */
#define ENVOYERR_NULLPTR         505UL    /* illegal NULL-pointer argument */
#define ENVOYERR_ILLEGALINPLACE	 507UL    /* requested operation cannot be */
			/* performed in-place by server */
/*
**  error codes for errors originating with nipc.library (520-539)
*/
/* FindEntity Errors */
#define ENVOYERR_UNKNOWNHOST     521UL  /* The requested host is unknown */
#define ENVOYERR_UNKNOWNENTITY   522UL  /* The requested Entity is unknown */
#define ENVOYERR_NORESOLVER      523UL  /* Couldn't contact the mechanism for
                                          finding a remote Entity */
/* Transaction Errors */
#define ENVOYERR_ABORTED         530UL  /* This transaction has been
                                          AbortTransaction()'d. */
#define ENVOYERR_CANTDELIVER     531UL  /* The library has given up on
                                          delivering this transaction */
#define ENVOYERR_ABOUTTOWAIT     532UL  /* The library would have to block on
                                          delivering this Transaction (PLANNED) */
#define ENVOYERR_REFUSED         533UL  /* The server's library refused this
                                          Transaction (because of resource limitations)
                                          [MIGHT be implemented] */
#define ENVOYERR_TIMEOUT         534UL  /* This transaction has timed out */

/*
**  error codes for errors originating with accounts.library (540-559)
*/
#define ENVOYERR_UNKNOWNUSER	 541UL  /* The specified user does not exist */
#define ENVOYERR_UNKNOWNGROUP	 542UL  /* The specified group does not exist */
#define ENVOYERR_UNKNOWNMEMBER	 546UL  /* The specified user is not a member of the
					   specified group. */
#define ENVOYERR_LASTUSER	 543UL  /* No more users in the database. (Note: this
					   really isn't an error.) */
#define ENVOYERR_LASTGROUP	 544UL  /* No more groups */
#define ENVOYERR_LASTMEMBER	 545UL  /* No more users in the specified group. */

/*
**  error codes for errors originating with services.library (560-580)
*/

#define ENVOYERR_UNKNOWNSERVICE	 560UL	/* The request services doesn't exists on
					   the remote host. */
#define ENVOYERR_OPENSERVICEFAIL 561UL  /* The Services Manager couldn't open					   
					   the requested service. */
#define ENVOYERR_BADSTARTSERVICE 562UL  /* General problem during service startup. */					   

/* Note: Custom services are free to define their own error codes if there is a
** need for more specific information.
*/

/*
**  Application specific error codes.  These may only be passed between clients
**  and servers.  Two different applications may re-use these, so no assignments
**  are made. Define application specific errors as (ENVOYERR_APP + X) and use
**  100 error codes maximum (i.e., 1000-1099).
*/
#define ENVOYERR_APP	1000UL

#endif /* ENVOY_ERRORS_H */

