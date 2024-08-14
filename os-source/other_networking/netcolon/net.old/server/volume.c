
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the authors:					   BBS:      */
/* | o	| ||   John Toebes     Dave Baker				     */
/* |  . |//								     */
/* ======								     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Volume Manipulation */
/* RmtCurentVol  RmtRenameDisk RmtDiskInfo RmtInfo */
/*
**      $Filename: volume.c $ 
**      $Author: J_Toebes $
**      $Revision: 1.15 $
**      $Date: 91/01/16 20:37:57 $
**      $Log:	volume.c,v $
 * Revision 1.15  91/01/16  20:37:57  J_Toebes
 * correct syntax error
 * 
 * Revision 1.14  91/01/16  20:26:19  J_Toebes
 * Added password checking
 * 
 * Revision 1.13  91/01/11  10:45:22  J_Toebes
 * Add support for password and login packets.
 * 
 * Revision 1.12  90/12/31  15:25:04  J_Toebes
 * Changed LCHAIN structure to an ELOCK, corrected parameters to freedevlist,
 * added support for ACTION_INFO on the rootlock.
 * 
 * Revision 1.11  90/12/30  15:39:09  J_Toebes
 * Add support for Network Init and Network Login
 * 
 * Revision 1.10  90/12/29  13:08:16  J_Toebes
 * Corrected INFODATA packet to send full infodata structure over.  Also
 * Eliminated duplicate names when the volume and device are the same name.
 * 
 * Revision 1.9  90/11/29  01:58:56  J_Toebes
 * Add freeing of locks when a connection terminates
 * 
 * Revision 1.8  90/11/28  01:54:34  J_Toebes
 * Corrected device list types
 * 
 * Revision 1.7  90/11/23  15:08:10  J_Toebes
 * Added in session support of access control lists.
 * 
 * Revision 1.6  90/11/15  08:17:39  J_Toebes
 * Correct multiple bugs associated with new version.  Sessions still are not supported.
 * 
 * Revision 1.5  90/11/05  06:56:03  J_Toebes
 * Major rewrite to make server multi-threaded with support for multiple
 * devices.  At this point in time it is not debugged and the concept of
 * sessions is missing.
 * 
**
**/

#include "server.h"

ACTFN(RmtInfo)
{
   /* Arg1: Lock                */
   /* Arg2: Name (in DataP)     */
   BUG(("RmtInfo\n"));

   spkt->sp_Pkt.dp_Arg1 = ((ELOCK)spkt->sp_Pkt.dp_Arg1)->lock;
   spkt->sp_Pkt.dp_Arg2 = (long)MKBADDR(spkt->sp_RP->DataP);
   spkt->sp_RP->DLen    = sizeof(struct InfoData);

   if(spkt->sp_Pkt.dp_Arg1 == ROOTLOCK)
   {
      struct InfoData *info = (struct InfoData *)spkt->sp_RP->DataP;
      info->id_NumSoftErrors = 0;
      info->id_UnitNumber    = 0;
      info->id_DiskState     = ID_VALIDATED;
      info->id_DiskType      = ID_DOS_DISK;
      info->id_NumBlocks     = 1;
      info->id_NumBlocksUsed = 1;
      info->id_VolumeNode    = NULL;
      info->id_InUse         = 0;
      spkt->sp_Pkt.dp_Res1   = DOS_TRUE;
   }
   else
   {
      Dispatch(global, spkt, ACTION_RETURN, 
            ((struct FileLock *)BADDR(spkt->sp_Pkt.dp_Arg1))->fl_Task);
   }
}

        
ACTFN(RmtNetInit)
{
   int len;
   
   len = spkt->sp_RP->DLen;
   if (len >= MAXNODENAME) len = MAXNODENAME-1;
   memcpy(spkt->sp_Ses->name+1, spkt->sp_RP->DataP, len);
   spkt->sp_Ses->name[0] = len;
   spkt->sp_Ses->name[len+1] = 0;
   global->n.reply = 1;
}

