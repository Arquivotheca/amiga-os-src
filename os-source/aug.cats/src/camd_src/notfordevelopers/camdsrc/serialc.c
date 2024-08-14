/************************************************************************
*     C. A. M. D.	(Commodore Amiga MIDI Driver)                   *
*************************************************************************
*									*
* Design & Development	- Roger B. Dannenberg				*
*			- Jean-Christophe Dhellemmes			*
*			- Bill Barton					*
*                       - Darius Taghavy                                *
*                                                                       *
* Copyright 1990 by Commodore Business Machines				*
*************************************************************************
*                                                                       *
* serialc.c   - Amiga internal serial port I/O functions                *
*                                                                       *
************************************************************************/
/* SYSTEM HEADERS */
#include <exec/types.h>
#include <exec/execbase.h>	/* for FlushDevice() */
#include <exec/interrupts.h>
#include <hardware/cia.h>	/* for DTR bit */
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <midi/camddevices.h>
#include <resources/misc.h>

/* CAMD HEADERS */
#include "camdlib.h"


/* CAMD DEFINES */
#define MIDIBPS       31250	    /* MIDI baud rate */

#define INTF_SET      INTF_SETCLR
#define INTF_CLR      0


/* CAMD DATA STRUCTURES */
struct SerInt 
{
   struct Interrupt Int;
   struct Interrupt *OldInt;
};

static struct SerInt SerXmitInt, SerRecvInt;
static UWORD oldintena; 	/* used to preserve original intena TBE and RBF settings */

/* hardware */
extern struct Custom far custom;
extern struct CIA far ciab;	/* for DTR line */

/* FUNCTION PROTOTYPES */
/* seriala.asm */
extern void IntSerXmitHandler(), IntSerRecvHandler();   /* serial interrupt handlers */
extern void ActivateIntSerXmit();
extern void WaitIntSer(void);

/* internal */
static BOOL AllocSerial (char *name);
static BOOL AllocSerMisc (char *name);
static void FreeSerial (void);
static void FlushDevice (char *name);

/***************************************************************
*
*   OpenIntSer
*
*   FUNCTION
*	Gain low-level access to Amiga internal serial port.
*
*   INPUTS
*	data - Data pointer for interrupts.
*
*   RESULTS
*	Pointer to ActivateIntSerXmit function on success.
*
*  changed: dart
***************************************************************/

struct MidiPortData *OpenIntSer(APTR data)
{
static struct MidiPortData mpd = { ActivateIntSerXmit };
register struct Custom *cust;
char *name;

   cust = &custom;
   name = CamdBase->Public.Lib.lib_Node.ln_Name;

   SerXmitInt.Int.is_Node.ln_Name = name;
   SerXmitInt.Int.is_Node.ln_Type = NT_INTERRUPT;
   SerXmitInt.Int.is_Code = IntSerXmitHandler;
   SerXmitInt.Int.is_Data = data;

   SerRecvInt.Int.is_Node.ln_Name = name;
   SerRecvInt.Int.is_Node.ln_Type = NT_INTERRUPT;
   SerRecvInt.Int.is_Code = IntSerRecvHandler;
   SerRecvInt.Int.is_Data = data;

   if (AllocSerial(name))
      {
      oldintena = cust->intenar & (INTF_TBE | INTF_RBF);
      cust->intena = INTF_CLR | INTF_TBE | INTF_RBF;  /* disable interrupts */
      cust->intreq = INTF_CLR | INTF_TBE | INTF_RBF;  /* clear any pending interrupts */

      /* SET MIDI BAUD RATE */
   /* bits 0-14: period.  bit 15: 9/8 bit selection, serper is period-1 */
      cust->serper = (EClockFreq() * 5 + MIDIBPS/2) / MIDIBPS - 1;       

      SerXmitInt.OldInt = SetIntVector (INTB_TBE, &SerXmitInt.Int);
      SerRecvInt.OldInt = SetIntVector (INTB_RBF, &SerRecvInt.Int);

      cust->intena = INTF_SET | INTF_RBF;     /* enable RBF */
      cust->intreq = INTF_SET | INTF_TBE;     /* set a pending TBE */

   	/* ----------------------------------------------------------
         Leaving TBE disabled and priming the interrupt permit the
         xmit handler to simply enable the TBE interrupt when it's
         time to send the first byte.  Using this method, the xmit
	 code never needs to prime the transmitter by actually
	 sending a byte.  The final TBE interrupt request from a
	 stream of bytes is deferred (i.e. not cleared) until there
	 are more bytes to send.
         ---------------------------------------------------------- */

      ciab.ciapra &= ~CIAF_COMDTR; /* turn on DTR line (active low) */
      return &mpd;
      }
   return NULL;
}


/***************************************************************
*
*   CloseIntSer
*
*   FUNCTION
*	Release low-level access to Amiga internal serial port.
*
*   INPUTS
*	None
*
*   RESULTS
*	None
*
***************************************************************/

void CloseIntSer(void)
{
   WaitIntSer();                 /* wait for hardware to clear */

   ciab.ciapra |= CIAF_COMDTR; 			/* turn off DTR (active low) */

   custom.intena = INTF_CLR | INTF_TBE | INTF_RBF;	/* disable these interrupts */
   custom.intreq = INTF_CLR | INTF_TBE | INTF_RBF;	/* clear pending interrupts */

   /* restore previous interrupts */
   SetIntVector (INTB_TBE, SerXmitInt.OldInt);
   SetIntVector (INTB_RBF, SerRecvInt.OldInt);

   if (oldintena) custom.intena = INTF_SET | oldintena;

   FreeSerial();
}


/*****************************
* serial resource allocation * --------------------------------------------
*****************************/
static BOOL AllocSerial(char *name)
{
BOOL gotit;

   Forbid();
   if (!(gotit = AllocSerMisc(name))) 
      {
      /* ----------------------------------------------------
         A bug in 1.3 in serial.device does not free up
         misc.resouce, so we'll free up serial misc.resource 
         channels by doing a RemDevice() on serial.device 
         ---------------------------------------------------- */
      FlushDevice ("serial.device"); 
      gotit = AllocSerMisc(name);
      }
   Permit();

   return gotit;
}

/***************************
* misc resource allocation * ----------------------------------------------
***************************/
static BOOL AllocSerMisc (char *name)
{
   if (AllocMiscResource (MR_SERIALBITS,name)) return FALSE;

   if (AllocMiscResource (MR_SERIALPORT,name)) 
      {
      FreeMiscResource (MR_SERIALBITS);
      return FALSE;
      }

   return TRUE;
}

static void FreeSerial(void)
{
   FreeMiscResource (MR_SERIALBITS);
   FreeMiscResource (MR_SERIALPORT);
}

static void FlushDevice (char *name)
{
struct Device *dev;

   Forbid();
   if (dev = (struct Device *)
      FindName(&((struct ExecBase *)CamdBase->SysBase)->DeviceList, name))
      RemDevice(dev);
   Permit();
}
/* eof */
