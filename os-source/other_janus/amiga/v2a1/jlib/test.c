
/* *** test.c *****************************************************************
 *
 * test.c -- main routine for testing janus.library
 *
 * Copyright (C) 1986, 1987, 1988, Commodore Amiga Inc.
 * All rights reserved
 * 
 * Date        Name               Description
 * ----------  -----------------  ---------------------------------------------
 * Feb 1988    -RJ Mical-         Hacked in new services and memory test code 
 * Early 1986  Katin/Burns clone  Created this file
 *
 * ************************************************************************* */



#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/ports.h"
#include "exec/memory.h"
#include "exec/interrupts.h"

#include "hardware/custom.h"
#include "hardware/intbits.h"

#include "janus/janus.h"
#include "janus/janusvar.h"
#include "janus/janusreg.h"
#include "janus/memrw.h"

#define SERVICE_FUNCS
#include "janus/services.h"
#include "serviceint.h"



#define NOT   !



extern struct Custom custom;


UBYTE *GetJanusStart();


struct JanusAmiga *JanusBase;
struct Interrupt keyInt;



struct memarray {
    UWORD size;
    UWORD type;
    ULONG address;
};
struct memarray testarray[] = {
    { 10, MEMF_PARAMETER, 0 },
    { 100, MEMF_PARAMETER, 0  },
    { 10, MEMF_PARAMETER | MEM_BYTEACCESS, 0 },
    { 10, MEMF_PARAMETER | MEM_WORDACCESS, 0 },
    { 10, MEMF_PARAMETER | MEM_GRAPHICACCESS, 0 },
    { 10, MEMF_PARAMETER | MEM_IOACCESS, 0 },
    { 10, MEMF_BUFFER    | MEM_BYTEACCESS, 0 },
    { 10, MEMF_BUFFER    | MEM_WORDACCESS, 0 },
    { 10, MEMF_BUFFER    | MEM_GRAPHICACCESS, 0 },
    { 0, 0, 0 }
};



struct JanusRemember *GlobalJKey = NULL;


VOID PrintRemember(key)
struct JanusRemember *key;
{
   printf("NextRemember=$%04lx ", key->NextRemember);
   printf("Offset=$%04lx ", key->Offset);
   printf("Size=$%lx ", key->Size);
   printf("Type=$%lx\n", key->Type);
}



VOID PrintRemembers(key)
struct JanusRemember *key;
{
   RPTR offset;

   while (key)
      {
      PrintRemember(key);
      offset = key->NextRemember;
      if (offset != 0xFFFF)
         key = (struct JanusRemember *)JanusOffsetToMem(offset, 
               MEMF_PARAMETER | MEM_WORDACCESS);
      else key = NULL;
      }
}



VOID exampleAllocJ(globalkey)
struct JanusRemember **globalkey;
{
    BOOL success;
    struct JanusRemember *localkey;

    success = FALSE;
    localkey = NULL;

    if (AllocJRemember(&localkey, 10000, MEMF_BUFFER | MEM_WORDACCESS))
      if (AllocJRemember(&localkey, 100, MEMF_BUFFER | MEM_WORDACCESS))
        if (AllocJRemember(&localkey, 1, MEMF_BUFFER | MEM_WORDACCESS))
          success = TRUE;

    if (success) AttachJRemember(globalkey, &localkey);
    else FreeJRemember(&localkey, TRUE);
    PrintRemembers(*globalkey);
}



VOID testattach()
{
    exampleAllocJ(&GlobalJKey);
    exampleAllocJ(&GlobalJKey);
    exampleAllocJ(&GlobalJKey);
    FreeJRemember(&GlobalJKey, TRUE);
}



VOID testremember()
{
   struct JanusRemember *key;

   key = NULL;
   printf("key=$%lx  AllocJRemember($%lx %ld $%lx)=%ld\n",
      key, &key, 100, MEMF_BUFFER | MEM_WORDACCESS,
      AllocJRemember(&key, 100, MEMF_BUFFER | MEM_WORDACCESS));
   printf("key=$%lx  AllocJRemember($%lx %ld $%lx)=%ld\n",
      key, &key, 1000, MEMF_BUFFER | MEM_WORDACCESS,
      AllocJRemember(&key, 1000, MEMF_BUFFER | MEM_WORDACCESS));
   printf("key=$%lx  AllocJRemember($%lx %ld $%lx)=%ld\n",
      key, &key, 10000, MEMF_BUFFER | MEM_WORDACCESS,
      AllocJRemember(&key, 10000, MEMF_BUFFER | MEM_WORDACCESS));
   printf("key=$%lx  AllocJRemember($%lx %ld $%lx)=%ld\n",
      key, &key, 64000, MEMF_BUFFER | MEM_WORDACCESS,
      AllocJRemember(&key, 64000, MEMF_BUFFER | MEM_WORDACCESS));
   FreeJRemember(&key, TRUE);
   printf("key=$%lx\n", key);
}



