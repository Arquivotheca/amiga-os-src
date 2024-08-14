;/* playmf.c - Execute me to compile me with SAS C 6.x
SC playmf.c data=near nominc strmer streq nostkchk saveds ign=73
Slink FROM LIB:c.o,playmf.o TO playmf LIBRARY LIB:SC.lib,LIB:Amiga.lib ND NOICONS
quit
*/

/*=======================================================================

This file is a prototype score player which reads standard MIDI files,
format type 0 or 1.  It conforms to the 1.0 specification for standard 
MIDI files by Dave Oppenheim of Opcode Systems.  It uses both the CAMD and 
RealTime libraries by Roger Dannenberg, Bill Barton, David Joiner, et al.

Design and coding by Dan Baker, Commodore Business Machines.

The tempo handling used by this program is a bit crude and is known to have 
deficiencies.  It should not be used as an example of proper tempo handling.  
This program also makes extensive use of globals and other techniques which 
are convenient to the program's author.  Apologies to the reader.
=======================================================================*/

/* System Include Files */
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>

/* CAMD MIDI Library Includes */
#include <clib/camd_protos.h>
#include <midi/camd.h>
#include <midi/camdbase.h>
#include <midi/mididefs.h>
#include <pragmas/camd_pragmas.h>

/* CAMD Real Time Library Includes */
#include <clib/realtime_protos.h>
#include <libraries/realtime.h>
#include <pragmas/realtime_pragmas.h>

/* System function prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

/* Lattice Standard I/O */
#include <stdlib.h>
#include <stdio.h>


/**********    debug macros     ***********/
#define MYDEBUG  1
void kprintf(UBYTE *fmt,...);
void dprintf(UBYTE *fmt,...);
#define DEBTIME 0
#define bug printf
#if MYDEBUG
#define D(x) (x); if(DEBTIME>0) Delay(DEBTIME);
#else
#define D(x) ;
#endif /* MYDEBUG */
/********** end of debug macros **********/


#define MINARGS 2

UBYTE *vers = "\0$VER: playmf 40.1";
UBYTE *usage = "Usage: playmf midifilename";

#ifdef __SASC
void __regargs __chkabort(void) {}      /* Disable SAS CTRL-C checking. */
#else
#ifdef LATTICE
void chkabort(void) {}                  /* Disable LATTICE CTRL-C checking */
#endif
#endif

/*-------------------*/
/*     Prototypes    */
/*-------------------*/
void kill(char *killstring);
ULONG ComVarLen (UBYTE *value);
UBYTE *DecodeEvent(UBYTE *,struct DecTrack *, ULONG);
LONG transfer(struct DecTrack *,ULONG,LONG);
ULONG changetempo(ULONG,ULONG,ULONG);

/*---------------------*/
/* S M F  Header  File */
/*---------------------*/
/* Four-character IDentifier builder*/
#define MakeID(a,b,c,d)  ( (LONG)(a)<<24L | (LONG)(b)<<16L | (c)<<8 | (d) )
#define MThd MakeID('M','T','h','d')
#define MTrk MakeID('M','T','r','k')

struct SMFHeader {LONG     ChunkID;  /* 4 ASCII characters */
                  LONG     VarLeng;
                  WORD     Format;
                  UWORD    Ntrks;
                  WORD     Division;
                 };


struct DecTrack { ULONG absdelta;   /* 32-bit delta */
                  ULONG nexclock;   /* storage */
                  UBYTE status;     /* status from file */
                  UBYTE rstatus;    /* running status from track */
                  UBYTE d1;         /* data byte 1 */
                  UBYTE d2;         /* data byte 2 */
                  ULONG absmlength; /* 32-bit absolute metalength */
                  UBYTE *endmarker;
                  UBYTE metatype;   /* meta event type */
                  BOOL playable;
                  UBYTE pad3;
                };

/*------------------*/
/* MIDI Header File */
/*------------------*/
#define MAXTRAX 64L
#define TWOxMAXTRAX 128L
#define MIDIBUFSIZE 512L

/* Library Bases */
struct Library  *CamdBase,
                *RealTimeBase;