ACTFN(RmtNetLogin)
{
   struct ACCESS *access;
   
   spkt->sp_RP->DataP[spkt->sp_RP->DLen] = '\0';

   /* If they attempted to log in earlier but never gave the password, just     */
   /* free the previous attempt as it will never be completed.                  */
   if (spkt->sp_Ses->pending != NULL)
   {
      freeaccess(global, spkt->sp_Ses->pending);
      spkt->sp_Ses->pending = NULL;
   }
   access = loadaccess(global, spkt->sp_RP->DataP);
   if (access == NULL)
   {
      spkt->sp_RP->Arg1 = DOSFALSE;
      spkt->sp_RP->Arg2 = ERROR_OBJECT_NOT_FOUND;
   }
   else
   {
      /* See if there was any password at all on the Accesslist         */
      /* If there was we will have to delay the access until they give  */
      /* us a password.                                                 */
      if (access->name[0])
      {
         spkt->sp_Ses->pending = access;
         spkt->sp_RP->Arg1 = DOSFALSE;
         spkt->sp_RP->Arg2 = ERROR_NO_RIGHTS;
         spkt->sp_RP->DLen = 0;
/*       spkt->sp_RP->DBuf */
      }
      else
      {
         freedevlist(global, spkt->sp_Ses->devices);
         spkt->sp_Ses->devices = builddevlist(global, access);
         spkt->sp_RP->Arg1 = DOSTRUE;
      }
   }
}

ACTFN(RmtNetPassword)
{
   struct ACCESS *access;

   access = spkt->sp_Ses->pending;
   if (access == NULL)
   {
      spkt->sp_RP->Arg1 = DOSFALSE;
      spkt->sp_RP->Arg2 = ERROR_ACTION_NOT_KNOWN;
      return;
   }
   if (memcmp(spkt->sp_RP->DataP, access->name, access->name[0]+1))
   {
      freeaccess(global, spkt->sp_Ses->pending);
      spkt->sp_Ses->pending = NULL;
      spkt->sp_RP->Arg1 = DOSFALSE;
      spkt->sp_RP->Arg2 = ERROR_OBJECT_NOT_FOUND;
      return;
   }
   /* They gave us a good password, let them have access to it */
   freedevlist(global, spkt->sp_Ses->devices);
   spkt->sp_Ses->devices = builddevlist(global, access);
   spkt->sp_RP->Arg1 = DOSTRUE;
}

ACTFN(RmtNetStart)
{
   SESSION *ses;
   struct Library *DriverBase = spkt->sp_Driver->libbase;

   /* This is where we come to establish a session connection event     */
   /* The session ID is the first argument.  We will eventually get     */
   /* a stop for the connection.                                        */
   /* The main reason for this is to keep track of any resources        */
   /* that are established with the connection and to free them when    */
   /* the connection is terminated.  Since we are not currently tracking*/
   /* the file handles and locks we will ignore this event for now.     */
   
   /* Create a new session */
   ses = AllocMem(sizeof(SESSION), MEMF_CLEAR);
   if (ses != NULL)
   {
      ses->next = global->sessions;
      global->sessions = ses;
      strcpy(ses->name, "\x04John");
      ses->devices = builddevlist(global, loadaccess(global, "__DEFAULT__"));
      spkt->sp_Ses = ses;
      ses = SDNGetConData(spkt->sp_Driver->drglobal, spkt->sp_RP);
      SDNSetConData(spkt->sp_Driver->drglobal, spkt->sp_RP, spkt->sp_Ses);
      if (ses != NULL)
      {
         freedevlist(global, ses->devices);
      }
   }
   global->n.reply = -1;      /* Delete the fake message, do not reply  */
}

ACTFN(RmtNetStop)
{
   /* They have left us, terminate any resources that we have gotten    */
   /* for them.                                                         */
   SESSION *ses;
   ELOCK lch, lnext;
   struct FCHAIN *fch, *fnext;
   ses = (SESSION *)spkt->sp_RP->Arg1;
   if (ses != NULL)
   {
      freedevlist(global, ses->devices);
      for(lch = ses->locks; lch != NULL; lch = lnext)
      {
         UnLock(lch->lock);
         lnext = lch->next;
         FreeMem(lch, sizeof(struct lchain));
      }
      for(fch = ses->fhs; fch != NULL; fch = fnext)
      {
         Close(MKBADDR(fch));
         fnext = fch->next;
         FreeMem(fch, sizeof(struct FCHAIN));
      }
      FreeMem(ses, sizeof(SESSION));
   }   
   global->n.reply = -1;      /* Delete the fake message, do not reply  */
}

