/* Device.c - Device support routines */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987, 1988 The Software Distillery.  All Rights  */
/* |. o.| || Reserved.  This program may not be distributed without the    */
/* | .  | || permission of the authors:                            BBS:    */
/* | o  | ||   John Toebes     Doug Walker    Dave Baker                   */
/* |  . |//                                                                */
/* ======                                                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "handler.h"

#if TESTING
#define MYNAME "\7Testnet"
#define DEVNAME "\4TEST"
#define CONFIG  "DEVS:networks/testhan-config" VERSTRING
#else
#define MYNAME "\3Net"
#define DEVNAME "\3NET"
#define CONFIG "DEVS:networks/handler-config" VERSTRING
#endif

#define isWHITE(x) ((x) == ' ' || (x) == '\t')

void ParseConfig(global)
GLOBAL global;
{
   BPTR fh;
   char libname[24];
   char nodename[RNAMELEN+2];
   char *s, *t;
   char buf[101];
   int len, i, j;
   DRIVER *driver = NULL;
   struct Library *DriverBase;

   fh = Open(CONFIG, MODE_OLDFILE);
   if (fh == NULL)
   {
      return;
   }

   len = 0;
   for(;;)
   {
      i = Read(fh, buf+len, 100-len);
      if (i < 0) break;
      len += i;
      if (len == 0) break;
      for (i = 0; i < 100; i++)
         if (buf[i] == '\n')
            break;
      /* Null terminate the buffer */
      buf[i] = 0;
      if(!memcmp(buf, "AUTO", 4))
      {
         if(driver == NULL)
         {
            BUGR("Invalid AUTO specification")
         }
         else
         {
            s=buf+4;
            while(*s != '\0')
            {
               /* Skip white space */
               while(isWHITE(*s)) s++;

               /* Find out how long the name is */
               for(j=0; s[j] && !isWHITE(s[j]); j++);

               /* Make a null-terminated BSTR out of the name */
               memcpy(nodename+1, s, j);
               nodename[0] = j;
               nodename[j+1] = '\0';

               /* Add the node */
               AddNode(global, nodename);

               /* Increment 's' past the name */
               s += j;
            }
         }
      }
      else if (buf[0] != '#')
      {
         /* We have one to configure */
         /* Parse out the device name and open the library for it */
         /* Then we call the device dependent */
         driver = (DRIVER *)DosAllocMem(global, sizeof(DRIVER));
         if (driver == NULL) break;         
         memcpy(libname, "NET-", 4);
         t = libname+4;
         s = buf;
         while(*s && !isWHITE(*s))
            *t++ = *s++;
         strcpy(t, ".library");
         OBUG(("Config to open library %s\n", libname));
         if (!global->sigbits)
         {
            /* We should check to see that we actually get the signal bit and   */
            /* do something sensible, but for now let it slide                  */
            global->sigbits = 1L << AllocSignal(-1);
         }
         driver->sigmask = global->sigbits;
         DriverBase = driver->libbase = OpenLibrary(libname, 0);
         if (driver->libbase == NULL ||
             SDNHanInit(&driver->drglobal, &driver->sigmask, s) != SDN_OK)
         {
            BUGR("Cannot init requested library");
            DosFreeMem((char *)driver);
         }
         else
         {
            OBUG(("Library initialized\n"));
            /* Now eliminate their signal bits from what we gave them   */
            global->sigbits &= ~driver->sigmask;
            global->waitbits |= driver->sigmask;
            
            /* Add them to the end of the chain because we are using this list  */
            /* as a priority order of drivers to be evaluated.                  */
            driver->next = NULL;
            if (global->drivers == NULL)
               global->drivers = driver;
            else
            {
               DRIVER *dp;
               for (dp = global->drivers; dp->next != NULL; dp = dp->next);
               dp->next = driver;
            }
         }
      }
      memcpy(buf, buf+i+1, 101-i);
      len -= i+1;
   }
   Close(fh);
}


