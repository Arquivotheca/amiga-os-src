# ---------------------------------------------------------------------------------
# Makefile for: network support library  (socket library) 
#                 both Manx 5.0 and Lattice 5.10
#
# $Locker:  $
#
# $Id: makefile,v 1.1 90/12/10 16:10:43 bj Exp $
#
# $Revision: 1.1 $
#
# $Header: HOG:Other/inet/src/lib/net2/RCS/makefile,v 1.1 90/12/10 16:10:43 bj Exp $
#
#-----------------------------------------------------------------------------------

MOBJ5 =stat.m5 getdomain.m5 inet_makeaddr.m5 getnetbyaddr.m5 \
	getnetbyname.m5 getnetent.m5 inet_network.m5 inet_lnaof.m5 \
	inet_netof.m5 nlist.m5 getdtablesize.m5 getproto.m5 q.m5 dir.m5 \
	chdir.m5 mkdir.m5 strtok.m5 syslog.m5 popen.m5 random.m5 \
	doprnt.m5 getservent.m5 sethostname.m5 getprotoname.m5 \
	getprotoent.m5 getopt.m5 getwd.m5 \
	sendmsg.m5 sendto.m5 setsockopt.m5 shutdown.m5 \
	basename.m5 getsockopt.m5 getpeername.m5 alarm.m5 \
	getumask.m5 gettz.m5 readv.m5 rexec.m5 \
	getsockname.m5 gettimeofday.m5 setuid.m5 bindresvport.m5 rcmd.m5 \
	perror.m5 sys_err.m5 sleep.m5 getpid.m5 inet_ntoa.m5 \
	gethostname.m5 getinet.m5 gethostent.m5 inet_addr.m5 ffs.m5 \
	fstat.m5 getuid.m5 herror.m5 writev.m5 bind.m5 getgid.m5 \
	requester.m5  password.m5 ioctl.m5 strcasecmp.m5 \
	listen.m5 accept.m5 socket.m5 send.m5 \
	recv.m5 connect.m5 sostub.m5 exit.m5 signal.m5 access.m5 \
	console.m5 startnetwork.m5 inet_up.m5 dos_packet.m5 \
	bmov.m5 bcmp.m5 bset.m5 rev.m5

RES32 =	res_debug.m res_mkquery.m res_comp.m res_send.m

OBJL =	stat.l getdomain.l inet_makeaddr.l getnetbyaddr.l \
	getnetbyname.l getnetent.l inet_network.l inet_lnaof.l \
	inet_netof.l nlist.l getdtablesize.l getproto.l q.l dir.l \
	chdir.l mkdir.l strtok.l syslog.l popen.l random.l \
	doprnt.l getservent.l sethostname.l getprotoname.l \
	getprotoent.l getopt.l getwd.l \
	sendmsg.l sendto.l setsockopt.l shutdown.l \
	basename.l getsockopt.l getpeername.l alarm.l \
	getumask.l gettz.l readv.l daemon.l daemonclose.l rexec.l \
	getsockname.l gettimeofday.l setuid.l bindresvport.l rcmd.l \
	perror.l sys_err.l sleep.l getpid.l inet_ntoa.l \
	gethostname.l getinet.l gethostent.l inet_addr.l ffs.l \
	fstat.l getuid.l herror.l writev.l bind.l getgid.l \
	requester.l  password.l ioctl.l strcasecmp.l \
	listen.l accept.l socket.l send.l index.l \
	recv.l connect.l exit.l signal.l access.l \
	console.l startnetwork.l inet_up.l dos_packet.l \
	daemon_init.l bmov.l bcmp.l bset.l rev.l


PROBS	= fstat.l

#
# make the autodocs
#

docs:
	join #?.(c|asm) as ram:temp_docs
	autodoc >docs -t4 ram:temp_docs
	delete ram:temp_docs

.asm.m5:
	as $*.asm -o $*.m5

.c.m5:
	cc -so -wo -inetinc: -DAMIGA $*.c -o $*.m5

all:	socketM.lib socketS.lib

socketM.lib: $(MOBJ5)
	lb $@ $(MOBJ5)

clean:
	delete #?.l #?.m5

install: socketm.lib socketS.lib 
	copy socketS.lib NETBIN:lib 
	copy socketm.lib NETBIN:lib/socketM.lib 

#
# SAS format socketS.lib
#

.asm.l:
	casm -A $*.asm -o $*.l

.c.l:
	lc -b1 -O -v -DAMIGA -inetinc: -o$*.l $*.c

socketS.lib:	$(OBJL)
	list >oml.list *.l nohead quick 
	oml socketS.lib @oml.list
	delete oml.list

bump:
	bumprev 36 socketlib_rev

