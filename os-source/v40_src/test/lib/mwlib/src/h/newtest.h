/******************************************************************************
    NewTest.h
    
    Fred Mitchell               Version 1.12                 8-02-89
    
    (c) 1989 Commodore, INC. All Rights Reserved.
******************************************************************************/

#ifndef _NEW_TEST_
#define _NEW_TEST_  1

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>

#define NTPORT  "NewDaemon.Port"
#define NTREPLY "NewDaemon.Reply"
#define REXXPORT "REXX"

/* #define NTREXX  "NewDaemonArexx.Port" we don't need this! */

typedef enum NTCommand  {
    ntc_noop,               /* no operation */
    ntc_alloc,              /* allocate */
    
    ntc_del_club,           /* delete club    */
    ntc_del_all,            /* delete everything    */
    
    ntc_get_club,           /* get memory block */
    ntc_get_club_node,      /* get node */
    
    ntc_suicide,            /* remove demon from system */
    ntc_inquire,            /* return pointer to head of club list */
    
    ntc_make_ret_block,     /* make a return block  */
    ntc_send_ret_block,     /* send this block to */
    ntc_kill_ret_block,     /* kill (delete) it */
    
    ntc_unique_id,          /* return a unique id number */

    ntc_alloc_idcmp_port,   /* allocate an IDCMP port */
    ntc_get_idcmp_port,     /* get an IDCMP port */
    ntc_del_idcmp_port,     /* delete specific port */
    ntc_del_all_idcmp_ports,/* delete all IDCMP ports */
    } NTCommand;

typedef enum NTType {
    ntt_general,            /* unknown or general type */
    ntt_test_buffer,        /* created by the TT program to test demon */
    ntt_screen,             /* screen pointer */
    ntt_window,             /* window pointer */
    ntt_image,              /* ImagePacket */
    ntt_rastport,           /* rastport pointer */
    ntt_ret_block,          /* special Return Block */
    ntt_gadget,             /* gadget */
    
    /* John's Requests
    */
    ntt_list,
    ntt_node,
    ntt_minlist,
    ntt_minnode
    } NTType;

typedef enum NTRetType  {       /* Return Block Types */
    ntr_integer,            /* integer return */
    ntr_intuimessage,       /* IntuiMessage received */
    ntr_string,             /* 
    ntr_arexx               /* ARexx-style  */
    } NTRetType;

typedef struct NTMessage {      /* Message Packet */  
    struct Message m;       
    NTCommand command;      /* command to be processed */
    char *name;             /* club name */
    ULONG size;             /* size of allocated area */
    NTType type;            /* type of structure - what this will be used for */
    ULONG flags;            /* memory flags */
    UBYTE *block;           /* allocated member */
    
    ULONG id;               /* Unique ID associated with this */
    } NTMessage;

typedef struct NTClub   {       /* The Club */
    struct NTClub *next;    /* next club */
    char *name;             /* name of club */

    ULONG size;             /* size of memory block */
    NTType type;            /* type of memory this is */
    ULONG flags;
    char *block;            /* pointer to memory block */
    } NTClub;

/* NewDaemon will construct a tagged Return Block name using the address
    of the requesting task. These blocks will be created on demand by
    NewDaemon for returns end-route to ARexx. All memory related to
    the Return Block must be allocated via AllocRemember(), using the
    supplied key. WARNING: if NewDaemon commits suicide, the memory will
    be yanked away, possibly causing a crash.  */

typedef struct NTRet    {       /* The Return Block */
    struct NTRet *next, *prev;

    ULONG id;               /* unique process script ID (never zero) */
    NTRetType type;         /* Type of return message   */
    ULONG count;            /* How many returns 0-15    */
    ULONG r[16];            /* Either pointers or return values */
    struct Remember *key;   /* Memory control for ret   */
    } NTRet;

typedef struct NTIDCMP  {
    struct NTIDCMP *next, *prev;
    ULONG class, code, qualifier,
          mousex, mousey;
    APTR iaddress;
    } NTIDCMP;

typedef struct NTPort   {       /* The Port Block   */
    struct NTPort *next, *prev;
    ULONG id;   /* Process ID to which this port belongs */
    struct MsgPort *port;
    
    NTIDCMP *beg, *end;     /* list of messages received on this port */
    } NTPort;
    
#endif