int
InitDevice(global)
GLOBAL global;
{
   struct DosList *dl;
   struct DosInfo *info;
   struct RootNode *root;
   int i;

   BUGP("InitDevice: Enter")
   OBUG(("InitDevice: Enter\n"));

   if(!(global->n.devport=CreatePort(NULL,0))) 
   {
      BUG(("***ERROR: Can't CreatePort\n"));
      return(0);
   }

   SetTaskPri(global->n.self, 5);

   global->work = DosAllocMem(global, 512);

   global->prmhpkt = (HPACKET *)DosAllocMem(global, 
                              PERMHPACK*sizeof(struct HandlerPacket));

   if(!global->work || !global->prmhpkt)
   {
      BUGR("No memory!")
      return(1);
   }

   /* Initialize the permanent hpkts */
   for(i=0; i < PERMHPACK-1; i++)
      global->prmhpkt[i].hp_Next = global->prmhpkt+(i+1);

   global->prmhpkt[PERMHPACK-1].hp_Next = global->prmhpkt;

   BUGP("Init NetNode")

   /* Initialize NetNode struct for ourselves - at root of netnodes */
   global->netchain.next = NULL;
   global->netchain.status = NODE_UP;
   global->netchain.RootLock.incarnation = 0;
   global->netchain.bufsize = HAN_NETBUFSIZE;
   global->netchain.name = DosAllocMem(global, RNAMELEN+2);
   MBSTR(MYNAME, global->netchain.name);
   global->netchain.ioptr = NULL;
   global->netchain.RootLock.NetNode = &global->netchain;
   global->netchain.RootLock.RPtr = (RNPTR)1;  /* Special value */

   global->numnodes = global->upnodes = 1;

   dl = (struct DosList *)DosAllocMem(global, 
      sizeof(struct DosList)+strlen(DEVNAME)+1);
   strcpy((char *)(dl+1), DEVNAME);
   dl->dol_Type = DLT_DEVICE;
   dl->dol_Task = (struct MsgPort *)global->n.port;
   dl->dol_Name = MKBADDR((char *)(dl+1));

   /* Install a device node for our device */
   Forbid();

   root  = (struct RootNode *)DOSBase->dl_Root;
   info  = (struct DosInfo  *)BADDR(root->rn_Info);
   dl->dol_Next = info->di_DevInfo;
   info->di_DevInfo = MKBADDR(dl);

   Permit();


   /* Read the user config file and initialize devices */
   ParseConfig(global);

   /* THIS ISN'T THE RIGHT WAY TO DO THIS.... it should be device-dependent */
   global->n.NetBufSize = HAN_NETBUFSIZE;

#if 0
   global->devslock = (struct FileLock *)BADDR(Lock("DEVS:NETWORKS", SHARED_LOCK));
#endif

   BUGP("InitDevice: Exit")

   return(0);
}

struct NetNode *
AddNode(global, name)
GLOBAL global;
char *name;
{
    struct NetNode *node;
    DRIVER *dp;
    struct Library *DriverBase;
    HPACKET *hpkt;
    struct RPacket *RP;

   OBUGBSTR("AddNode: Adding ", name);

   /* First see if the node's name is unique */
   for (node = global->netchain.next; node; node = node->next) 
   {
      if(!stricmp(node->name, name)) 
         return(node);
   }

   OBUG(("AddNode: Node not in chain\n"));
   /* Node is new, add it */
   if (!(node=(struct NetNode *)
      DosAllocMem(global,(long)sizeof(struct NetNode)+strlen(name)+2)))
   {
      BUG(("***AddNode: Out of memory!\n"));
      return(NULL);
   }
   node->name = (char *)(node+1);
   strcpy(node->name, name);
   node->devname[0] = '\0';
   node->RootLock.incarnation = 0;
   node->bufsize = HAN_NETBUFSIZE;  /* Should be set by driver, really */
   global->numnodes++;

   /* Set up dummy lock for lock templating */
   node->RootLock.NetNode = node;
   node->RootLock.RPtr = 0;
   node->status = NODE_DEAD;

   /* Loop through all the drivers, see if any recognize this name */
   for(dp=global->drivers; 
       dp;
       dp=dp->next)
   {
      DriverBase = dp->libbase;
      if(SDNInitNode(dp->drglobal, name+1, &node->ioptr) != SDN_ERR) break;
   }

   if(dp)
   {
      /* Chain the node into the list of network nodes */
      OBUG(("AddNode: Node successfully initialized\n"))
      node->next = global->netchain.next;
      global->netchain.next = node;

      node->status = NODE_UP;
      node->driver = dp;

      /* Tell the remote side what we are calling this node */
      hpkt = GetHPacket(global);
      if(!hpkt || !(hpkt->hp_RP = RP = AllocRPacket(node, node->name[0]))) 
         return(NULL);
      RP->Type = ACTION_NETWORK_INIT;
      RP->DLen = node->name[0];
      memcpy(RP->DataP, node->name+1, RP->DLen);
      RP->handle = hpkt;
      hpkt->hp_NNode = node;
      hpkt->hp_Driver = node->driver;
      hpkt->hp_Pkt = NULL;
      hpkt->hp_NPFlags = 0;
      RemotePacket(global, hpkt);
   }
   else
   {
      OBUG(("AddNode: Can't InitNode '%s'\n", name))
      DosFreeMem((void *)node);
      return(NULL);
   }
   return(node);
}

int
TermDevice(global)
GLOBAL global;
{
   struct DosList *dl, *ldl;
   struct DosInfo *info;
   struct RootNode *root;
   DRIVER *driver, *ndriver;
   struct Library *DriverBase;

   Forbid();

   root  = (struct RootNode *)DOSBase->dl_Root;
   info  = (struct DosInfo  *)BADDR(root->rn_Info);
   for(ldl=NULL, dl= (struct DosList  *)BADDR(info->di_DevInfo);
       dl; ldl=dl, dl=(struct DosList *)BADDR(dl->dol_Next))
   {
      if(dl->dol_Type == DLT_DEVICE && 
         dl->dol_Task == (struct MsgPort *)global->n.port) break;
   }
   if(dl)
   {
      if(ldl) ldl->dol_Next = dl->dol_Next;
      else    info->di_DevInfo = MKBADDR(dl->dol_Next);
   }
 
   Permit();

   if(dl) DosFreeMem((char *)dl);

   for(driver=global->drivers; driver; driver=ndriver)
   {
      DriverBase = driver->libbase;
      SDNHanTerm(driver->drglobal);
      /*CloseLibrary(driver->libbase);*/
      ndriver=driver->next;
      DosFreeMem((char *)driver);
   }
   global->drivers = NULL;

   return(0);
}