/*                                        
  access/       - contains the default access rules for all file systems to be
                  exported.  These files will be in the form:
                     Default: (Include|Readonly|Shared|Exclude)
                     Password: <password>
                     <assign>: [(Include|Shared|Readonly|Exclude)] [Assign name]
                  The Default: and Password: lines MUST be the first two lines in
                  that order.   On each subsequent line will be those logical assigns
                  that are to be treated as other than the default.  If no other
                  assigns to be treated specially then the file may consist of only
                  two lines.  The Password keyword is used to validate access to the
                  network.  If no password is given, access is available to anyone.
                  Default may be one of 4 selections:
                      DEFAULT: INCLUDE  -  Indicates that all top level nodes are to
                                           included in the root file system.  Access
                                           to the nodes is in a shared R/W mode
                      DEFAULT: SHARED   -  Same as DEFAULT: INCLUDE
                      DEFAULT: READONLY -  Indicates that all top level nodes are to
                                           be included in the root file system but in
                                           a read-only mode.
                      DEFAULT: EXCLUDE  -  Indicate the no nodes are to be included in
                                           the root file system.
                  These same keywords may be used on any assign that appears on a
                  subsequent line.  It may also have the keyword ASSIGN name which
                  indicates that the assigned name to appear is not really an existing
                  assign, but a local assign to be made to the given directory.  Note
                  that when this type of assign is used, no access to anywhere above
                  in the directory tree is to be allowed.
                - Any line that starts with a pound sign (#) or a semicolon (;)
                  will be treated as a comment and ignored.
                - The default access file will be called __DEFAULT__. if it is not
                  present, then a default file consisting of the commands:
                     DEFAULT: SHARED
                     PASSWORD:
                  will be assumed allowing complete access to all entries and no
                  password access required.
                - Additional files will be present in the directory corresponding to
                  users who are to be granted access.  This allows different users
                  to have different levels of access.
*/

#define BUFSIZE 255

void freeaccess(global, access)
GLOBAL global;
struct ACCESS *access;
{
   struct ACCESS *next;
   int size;
   
   if (access == NULL) return;

   next = access->next;
   FreeMem(access, sizeof(struct ACCESS)+MAXPW);
   access = next;
   while(access != NULL)
   {
      size = strlen(access->name)+1;
      if (access->class & CLASS_ASSIGN)
      {
         size += strlen(access->name+size)+1;
      }
      next = access->next;
      FreeMem(access, sizeof(struct ACCESS)+size);
      access = next;
   }
}

