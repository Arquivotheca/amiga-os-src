/* rsh
**
**	31-Jan-91 dlarson
**		added shutdown line
*/

parse arg host cmd
if host == "" | cmd == "" then do
	say "usage: rsh hostname command.."
	exit 10
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
end
call addlib 'socketrexx',-1

remote.domain = "inet"
remote.sin_port = socket_getservbyname("shell", "tcp")
if remote.sin_port == "" then do
	say "Could not resolve service port number."
	exit 20
end
remote.sin_addr = socket_gethostbyname(host)
if remote.sin_addr == "" then do
	say "Could not resolve address for " || host || "."
	exit 20
end

locuser = socket_getuser()
if locuser == "" then do
	say "Local user name not set, exiting"
	call sockclose(s)
end
remuser = locuser

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
	sockclose(s)
	exit 20
end

if socket_connect(s, remote) < 0 then do
	say "Could not connect to remote because" socket_errtxt(INETerror)
	socket_close(s)
	exit 20
end

str = '0'x || remuser || '0'x || locuser || '0'x || cmd || '0'x
call socket_send(s, str)
socket_shutdown(s, "output") /*  Original author forgot this! -dlarson */
do forever
	drop buf
	cnt = socket_recv(s, buf, 1024)
	if cnt <= 0 then break
	call writech(STDOUT, buf)
end
call socket_close(s)
exit
