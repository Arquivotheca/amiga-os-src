/* -----------------------------------------------------------------------
 * background.c
 *
 * $Locker:  $
 *
 * $Id: background.c,v 1.2 91/08/07 13:15:36 bj Exp $
 *
 * $Revision: 1.2 $
 *
 * $Log:	background.c,v $
 * Revision 1.2  91/08/07  13:15:36  bj
 * rcs header added.
 * Autodoc changes.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/background.c,v 1.2 91/08/07 13:15:36 bj Exp $
 *
 *------------------------------------------------------------------------
 */

/****** background ******************************************
*
*	What is a socket library?
*
* The standard programmer's interface to network protocols on most
* Un*x machines is the Berkley socket abstraction.  It is usually
* provided as a link library. You should have as documentation the
* books "Unix Network Programming" by W. Richard Stephens
* (Prentice-Hall, 1990) and "Internetworking with TCP/IP, Volume II"
* by Douglas Comer and David Stephens (Prentice-Hall, 1991).  We
* don't get a kick-back from Prentice-Hall, but we do use these books
* every day and we do know that writing programs using TCP/IP and
* sockets can be difficult.
*
*	Why a shared socket library?
*
* A shared library provides many benefits.  First, it greatly reduces
* code size.  Second, it is compiler-independent.  However, the most
* important benefit is that it is easily upgradable.  New libraries
* with bug fixes, speed improvements, or additional functions can be
* utilized by existing code without recompilation.  In the case of
* the socket library, this means we can later add support for name
* resolution, better configuration, DES, etc.
*
*	Who should use this?
*
* Everyone.  The link socket library that received very limited alpha
* distribution is obsolete.  Our primary goal in writing this socket
* library is compatibility. BSD, SVR4, X/Open, POSIX and OSF sources
* have been consulted when necessary.  All standard Unix socket
* functions, with all their peculiarities have been faithfully ported
* :^)  Unfortunately, due to the fact that these functions were not
* designed with shared libraries or the Amiga in mind, some
* compromises were made. 
*
*	How to use this? 
*
* See "What is a socket library?" above.  To port existing socket code 
* (from Un*x or from our obsolete link library), you must:
*
*	OpenLibrary the socket.library and call setup_sockets()
*	not call read() or write() on sockets (use send() and recv())
*	call s_close() rather than close() for sockets
*	call s_ioctl() rather than ioctl() for sockets
*
* In case you're wondering, our old link library was
* compiler-specific and included the compiler's library code for
* read(), write(), close() and ioctl(). On Un*x machines, these
* functions are just calls to the kernel which can deal with sockets
* and other files.  Obviously, this is not possible with a shared
* library on a machine where these functions come from link
* libraries, hence the s_*() functions.
*
*	About errno:
*
* 'C' programmers should be familiar with the global variable
* 'errno.'  It is used extensively in standard socket implementations
* to provide details about error conditions.  We take care of this in
* the shared library by passing a pointer to 'errno' into the shared
* library with setup_sockets().  You can, of course, pass a pointer
* to any longword-aligned, four-byte chunk of memory you own, so this
* will work for non-'C'-language programmers.
*
*	About integers:
*
* All integers are assumed to be 32-bit.  None are specified as long
* in order to maintain Un*x compatibility.  If you are using a
* compiler which doesn't use 32-bit ints, you must make sure that all
* ints are converted to longs by your code.
*
*	About assembly langauge:
*
* To be complete, we probably should include assembly header files.
* We don't.
*
*	About the get*() functions:
*
* This is standard with the Un*x functions, too, but is worth noting:
* These function return a pointer to a static buffer.  The buffer returned
* by a call to getX*() is cleared on the next invocation of getX*().  For
* example, the buffer pointed to by the return of gethostent() is cleared
* by a call to gethostent(), gethostbyaddr() or gethostbyname(), but not
* by a call to getprotoent(), etc.
*
* As noted in the autodocs, none of the get*ent(), set*ent() or end*ent()
* functions should normally be used except for in porting existing programs
* (and internally).
*
*	About library bases:
*
* The shared socket library returns a different library base for each
* OpenLib() and uses these different library bases to keep track of some
* global data for each opener.  If you start a new process with a new
* context, the new process must open and initialize socket.library.  Mere
* tasks should not access the socket.library, only processes should.
*
*	Example:
*
* \* your includes *\
* #include <exec/types.h>
* #include <exec/libraries.h>
* #include <exec/execbase.h>
* #include <dos/dos.h>
* #include <proto/all.h>
* #include <sys/types.h>
* #include <sys/socket.h>
* #include <stdio.h>
* #include <errno.h>
*
* \* the next two lines must always be here *\
* #include <ss/socket.h>
* struct Library *SockBase ;
*
* \* this is the maximum number of sockets that you want *\
* #define MAXSOCKS 10
*
* main()
* {
*	if((SockBase = OpenLibrary( "inet:libs/socket.library", 1L )) == NULL) {
*		printf("Error opening socket.library\n");
*		Exit(10);
*	}
*	setup_sockets( MAXSOCKS, &errno );
*
*	\* normal socket code... (see AS225 dev disk for complete examples) *\
*
*	cleanup_sockets();
*	CloseLibrary( SockBase ) ;
* }
*
*	Legalese:
*
* This shared socket library (and it's documentation) are Copyright © 1991
* Commodore-Amiga, Inc.  All Rights Reserved.  The shared socket library
* was written by Martin Hunt.  Dale Larson helped with the documentation and
* testing.
*
* Parts of this shared socket library and the associated header files are
* derived from code which is:
*
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
*
****************************************/