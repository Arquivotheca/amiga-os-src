
/****************************************************************************
*
* janusreg.h -- janus hardware registers (from amiga point of view)
*
* Copyright (c) 1986, Commodore Amiga Inc., All rights reserved.
*
****************************************************************************/

/* hardware interrupt bits all bits are active low */
#define JINTB_MINT	(0)	    /* mono video ram written to */
#define JINTB_GINT	(1)	    /* color video ram written to */
#define JINTB_CRT1INT	(2)	    /* mono video control registers changed */
#define JINTB_CRT2INT	(3)	    /* color video control registers changed */
#define JINTB_ENBKB	(4)	    /* keyboard ready for next character */
#define JINTB_LPT1INT	(5)	    /* parallel control register */
#define JINTB_COM2INT	(6)	    /* serial control register */
#define JINTB_SYSINT	(7)	    /* software int request */

#define JINTF_MINT	(1<<0)
#define JINTF_GINT	(1<<1)
#define JINTF_CRT1INT	(1<<2)
#define JINTF_CRT2INT	(1<<3)
#define JINTF_ENBKB	(1<<4)
#define JINTF_LPT1INT	(1<<5)
#define JINTF_COM2INT	(1<<6)
#define JINTF_SYSINT	(1<<7)







/*
** The amiga side of the janus board has four sections of its address space.
** Three of these parts are different arrangements of the same memory.	The
** fourth part has the specific amiga accesible IO registers (jio_??).
** The other three parts all contain the same data, but the data is arranged
** in different ways: Byte Access lets the 68k read byte streams written
** by the 8088, Word Access lets the 68k read word streams written by the
** 8088, and Graphic Access lets the 68k read medium res graphics memory
** in a more efficient manner (the pc uses packed two bit pixels; graphic
* access rearranges these data bits into two bytes, one for each bit plane).
*/

#define ByteAccessOffset	0x00000
#define WordAccessOffset	0x20000
#define GraphicAccessOffset	0x40000
#define IoAccessOffset		0x60000

/*
** within each bank of memory are several sub regions.	These are the
** definitions for the sub regions
*/

#define BufferOffset		0x00000
#define ColorOffset		0x10000
#define ParameterOffset 	0x18000
#define MonoVideoOffset 	0x1c000
#define IoRegOffset		0x1e000

#define BufferSize		0x10000
#define ParameterSize		0x04000


/*
** These are the definitions for the io registers.  All the registers are byte
** wide and the address are for Byte Access addresses
*/
#define jio_KeyboardData    0x061f     /* data that keyboard will read */
#define jio_SystemStatus    0x003f     /* pc only register */
#define jio_NmiEnable	    0x005f     /* pc only register */

#define jio_Com2XmitData    0x007d
#define jio_Com2ReceiveData 0x009d
#define jio_Com2IntEnableW  0x00bd
#define jio_Com2IntEnableR  0x00dd
#define jio_DivisorLSB	    0x007f
#define jio_DivisorMSB	    0x009f
#define jio_Com2IntID	    0x00ff
#define jio_Com2LineCntrl   0x011f
#define jio_Com2ModemCntrl  0x013f
#define jio_Com2LineStatus  0x015f
#define jio_Com2ModemStatus 0x017f

#define jio_Lpt1Data	    0x019f
#define jio_Lpt1Status	    0x01bf
#define jio_Lpt1Control     0x01df

#define jio_MonoAddressInd  0x01ff  /* current index into crt data regs */
#define jio_MonoData	    0x02a1  /* every other byte for 16 registers */
#define jio_MonoControlReg  0x02ff

#define jio_ColorAddressInd 0x021f  /* current index into crt data regs */
#define jio_ColorData	    0x02c1  /* every other byte for 16 registers */
#define jio_ColorControlReg 0x023f
#define jio_ColorSelectReg  0x025f
#define jio_ColorStatusReg  0x027f

#define jio_DisplaySystemReg 0x029f


#define jio_IntReq	    0x1ff1     /* read clears, pc -> amiga ints */
#define jio_PcIntReq	    0x1ff3     /* r/o, amiga -> pc ints */
#define jio_ReleasePcReset  0x1ff5     /* r/o, strobe release pc's reset */
#define jio_RamSize	    0x1ff7     /* r/o, give ram addresses */
#define jio_IntEna	    0x1ff9     /* r/w, enables pc int lines */
#define jio_PcIntGen	    0x1ffb     /* w/o, bit == 0 -> cause pc int */
#define jio_Control	    0x1ffd     /* w/o, random control lines */
#define jio_RamBaseAddr     0x1fff     /* r/w, sets extra ram base address */

/* now the magic bits in each register (and boy, are there a lot of them!) */

/* bits for PcIntReq, PcIntGen registers */
#define JPCINTB_IRQ1	(0)	    /* active high */
#define JPCINTB_IRQ3	(1)	    /* active low */
#define JPCINTB_IRQ4	(2)	    /* active low */
#define JPCINTB_IRQ7	(3)	    /* active low */

#define JPCINTF_IRQ1	(1<<0)
#define JPCINTF_IRQ3	(1<<1)
#define JPCINTF_IRQ4	(1<<2)
#define JPCINTF_IRQ7	(1<<3)

/* pc side interrupts */
#define JPCKEYINT   (0xff)	/* keycode available */
#define JPCSENDINT  (0xfc)	/* system request */
#define JPCLPT1INT  (0xf6)	/* parallel port acknowledge */

/* bits for RamSize */
#define JRAMB_EXISTS	(0)	   /* set if there is any ram at all */
#define JRAMB_2MEG	(1)	   /* set if 2 meg, clear if 1/2 meg */

#define JRAMF_EXISTS	(1<<0)
#define JRAMF_2MEG	(1<<1)

/* bits for control register */
#define JCNTRLB_ENABLEINT   (0)     /* enable amiga interrupts */
#define JCNTRLB_DISABLEINT  (1)     /* disable amiga interrupts */
#define JCNTRLB_RESETPC     (2)     /* reset the pc. remember to strobe */
				    /*	ReleasePcReset afterwards   */
#define JCNTRLB_CLRPCINT    (3)     /* turn off all amiga->pc ints */


/* constants for sizes of various janus regions */
#define JANUSTOTALSIZE	(512*1024)	/* 1/2 megabyte */
#define JANUSBANKSIZE	(128*1024)	/* 128K per memory bank */
#define JANUSNUMBANKS	(4)		/* four memory banks */
#define JANUSBANKMASK	(0x60000)	/* mask bits for bank region */


