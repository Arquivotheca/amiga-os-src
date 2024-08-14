/*
 * EGConvert error status
 */
#ifndef        __EGCDEF__
#define        __EGCDEF__
#include    <stdio.h>

#define ERR_MALLOC      ( -1 )         /* Memory allocation */
#define ERR_MAXUSER     ( -2 )         /* Over max user */
#define ERR_PERMISSION  ( -3 )         /* Permission denined */
#define ERR_IPC         ( -4 )         /* IPC error */
#define ERR_IPCSIZE     ( -5 )         /* IPC data size error */
#define ERR_SELDICT     ( -6 )         /* Select dictionary error */
#define ERR_EXIT        ( -7 )         /* EGConvert exit */
#define ERR_FREAD       ( -8 )         /* File read error */
#define ERR_FSEEK       ( -9 )         /* File seek error */
#define ERR_FTYPE       ( -10 )        /* User dict. file type error */
#define ERR_FWRITE      ( -11 )        /* File write error */
#define ERR_CANTOPEN    ( -12 )        /* File open error */
#define ERR_CONNECT     ( -13 )        /* User not connect */
#define ERR_SEND        ( -14 )        /* IPC data send error */
#define ERR_RECV        ( -15 )        /* IPC data recieve error */
#define ERR_RECVSIZE    ( -16 )        /* IPC data recieve size error */
#define ERR_TIMEOUT     ( -17 )        /* Request time out */
#define ERR_NOTSUPPORT  ( -18 )        /* Request cmd is not supported */
#define ERR_REQCMD      ( -19 )        /* Illegal request command */
#define ERR_USRNOTFOUND ( -20 )        /* User not found */
#define ERR_FNOTFOUND   ( -21 )        /* File not found */
#define ERR_FLOCKED     ( -22 )        /* File is locked */
#define ERR_FTYPENFS    ( -23 )        /* File system NFS is not support */
#define ERR_FUPDATE     ( -24 )        /* File was update by other user */
#define ERR_FNAMELENGTH ( -25 )        /* Name length is over than MAXLEN */
#define ERR_HOST        ( -26 )        /* Remote host not support */
#define ERR_UNKOWNSERV  ( -27 )        /* Unkown service */
#define ERR_UNKOWNHOST  ( -28 )        /* Unkown host */
#define ERR_PROTO_VER   ( -29 )        /* mismatch protocol version */
#define ERR_MISDICVER   ( -30 )        /* mismatch dict version */

/* for old version */
#define EGC_MPATH    "egdicm.dic"      /* Main dictionary name */
#define EGC_UPATH    "egdicu.dic"      /* Default user dictionary name */
#define ENV_MAINDIC  "EGCONVERT_DICM"  /* Enviroment of main dict name */
#define ENV_USERDIC  "EGCONVERT_DICU"  /* Enviroment of user dict name */
#define MAINDICT     10                /* Main dictionary */
#define USERDICT     20                /* User dictionary */

#ifdef UNIX
typedef FILE   *EGCFILE;
#define DELIMITER     "/"
#define OPENMODE      "r+"             /* Dictionary open mode */
#define CREATMODE     "w+"             /* Dictionary create mode */
#define READMODE      "r+"             /* Dictionary read write mode */
#define MAINDIR       "/usr/local/lib/ergo/dic"
#define USERDIR       "/usr/local/lib/ergo/dic/usr"
#define EXPTDIR       "/usr/local/lib/ergo/dic/exp"
#endif

#ifdef DOS_EGC

typedef WORD   EGCFILE;
#define DELIMITER     "\\"
#define OPENMODE      0                /* Dictionary open mode */
#define CREATMODE     1                /* Dictionary create mode */
#define READMODE      2                /* Dictionary read write mode */
#define MAINDIR       "\\"
#define USERDIR       "\\"
#define EXPTDIR       "\\"
#endif

#ifdef MAC
#include "StdIO.h"
typedef FILE   *EGCFILE;
#define OPENMODE      "r+b"            /* Dictionary open mode */
#define CREATMODE     "w+b"            /* Dictionary create mode */
#endif

#if (defined(DOS_EGC) && defined(DOS_EGBRIDGE))
#include    <assert.h>
#define DBGLINE {\
                dbgsnap(22, 1, "FILE", __FILE__, 0x10);\
                dbgmsg(20, 1, "LINE ", __LINE__);\
                }
#define PASCAL  _pascal
#endif

#endif    /* __EGCDEF__ */