/* Compiler glue: stub functions for camd.library */
struct MidiNode *CreateMidi(Tag tag, ...)
{	return CreateMidiA((struct TagItem *)&tag );
}

BOOL SetMidiAttrs(struct MidiNode *mi, Tag tag, ...)
{	return SetMidiAttrsA(mi, (struct TagItem *)&tag );
}

struct MidiLink *AddMidiLink(struct MidiNode *mi, LONG type, Tag tag, ...)
{	return AddMidiLinkA(mi, type, (struct TagItem *)&tag );
}


/* Compiler glue: stub functions for realtime.library */
BOOL SetPlayerAttrs(struct Player *pi, Tag tag, ...)
{	return SetPlayerAttrsA(pi, (struct TagItem *)&tag );
}

struct Player *CreatePlayer(Tag tag, ...)
{	return CreatePlayerA( (struct TagItem *)&tag );
}

#define TSHIFT  4

/*--------------*/
/*   Globals    */
/*--------------*/
struct MidiLink   *pMidiLink;
struct MidiNode   *pMidiNode;
struct Player	  *pPlayer;
BPTR               smfhandle;
UBYTE             *smfdata,*ptrack[TWOxMAXTRAX],*pData,trackct;
LONG               midiSignal,smfdatasize,tempo_offs;
ULONG		   fillclock[2];
ULONG              oldclock,sizeDTrack,tfactor,initfactor,division,donecount;
UBYTE             *pfillbuf[2],lastRSchan;
BOOL		  Playing;


#define ALLNOTESOFFLEN	48
UBYTE AllNotesOff[ALLNOTESOFFLEN] = {
MS_Ctrl | 0,  MM_AllOff, 0,
MS_Ctrl | 1,  MM_AllOff, 0,
MS_Ctrl | 2,  MM_AllOff, 0,
MS_Ctrl | 3,  MM_AllOff, 0,
MS_Ctrl | 4,  MM_AllOff, 0,
MS_Ctrl | 5,  MM_AllOff, 0,
MS_Ctrl | 6,  MM_AllOff, 0,
MS_Ctrl | 7,  MM_AllOff, 0,
MS_Ctrl | 8,  MM_AllOff, 0,
MS_Ctrl | 9,  MM_AllOff, 0,
MS_Ctrl | 10, MM_AllOff, 0,
MS_Ctrl | 11, MM_AllOff, 0,
MS_Ctrl | 12, MM_AllOff, 0,
MS_Ctrl | 13, MM_AllOff, 0,
MS_Ctrl | 14, MM_AllOff, 0,
MS_Ctrl | 15, MM_AllOff, 0
};

/*-----------------------------*/
/*     CODE  Starts  Here      */
/*-----------------------------*/

ULONG
ComVarLen (value)
UBYTE *value;
{
   register ULONG newval;
   register UBYTE x;

   x=0;newval=0L;
   while(x<4)
   {
      newval <<= 7;
      newval |=  *(value+x) & 0x7f;
      if(*(value+x) < 0x80)x=4;
      x++;
   }
   return(newval);
}


