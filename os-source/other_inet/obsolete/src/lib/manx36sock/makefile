#
# Makefile for network support library
#

MOBJ32 =stat.m getdomain.m inet_makeaddr.m getnetbyaddr.m \
	getnetbyname.m getnetent.m inet_network.m inet_lnaof.m \
	inet_netof.m nlist.m getdtablesize.m getproto.m q.m dir.m \
	chdir.m mkdir.m strtok.m syslog.m popen.m random.m \
	doprnt.m getservent.m sethostname.m getprotoname.m \
	getprotoent.m getopt.m getwd.m recvmsg.m \
	sendmsg.m sendto.m setsockopt.m shutdown.m recvfrom.m \
	basename.m getsockopt.m getpeername.m alarm.m \
	getumask.m gettz.m readv.m daemon.m daemonclose.m rexec.m \
	getsockname.m gettimeofday.m setuid.m bindresvport.m rcmd.m \
	perror.m sys_err.m sleep.m getpid.m inet_ntoa.m \
	gethostname.m getinet.m gethostent.m inet_addr.m ffs.m \
	fstat.m getuid.m herror.m writev.m bind.m getgid.m \
	requester.m  password.m ioctl.m strcasecmp.m \
	listen.m accept.m socket.m send.m \
	recv.m connect.m sostub.m exit.m signal.m access.m \
	console.m startnetwork.m inet_up.m dos_packet.m \
	daemon_init.m bmov.m bcmp.m bset.m


RES32 =	res_debug.m res_mkquery.m res_comp.m res_send.m


#
# MANX format socket32.lib	- 32 bit int, near code, near data
#

sys_err.m:
	cc -Inetinc: -Z2500 -DAMIGA +l sys_err.c -o sys_err.m


docs:
	join #?.(c|asm) as ram:temp_docs
	autodoc >docs -t4 ram:temp_docs
	delete ram:temp_docs

.asm.m:
	as $*.asm -o $*.m

.c.m:
	cc  -Inetinc: -DAMIGA +l $*.c -o $*.m

all:	socket32.lib

socket32.lib: $(MOBJ32)
	lb $@ $(MOBJ32)

clean:
	delete #?.m 

install: socket32.lib
	copy socket32.lib NETBIN:lib clone

