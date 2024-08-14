        IFND ENVOY_ERRORS_I
ENVOY_ERRORS_I      SET     1

**
**      $Id: errors.i,v 1.13 92/05/29 10:13:49 dlarson Exp Locker:
**
**      Error codes for all Envoy libraries and etc.
**
**      (c) Copyright 1992 Commodore-Amiga, Inc.
**          All Rights Reserved
**

ENVOYERR_NOERROR         SET     0
**
**  General to all libraries (500-519)
**
ENVOYERR_NORESOURCES     SET     501     ** can't allocate something
ENVOYERR_CMDUNKNOWN      SET     502     ** trans_Command unknown to server program
ENVOYERR_NOTINPLACE      SET     503     ** Server can't do this command in-place
ENVOYERR_SMALLRESPBUFF   SET     504     ** insufficient response buffer size
ENVOYERR_NULLPTR         SET     505     ** illegal NULL-pointer argument
ENVOYERR_ILLEGALINPLACE  SET     507     ** requested operation cannot be
**                                       ** performed in-place by server
**
**  error codes for errors originating with nipc.library (520-539)
**
** FindEntity Errors
ENVOYERR_UNKNOWNHOST     SET     521   ** The requested host is unknown
ENVOYERR_UNKNOWNENTITY   SET     522   ** The requested Entity is unknown
ENVOYERR_NORESOLVER      SET     523   ** Couldn't contact the mechanism for
**                                        finding a remote Entity
** Transaction Errors
ENVOYERR_ABORTED         SET     530   ** This transaction has been
**                                        AbortTransaction()'d.
ENVOYERR_CANTDELIVER     SET     531   ** The library has given up
**                                        delivering this transaction
ENVOYERR_ABOUTTOWAIT     SET     532   ** The library would have to block on
**                                        delivering this Transaction (PLANNED)
ENVOYERR_REFUSED         SET     533   ** The server's library refused this
**                                        Transaction (because of resource limitations)
**                                        [MIGHT be implemented]
ENVOYERR_TIMEOUT         SET     534   ** This transaction has timed out

**
**  error codes for errors originating with services.library (560-580)
**

**
**  Application specific error codes.  These may only be passed between clients
**  and servers.  Two different applications may re-use these, so no assignments
**  are made. Define application specific errors as (ENVOYERR_APP + X) and use
**  100 error codes maximum (i.e., 1000-1099).
**
ENVOYERR_APP             SET     1000

        ENDC