VOID PrintCustomer(customer)
struct   ServiceCustomer *customer;
{
   kprintf("    customer=$%lx ", customer);
   kprintf("Next=$%lx ", customer->NextCustomer);
   kprintf("Flags=$%lx ", customer->Flags);
   kprintf("Task=$%lx ", customer->Task);
   kprintf("SigBit=$%lx\n", customer->SignalBit);
}



VOID PrintServiceData(data)
struct ServiceData *data;
{
   struct ServiceData *bdata;
   struct ServiceCustomer *customer;

   Forbid();

   data = (struct ServiceData *)MakeWordPtr(data);
   bdata = (struct ServiceData *)MakeBytePtr(data);

   kprintf("ServiceData=$%lx\n  ", data);
   kprintf("ApplicationID=$%lx ", data->ApplicationID);
   kprintf("LocalID=$%lx ", data->LocalID);
   kprintf("Flags=$%lx ", data->Flags);
#ifdef   CHANNELING
   kprintf("ChannelNumber=%ld\n  ", bdata->ChannelNumber);
#endif   /* of ifdef CHANNELING */
   kprintf("UserCount=%ld ", bdata->UserCount);
   kprintf("MemSize=%ld ", data->MemSize);
   kprintf("MemType=$%lx ", data->MemType);
   kprintf("AmigaMemPtr=$%lx ", data->AmigaMemPtr);
   kprintf("PCMemPtr=$%lx\n  ", data->PCMemPtr);
   kprintf("NextServiceData=$%lx ", data->NextServiceData);
   kprintf("FirstAmigaCustomer=$%lx\n", data->FirstAmigaCustomer);

   customer = (struct ServiceCustomer *)data->FirstAmigaCustomer;
   while (customer)
      {
      PrintCustomer(customer);
      customer = customer->NextCustomer;
      }

   Permit();
}


PrintError(error)
SHORT error;
{
   switch (error)
      {
      case JSERV_OK:
         printf("Error=JSERV_OK\n");
         break;
      case JSERV_NOJANUSMEM:
         printf("Error=JSERV_NOJANUSMEM\n");
         break;
      case JSERV_NOAMIGAMEM:
         printf("Error=JSERV_NOAMIGAMEM\n");
         break;
      case JSERV_NOPCMEM:
         printf("Error=JSERV_NOPCMEM\n");
         break;
      case JSERV_NOSERVICE:
         printf("Error=JSERV_NOSERVICE\n");
         break;
      case JSERV_DUPSERVICE:
         printf("Error=JSERV_DUPSERVICE\n");
         break;
      }
}



LONG   AppID = 101;
SHORT   LocalID = 2;


