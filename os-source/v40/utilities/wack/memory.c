
/***********************************************************************
*
*			   G R A N D W A C K
*
************************************************************************
*
*	Copyright (C) 1985, Commodore-Amiga. All rights reserved.
*
************************************************************************
*
*   Source Control:
*
*	$Id: memory.c,v 1.5 91/04/24 20:53:27 peter Exp $
*
*	$Locker:  $
*
*	$Log:	memory.c,v $
 * Revision 1.5  91/04/24  20:53:27  peter
 * Changed $Header to $Id.
 * 
 * Revision 1.4  90/02/21  11:44:30  kodiak
 * rename dbase to dosbase, and add dos device listing to it
 * 
 * Revision 1.3  89/11/22  16:24:14  kodiak
 * 1.5: Forbid()/Permit() using BGACK hold; "faster" symbol loading option.
 * 
 * Revision 1.2  88/01/21  13:37:21  root
 * kodiak's copy of jimm's version, snapshot jan 21
 * 
*
***********************************************************************/
/***********************************************************************
*
*	Remote Memory Access Functions
*
***********************************************************************/

#include	<sys/ioctl.h>

extern short dd;
extern short remote;
extern short Memory[];

short nestcount = 0;

Forbid()
{
    if (nestcount++ == 0)
	if (ioctl(dd, _IO(B, 3)) == -1)
	    quit("ioctl error");
}

Permit()
{
    if (--nestcount == 0)
	if (ioctl(dd, _IO(B, 2)) == -1)
	    quit("ioctl error");
}

PutMem(addr,val)
    long addr;
    short val;
{
    if (remote) {
	lseek (dd, addr & 0xffffff, 0);

	if (write (dd, &val, 2) <= 0)
	    quit ("write error");
    }
    else {
	Memory[addr] = val;
    }

    return (val);
}

PutMemL(addr,val)
    long addr;
    long val;
{
    if (remote) {
	lseek (dd, addr & 0xffffff, 0);

	if (write (dd, &val, 4) <= 0)
	    quit ("write error");
    }

    return (val);
}

char GetMemB(addr)
    long addr;
{
    char   temp;

    lseek (dd, addr & 0xffffff, 0);

    read (dd, &temp, 1);
    return (temp);
}

short GetMem(addr)
    long addr;
{
    short   temp;

    lseek (dd, addr & 0xffffff, 0);

/*  if (read (dd, &temp, 2) <= 0)
	quit ("read error"); */
    read (dd, &temp, 2);
    return (temp);
}

long GetMemL(addr)
    long addr;
{
    long   temp;

    if (remote) {
	lseek (dd, addr & 0xffffff, 0);

/*	if (read (dd, &temp, 4) <= 0)
	    quit ("read error"); */
	read (dd, &temp, 4);
	return (temp);
    }
}

getBlock(addr,buffer,size)
    long addr;
    char *buffer;
    long size;
{
    if (remote) {
	lseek (dd, addr & 0xffffff, 0);

/*	if (read (dd, buffer, size) <= 0)
	    quit ("read error"); */
	read (dd, buffer, size);
    }
}
