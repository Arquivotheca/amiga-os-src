#ifndef JANUS_JANUSREG_H
#define JANUS_JANUSREG_H

/************************************************************************
 * (Amiga side file)
 *
 * janusreg.h -- janus hardware registers (from amiga point of view)
 *
 * Copyright (c) 1986, Commodore Amiga Inc., All rights reserved.
 *
 * 7-15-88 - Bill Koester - Modified for self inclusion of required files
 *10-23-89 - Bill Koester - Added DISKTO[PC|AMIGA] bits for floppy switch
 ************************************************************************/

/* hardware interrupt bits (all bits are active low)                    */

#define JINTB_MINT    (0)     /* mono video ram written to              */
#define JINTB_GINT    (1)     /* color video ram written to             */
#define JINTB_CRT1INT (2)     /* mono video control registers changed   */
#define JINTB_CRT2INT (3)     /* color video control registers changed  */
#define JINTB_ENBKB   (4)     /* keyboard ready for next character      */
#define JINTB_LPT1INT (5)     /* parallel control register              */
#define JINTB_COM2INT (6)     /* serial control register                */
#define JINTB_SYSINT  (7)     /* software int request                   */

#define JINTF_MINT    (1<<0)
#define JINTF_GINT    (1<<1)
#define JINTF_CRT1INT (1<<2)
#define JINTF_CRT2INT (1<<3)
#define JINTF_ENBKB   (1<<4)
#define JINTF_LPT1INT (1<<5)
#define JINTF_COM2INT (1<<6)
#define JINTF_SYSINT  (1<<7)


/* These are the defs for the io registers.  All the registers are byte */
/* wide and the address are for Byte Access addresses                   */

#define jio_1000KeyboardData  0x061f /* data that keybd will read in a1000 */
#define jio_2000KeyboardData  0x1fff /* data that keybd will read in a2000 */

#define jio_SystemStatus      0x003f /* pc only register                   */
#define jio_NmiEnable         0x005f /* pc only register                   */

#define jio_Com2XmitData      0x007d
#define jio_Com2ReceiveData   0x009d
#define jio_Com2IntEnableW    0x00bd
#define jio_Com2IntEnableR    0x00dd
#define jio_Com2DivisorLSB    0x007f
#define jio_Com2DivisorMSB    0x009f
#define jio_Com2IntID         0x00ff
#define jio_Com2LineCntrl     0x011f
#define jio_Com2ModemCntrl    0x013f
#define jio_Com2LineStatus    0x015f
#define jio_Com2ModemStatus   0x017f

#define jio_Lpt1Data          0x019f /* data byte                          */
#define jio_Lpt1Status        0x01bf /* see equates below                  */
#define jio_Lpt1Control       0x01df /* see equates below

#define jio_MonoAddressInd    0x01ff /* current index into crt data regs   */
#define jio_MonoData          0x02a1 /* every other byte for 16 registers  */
#define jio_MonoControlReg    0x02ff

#define jio_ColorAddressInd   0x021f /* current index into crt data regs   */
#define jio_ColorData         0x02c1 /* every other byte for 16 registers  */
#define jio_ColorControlReg   0x023f
#define jio_ColorSelectReg    0x025f
#define jio_ColorStatusReg    0x027f

#define jio_DisplaySystemReg  0x029f

#define jio_IntReq            0x1ff1 /* read clears, pc -> amiga ints      */
#define jio_PcIntReq          0x1ff3 /* r/o, amiga -> pc ints              */
#define jio_ReleasePcReset    0x1ff5 /* r/o, strobe release pc's reset     */
#define jio_PCconfiguration   0x1ff7 /* r/w, give/set PC config.           */
#define jio_IntEna            0x1ff9 /* r/w, enables pc int lines          */
#define jio_PcIntGen          0x1ffb /* w/o, bit == 0 -> cause pc int      */
#define jio_Control           0x1ffd /* w/o, random control lines          */

/* now the magic bits in each register (and boy, are there a lot of them!) */

/* bits for Lpt1Status register                                            */

