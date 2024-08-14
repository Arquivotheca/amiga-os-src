#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/expansionbase.h>
#include <libraries/filehandler.h>
#include <proto/all.h>
#include <string.h>

void kprintf(char *,...);
void sprintf(char *,char *,...);
#define  D(x) ;
#define  D1(x) ;

#define printf kprintf

struct World {
   struct ExpansionBase *ExpansionBase;
   };

#define KICK13  1 /* 1 for use with 1.3 0 for 2.x */

#if KICK13
LONG *Config13 = (LONG *)(0xF00000L+8L);
#else
LONG *Config14 = (LONG *)(0xF80000L+0x7F000L+8L);
#endif

int    MyInit(void);
void   printbstr(BSTR);
BOOL   isfloppy(BSTR);
void   BumpPart(struct World *,char *s);
LONG   GetHighPri(struct World *world);
void   KillHDParts(struct World *world);
BOOL   MatchPart(BSTR bp,char *s);
VOID   DisablePart(struct World *world,char *s);
BOOL   isRAD(struct FileSysStartupMsg *fssm);
BOOL   MatchBSTR(BSTR bp,char *s);

int MyInit()
{
   struct Library *library;
   struct World   *world;
   struct World   myworld;

   D( kprintf("###################\n") );
#if KICK13
   D( kprintf("1.3 Magic module Begins\n") );
#else
   D( kprintf("2.x Magic module Begins\n") );
#endif

   world=&myworld;

   library = OpenLibrary("expansion.library",0);
   if(library==0)
   {
      D( kprintf("No expansion.library\n") );
      goto out;
   }
   world->ExpansionBase=(struct ExpansionBase *)library;

#if KICK13
   D( printf("Config13=%ld\n",*Config13); )
#else
   D( printf("Config14=%ld\n",*Config14); )
#endif

#if KICK13
   if(*Config13==2)
   {
      D( printf("Config13 = HardDrive boot\n"); )
      BumpPart(world,"WB_1.3");
      DisablePart(world,"WB_2.x");
   } else
      if(*Config13==1)
      {
         D( printf("Config13 = Floppy boot\n"); )
         KillHDParts(world);
      } else {
         D( printf("Unkown Config type\n"); )
      }
#else
   if(*Config14==2)
   {
      D( printf("Config14 = HardDrive boot\n"); )
      BumpPart(world,"WB_2.x");
      DisablePart(world,"WB_1.3");
   } else
      if(*Config14==1)
      {
         D( printf("Config14 = Floppy boot\n"); )
         KillHDParts(world);
      } else {
         D( printf("Unkown Config type\n"); )
      }
#endif

out:
#if KICK13
   D( kprintf("1.3 Magic module exiting\n") );
#else
   D( kprintf("2.x Magic module exiting\n") );
#endif
   if(world->ExpansionBase) CloseLibrary((struct Library *)world->ExpansionBase);
   return(0);
}

#define ExpansionBase (world->ExpansionBase)

