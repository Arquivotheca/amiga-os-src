
/*********************************************************************
*
* services.h -- define common service numbers between ibm-pc and amiga
*
* Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
*
**********************************************************************/


#ifndef JANUS_SERVICES_H
#define JANUS_SERVICES_H

/**/
/* this is the table of hard coded services.  Other services may exist
/* that are dynamically allocated.
/**/


/* service numbers constrained by hardware */
#define JSERV_MINT	0	/* monochrome display written to */
#define JSERV_GINT	1	/* color display written to */
#define JSERV_CRT1INT	2	/* mono display's control registers changed */
#define JSERV_CRT2INT	3	/* color display's control registers changed */
#define JSERV_ENBKB	4	/* keyboard ready for next character */
#define JSERV_LPT1INT	5	/* parallel control register */
#define JSERV_COM2INT	6	/* serial control register */

/* hard coded service numbers */
#define JSERV_PCBOOTED	7	/* PC is ready to service soft interrupts */

#define JSERV_SCROLL	8	/* PC is scrolling its screen */
#define JSERV_HARDDISK	9	/* Amiga reading PC hard disk */
#define JSERV_READAMIGA 10	/* PC reading Amiga mem */
#define JSERV_READPC	11	/* Amiga reading PC mem */
#define JSERV_AMIGACALL 12	/* PC causing Amiga function call */
#define JSERV_PCCALL	13	/* Amiga causing PC interrupt */
#define JSERV_NEWASERV	14	/* PC initiating Amiga side of a new service */
#define JSERV_NEWPCSERV 15	/* Amiga initiating PC side of a new service */


#endif !JANUS_SERVICES_H