VOID testservices(arg)
SHORT arg;
{
   struct ServiceData *data1, *data2, *data3;
   struct ServiceData *bdata1, *bdata2, *bdata3;
   SHORT i, error;
   ULONG signal;
   UBYTE *ptr;

   switch (arg)
      {
      case 1:
         error = GetService(&data1, AppID, LocalID, 
               AllocSignal(-1), GETS_WAIT);
         if (NOT error) PrintServiceData(data1);
         else PrintError(error);
         break;
      case 2:
         error = AddService(&data3, AppID, LocalID, 
               100, MEMF_BUFFER, 
               AllocSignal(-1), ADDS_LOCKDATA);
         if (NOT error) PrintServiceData(data3);
         else PrintError(error);
         break;
      case 3:
         error = GetService(&data1, AppID, LocalID, 
               AllocSignal(-1), GETS_WAIT);
         if (NOT error) 
            {
            PrintServiceData(data1);
            bdata1 = (struct ServiceData *)MakeBytePtr(
                  data1);
            Forbid();
            kprintf("%lx: about to CallService()\n", data1);
#ifdef   CHANNELING
            CallService(bdata1->ChannelNumber);
#endif   /* of ifdef CHANNELING */
#ifndef   CHANNELING
            CallService(bdata1);
#endif   /* of ifndef CHANNELING */
            Permit();
            Forbid();
            kprintf("%lx: about to RelService()\n", data1);
            ReleaseService(data1);
            Permit();
            }
         else PrintError(error);
         break;
      case 4:
         error = GetService(&data2, AppID, LocalID, 
               AllocSignal(-1), GETS_WAIT);
         if (NOT error) 
            {
            bdata2 = (struct ServiceData *)MakeBytePtr(data2);
            i = data2->UserCount;
            PrintServiceData(data2);
            Forbid();
            kprintf("%lx: about to RelService()\n", data2);
            ReleaseService(data2);
            if (i > 1) PrintServiceData(data2);
            Permit();
            }
         else PrintError(error);
         break;
      case 5:
         error = GetService(&data1, AppID, LocalID, 
               AllocSignal(-1), NULL);
         if (NOT error) 
            {
            Forbid();
            PrintServiceData(data1);
            bdata1 = (struct ServiceData *)MakeBytePtr(data1);
            kprintf("%lx: about to RelService()\n", bdata1);
            ReleaseService(bdata1);
            Permit();
            }
         else PrintError(error);
         break;
      case 6:
         signal = AllocSignal(-1);
         if (signal != -1)
            {
            error = GetService(&data1, AppID, LocalID, 
                  signal, GETS_WAIT);
            if (NOT error) 
               {
               Forbid();
               PrintServiceData(data1);
               bdata1 = (struct ServiceData *)MakeBytePtr(data1);
               Wait(1 << signal);
               kprintf("I GOT A SIGNAL!  YEAH!\n");
               kprintf("%lx:  about to RelServ()\n",
                  data1);
               ReleaseService(data1);
               Permit();
               }
            else PrintError(error);
            }
         break;
      case 7:
         signal = AllocSignal(-1);
         if (signal != -1)
            {
            error = GetService(&data1, AppID, LocalID, 
                  signal, GETS_WAIT);
            if (NOT error) 
               {
               PrintServiceData(data1);
               bdata1 = (struct ServiceData *)MakeBytePtr(data1);
               for (;;)
                  {
                  Wait(1 << signal);
                  kprintf("FOREVER GOT A SIGNAL!\n");
                  }
               }
            else PrintError(error);
            }
         break;
      case 8:
         error = AddService(&data3, AppID, LocalID, 100, 
               MEMF_BUFFER, 
               AllocSignal(-1), ADDS_LOCKDATA);
         if (NOT error) 
            {
            Forbid();
            PrintServiceData(data3);
            kprintf("%lx: about to DeleteService()\n", data3);
            DeleteService(data3);
            Permit();
            }
         else PrintError(error);
         break;
      case 9:
         signal = AllocSignal(-1);
         if (signal != -1)
            {
            error = GetService(&data1, AppID, LocalID, 
                  signal, GETS_WAIT);
            if (NOT error) 
               {
               PrintServiceData(data1);
               bdata1 = (struct ServiceData *)MakeBytePtr(data1);
               do
                  {
                  Wait(1 << signal);
                  kprintf("DEL WATCHER GOT A SIGNAL!\n");
                  }
               while ((data1->Flags & SERVICE_DELETED) == 0);
               ReleaseService(data1);
               }
            else PrintError(error);
            }
         break;
      case 10:
         signal = AllocSignal(-1);
         if (signal == -1) break;
         error = AddService(&data3, AppID, LocalID, 100, 
               MEMF_BUFFER, 
               signal, NULL);
         if (NOT error) 
            {
            Wait(1 << signal);
            Forbid();
            PrintServiceData(data3);
            kprintf("%lx: about to DeleteService()\n", data3);
            DeleteService(data3);
            Permit();
            }
         else PrintError(error);
         break;
      case 11:
         signal = AllocSignal(-1);
         if (signal == -1) break;
         error = AddService(&data3, AppID, LocalID, 100, 
               MEMF_BUFFER, 
               signal, NULL);
         if (NOT error) 
            {
            AllocServiceMem(data3, 100, MEMF_BUFFER 
                  + MEM_BYTEACCESS);
PrintRemembers( JanusOffsetToMem(data3->JRememberKey, 
   MEMF_PARAMETER | MEM_WORDACCESS) );
            AllocServiceMem(data3, 100, MEMF_BUFFER 
                  + MEM_BYTEACCESS);
            ptr = AllocServiceMem(data3, 100, MEMF_BUFFER 
                  + MEM_BYTEACCESS);
PrintRemembers( JanusOffsetToMem(data3->JRememberKey, 
   MEMF_PARAMETER | MEM_WORDACCESS) );
            FreeServiceMem(data3, ptr);
PrintRemembers( JanusOffsetToMem(data3->JRememberKey, 
   MEMF_PARAMETER | MEM_WORDACCESS) );
            AllocServiceMem(data3, 100, MEMF_BUFFER 
                  + MEM_BYTEACCESS);
            AllocServiceMem(data3, 100, MEMF_BUFFER 
                  + MEM_BYTEACCESS);
            ptr = AllocServiceMem(data3, 100, MEMF_BUFFER 
                  + MEM_BYTEACCESS);
            FreeServiceMem(data3, NULL);
PrintRemembers( JanusOffsetToMem(data3->JRememberKey, 
   MEMF_PARAMETER | MEM_WORDACCESS) );
            DeleteService(data3);
            }
         else PrintError(error);
         break;
      default:
         printf("Don't know how to handle %ld\n", arg);
         break;
      }
}