#define JPCLSB_STROBE    (0)
#define JPCLSB_AUTOFEED  (1)
#define JPCLSB_INIT      (2)
#define JPCLSB_SELECTIN  (3)
#define JPCLSB_IRQENABLE (4)     /* active 1                               */

#define JPCLSF_STROBE    (1<<0)
#define JPCLSF_AUTOFEED  (1<<1)
#define JPCLSF_INIT      (1<<2)
#define JPCLSF_SELECTIN  (1<<3)
#define JPCLSF_IRQENABLE (1<<4)

/* bits for Lpt1Control register                                           */

#define JPCLCB_ERROR   (3)
#define JPCLCB_SELECT  (4)
#define JPCLCB_NOPAPER (5)
#define JPCLCB_ACK     (6)
#define JPCLCB_BUSY    (7)

#define JPCLCF_ERROR   (1<<3)
#define JPCLCF_SELECT  (1<<4)
#define JPCLCF_NOPAPER (1<<5)
#define JPCLCF_ACK     (1<<6)
#define JPCLCF_BUSY    (1<<7)

/* bits for PcIntReq, PcIntGen registers                                   */

#define JPCINTB_IRQ1 (0)         /* active high                            */
#define JPCINTB_IRQ3 (1)         /* active low                             */
#define JPCINTB_IRQ4 (2)         /* active low                             */
#define JPCINTB_IRQ7 (3)         /* active low                             */

#define JPCINTF_IRQ1 (1<<0)
#define JPCINTF_IRQ3 (1<<1)
#define JPCINTF_IRQ4 (1<<2)
#define JPCINTF_IRQ7 (1<<3)

/* pc side interrupts                                                      */

#define JPCKEYINT   (0xff)       /* keycode available                      */
#define JPCSENDINT  (0xfc)       /* system request                         */
#define JPCLPT1INT  (0xf6)       /* printer acknowledge                    */


/* bits for RamSize                                                        */

#define JRAMB_EXISTS (0)         /* unset if there is any ram at all       */
#define JRAMB_2MEG   (1)         /* set if 2 meg, clear if 1/2 meg         */

#define JRAMF_EXISTS (1<<0)      /* unset if there is any ram at all       */
#define JRAMF_2MEG   (1<<1)      /* set if 2 meg, clear if 1/2 meg         */

/* bits for control register                                               */

#define JCNTRLB_ENABLEINT   (0)   /* enable amiga interrupts               */
#define JCNTRLB_DISABLEINT  (1)   /* disable amiga interrupts              */
#define JCNTRLB_RESETPC     (2)   /* reset the pc. remember to strobe      */
                                  /* ReleasePcReset afterwards             */
#define JCNTRLB_CLRPCINT    (3)   /* turn off all amiga->pc ints (except   */
                                  /* keyboard                              */
#define JCNTRLB_CLRBUSY     (4)   /* interfaces parallel busy bit          */
#define JCNTRLB_DISKTOAMIGA (5)   /* Floppy switch bits                    */
#define JCNTRLB_DISKTOPC    (6)
#define JCNTRLF_ENABLEINT   (1<<0)/* enable amiga interrupts               */
#define JCNTRLF_DISABLEINT  (1<<1)/* disable amiga interrupts              */
#define JCNTRLF_RESETPC     (1<<2)/* reset the pc. remember to strobe      */
                                  /* ReleasePcReset afterwards             */
#define JCNTRLF_CLRPCINT    (1<<3)/* turn off all amiga->pc ints (except   */
                                  /* keyboard                              */
#define JCNTRLF_CLRBUSY     (1<<4)/* interfaces parallel busy bit          */
#define JCNTRLF_DISKTOAMIGA (1<<5)/* Switch disk to Amiga for floppy switch*/
                                  /* Active low                            */
#define JCNTRLF_DISKTOPC    (1<<6)/* Switch disk to PC for floppy switch   */
                                  /* Active low                            */



#endif /* End of JANUS_JANUSREG_H conditional compilation                  */