#define MODE_NONE  0
#define MODE_TOKEN 1
#define MODE_QUOTE 2
#define MODE_DONE  3
struct ACCESS *
loadaccess(global, name)
GLOBAL global;
char *name;
{
   char buf[BUFSIZE+1];
   int i, namelen, len, tlen, assignlen, needassign, mode, class;
   char *p, *tokptr, *assignname;
   struct ACCESS *alist, *listent;
   BPTR fh;

              /*123456789012345678901*/
   strcpy(buf, "DEVS:networks/access/");
   len = strlen(name);
   if (len > 31) len = 31;
   memcpy(buf+21, name, len);
   buf[len+21] = 0;

   alist = AllocMem(sizeof(struct ACCESS)+MAXPW, MEMF_CLEAR);
   if (alist == NULL) return(NULL);
   listent = alist;

   alist->class = CLASS_READONLY;
   alist->name[0] = 0;

   fh = Open(buf, MODE_OLDFILE);
   if (fh == NULL)
   {
      /* No access information present, use our standard default */
      return(alist);
   }

   len = 0;
   for(;;)
   {
      i = Read(fh, buf+len, BUFSIZE-len);
      if (i < 0) break;
      len += i;
      if (len == 0) break;
      for (i = 0; i < BUFSIZE; i++)
         if (buf[i] == '\n')
            break;
      /* Null terminate the buffer */
      buf[i] = 0;
      if (buf[0] != '#' && buf[0] != ';' && (p = strchr(buf, ':')) != NULL)
      {
         /* We have a valid entry.  Parse out the name */
         *p++ = 0;
         namelen = p-buf;
         
         /* Now parse out the tokens on the line.  We should see one of */
         /* EXCLUDE, READONLY, SHARED, INCLUDE, ASSIGN */
         /* While we are not at the end of the line we should parse the tokens */
         mode = 0;
         class = CLASS_SHARED;
         assignname = NULL;
         assignlen = 0;
         needassign = 0;
         if (*p)
         {
            do
            {
               /* Find the next non-blank token */
               if (*p == 0)
               {
                  if (mode != MODE_NONE) mode = MODE_DONE;
               }
               else if (*p == ' ' || *p == '\t' || *p == '\f')
               {
                  if (mode == MODE_TOKEN) mode = MODE_DONE;
               }
               else if (*p == '"')
               {
                  switch(mode)
                  {
                     case MODE_QUOTE:
                        mode = MODE_DONE;
                        break;
                     case MODE_NONE:
                        tokptr = p+1;
                        mode = MODE_QUOTE;
                        break;
                  }
               }
               else
               {
                  switch(mode)
                  {
                     case MODE_QUOTE:
                        break;
                     case MODE_NONE:
                        tokptr = p;
                     case MODE_TOKEN:
                        *p |= 0x20;  /* Convert to lowercase if not already */
                        mode = MODE_TOKEN;
                        break;
                  }
               }
               if (mode == MODE_DONE)
               {
                  tlen = p-tokptr;
                  if (needassign)
                  {
                     assignname = tokptr;
                     assignlen  = tlen+1;
                     class |= CLASS_ASSIGN;
                     needassign = 0;
                  }
                  else
                  {
                     switch(*tokptr)
                     {
                        case 'e': if (!memcmp(tokptr, "exclude", tlen))
                                     class=CLASS_EXCLUDE;
                                  break;
                        case 'r': if (!memcmp(tokptr, "readonly", tlen))
                                     class=CLASS_READONLY;
                                  break;
                        case 's': if (!memcmp(tokptr, "shared", tlen))
                                     class=CLASS_SHARED;
                                  break;
                        case 'i': if (!memcmp(tokptr, "include", tlen))
                                     class=CLASS_SHARED;
                                  break;
                        case 'a': if (!memcmp(tokptr, "assign", tlen))
                                     needassign = 1;
                                  break;
                     }
                  }
                  mode = MODE_NONE;
               }
            } while (*p++);
         }
         
         /* class = Class of this entry            */
         /* buf holds the entry name for len bytes */
         /* assignname = NULL or name of assign    */
         /* assignlen  = length of assigned name   */
         /* First we need to check for the default one or the password */
         if (!strcmp(buf, "DEFAULT"))
         {
            alist->class = class;
         }
         else if (!strcmp(buf, "PASSWORD"))
         {
            if (tlen > 32) tlen = 32;
            memcpy(alist->name, tokptr, tlen);
            alist->name[tlen] = 0;
         }
         else
         {
            /* Allocate a new entry */
            listent =
            listent->next = AllocMem(sizeof(struct ACCESS)+assignlen+namelen,
                                     MEMF_CLEAR);
            if (listent == NULL) break;
            listent->class = class;
            memcpy(listent->name, buf, namelen);
            if (assignlen)
            {
               assignlen--;
               memcpy(listent->name+namelen, assignname, assignlen);
               listent->name[namelen+1+assignlen] = 0;
            }
         }
      }
      memcpy(buf, buf+i+1, BUFSIZE-i);
      len -= i+1;
   }
   
   /* If we have a default, set up the default entry */
   
   Close(fh);
   return(alist);
}

void freedevlist(global, devlist)
GLOBAL global;
struct DEVLIST *devlist;
{
   struct DEVLIST *next;
   while(devlist != NULL)
   {
      next = devlist->next;
      FreeMem(devlist, sizeof(struct DEVLIST)+devlist->len);
      devlist = next;
   }
}

/* Build the list of all accessible devices */
struct DEVLIST *builddevlist(global, access)
GLOBAL global;
struct ACCESS *access;
{
   struct DosList *dev, *doslist, *devlook;
   struct DEVLIST *base, *cur, *new;
   struct ACCESS *this;
   BPTR lock;
   char *name;
   struct FileInfoBlock __aligned fib;

   if (access == NULL) return(NULL);
   base = cur = new = NULL;
   
