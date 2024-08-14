;/* intlist.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 intlist.c
Blink FROM LIB:c.o,intlist.o TO intlist LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

/* intlist - by Carl Sassenrath, modified slightly by CAS */

#include <exec/types.h>
#include <exec/interrupts.h>
#include <exec/execbase.h>
#include <hardware/custom.h>

#ifdef LATTICE
#include <proto/all.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

void PrintServers(struct List *slist);
char *GetNodeName(struct Node *node);
int  GetNodePri(struct Node *node);
void PrintAllIntr(void);


char *vers = "\0$VER: intlist 36.10";


extern struct Custom far custom;

#define REG register
#define FIRST(n) (struct Interrupt *)((n)->lh_Head)
#define EMPTY(n) ((n)->lh_Head == (struct Node *)&n->lh_Tail)
#define NEXTINT(i)  (struct Interrupt *)((i)->is_Node.ln_Succ)
#define LAST(i)  (((i)->is_Node.ln_Succ)->ln_Succ == 0)

struct ExecBase *EB;

char *IntrName[] =
    { "TBE    ", "DISKBLK", "SOFTINT", "PORTS  ",
      "COPER  ", "VERTB  ", "BLIT   ", "AUD0   ",
      "AUD1   ", "AUD2   ", "AUD3   ", "RBF    ",
      "DSKSYNC", "EXTER  ", "INTEN  ", "NMI    "
    };

char IntrPri[] = {1,1,1,2,3,3,3,4,4,4,4,5,5,6,6,7};

void main(int argc, char **argv)
   {
   extern struct ExecBase *SysBase;

   EB = SysBase;

   printf("IntList - Based on Guru's Guide by Carl Sassenrath\n");
   PrintAllIntr();
   exit(0);
   }

char *GetNodeName(struct Node *node)
   {
   if(node==NULL)  return ("");
   return(node->ln_Name==NULL ? "(missing)" : node->ln_Name);
   }

int GetNodePri(struct Node *node)
   {
   return(node==NULL ? 0 : node->ln_Pri);
   }

void PrintAllIntr(void)
   {
   int i;
   struct IntVector *iv = &EB->IntVects[0];
   APTR sc = (APTR)EB->IntVects[3].iv_Code;

   printf("IV E  INTR   P S   CODE     DATA    NPR     NAME\n");
   printf("-- - ------- - - -------- -------- ----- ----------\n");

   for(i=0; i<16; i++, iv++)
      {
      printf("%2ld %s %s %ld ", i,
               (custom.intenar & (1<<i)) ? "*" : " ",
               IntrName[i], IntrPri[i]);

      if((ULONG)iv->iv_Code == (ULONG)sc)
         {
         PrintServers(iv->iv_Data);
         }
      else if(iv->iv_Code != 0)
         {
         printf("  %08lx %08lx       %s\n",
                 iv->iv_Code, iv->iv_Data, GetNodeName(iv->iv_Node));
         }
      else printf("\n");
      }
   }


void PrintServers(struct List *slist)
   {
   struct Interrupt *s;

   if(EMPTY(slist))
      {
      printf("* (empty list)\n");
      return;
      }

   for(s=FIRST(slist); NEXTINT(s); s=NEXTINT(s))
      {
      printf("* %08lx %08lx %4ld  %s\n",
                 s->is_Code,s->is_Data,GetNodePri(s),GetNodeName(s));
      if(!LAST(s)) printf("               ");
      }
   }


