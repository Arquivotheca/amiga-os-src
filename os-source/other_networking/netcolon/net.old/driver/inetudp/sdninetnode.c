/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1990 The Software Distillery.                    *
* |. o.| ||          All Rights Reserved                                  *
* | .  | ||          Written by John Toebes and Doug Walker               *
* | o  | ||          The Software Distillery                              *
* |  . |//           207 Livingstone Drive                                *
* ======             Cary, NC 27513                                       *
*                    BBS:(919)-460-7430                                   *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "netcomm.h"
#include "sdninet.h"

static char *any(char *cp, char *match);
static long parse_addr(char *cp);
static long gethostaddr(char *name);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNInitNode                                                        */
/*                                                                             */
/* PURPOSE: Establish a new node connection                                    */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    rc = SDNInitNode(drglobal, nnode);                                       */
/*                                                                             */
/*    drglobal   APTR             (in)     Driver-specific global data pointer */
/*                                                                             */
/*    nnode      struct NetNode * (in/out) On input, name of node is set.      */
/*                                         On output, driver sets the 'status' */
/*                                            and 'ioptr' fields.              */
/*                                                                             */
/*    rc         int              (ret)    SDN_ERR, SDN_OK                     */
/*                                                                             */
/* NOTES:                                                                      */
/*    Called by the handler to establish communications with a specific        */
/*    remote node.                                                             */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int LIBENT INETSDNInitNode     (register __a0 INETGLOBAL tg,
                                register __a1 char *name,
                                register __a2 APTR *con,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   int s, i;
   struct sockaddr_in sin;
   struct selectargs sa;
   struct RPacket phoneyrpak;
   
   /* Find us an empty slot to hold the connection */
   for(i = 0; i < MAXCONNECT; i++)
      if (tg->connect[i].usage == -1)
         break;

   if (i >= MAXCONNECT)
   {
      BUG(("Too many connections, ignoring it\n"));
      return(SDN_ERR);
   }

   if ((tg->connect[i].addr = gethostaddr(name)) == -1)
   {
      BUG(("No such address '%s'\n", name))
      return(SDN_ERR);
   }
   
   phoneyrpak.Type = ACTION_NETWORK_START;
   if (sendto(tg, tg->connect[i].addr, &phonerpak, sizeof(phoneyrpak)) < 0)
   {
      BUG(("********ERROR: Start Connection Failed\n"))
      BUGR("No Connect")
      return(SDN_ERR);
   }

   tg->connect[i].usage = 0;
   tg->connect[i].opack = 0;
   tg->connect[i].data  = NULL;
   
   *con = (APTR)&tg->connect[i];
   BUG(("Connection is now %ld for %08lx '%s' errno=%d\n", 
         nnode->ioptr, nnode, name, errno));

   return(SDN_OK);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNTermNode                                                        */
/*                                                                             */
/* PURPOSE: Free Up all Driver resources associated with a Node                */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    SDNTermNode(drglobal, nnode);                                            */
/*                                                                             */
/*    drglobal  APTR              (in)     Driver-specific global data pointer */
/*                                                                             */
/*                                                                             */
/*    nnode     struct NetNode *  (in)     NetNode to terminate                */
/*                                                                             */
/* NOTES:                                                                      */
/*    This routine is void.                                                    */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void LIBENT INETSDNTermNode    (register __a0 INETGLOBAL tg,
                                register __a1 APTR con,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   struct INETCON *conp;
   struct RPacket phoneyrpak;
   conp = (struct INETCON *)con;
   
   if ((conp->addr !== -1)
   {
      /* We should probably check the outstanding packets before terminating */
      /* the node.                                                           */
      phoneyrpak.Type = ACTION_NETWORK_TERM;
      sendto(tg, tg->connect[i].addr, &phonerpak, sizeof(phoneyrpak));

      conp->usage = -1;
      conp->opack = 0;
      conp->addr  = -1;
   }
   
}


/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/****** socket/inet_addr ******************************************
*
*   NAME
*		inet_addr -- make internet address from string
*
*   SYNOPSIS
*		#include <sys/types.h>
*		#include <sys/socket.h>
*		#include <netinet/in.h>
*		#include <arpa/inet.h>
*
*		addr = inet_addr( string )
*
*		u_long inet_addr ( char * ); 
*
*   FUNCTION
*		Converts a string to an internet address. All internet addresses
*		are in network order.
*
*	INPUTS
*		string		address string. "123.45.67.89" for example
*
*   RESULT
*		INADDR_NONE is returned if input is invalid.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/*
 * Internet address interpretation routine.
 * All the network library routines call this
 * routine to interpret entries in the data bases
 * which are expected to be an address.
 * The value returned is in network order.
 */

#define isdigit(x)  (x >= '0' && x <= '9')
#define isxdigit(x) ((x >= '0' && x <= '9') || \
                     (x >= 'a' && x <= 'f') || \
                     (x >= 'A' && x <= 'F'))
#define islower(x)  (x >= 'a' && x <= 'z')
#define isspace(x)  (x == ' ' || x == '\t' || x == '\n' || x == '\r' || x == 11)

static char *any(char *cp, char *match)
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return (NULL);
}