   /* We are doing an examine next loop through the entries in the */
   /* Directory.  We need to walk the Device list here for the     */
   /* operation.  For now we are bad boys and walk it under forbid */
   /* With 2.0 we should use the new doslist access routines       */
   Forbid();
   doslist = (struct DosList *)
          BADDR(((struct DosInfo *)
                     BADDR(((struct RootNode *)DOSBase->dl_Root)->rn_Info)
                     )->di_DevInfo);

   for(dev = doslist; dev != NULL; dev = (struct DosList *)(BADDR(dev->dol_Next)))
   {
      /* Now walk through all the device node entries determining what to  */
      /* do with them.                                                     */
      name = (char *)BADDR(dev->dol_Name);
      
      /* First see if the name appears on any special entry in the access list */
      for(this = access->next; this != NULL; this = this->next)
      {
         if (!stricmp(this->name, name+1)) break;
      }
      if (this == NULL) this = access;  /* Use the default one */
      if (this->class == CLASS_EXCLUDE || (this->class & CLASS_ASSIGN)) continue;

      new = AllocMem(sizeof(struct DEVLIST)+name[0], MEMF_CLEAR);
      if (new == NULL)
      {
         /* OOps, out of memory, gotta do something about it */
         Permit();
         return(base);
      }

      if (base == NULL)
      {
         base = cur = new;
      }
      else
      {
         cur = cur->next = new;
      }
      
      if (this->class == CLASS_READONLY)
      {
         cur->protect = FIBF_WRITE|FIBF_DELETE|FIBF_EXECUTE;
      }
      else
      {
         cur->protect = 0;
      }

      /* If there is no volume node for the device we can assume that it     */
      /* is a file.                                                          */
      BUG(("Key=%08lx Type = %ld Startup=%08lx\n", dev, dev->dol_Type, dev->dol_misc.dol_handler.dol_Startup));
      if (dev->dol_Type == DLT_DEVICE &&
          dev->dol_misc.dol_handler.dol_Startup < 100)
      {
         /* We should check to see that there is also no volume node for this */
         /* entry.                                                            */
         for(devlook = doslist; devlook != NULL;
             devlook = (struct DosList *)(BADDR(devlook->dol_Next)))
            if (devlook->dol_Type == DLT_VOLUME &&
                devlook->dol_Task == dev->dol_Task) break;
         /* It is a "file" */
         if (devlook == NULL)
         {
            cur->type = ST_FILE;
         }
         else
         {
            /* Just to be sure, make sure that the two names are not the same */
            /* This will prevent duplicate names from appearing on the list   */
            if (!stricmp(BADDR(devlook->dol_Name), name))
            {
               FreeMem(new, sizeof(struct DEVLIST)+name[0]);
               continue;
            }
            cur->type = ST_USERDIR;
         }
      }
      else
      {
         cur->type = ST_USERDIR;
      }
/*    cur->port = dev->dol_Task; */
      cur->lock = dev->dol_Lock;
      memcpy(&cur->len, name, name[0]+2);
   }
   Permit();

   /* Now go through and find all the assigns that they asked to do */
   for (this = access->next; this != NULL; this = this->next)
   {
      if (this->class & CLASS_ASSIGN)
      {
         lock = Lock(this->name+1+strlen(this->name), SHARED_LOCK);
         if (lock != NULL)
         {
            /* Now figure out whether it is a file or a directory */
            /* For this we need to do an Examine() */
            if (!Examine(lock, &fib))
            {
               UnLock(lock);
            }
            else
            {
               new = AllocMem(sizeof(struct DEVLIST)+strlen(this->name), MEMF_CLEAR);
               if (new == NULL)
               {
                  UnLock(lock);
                  break;
               }
               if (base == NULL)
               {
                  base = cur = new;
               }
               else
               {
                  cur = cur->next = new;
               }

               if (this->class == (CLASS_ASSIGN | CLASS_READONLY))
               {
                  cur->protect = FIBF_WRITE|FIBF_DELETE|FIBF_EXECUTE;
               }
               else
               {
                  cur->protect = 0;
               }
/*             cur->port = dev->dol_Task; */
               cur->lock = lock;
               cur->len = strlen(this->name);
               if (fib.fib_DirEntryType > 0) /* A directory */
                  cur->type = ST_LINKDIR;
               else
                  cur->type = ST_LINKFILE;
               strcpy(cur->name, this->name);
            }
         }
      }
   }
   return(base);
}