void printbstr(bp)
BSTR bp;
{
   int x;
   UBYTE count;
   UBYTE *p;

   p=(UBYTE *)(bp << 2);
   count=*p;
   /* D( kprintf("count = %ld ",count) ); */
   for(x=1;x<=count;x++)
      printf("%lc",(ULONG)p[x]);
}
BOOL isfloppy(bp)
BSTR bp;
{
   UBYTE count;
   UBYTE *p;

   p=(UBYTE *)(bp << 2);
   count=*p;
   D( kprintf("isfloppy() count = %ld ",count) );
   if(count!=3) return(FALSE);
   if((p[1]!='D')&&(p[1]!='d')) return(FALSE);
   if((p[2]!='F')&&(p[2]!='f')) return(FALSE);
   if((p[3]!='0')&&(p[3]!='1')&&(p[3]!='2')&&(p[3]!='4')) return(FALSE);
   D( kprintf("isfloppy() MATCH") );
   return(TRUE);
}
void BumpPart(struct World *world,char *s)
{
   struct BootNode *bn;
   struct DeviceNode *dn;
   struct FileSysStartupMsg *fssm;
   struct DosEnvec          *ev;

   D( kprintf("Begin MountList Search\n") );
   for(bn=(struct BootNode *)ExpansionBase->MountList.lh_Head;
       bn->bn_Node.ln_Succ;
       bn=(struct BootNode *)bn->bn_Node.ln_Succ)
   {
      D( printf("Node: ") );
      if(bn->bn_DeviceNode)
      {
         dn=(struct DeviceNode *)bn->bn_DeviceNode;
         fssm=(struct FileSysStartupMsg *)(((ULONG)dn->dn_Startup) << 2);
         ev=(struct DosEnvec *)(((ULONG)fssm->fssm_Environ) << 2);

         if(bn->bn_Node.ln_Type == NT_BOOTNODE)
         {
            D( printf("checking bootnode\n"); )
            if( MatchPart(dn->dn_Name,s))
            {
               /* bn->bn_Node.ln_Type=0x00; */
               bn->bn_Node.ln_Pri=GetHighPri(world);
               if(bn->bn_Node.ln_Pri<127)
                  bn->bn_Node.ln_Pri+=1;
               ev->de_BootPri=bn->bn_Node.ln_Pri;
               Remove((struct Node *)bn);
               Enqueue(&ExpansionBase->MountList,(struct Node *)bn);
               return;
            }
         }

         D( printbstr(dn->dn_Name) );
         D( printf(" Pri=%ld",bn->bn_Node.ln_Pri) );
         D( printf(" DOSType=%lx",(ULONG)ev->de_DosType) );
         D( printf(" BP=%ld",ev->de_BootPri) );
         D( if(bn->bn_Node.ln_Type & NT_BOOTNODE) printf(" BOOTABLE"); )
         D( printf(" NodeType=0x%lx\n",bn->bn_Node.ln_Type); )
      }
      D( printf("\n") );
   }
   D( kprintf("End MountList Search\n") );
}
LONG GetHighPri(struct World *world)
{
   struct BootNode *bn;
   struct DeviceNode *dn;
   struct FileSysStartupMsg *fssm;
   struct DosEnvec          *ev;
   LONG                     hp=-1;

   D( kprintf("Begin MountList Search\n") );
   for(bn=(struct BootNode *)ExpansionBase->MountList.lh_Head;
       bn->bn_Node.ln_Succ;
       bn=(struct BootNode *)bn->bn_Node.ln_Succ)
   {
      D( printf("Node: ") );
      if(bn->bn_DeviceNode)
      {
         dn=(struct DeviceNode *)bn->bn_DeviceNode;
         fssm=(struct FileSysStartupMsg *)(((ULONG)dn->dn_Startup) << 2);
         ev=(struct DosEnvec *)(((ULONG)fssm->fssm_Environ) << 2);

         if(bn->bn_Node.ln_Type == NT_BOOTNODE)
         {
            D( printf("checking bootnode\n"); )
            if( ! isfloppy(dn->dn_Name))
            {
               if( ! isRAD(fssm) )
               /* if( ! isPart(dn->dn_Name)) */
               {
                  if(bn->bn_Node.ln_Pri > hp)
                     hp=bn->bn_Node.ln_Pri;
               }
            }
         }

         D( printbstr(dn->dn_Name) );
         D( printf(" Pri=%ld",bn->bn_Node.ln_Pri) );
         D( printf(" DOSType=%lx",(ULONG)ev->de_DosType) );
         D( printf(" BP=%ld",ev->de_BootPri) );
         D( if(bn->bn_Node.ln_Type & NT_BOOTNODE) printf(" BOOTABLE"); )
         D( printf(" NodeType=0x%lx\n",bn->bn_Node.ln_Type); )
      }
      D( printf("\n") );
   }
   D( kprintf("End MountList Search\n") );
   return(hp);
}
void KillHDParts(world)
struct World *world;
{
   struct BootNode *bn;
   struct DeviceNode *dn;
   struct FileSysStartupMsg *fssm;
   struct DosEnvec          *ev;

