
/*
 * $Header: envoy:src/libs/nipc.library/RCS/nstat.c,v 1.1 92/02/13 10:03:14 gregm Exp Locker: gregm $
 */

#include <exec/types.h>
#include <exec/lists.h>
#include "pragmas/nipc_pragmas.h"
#include "nipcinternal.h"

struct Library *NIPCBase;

void addtodot();
void twohex();

main(argc,argv)
int argc;
char *argv[];
{
 struct stats localstats;
 struct sanadev *intf;
 char plain[80];
 char *flgstr[8]={"   ","  A"," O "," OA","W  ","W A","WO ","WOA"};
 char *states[6]={"CLOSED","LISTEN","SENT","RCVD","OPEN","CLOSE (dead)"};
 char *entflgs[8]={"   ","  P"," A "," AP","L  ","L P","LA ","LAP"};


 NIPCBase = (struct Library *) OpenLibrary("nipc.library",0L);
 if (!NIPCBase)
   {
   printf("Can't get the nipc.library open!\n");
   exit(1);
   }

 printf("NIPC Statistics\n---------------\n");
 StartStats(&localstats);

 printf("NIPC has the following interfaces available:\n");
 if (IsListEmpty( (struct List *) localstats.DevList))
   printf("none\n");
 else
   {
   printf("IPAddress\tIPType\tARPType\tHType\tMTU\tBPS\t\tFlags\tHardAddr\n");
   intf = (struct sanadev *) localstats.DevList->mlh_Head;
   while (intf->sanadev_Node.mln_Succ)
      {
      int x;
      addtodot(intf->sanadev_IPAddress,&plain);
      printf("%s\t%d\t%d\t%d\t%d\t%d\t%s\t",&plain,intf->sanadev_IPType,intf->sanadev_ARPType,
                                        intf->sanadev_HardwareType,intf->sanadev_MTU,
                                        intf->sanadev_BPS,flgstr[intf->sanadev_Flags & 7]);

      if (intf->sanadev_AddressSize)
         for (x = 0; (x < ((intf->sanadev_AddressSize-1)/8+1)); x++)
            twohex(intf->sanadev_HardAddress[x]);
      printf("\n");


      intf = (struct sanadev *) intf->sanadev_Node.mln_Succ;
      }
   }

 printf("\n\nIP is aware of the following protocol numbers:\n");
 if (IsListEmpty( (struct List *) localstats.ProList))
   printf("none\n");
 else
   {
   struct ipproto *pr;
   pr = (struct ipproto *) localstats.ProList->mlh_Head;
   while (pr->ipproto_link.mln_Succ)
      {
      printf("%d ",pr->ipproto_ProtocolNumber);
      pr = (struct ipproto *) pr->ipproto_link.mln_Succ;
      }
   printf("\n");
   }

 printf("\nRDP Connections currently are:\n");
 if (IsListEmpty( (struct List *) localstats.ConnList))
   printf("none\n");
 else
   {
   struct RDPConnection *c;
   c = (struct RDPConnection *) localstats.ConnList->mlh_Head;

   while (c->conn_link.mln_Succ)
      {
      addtodot(c->conn_OurAddress,&plain);
      printf("Local IP Address %s\n",&plain);
      printf("Local port %ld\n",c->conn_OurPort);

      if (c->conn_State == STATE_OPEN)
         {
         addtodot(c->conn_TheirAddress,&plain);
         printf("Connected to IP Address %s\n",&plain);
         printf("             port %ld\n",c->conn_TheirPort);
         }

      if (c->conn_State == STATE_SENT)
         {
         addtodot(c->conn_TheirAddress,&plain);
         printf("Trying to connect to IP Address %s\n",&plain);
         printf("                     port %ld\n",c->conn_TheirPort);
         }

      printf("Connection state is %s\n",states[c->conn_State]);
      printf("Next seq number to send 0x%lx\n",c->conn_SendNxt);
      printf("Seq # of oldest unacked packet 0x%lx\n",c->conn_SendOldest);
      printf("Max number of unacked live packets allowed (xmit) 0x%lx\n",c->conn_SendMax);
      printf("Max number of unacked live packets allowed (recv) 0x%lx\n",c->conn_RecvMax);
      printf("Seq # of last packet received in seq 0x%lx\n",c->conn_Current);
      printf("Max packet size (xmit) 0x%lx\n",c->conn_SendMaxSize);
      printf("Max packet size (recv) 0x%lx\n",c->conn_RecvMaxSize);
      printf("Connection state retransmits %x\n",c->conn_ConnRetransmits);
      printf("Data packet retransmits %x\n",c->conn_DataRetransmits);
      printf("Seq # of last packet delivered 0x%lx\n",c->conn_SeqDelivered);

      c = (struct RDPConnection *) c->conn_link.mln_Succ;
      printf("\n");
      }
   }

 printf("Entities:\n");
 if ( IsListEmpty( (struct List *) localstats.EntList))
   printf("none\n");
 else
   {
   struct Entity *e;
   e = (struct Entity *) localstats.EntList->mlh_Head;
   while (e->entity_Port.mp_Node.ln_Succ)
      {
      int x;
      struct Node *n;

      if (e->entity_Port.mp_Node.ln_Name)
         printf("Entity Name: %s\n",e->entity_Port.mp_Node.ln_Name);
      else
         printf("No Entity Name\n");

      printf("Flags: %s\n",entflgs[e->entity_Flags & 7]);

      Forbid();
      x = 0;
      if (IsListEmpty(&e->entity_Port.mp_MsgList))
         {
         Permit();
         printf("Entity has no pending Transactions (recv).\n");
         }
      else
         {
         n = e->entity_Port.mp_MsgList.lh_Head;
         while (n->ln_Succ)
            {
            x++;
            n = n->ln_Succ;
            }
         Permit();
         printf("Entity has 0x%lx pending Transactions (recv)\n",x);
         }

      ObtainSemaphore(&e->entity_linksema);
      if (IsListEmpty( (struct List *) &e->entity_linklist))
         printf("No links attached to this Entity\n");
      else
         {
         struct Entity *le;
         le = (struct Entity *) e->entity_linklist.mlh_Head;
         while (le->entity_Port.mp_Node.ln_Succ)
            {
            printf("\t---\n");
            if (le->entity_Port.mp_Node.ln_Name)
               printf("\tLink named: %s\n",le->entity_Port.mp_Node.ln_Name);
            else
               printf("\tLink is not named\n");

            if (le->entity_linkname)
               printf("\tRepresents entity name %s\n",le->entity_linkname);
            else
               printf("\tNo representative name\n");

            if (le->entity_Flags & ENTF_LINK)
               {
               printf("\tOwner is 0x%lx\n",le->entity_transsema.ss_Owner);
               ObtainSemaphore(&le->entity_transsema);
               printf("\tGOT SEMAPHORE\n");
               if (!IsListEmpty((struct List *) &le->entity_translist))
                  {
                  struct Transaction *t;
                  int y=0;
                  t = (struct Transaction *) le->entity_translist.mlh_Head;
                  while (t->trans_Msg.mn_Node.ln_Succ)
                     {
                     y++;
                     t = (struct Transaction *) t->trans_Msg.mn_Node.ln_Succ;
                     }
                  printf("\tLink has %d unfinished transactions (xmit)\n",y);
                  }
               else
                  printf("\tNo unfinished transactions (xmit).\n");
               ReleaseSemaphore(&le->entity_transsema);
               }

            printf("\tLink flags: %s\n",entflgs[le->entity_Flags & 7]);

            le = (struct Entity *) le->entity_Port.mp_Node.ln_Succ;
            }
         }


      ReleaseSemaphore(&e->entity_linksema);

      e = (struct Entity *) e->entity_Port.mp_Node.ln_Succ;
      }



   }





 EndStats();



 CloseLibrary(NIPCBase);

}

void addtodot(address,string)
ULONG address;
char *string;
{
 sprintf(string,"%d.%d.%d.%d",((address >> 24) & 0xFF),
                              ((address >> 16) & 0xFF),
                              ((address >> 8) & 0xFF),
                              (address & 0xFF) );
}

void twohex(numb)
int numb;
{

 printf("%x%x",((numb>>4) & 0xf),(numb & 0xf));

}
