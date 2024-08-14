/* hkey.c */

#include "mycxapp.h"

#ifdef LATTICE
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
#endif

UBYTE *vers = "\0$VER: hkey 37.11";

UBYTE *autocon="CON:0/30/640/150/HKey Command/auto/close/wait";


ULONG setoffs = FOFF;

UBYTE *keynames[TOTKEYS] = {
   "F1","F2","F3","F4","F5","F6","F7","F8","F9","F10",
   "shift F1","shift F2","shift F3","shift F4","shift F5",
   "shift F6","shift F7","shift F8","shift F9","shift F10",
   "alt F1","alt F2","alt F3","alt F4","alt F5",
   "alt F6","alt F7","alt F8","alt F9","alt F10",
   "shift alt F1","shift alt F2","shift alt F3","shift alt F4","shift alt F5",
   "shift alt F6","shift alt F7","shift alt F8","shift alt F9","shift alt F10"
   };

struct HKInfo hkeys[TOTKEYS] = { NULL };


BOOL MySetupCX()
{
   LONG	error;
   UBYTE *incommand;
   int k;

   for(k=0; k<TOTKEYS; k++)
	{
	hkeys[k].key = keynames[k];
	incommand = (UBYTE *)ArgString(ttypes,keynames[k],"");
      	D( printf("did argstring on %s\n", keynames[k]) );
	if(hkeys[k].command =
	    (UBYTE *)AllocMem(CMDLEN,MEMF_PUBLIC|MEMF_CLEAR))
	    {
	    strcpy(hkeys[k].command,incommand);
	    }
	else return(FALSE);
	}
	
   for(k=0; k<TOTKEYS; k++)
	{
	if(hkeys[k].command[0] != '\0')  /* only attach do-something ones */
	    {
   	    AttachCxObj(broker,hkeys[k].cxobj=HotKey( hkeys[k].key,cxport,k));
      	    D( printf("did attach on %s\n", keynames[k]) );
 	    }
	}


   if (error = CxObjError(broker))
   	{
      	D( printf("accumulated broker error %ld\n", error) );
      	return(FALSE);
   	}
   return(TRUE);
}


VOID MyShutdownCX()
{
   int k;

   for(k=0; k<TOTKEYS; k++)
	{
	if(hkeys[k].command)   /* if buffer was allocated */
		{
		FreeMem(hkeys[k].command, CMDLEN);
		hkeys[k].command = NULL;
		}
	}
}


VOID MyHandleGadgets( gad,code )
ULONG gad,code;
{
   extern ULONG setoffs;
   int k,g;
   UBYTE *sgbuf;

   switch(gad)
   {
      case MY_NWAY:
            FlushGadgets();
            setoffs=code * 10;
      	    D( printf("NWAY called flush - next setoffs = %ld\n",setoffs ) );
            refreshGadgets();
            break;
      case GAD_F1:
      case GAD_F2:
      case GAD_F3:
      case GAD_F4:
      case GAD_F5:
      case GAD_F6:
      case GAD_F7:
      case GAD_F8:
      case GAD_F9:
      case GAD_F10:
	    g = gad - GAD_F1;
	    k = setoffs + g;

            sgbuf=(UBYTE *)(((struct StringInfo *)(F_Gadget[g]->SpecialInfo))->Buffer);

	    if(hkeys[k].cxobj) /* that key was already installed */
		{
		DeleteCxObjAll(hkeys[k].cxobj);         /* delete old cx  */
		hkeys[k].cxobj = NULL;			/* and NULL out   */
		}

	    if(sgbuf[0])	/* if new command in string gadget    */
		{
		if(!(hkeys[k].command))	  /* if no cmd buf yet, alloc */
		    {
		    hkeys[k].command = (UBYTE *)
			      AllocMem(CMDLEN,MEMF_PUBLIC|MEMF_CLEAR);
		    }
		if(hkeys[k].command)		/* if now have cmd buf */
		    {
		    strcpy(hkeys[k].command,sgbuf);  /* copy and attach */
   	    	    AttachCxObj(broker,
			hkeys[k].cxobj=HotKey(hkeys[k].key,cxport,k));
		    }
		}
	    else    /* null command for that gadget (cxobj already deleted) */
		{
		if(hkeys[k].command)	hkeys[k].command[0] = '\0';
		}

	    D( printf("new cmd for offs %d: %s\n",k,hkeys[k].command));
            break;
   }
}



VOID FlushGadgets()
{
   int g,k;
   extern ULONG setoffs;

   for(g=0,k=setoffs;g<10;g++,k++)
	{
	D( printf("Flush: gadget %d buffer contains: %s\n",g,
	     ((struct StringInfo *)(F_Gadget[g]->SpecialInfo))->Buffer) );
        strcpy(hkeys[k].command,
                ((struct StringInfo *)(F_Gadget[g]->SpecialInfo))->Buffer);
	}

}


VOID MyHandleCXMsg(id)
ULONG id;
{
UBYTE *command;

   D( printf("id received%ld\n",id) );
   if((id < TOTKEYS)&&(command=hkeys[id].command)&&(command[0] != '\0')) 
	{
	if(command[0] == '@')  doInternal(command);
	else doExternal(command);
	}
}


VOID doExternal(UBYTE *command)
    {
    struct TagItem stags[4];
    BPTR file;

    if(file = Open(autocon,MODE_OLDFILE))
	{
	stags[0].ti_Tag = SYS_Input;
	stags[0].ti_Data = file;
	stags[1].ti_Tag = SYS_Output;
	stags[1].ti_Data = NULL;
	stags[2].ti_Tag = SYS_Asynch;
	stags[2].ti_Data = TRUE;
	stags[3].ti_Tag = TAG_DONE;
        System(command, stags);
	}
    }