VOID testmem()
{
   struct memarray *ma;

   for ( ma = testarray; ma->size; ma++ )
      {
      printf( "AllocJanusMem( %ld, %ld )", ma->size, ma->type );
      ma->address = AllocJanusMem( ma->size, ma->type );
      printf( " = 0x%lx\n", ma->address );
      }

   for ( ma = testarray; ma->size; ma++ )
      {
      printf( "FreeJanusMem( 0x%lx, %ld )\n", ma->address, ma->size );
      FreeJanusMem( ma->address, ma->size );
      }
}



VOID intserver()
{
    printf( "intserver: called\n" );
}



VOID PostFakeInt( jintnum )
int jintnum;
{
    struct JanusAmiga *ja = JanusBase;
    struct JanusBase *jb = (struct JanusBase *) ja->ja_ParamMem;
    UBYTE (*intvec)[2];

    intvec = (UBYTE (*)[2] ) ((ULONG)jb + jb->jb_Interrupts);

    intvec[jintnum][1] = JPCSENDINT;

/*???    ja->ja_LibNode.ja_Flags |= JAF_FAKEINT;*/
    custom.intreq = INTF_PORTS | INTF_SETCLR;
}



VOID testints()
{
    int     jintnum = 8;
    struct Interrupt intserv;

    intserv.is_Code = (VOID (*)()) intserver;


    SetJanusHandler( jintnum, &intserv );

    printf( "before set enable: req 0x%lx, ena 0x%lx\n",
       JanusBase->ja_IntReq, JanusBase->ja_IntEna );
    SetJanusEnable( jintnum, 1 );

    printf( "before forcing int: req 0x%lx, ena 0x%lx\n",
       JanusBase->ja_IntReq, JanusBase->ja_IntEna );
    PostFakeInt( jintnum );

    printf( "after forcing int: req 0x%lx, ena 0x%lx\n",
       JanusBase->ja_IntReq, JanusBase->ja_IntEna );
    SetJanusHandler( jintnum, 0 );
    SetJanusEnable( jintnum, 0 );
}



VOID testoffset()
{
    UWORD   oldoffset[MAXHANDLER];
    int     i;

    for ( i = 0; i < MAXHANDLER; i++ )
   {
   oldoffset[i] = GetParamOffset( i );
   SetParamOffset( i, i );
   printf( "offset %ld: was %ld, now is %ld\n",
   i, oldoffset[i], GetParamOffset( i ) );
       }

    for( i = 0; i < MAXHANDLER; i++ )
   SetParamOffset( i, oldoffset[i] );

    printf( "GetJanusStart() = 0x%lx\n", GetJanusStart() );
}



VOID testmemtype()
{
    struct memarray *me;

    /* use the values in test array to check these calls */
    for ( me = testarray; me->size; me++ ) 
   {
   printf( "JanusMemType( 0x%lx ) = 0x%lx, want 0x%lx\n",
      me->address, JanusMemType( me->address ), me->type );
   if ( JanusMemType( me->address ) != me->type ) 
       {
       printf( "JanusMemType( 0x%lx ) = 0x%lx, not 0x%lx\n",
          me->address, JanusMemType( me->address ), me->type );
       Debug();
       }
   }

    for ( me = testarray; me->size; me++ ) 
   {
   printf( "addr 0x%lx, type 0x%lx: ", me->address, me->type );
   printf( "JanusMemBase 0x%lx, ", JanusMemBase( me->type ) );
   printf( "Offset 0x%lx\n", JanusMemToOffset( me->address ) );
   }
}