void
main(int argc, char **argv)
{
   char              *smfname,iobuffer[14];
   struct SMFHeader  *pSMFHeader;
   UBYTE             *pbyte,x;
   WORD               w;
   LONG               rdcount,y,z,res;
   BOOL               notdone,timerr;
   ULONG              masterswitch,lowclock,
                      ylength[2],wakeup;

   BYTE               oldpri; /* Priority to restore */
   struct Task       *mt;     /* Pointer to this task */

   UBYTE fillbuf1[MIDIBUFSIZE]; /* These buffers hold the notes translated */
   UBYTE fillbuf2[MIDIBUFSIZE]; /* from the midifile file for playback          */
   struct DecTrack *pDTrack[MAXTRAX];


   if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?')))
	{
	printf("%s\n",usage);
	exit(RETURN_OK);
	}

   oldclock=0L;
   smfdata=NULL;
   smfhandle=0;
   pMidiLink=NULL;
   pMidiNode=NULL;
   pPlayer=NULL;
   RealTimeBase=NULL;
   CamdBase=NULL;
   mt=NULL;
   notdone=TRUE;
   pfillbuf[0]=fillbuf1;
   pfillbuf[1]=fillbuf2;
   fillclock[0]=0L;
   fillclock[1]=0L;
   donecount=0L;
   lastRSchan=0xf1; /* Status of $F1 is undefined in Standard MIDI file Spec */
   tempo_offs = 0;
   tfactor= 12 << TSHIFT;
   Playing = FALSE;

   /*--------------------------------------*/
   /* Open the CAMD and RealTime libraries */
   /*--------------------------------------*/
   CamdBase=OpenLibrary("camd.library",0L);
   if(!CamdBase)
      kill("Can't open CAMD MIDI Library\n");

   RealTimeBase=OpenLibrary("realtime.library",0L);
   if(!RealTimeBase)
      kill("Can't open CAMD Real Time Library\n");

   /*---------------*/
   /* Open the File */
   /*---------------*/
   if(argv[1]==NULL) 
       kill("No file name given.\n");
   smfname=argv[1];
   smfhandle= Open( (UBYTE *)smfname , MODE_OLDFILE );
   if(smfhandle==0)
       kill("Cannot open file.\n");


   /*-------------------------------*/
   /* Read the SMF Header ID/Length */
   /*-------------------------------*/
   rdcount = Read(smfhandle,iobuffer,14);
   if(rdcount==-1) 
       kill ("Bad file during read.\n");
   if(rdcount<14)  
       kill ("Not a midifile, too short.\n");

   /*-----------------*/
   /* Evaluate Header */
   /*-----------------*/
   pSMFHeader=(struct SMFHeader *)iobuffer;
   if (pSMFHeader->ChunkID != MThd ) 
      kill("Midifile has unknown header ID.\n");

   if (pSMFHeader->VarLeng != 6 ) 
      kill("Midifile has unknown header.\n");

   if (pSMFHeader->Format == 0 ) 
      printf("Parsing midifile format type 0\n");
   else if (pSMFHeader->Format == 1 ) 
      printf("Parsing midifile format type 1\n");
   else                          
      kill("Can't parse this midifile type.\n");


   if(pSMFHeader->Ntrks >MAXTRAX )
      kill("Midifile has more than MAXTRAX tracks.\n");
   else
      printf("Midifile has %ld tracks\n",pSMFHeader->Ntrks);


   /*--------------------*/
   /* Evaluate time base */
   /*--------------------*/
   if (pSMFHeader->Division < 0)
   {
      /* Real time: find frame rate */
      w=(pSMFHeader->Division >> 8) & 0x007f;
      if(w!=24 && w!=25 && w!=29 && w!=30)
         kill("Non-metrical time specified; not MIDI/SMPTE frame rate\n");
      else 
         kill("Non-metrical time; MIDI/SMPTE frame rate\n");

      /* Real-time: find seconds resolution */
      w=pSMFHeader->Division & 0x007f;
      if(w==4)
         kill("Non-metrical time in quarter seconds (MTC resolution)\n");
      else if(w==8 || w==10 || w==80)
         kill("Non-metrical time in 1/nths of a second (bit resolution)\n");
      else
         kill("Non-metrical time in 1/nths of a second\n");
   }
   else
   {
      printf("Midifile has 1/%ldth quarter notes per delta tick\n",
              pSMFHeader->Division);
      division=(ULONG)pSMFHeader->Division;
 
      /* According to "Standrd MIDI Files 1.0", July 1988, page 5 */
      /* para. 4: "...time signature is assumed to be 4/4 and the */
      /*           tempo 120 beats per minute."                   */
      tfactor=changetempo(500000L,tfactor,0L); /* One quarter note every half second */
   }


   /*--------------------------------------*/
   /* Calculate size and Read rest of file */
   /*--------------------------------------*/
   y = Seek(smfhandle,0L,OFFSET_END);
   if(y==-1)
      kill("Midifile or drive problem\n");
   z = Seek(smfhandle,y,OFFSET_BEGINNING);
   if(z==-1)
      kill("Midifile or drive problem\n");

   smfdatasize=z-y; /* CDTV demo disk uses 12000 here */

   smfdata=AllocMem(smfdatasize,MEMF_PUBLIC | MEMF_CLEAR);
   if(smfdata==NULL) 
      kill("No memory for Midifile read.\n");

   rdcount = Read(smfhandle,smfdata,smfdatasize);
   if(rdcount==-1)          
      kill ("Bad file during read.\n");

   /* Commented out for CD ROM demo */
   if(rdcount<smfdatasize)  
      kill ("Midifile too short.\n");


   /*----------------------*/
   /* Find the MIDI tracks */
   /*----------------------*/
   trackct=0;
   pbyte=smfdata;

   while((pbyte-smfdata < smfdatasize) && (trackct < MAXTRAX))
   {
      if((*pbyte=='M')&&(*(pbyte+1)=='T')&&
         (*(pbyte+2)=='r')&&(*(pbyte+3)=='k'))
      {
         /* Beginning of track */
         ptrack[trackct]=pbyte+8;
         /* End of track marker */
         ptrack[MAXTRAX+trackct-1]=pbyte;
         trackct++;
         pbyte+=4;
      }
      else pbyte++;
   }

   /* End of track marker */   
   ptrack[MAXTRAX+trackct-1]=pbyte;

   if(trackct != pSMFHeader->Ntrks)
      kill("Too many or missing tracks\n");
   printf("There are %ld tracks in this Midifile.\n",trackct);


   /*----------------------------------------------*/
   /* Set up a MidiNode and a MidiLink.  Link the  */
   /* node to the default "out.0" MidiCluster .    */
   /*----------------------------------------------*/
   pMidiNode=CreateMidi( MIDI_Name,    "PlayMF Player",
                         MIDI_MsgQueue, 0L,     /* This is a send-only   */
                         MIDI_SysExSize,0L,     /* MIDI node so no input */
                         TAG_END);              /* buffers are needed.   */
   if(!pMidiNode)
      kill("No memory for MIDI Node\n");

   pMidiLink=AddMidiLink( pMidiNode, MLTYPE_Sender, 
                                     MLINK_Comment,  "PlayMF Player Link",
                                     MLINK_Parse,    TRUE,
                                     MLINK_Location, "out.0",
                                     TAG_END);
   if(!pMidiLink)
      kill("No memory for MIDI Link\n");


   /*----------------------------------------*/
   /* Set up DTrack structure for each track */
   /*----------------------------------------*/
   sizeDTrack=trackct*sizeof(struct DecTrack);
   pData=AllocMem( sizeDTrack, MEMF_PUBLIC | MEMF_CLEAR );
   if(!pData)
      kill("No memory for work area...\n");

   for(x=0;x<trackct;x++)
   {
      pDTrack[x]=(struct DecTrack *)
                 (x * sizeof(struct DecTrack) + pData);
      /* add end marker */
      pDTrack[x]->endmarker = ptrack[MAXTRAX+x];
   }


   /*------------------------------------------------*/  
   /* Get events from track into DecTrack structures */
   /*------------------------------------------------*/
   y=0;
   masterswitch=0L;
   lowclock=0xffffffff;

   /* Initialize DecTrack structures */
   for(x=0;x<trackct;x++)
   {  
      /* Takes a pointer to the delta of a raw <MTrk event>, a pointer   */
      /* to a DecTrack decoding structure to store the decoded event and */ 
      /* a switch that tells which of the two buffers to use.  Returns a */
      /* pointer to the next raw <MTrk event> in the track or 0 if the   */
      /* track is exhausted.                                             */
      ptrack[x] = DecodeEvent( ptrack[x] , pDTrack[x] , masterswitch );
      if(pDTrack[x]->nexclock < lowclock && ptrack[x])
         /* Find the first event */
         lowclock=pDTrack[x]->nexclock;
   }

   /*-----------------------------------*/
   /* Transfer first events to A buffer */
   /*-----------------------------------*/
   for(x=0;x<trackct;x++)
   {
      if((pDTrack[x]->nexclock==lowclock) && ptrack[x])
      {
         /* Transfer event to parse buffer and handle successor */
         y=transfer(pDTrack[x],masterswitch,y);
         z=1;
         while(z==1)
         {
            ptrack[x]=DecodeEvent(ptrack[x],pDTrack[x],masterswitch);
            /* Next delta is zero... */
            if( !(pDTrack[x]->absdelta) && ptrack[x])
            {
               y=transfer(pDTrack[x],masterswitch,y);
            }
            else {z=0;}
         }
      }
   }
   ylength[masterswitch]=y;
   fillclock[masterswitch]=(ULONG)(((tfactor*lowclock)+tempo_offs) >> TSHIFT);



   /*------------------------------------*/
   /* Transfer second events to B buffer */
   /*------------------------------------*/
   y=0;
   masterswitch=1L;
   lowclock=0xffffffff;
   for(x=0;x<trackct;x++)
   {
      if(pDTrack[x]->nexclock < lowclock && ptrack[x])
         lowclock=pDTrack[x]->nexclock;
   }

   for(x=0;x<trackct;x++)
   {
      if(pDTrack[x]->nexclock==lowclock && ptrack[x])
      {
         /* Transfer event to parse buffer and handle successor */
         y=transfer(pDTrack[x],masterswitch,y);
         z=1;
         while(z==1)
         {
            ptrack[x]=DecodeEvent(ptrack[x],pDTrack[x],masterswitch);
            /* Next delta is zero... */
            if( !(pDTrack[x]->absdelta) && ptrack[x])
            {
               y=transfer(pDTrack[x],masterswitch,y);
            }
            else {z=0;}
         }
      }
   }

   ylength[masterswitch]=y;
   fillclock[masterswitch]=(ULONG)(((tfactor*lowclock)+tempo_offs) >> TSHIFT);

   /*-----------------------------------------------------*/
   /* Priority Must Be Above Intuition and Graphics Stuff */
   /*-----------------------------------------------------*/ 
   mt=FindTask(NULL);
   oldpri=SetTaskPri(mt,21);


   /*---------------------------------------------------------------*/ 
   /* Set up a Player and a Conductor to get timing information */
   /*---------------------------------------------------------------*/ 
   midiSignal = AllocSignal(-1L);
   if(midiSignal==-1) 
       kill("Couldn't allocate a signal bit\n");

   pPlayer=CreatePlayer(     PLAYER_Name,	"PlayMF Player",
                             PLAYER_Conductor,  "PlayMF Conductor",
                             PLAYER_AlarmSigTask, mt,
                             PLAYER_AlarmSigBit,midiSignal,
                             TAG_END);
   if(!pPlayer)
       kill("Can't create a Player\n");

   /*---------------------------------*/
   /* Make sure the clock is stopped. */
   /*---------------------------------*/
   res = SetConductorState( pPlayer, CONDSTATE_STOPPED, 0L );
   if(res!=0)
       kill("Couldn't stop conductor\n");

   /*-------------------------------------------*/
   /* Play the first batch of notes in Buffer A */
   /*-------------------------------------------*/
   if(ylength[masterswitch^1]!=0)
      {
      ParseMidi(pMidiLink,pfillbuf[masterswitch^1],
                          ylength[masterswitch^1]);
      }

   /*------------------------------------*/
   /* and start the RealTime alarm clock */
   /*------------------------------------*/
   res = SetConductorState( pPlayer, CONDSTATE_RUNNING, 0L );
   if(res!=0)
       kill("Couldn't start conductor\n");

   /*---------------------*/
   /* and set the alarm.  */
   /*---------------------*/
   timerr = SetPlayerAttrs( pPlayer, 
                            PLAYER_AlarmTime, fillclock[masterswitch],
                            PLAYER_Ready, TRUE,
                            TAG_END);
   if(!timerr)
       kill("Couldn't set player attrs\n");

   Playing = TRUE;

   /*-----------------*/
   /* MAIN EVENT LOOP */
   /*-----------------*/   
   while(donecount<trackct)
   {
      masterswitch ^= 1L;
      y=0;
      lowclock=0xffffffff;

      /*------------------------------------------------*/  
      /* Get events from track into DecTrack structures */
      /*------------------------------------------------*/
      for(x=0;x<trackct;x++)
      {
         if((pDTrack[x]->nexclock < lowclock) && ptrack[x])
            lowclock=pDTrack[x]->nexclock;
      }

      /*-----------------------------------*/
      /* Transfer events to current buffer */
      /*-----------------------------------*/
      for(x=0;x<trackct;x++)
      {
         if((pDTrack[x]->nexclock==lowclock) && ptrack[x])
         {
            /* Transfer event to parse buffer and handle successor */
            y=transfer(pDTrack[x],masterswitch,y);
            z=1;
            while(z==1)
            {
               ptrack[x]=DecodeEvent(ptrack[x],pDTrack[x],masterswitch);
               /* Next delta is zero... */
               if( !(pDTrack[x]->absdelta) && ptrack[x] )
               {
                  y=transfer(pDTrack[x],masterswitch,y);
               }
               else {z=0;}
            }
         }
      }

      ylength[masterswitch]=y;
      fillclock[masterswitch]=(LONG)(((tfactor*lowclock)+tempo_offs)>>TSHIFT);

      /*---------------------------------------------------------------*/
      /* Wait() for the CAMD alarm or a CTRL-C keypress from the user. */
      /*---------------------------------------------------------------*/
      if(timerr)
         wakeup=Wait(1L<< midiSignal | SIGBREAKF_CTRL_C);

      if(wakeup & SIGBREAKF_CTRL_C)
      {
	 if(!(wakeup & 1L<<midiSignal)) Wait(1L<< midiSignal | SIGBREAKF_CTRL_C);
         /* Restore Priority */
         if(mt!=NULL) SetTaskPri(mt,oldpri);
         /* And Quit */
         kill("User abort\n");
      }

      /*-------------------------------*/
      /* Start the next set of events  */
      /*-------------------------------*/
      if(ylength[masterswitch^1]!=0)
         {
         ParseMidi(pMidiLink,pfillbuf[masterswitch^1],
                             ylength[masterswitch^1]);
         }

      /*---------------------*/
      /* and set the alarm.  */
      /*---------------------*/
      timerr = SetPlayerAttrs( pPlayer, 
                               PLAYER_AlarmTime, fillclock[masterswitch],
                               PLAYER_Ready, TRUE,
                               TAG_END);
      if(!timerr)
          kill("Couldn't set player attrs 2\n");
   }

   /*-----------------------------------*/
   /* Finish off the last set of events */
   /*-----------------------------------*/
   masterswitch^=1L;

   if(timerr)
      wakeup=Wait(1L<< midiSignal | SIGBREAKF_CTRL_C);


   if(ylength[masterswitch^1]!=0)
      {
      ParseMidi(pMidiLink,pfillbuf[masterswitch^1],
                       ylength[masterswitch^1]);
      }

   /* Restore Priority */
   if(mt!=NULL) SetTaskPri(mt,oldpri);

   kill("");
}


