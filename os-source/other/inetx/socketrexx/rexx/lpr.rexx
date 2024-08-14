/* lpr: queue a printjob on given server */
parse arg "-P" printqueue filename the_rest

host = printcap(printqueue)
if host == "" then do
	str = "lpr: could not find host that serves queue" printqueue
	say str " -- check printcap.rexx file"
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

pr.domain = "inet"
pr.sin_addr = socket_gethostbyname(host)
if pr.sin_addr == "" then do
	say "Could not resolve address for" host "."
	exit 20
end
pr.sin_port = socket_getservbyname("printer", "tcp")
if pr.sin_port == "" then do
	say "Could not resolve service port number."
	exit 20
end

/* build a connect to remote server */
s = socket_socket("inet", "stream")
if s < 0 then do
	say "Could not open a socket because" socket_errtxt(INETerror)
	exit 20
end
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

NUL = '0'x
LF = 'a'x

/* Ask server if it will accept a new print job */
QUEUEjob = '02'x || printqueue || LF
call socket_send(s, QUEUEjob)
call ackit("Server did not accept job")

/* Send control file to server */
hostname = socket_gethostname()
username = socket_getuser()
jobnum = 112	/* should be different each time!!!! */
cfilename = "cfA" || jobnum || hostname
dfilename = "dfA" || jobnum || hostname

/* format command file per RFC 1179 */
CMDfile = "H" || hostname || LF
CMDfile = CMDfile || "P" || username  || LF
CMDfile = CMDfile || "J" || filename  || LF
CMDfile = CMDfile || "C" || hostname  || LF
CMDfile = CMDfile || "L" || username  || LF
CMDfile = CMDfile || "f" || dfilename || LF
CMDfile = CMDfile || "U" || dfilename || LF
CMDfile = CMDfile || "N" || filename  || LF

/* send control file */
call socket_send(s, '02'x || Length(CMDfile) cfilename || LF)
call ackit("Server did not accept control file transfer request")
call socket_send(s, CMDfile || NUL)
call ackit("Server did not accept control file transfer")

/* send data file - if non existent, then abort session */
say "opening" filename
if open(DATA, filename, "R") == 0 then do
	say "Could not open " || filename
	call socket_send(s, '01'x || LF)	/* abort xfer */
	call socket_close(s)
	exit 20
end
call socket_send(s, '03'x || seek(DATA, 0, 'E') dfilename || LF)
call ackit("Server did not accept data file transfer request")
seek(DATA, 0, 'B')
do forever
	buf = readch(DATA, 1024)
	if buf == "" then break;
	call send(s, buf)
end
call socket_send(s, NUL)

call socket_close(s)
exit 0

/* get ack byte back from server.  If byte == '0'x then cmd was accepted */
ackit:
	parse arg str

	cnt = socket_recv(s, ack, 1)
	if cnt <= 0 | ack ~== '00'x then do
		say str
		call socket_close(s)
		exit 20
	end
	return
