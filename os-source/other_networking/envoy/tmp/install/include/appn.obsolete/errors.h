#ifndef APPN_ERRORS_H
#define APPN_ERRORS_H
/*
**      $Id: errors.h,v 1.9 92/03/24 17:48:50 gregm Exp Locker: dlarson $
**
**      Error codes for all APPN libraries and etc.
**
**      (c) Copyright 1992 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
#define APPNERR_NOERROR 0
/*
**  General to all libraries or for use by applications (1-100)
*/
#define APPNERR_NORESOURCES     1UL    /* can't allocate something */
#define APPNERR_CMDUNKNOWN      2UL    /* trans_Command unknown to server program  */
#define APPNERR_NOTINPLACE      3UL    /* Server can't do this command in-place */
#define APPNERR_SMALLREPLBUFFER 4UL    /* trans_Reply buffer is too small for
                                          the server */
#define APPNERR_NULLPTR         5UL    /* illegal NULL-pointer argument */
/*
**  error codes for errors originating with nipc.library (101-200)
*/
/* FindEntity Errors */
#define APPNERR_UNKNOWNHOST     101UL  /* The requested host is unknown */
#define APPNERR_UNKNOWNENTITY   102UL  /* The requested Entity is unknown */
#define APPNERR_NORESOLVER      103UL  /* Couldn't contact the mechanism for
                                          finding a remote Entity */
/* Transaction Errors */
#define APPNERR_ABORTED         110UL  /* This transaction has been
                                          AbortTransaction()'d. */
#define APPNERR_CANTDELIVER     111UL  /* The library has given up on
                                          delivering this transaction */
#define APPNERR_ABOUTTOWAIT     112UL  /* The library would have to block on
                                          delivering this Transaction (PLANNED) */
#define APPNERR_REFUSED         113UL  /* The server's library refused this
                                          Transaction (because of resource limitations)
                                          [MIGHT be implemented] */
#define APPNERR_TIMEOUT         114UL  /* This transaction has timed out */

/*
**  error codes for errors originating with authentication.library (201-300)
*/
#define APPNERR_NOAUTHSERV      201UL  /* SelectServer() not called */
#define APPNERR_NOAUTHORITY     202UL  /* Authority is not sufficient for
                                          requested action */
#define APPNERR_NOOBJECT        203UL  /* Object not found in authentication
                                          database */
#define APPNERR_OBJEXISTS       204UL  /* Attempt to add object with same name as
                                          an existing object */
#define APPNERR_IOERR           205UL  /* Server can't read/write disk */
/*
**  error codes for errors originating with services.library (301-400)
*/

#endif /* APPN_ERRORS_H */