   D( kprintf("Begin MountList Search\n") );
   for(bn=(struct BootNode *)ExpansionBase->MountList.lh_Head;
       bn->bn_Node.ln_Succ;
       bn=(struct BootNode *)bn->bn_Node.ln_Succ)
   {
      D( printf("Node: ") );
      if(bn->bn_DeviceNode)
      {
         dn=(struct DeviceNode *)bn->bn_DeviceNode;
         fssm=(struct FileSysStartupMsg *)(((ULONG)dn->dn_Startup) << 2);
         ev=(struct DosEnvec *)(((ULONG)fssm->fssm_Environ) << 2);

         if(bn->bn_Node.ln_Type == NT_BOOTNODE)
         {
            D( printf("checking bootnode\n"); )
            if( ! isfloppy(dn->dn_Name))
            {
               bn->bn_Node.ln_Type=0x00;
            }
         }

         D( printbstr(dn->dn_Name) );
         D( printf(" Pri=%ld",bn->bn_Node.ln_Pri) );
         D( printf(" DOSType=%lx",(ULONG)ev->de_DosType) );
         D( printf(" BP=%ld",ev->de_BootPri) );
         D( if(bn->bn_Node.ln_Type & NT_BOOTNODE) printf(" BOOTABLE"); )
         D( printf(" NodeType=0x%lx\n",bn->bn_Node.ln_Type); )
      }
      D( printf("\n") );
   }
   D( kprintf("End MountList Search\n") );
}
BOOL MatchPart(BSTR bp,char *s)
{
   UBYTE count;
   UBYTE *p;

   p=(UBYTE *)(bp << 2);
   count=*p;

   D( printf("MatchPart("); )
   D( printbstr(bp); )
   D( kprintf(",%s) enter\n",s) );

   if(count!=6) return(FALSE);

   if(strnicmp(&p[1],s,6)!=NULL)
   {
      D( kprintf("MatchPart() FAIL\n") );
      return(FALSE);
   }

   D( kprintf("MatchPart() MATCH\n") );
   return(TRUE);
}
VOID DisablePart(struct World *world,char *s)
{
   struct BootNode *bn;
   struct DeviceNode *dn;
   struct FileSysStartupMsg *fssm;
   struct DosEnvec          *ev;

   for(bn=(struct BootNode *)ExpansionBase->MountList.lh_Head;
       bn->bn_Node.ln_Succ;
       bn=(struct BootNode *)bn->bn_Node.ln_Succ)
   {
      if(bn->bn_DeviceNode)
      {
         dn=(struct DeviceNode *)bn->bn_DeviceNode;
         fssm=(struct FileSysStartupMsg *)(((ULONG)dn->dn_Startup) << 2);
         ev=(struct DosEnvec *)(((ULONG)fssm->fssm_Environ) << 2);

         if(bn->bn_Node.ln_Type == NT_BOOTNODE)
         {
            if( ! isfloppy(dn->dn_Name))
            {
               if(MatchPart(dn->dn_Name,s))
               {
                  Remove((struct Node *)bn);
                  return;
               }
            }
         }
      }
   }
}
BOOL isRAD(struct FileSysStartupMsg *fssm)
{
/*
   printf("isRAD() enter device=");
   printbstr(fssm->fssm_Device);
   printf("\n");
*/
   return(MatchBSTR(fssm->fssm_Device,"ramdrive.device"));
}
BOOL MatchBSTR(BSTR bp,char *s)
{
   UBYTE bcount,scount;
   UBYTE *p;

   p=(UBYTE *)(bp << 2);
   bcount=*p;
   scount=strlen(s);

/*
   D( printf("MatchPart("); )
   D( printbstr(bp); )
   D( kprintf(",%s) enter\n",s) );
*/

   if(bcount!=scount) return(FALSE);

   if(strnicmp(&p[1],s,scount)!=NULL)
   {
      D( kprintf("MatchPart() FAIL\n") );
      return(FALSE);
   }

   D( kprintf("MatchPart() MATCH\n") );
   return(TRUE);
}