/*------------------------------*/
/* Cleanup and return resources */
/*------------------------------*/
void
kill(char *killstring)
{
   if(Playing)	       ParseMidi(pMidiLink,AllNotesOff,ALLNOTESOFFLEN);
   if(pPlayer)         DeletePlayer(pPlayer);
   if(midiSignal != -1)FreeSignal(midiSignal);
   if(pData)           FreeMem(pData,sizeDTrack);
   if(pMidiLink)       RemoveMidiLink(pMidiLink);
   if(pMidiNode)       DeleteMidi(pMidiNode);
   if(RealTimeBase)    CloseLibrary(RealTimeBase);
   if(CamdBase)        CloseLibrary(CamdBase);
   if(smfhandle)       Close(smfhandle);
   if(smfdata)         FreeMem(smfdata,smfdatasize);
   if(*killstring)     printf(killstring);
   exit(0);
}


/*--------------------------------------------------*/  
/* Translate from raw track data to a decoded event */
/*--------------------------------------------------*/  
UBYTE *DecodeEvent(UBYTE *ptdata,struct DecTrack *pDTdata, ULONG deswitch)
{
   LONG status;
   ULONG length;
   BOOL skipit;

   pDTdata->absdelta = 0L;
   pDTdata->playable = TRUE; /* Assume it's playble and not a meta-event */

   skipit=FALSE;
   do
   {
      /* is this track all used up? */             
      if( ptdata >= pDTdata->endmarker )
      {
         /* printf("Track done,...\n"); */
         donecount++;
         return(0L);
      }
      else /* there is more data to handle in the track */
      {
         /* Decode delta */
         pDTdata->absdelta += ComVarLen(ptdata);
         pDTdata->nexclock+= pDTdata->absdelta;

         /* Update pointer to event following delta */
         while(*ptdata>127)
            {
            ptdata++;
            }
         ptdata++;

         if(*ptdata>127) /* Event with status ($80-$FF): decode new status */  
         {
            status=*ptdata;
         
            pDTdata->status=status;
            pDTdata->rstatus=0;    /* No running status was used */
     
            ptdata++;
            
            if(status<240) /* Handle easy status $8x - $Ex */
            {
               skipit=FALSE;
               pDTdata->d1 = *ptdata;
               if(status<192 || status>223) /* $80-$BF, $E0-$EF: 2 data bytes */
               {
                  ptdata++;
                  pDTdata->d2=*ptdata;
               }
               else pDTdata->d2=0;            /* $C0-$DF: 1 data byte */
            }
            else /* Status byte $Fx, system exclusive or meta events  */
            {
               skipit=TRUE;
               
               if(status==0xff)            /* It's a meta event ($ff) */
               {
                  pDTdata->metatype=*ptdata;
              
                  ptdata++; /* Now pointing at length byte */

                  if(pDTdata->metatype==81)
                  {
                     /* Tempo change event.  There are 60 milllion    */
                     /* microseconds in a minute.  The lower 3 bytes  */ 
                     /* pointed to by ptdata give the microseconds    */
                     /* per MIDI quarter note. So, assuming a quarter */
                     /* note gets the beat, this equation             */
                     /*      60000000L /                              */
                     /*          ( *((ULONG *)ptdata) & 0x00ffffff ) )*/
                     /* gives beats per minute.                       */

                     tfactor = changetempo( *((ULONG *)ptdata) & 0x00ffffff,
				tfactor,
				pDTdata->nexclock - pDTdata->absdelta);

                     /* Tempo event is not playable.  This prevents the */
                     /* event from being transferred to the play buffer */
                     pDTdata->playable = FALSE; 

                     /* Even though this event can't be played, it     */
                     /* takes some time and should not be skipped.     */
                     skipit=FALSE;
                  }
                  length=ComVarLen(ptdata);
                  pDTdata->absmlength=length;
                  while(*ptdata>127)ptdata++;

                  ptdata+=length;
               }
               else if(status==0xf0 || status==0xf7) /* It's a sysex event */
               {
                  pDTdata->metatype=0xff;
                  printf("Sysex event");
                  length=ComVarLen(ptdata);
                  pDTdata->absmlength=length;
                  while(*ptdata>127)ptdata++;

                  ptdata+=length;
               }
               else        /* It's an unkown event type ($f1-$f6, $f8-$fe) */
               {
                  pDTdata->metatype=0xff;
                  printf("Unknown event"); 
               }
            }
         }
         else /* Event without status ($00-$7F): use running status */
         {
            skipit=FALSE;
            /* Running status data bytes */
            status=pDTdata->status;
            pDTdata->rstatus=status;

            if(status==0)
               kill("Bad file, data bytes without initial status.\n");

            pDTdata->d1=*ptdata;

            if(status<192 || status>223) /* $80-$BF, $E0-$EF: 2 data bytes */
            {
               ptdata++;
               pDTdata->d2=*ptdata;
            }
            else pDTdata->d2=0;         /* $C0-$DF: 1 data byte */
         }
         ptdata++;
      }
   }
   while(skipit);

   return(ptdata);
}


