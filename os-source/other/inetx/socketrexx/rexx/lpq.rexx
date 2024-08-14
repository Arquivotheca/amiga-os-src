/* lpq: query specified print queue of host */
parse arg "-P" printqueue long x
if printqueue == "" then do
	printqueue = "lp"
end

host = printcap(printqueue)
if host == "" then do
	str = "lpq: could not find host that serves queue " || printqueue
	say str || " -- check printcap.rexx file"
	exit 20
end

if ~show('L','rexxsupport.library') then do
	if ~addlib("rexxsupport.library",0,-30) then do
		say 'rexxsupport.library not found, exiting!'
		exit 20
	end
end
if ~showlist('P', 'socketrexx') then do
	say 'socketrexx is not running, trying to start it.'
	address command
	'run >nil: socketrexx'
	'waitforport socketrexx'
	if ~showlist('P', 'socketrexx') then do
		say 'Could not start socketrexx, exiting!'
		exit 20
	end
	call addlib 'socketrexx',-1
end

/* Initialize connection socket address */
pr.domain = "inet"
pr.sin_addr = socket_gethostbyname(host)
if pr.sin_addr == "" then do
	say "Could not resolve address for" host"."
	exit 20
end
pr.sin_port = socket_getservbyname("printer", "tcp")
if pr.sin_port == "" then do
	say "Could not resolve service port number."
	exit 20
end

s = socket_socket("inet", "stream")
if s < 0 then do
	say "Could not open a socket because" neterrtxt(INETerror)
	exit 20
end

/* lpd wants to talk to "reserved" local socket numbers only.  Reserved
 * sockets have addresses in the range 513..1023 inclusive.
 */
local.domain = "inet"
local.sin_addr = "0.0.0.0"
local.sin_port = random(513, 1023, time('s')) /* need rresvport()!!! */
if socket_bind(s, local) < 0 then do
	say "Could not bind local address because" socket_errtxt(INETerror)
	socket_close(s)
	exit 20
end

if socket_connect(s, pr) < 0 then do
	say "Could not connect to spooler because" socket_errtxt(INETerror)
	socket_close(s)
	exit 20
end

/* The format (see RFC1179) for the "Send queue state (short)" command of
 * lpd is: 0x03 || Queuename || Jobnamelist || LF
 */
QUERYSHORTcmd = '3'x
QUERYLONGcmd = '4'x

str = QUERYSHORTcmd
if long == "-l" then str = QUERYLONGcmd
str = str || printqueue 'a'x
call socket_send(s, str)

/* read queue status and echo -> console */
do forever
	drop buf
	cnt = socket_recv(s, buf, 1024)
	if cnt <= 0 then break
	call socket_writech(STDOUT, buf)
end

call socket_close(s)
exit