VOID testsendint()
{
    int i;
    int oldval;

    for ( i = 0; i < MAXHANDLER; i++ ) 
   {
   oldval = CheckJanusInt( i );
   SendJanusInt( i );
   printf( "jint %ld: before %ld after %ld\n",
       i, oldval, CheckJanusInt( i ) );
   }
}



#define BUFMEMSIZE  1024

VOID testreadwrite()
{
    struct MemReadWrite *mrw;
    UBYTE *bufmem;
    int oldoffset;

    /* ok.  do what we can to fake reading and writing */

    /* alloc parameter area */
    if ( ! (mrw = (struct MemReadWrite *)
   AllocJanusMem( sizeof( struct MemReadWrite ),
   MEMF_PARAMETER | MEM_WORDACCESS )) )
   {
   printf( "testreadwrite: could not allocate param mem\n" );
   return; 
   }

    if( ! (bufmem = (UBYTE *)
       AllocJanusMem( BUFMEMSIZE, MEMF_BUFFER | MEM_WORDACCESS )) )
   {
   printf( "testreadwrite: could not allocate buffer mem\n" );
   return;
   }

    printf( "testreadwrite: param 0x%lx, buffer 0x%lx\n", mrw, bufmem );
    oldoffset = GetParamOffset( JSERV_READAMIGA );
    SetParamOffset( JSERV_READAMIGA, JanusMemToOffset( mrw ) );

    mrw->mrw_Command = MRWC_NOP;
    mrw->mrw_Count = 30;
    mrw->mrw_Address = 4;
    mrw->mrw_Buffer = JanusMemToOffset( bufmem );
    mrw->mrw_Status = 0xff00;

    PostFakeInt( JSERV_READAMIGA );

    printf( "testreadwrite: after int: status is 0x%lx\n", mrw->mrw_Status );

    mrw->mrw_Status = 0xff00;
    mrw->mrw_Command = MRWC_READ;

    PostFakeInt( JSERV_READAMIGA );

    printf( "testreadwrite: after 2 int: stat 0x%lx, data 0x%lx\n",
       mrw->mrw_Status, * (ULONG *) bufmem );

    SetParamOffset( JSERV_READAMIGA, oldoffset );
    FreeJanusMem( bufmem, BUFMEMSIZE );
    FreeJanusMem( mrw, sizeof( struct MemReadWrite ) );
}



VOID keyintcode()
{
    printf("keyboard interrupt received.\n");
}



int GetHex()
{
    int i, c;

    i = 0;
    do 
   {
   c = KGetCh();
   if ((c >= '0') && (c <= '9')) 
       {
       i *= 16;
       i += c - '0';
       }
   else if ((c >= 'a') && (c <= 'f')) 
       {
       i *= 16;
       i += c - 'a' + 10;
       } 
   else if ((c >= 'A') && (c <= 'F')) 
       {
       i *= 16;
       i += c - 'F' + 10;
       } 
   else c = -1;
       }
    while (c != -1);
    return( i );
}



VOID testkeyboard()
{
    UBYTE *ioBase;

    keyInt.is_Code = (VOID (*)()) keyintcode;
    SetJanusHandler( JSERV_ENBKB, &keyInt );
    SetJanusEnable( JSERV_ENBKB, 1 );
    ioBase = GetJanusStart()+IoAccessOffset+IoRegOffset;       
    for (;;) 
   {
   printf("Enter keycode (hex):");
   ioBase[JanusBase->ja_KeyboardRegisterOffset] = GetHex();
   ioBase[jio_PcIntGen] = JPCKEYINT;    
   }
}


VOID main(argc, argv)
int argc;
char **argv;
{
   SHORT servicearg;
   UBYTE *ptr, c;

   servicearg = 0;

   if (argc > 1)
      {
      ptr = *++argv;
      servicearg = 0;
      while (c = *ptr++)
         servicearg = (servicearg * 10) + c - '0';
      }

   JanusBase = (struct JanusAmiga *) OpenLibrary( "janus.library",0 );
   printf("janus.library = 0x%lx\n", JanusBase);
   if( ! JanusBase ) exit(0);

   /* OK.  library is open, test a few functions. */
   testservices(servicearg);
/*???   testremember();*/
/*???   testattach();*/
/*???   testmem();*/
/*???   testints();*/
/*???   testoffset();*/
/*???   testmemtype();*/
/*???   testsendint();*/
/*???   testreadwrite();*/

   /* now try the real stuff */
   /* ---- test the keyboard interrupt */
/*???   testkeyboard();*/

   CloseLibrary( JanusBase );
   exit(0);
}

