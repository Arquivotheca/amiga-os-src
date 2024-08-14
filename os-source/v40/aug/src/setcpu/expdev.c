/*
	SetCPU V1.5
	by Copyright 1989 by Dave Haynie

	EXPANSION DEVICE MODULE
	
	This module maintains the code used to detect and translate
	a given expansion device.
*/

#include "setcpu.h"

/* ====================================================================== */

/* Stuff I need for this module. */

char *malloc();
int sscanf();

/* ====================================================================== */

/* This section manages devices with on-board ROM.  The actual 
   implementation of this stuff is pretty well hidden in this
   module. */
   
/* This structure manages the expansion devices that we know about. */

struct ExpDev {
   struct Node node;
   ULONG manuf;
   ULONG board;
   ULONG size;
   ULONG romoffset;
   ULONG romsize;
};
   
/* Here's the list of all the devices we know about. */

static struct List Devices = 
   {(struct Node *)&Devices.lh_Tail,NULL,(struct Node *)&Devices.lh_Head,0,0};

static struct ExpDev *CurrentDevice = NULL;

/* This function reads the list of expansion devices from the given file.
   The file should contain data for one device per line, with the 
   following format:
   
       MANUF	PROD	SIZE	START	LENGTH	id-string
       
   Except for the last field, all numbers are integers, which may be 
   expressed as decimal or hexidecimal.  The last field is a text
   string which may contain any information.  It's very likely that
   the line for each device will have to be supplied by the maker
   of any device, since there's no way to figure out if a device's
   ROM is on the same MMU page as an important I/O register which
   shouldn't ever be translated.  The description I use for the Amiga
   2090A controller is:
   
  	0x202 0x01 0x10000 0x8000 0x4000 CBM 2090A Disk Controller

   The actual fields are:
   
	MANUF		The Manufacturer's code for the PIC
	PROD		The Product code for the PIC
	SIZE		The configured size of the PIC
	START		The offset from the PIC base for the start of ROM
	LENGTH		The length of the ROM	
	id-string	Whatever the hell you want
*/

BOOL ReadExpDevs(name)
char *name;
{
   FILE *file;
   char buf[256],msg[256];
   struct ExpDev *ed;
   int len,i;
   
   if (!(file = fopen(name,"r"))) return FALSE;
   
   while (fgets(buf,256,file)) {
      ed = (struct ExpDev *)malloc(sizeof(struct ExpDev));
      if (sscanf(buf,"%lx%lx%lx%lx%lx%s",&ed->manuf,&ed->board,&ed->size,
                 &ed->romoffset,&ed->romsize,msg) >= 5) {
         if (len = strlen(msg)) {
            ed->node.ln_Name = (char *)malloc(len+1);
            for (i = 0; i <= len; ++i)
               ed->node.ln_Name[i] = (msg[i] == '_')?' ':msg[i];
         }
         AddHead(&Devices,(struct Node *)ed);
      } else
         free(ed);
   }
   fclose(file);
   CurrentDevice = (struct ExpDev *)Devices.lh_Head;
   return TRUE;
}

/* ====================================================================== */

/* This function returns the ROM information I'll need for a given 
   expansion device.  Because of some Commodore screwups, the A2090A
   appears the same device as other boards like the Commodore RAM
   board.  So I have the size passed as well, as a consistency 
   check.  The returned ExpROMData structure is allocated here, and
   can be freed when no longer needed. */
    
struct ExpROMData *GetExpROM() 
{
   struct ConfigDev *cd;
   struct DiagArea *da;   
   struct ExpROMData *ed;
   
   if (!CurrentDevice || !ExpansionBase) return NULL;
   cd = FindConfigDev(NULL,CurrentDevice->manuf,CurrentDevice->board);

   while (cd) {
      if (((ULONG)cd->cd_BoardSize == CurrentDevice->size) && 
          (cd->cd_Rom.er_Type & ERTF_DIAGVALID)) {
         da = (struct DiagArea *)(((ULONG)cd->cd_BoardAddr)+((ULONG)cd->cd_Rom.er_InitDiagVec));

        /* This is just a sanity check to make sure we really use the
           ROM out on this card -- a nybble or byte wide ROM will be in
           RAM already. */
         if ((da->da_Config & DAC_BUSWIDTH) == DAC_WORDWIDE)  {
            ed = (struct ExpROMData *)AllocMem(SizeOf(struct ExpROMData),MEMF_CLEAR);
            ed->ROMbase = ((ULONG)cd->cd_BoardAddr) + CurrentDevice->romoffset;
            ed->ROMsize = CurrentDevice->romsize;
            ed->imagebase = NULL;
            ed->tablebase = NULL;
            ed->name = (char *)AllocMem((ULONG)
                               strlen(CurrentDevice->node.ln_Name)+1,0L);
            strcpy(ed->name,CurrentDevice->node.ln_Name);
            ++CurrentDevice;
            return ed;
         }
      }
      cd = FindConfigDev(cd,CurrentDevice->manuf,CurrentDevice->board); 
   }
   CurrentDevice = (struct ExpDev *)CurrentDevice->node.ln_Succ;
   return NULL;
}

/* ====================================================================== */

/* This routine is called to configure the system devices without calling any
   ROM routines that could possibly goober up on older OS releases.  Other
   than skipping ROMs, this should do the same thing as the ConfigChain()
   function in expansion.library. */
   
void SafeConfigDevs()
{
   struct ConfigDev *cd;
   char *base = (char *)0xe80000;

   if (!ExpansionBase) return;

   /* Should also loop only as long as 
         !(ExpansionBase->eb_Flags & EBB_CLOGGED), 
      but I don't know what EBB_CLOGGED is yet.. */

   while (cd = AllocConfigDev()) {
      if (ReadExpansionRom(base,cd)) {
         FreeConfigDev(cd); 
         break;
      }

     /* Got it, let's get rid of any ROM critters. */
      cd->cd_Rom.er_Type &= ~ERTF_DIAGVALID;

    /* Now adding the board should be safe! */
      if (ConfigBoard(base,cd)) {
         FreeConfigDev(cd);
         break;
      }
      AddConfigDev(cd);
   }
}