static long parse_addr(char *cp)
{
	register u_long val, base, n;
	register char c;
	u_long parts[4], *pp = parts;

again:
	/*
	 * Collect number up to ``.''.
	 * Values are specified as for C:
	 * 0x=hex, 0=octal, other=decimal.
	 */
	val = 0; base = 10;
	if (*cp == '0') {
		if (*++cp == 'x' || *cp == 'X')
			base = 16, cp++;
		else
			base = 8;
	}
	while (c = *cp) {
		if (isdigit(c)) {
			val = (val * base) + (c - '0');
			cp++;
			continue;
		}
		if (base == 16 && isxdigit(c)) {
			val = (val << 4) + (c + 10 - (islower(c) ? 'a' : 'A'));
			cp++;
			continue;
		}
		break;
	}
	if (*cp == '.') {
		/*
		 * Internet format:
		 *	a.b.c.d
		 *	a.b.c	(with c treated as 16-bits)
		 *	a.b	(with b treated as 24 bits)
		 */
		if (pp >= parts + 4)
			return (INADDR_NONE);
		*pp++ = val, cp++;
		goto again;
	}
	/*
	 * Check for trailing characters.
	 */
	if (*cp && !isspace(*cp))
		return (INADDR_NONE);
	*pp++ = val;
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts;
	switch (n) {

	case 1:				/* a -- 32 bits */
		val = parts[0];
		break;

	case 2:				/* a.b -- 8.24 bits */
		val = (parts[0] << 24) | (parts[1] & 0xffffff);
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
			(parts[2] & 0xffff);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
		      ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
		break;

	default:
		return (INADDR_NONE);
	}
	val = htonl(val);
	return ((long)val);
}

#define MAXLINE 200
#undef SysBase

static long gethostaddr(char *name)
{
   BPTR fh;
   char buf[MAXLINE+1];
   int len, i;
   char *cp, *p;
   long addr;
   struct Library *SysBase = ABSEXECBASE;
   struct DosLibrary *DOSBase;
   
   if ((addr = parse_addr(name)) != INADDR_NONE)
   {
      return(addr);
	}

   DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 0);
   if (DOSBase == NULL) return(INADDR_NONE);

   /* We weren't given an address, so now we attempt to open the host */
   /* file and read a name out of it                                  */
   if ((fh = Open(HOSTFILE, MODE_OLDFILE)) == NULL)
   {
      CloseLibrary((struct Library *)DOSBase);
      return(INADDR_NONE);
   }

   len = 0;
   for(;;)
   {
      i = Read(fh, buf+len, MAXLINE-len);
      if (i < 0) break;
      len += i;
      if (len == 0) break;
      for (i = 0; i < MAXLINE; i++)
         if (buf[i] == '\n')
            break;
      /* Null terminate the buffer */
      buf[i] = 0;
      if (buf[0] != '#')
      {
         cp = any(buf, "#");
         if (cp) *cp = '\0';
         cp = any(buf, " \t");
         if (cp != NULL)
         {
            /* We have an entry that at least has a separator */
            *cp++ = '\0';
            addr = parse_addr(buf);
            if (addr != INADDR_NONE)
            {
               while(cp && *cp)
               {
                  while (*cp == ' ' || *cp == '\t')
                     cp++;
                  p = any(cp, " \t");
                  if (p != NULL)
                     *p++ = '\0';
                  if (!stricmp(cp, name))
                  {
                     Close(fh);
                     CloseLibrary((struct Library *)DOSBase);
                     return(addr);
                  }
                  cp = p;
               }
            }
         }
      }
      memcpy(buf, buf+i+1, MAXLINE+1-i);
      len -= i+1;
   }
   Close(fh);
   CloseLibrary((struct Library *)DOSBase);
	return(INADDR_NONE);
}
