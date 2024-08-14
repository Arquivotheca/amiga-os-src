/* daytime: get time from remote machine */
arg host
if host == "" then do
	say "usage: daytime hostname"
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
	'run >nil: inet:c/socketrexx'
	'waitforport socketrexx'
	if ~showlist('P', 'socketrexx') then do
		say 'Could not start socketrexx, exiting!'
		exit 20
	end
end
call addlib 'socketrexx',-1

addr.domain = "inet"
addr.sin_port = socket_getservbyname("daytime", "tcp")
if addr.sin_port == "" then do
	say "Could not get service port number, exiting."
	exit 20
end

addr.sin_addr = socket_gethostbyname(host)
if addr.sin_addr == "" then do
	say "Could not resolve address for" host
	exit 20
end

s = socket_socket("inet", "stream")
if s < 0 then do
	say "Could not open a socket because" socket_errtxt(INETerror)
	exit 20
end

if socket_connect(s, addr) < 0 then do
	say "Connection failed because" socket_errtxt(INETerror)
	exit 20
end

data = ""
do forever
	drop buf
	cnt = socket_recv(s, buf, 1024)
	if cnt < 0 then do
		say "Receive error -" socket_errtxt(INETerror)
		call socket_close(sock)
		exit 20
	end
	if cnt <= 0 then break
	data = data || buf
end

cr = 'd'x
lf = 'a'x
say strip(strip(data, 'B', cr), 'B', lf)

call socket_close(s)
exit