/*------------------------------------------------------------*/
/* Transfer the decoded event to the fill buffer for playback */
/*------------------------------------------------------------*/
LONG
transfer(struct DecTrack *pDT,ULONG mswitch,LONG ylen)
{
   ULONG y;
   y=(ULONG )ylen;

   if (pDT->playable)
   {
      if(pDT->rstatus == lastRSchan)
      { 
         /* Running status so just put the 2 data bytes in buffer */
         *(pfillbuf[mswitch] + y)=pDT->d1;
         y++;
         *(pfillbuf[mswitch] + y)=pDT->d2;
      }
      else 
      {
         /* New status so store status and data bytes */
         *(pfillbuf[mswitch] + y)=pDT->status;
         y++;
         *(pfillbuf[mswitch] + y)=pDT->d1;
         if(pDT->status<192 || pDT->status>223)
         {
            y++;
            *(pfillbuf[mswitch] + y)=pDT->d2;
         }
         lastRSchan=pDT->status;     
      }
      y++;
   }
   return((LONG)y);
}


/*-------------------------------------------------------------------------*/
/* Handle the Change Tempo event.   With the realtime.library, the timing  */
/* quantum is fixed at .83 milliseconds (1.2kHz).  This makes handling of  */
/* Playmf tempo a bit rough.  Tempo is controlled through a ULONG integer  */
/* named tfactor which is used to multiply the time deltas in the Playmf   */
/* file.  We can't multiply the time deltas by a fractional amount so we   */
/* will shift up before dividing down for tfactor, and shift down after    */
/* using the tfactor later.  This in effect gives us fractional control.   */
/* To deal with the fact that tempo changes would cause us to wait for     */
/* inappropriate past or future times on upcoming notes, we calculate a    */
/* tempo_offs time offset (also shifted up) for use in time calculations.  */
/*-------------------------------------------------------------------------*/
ULONG
changetempo(ULONG ctbpm, ULONG oldtfactor, ULONG eventtime)
{
   ULONG newtfactor,oldtime,newtime;

   /* ctbpm is the new microseconds per midi quarter note                */
   /* Division is the part of a quarter note represented by a delta of 1 */
   /* So ctbpm/division is the new microseconds per delta                */
   /* CAMD uses 1200 ticks/sec or 833 microseconds per tick              */
   /* We will compute a value which is:                                  */
   /* ((microseconds per delta) << TSHIFT ) / 833                        */
   /* The TSHIFT will give us some fractional control, and will be       */
   /*    shifted out later when the new tfactor) is used.                */

   newtfactor = ((ctbpm << TSHIFT) / division) / 833; /* new tfactor */

   if((eventtime)&&(Playing))
	{
	/* We know our midi event time relative to the start of the file.
	 * We can calculate what CAMD time that equals at our initial tempo,
	 * and also at the new tempo.  We will save this difference as
	 * tempo_offs, and use tempo_offs to adjust all main program
	 * midi time -> CAMD time calculations.
	 * In other words, we figure out what CAMD time we would have
	 * been at now, if we had been playing at the new tempo all along.
	 * Using that information, we can adjust all upcoming MIDI
	 * Alarm wakeup times up or down by the amount the new tempo
	 * tfactor would have thrown us off.  Note that tempo_offs is
	 * based on values shifted up by TSHIFT, and this will be shifted
	 * out later in the main program time calculations.
	 */
	oldtime = initfactor * eventtime; 
	newtime = newtfactor * eventtime;
	tempo_offs = oldtime - newtime;
	/*printf("playtime=%ld oldtime=%ld newtime=%ld tempo_offs = %ld\n",
		playtime, oldtime>>TSHIFT,newtime>>TSHIFT,tempo_offs>>TSHIFT); */
	}
   else
	{
	initfactor = newtfactor;
	}
   /* printf("changetempo: ctbpm=%ld newtfactor=%ld\n", ctbpm,newtfactor); */
   return(newtfactor);
}


