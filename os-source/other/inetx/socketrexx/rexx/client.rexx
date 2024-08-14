/* simple client example */
arg host

/* standard check on libraries & server */
if ~show('L','rexxsupport.library') then do
	if ~addlib("rexxsupport.library",0,-30) then do
		say 'rexxsupport.library not found, exiting!'
		exit 20
	end
end
if ~showlist('P', 'socketrexx') then do
	say 'netrexx is not running, trying to start it.'
	address command
	'run >nil: inet:c/socketrexx'
	'waitforport socketrexx'
	if ~showlist('P', 'socketrexx') then do
		say 'Could not start netrexx, exiting!'
		exit 20
	end
	call addlib 'socketrexx',-1
end

/*  Default host (if there is one, comment out the usage line):
**
**addr.domain = "inet"
**addr.sin_port = "2000"
**addr.sin_addr = "127.1"
*/
if host = "" then do
	say "usage: client.rexx hostname"
	exit 20
end

if host ~= "" then do
	addr.sin_addr = socket_gethostbyname(host)
	if addr.sin_addr == "" then do
		say "Could not resolve address for" host
		exit 20
	end
end

/* create an instance of a stream socket */
s = socket_socket("inet", "stream")
if s < 0 then do
	call perror("create socket")
	exit 20
end

/* attempt to connect to server */
if socket_connect(s, addr) ~= 0 then do
	call perror("connect")
	call socket_close(s)
	exit 20
end

data = ""
do forever
	/* recv some data from the socket */
	cnt = socket_recv(s, buf, 1024)
	if cnt < 0 then do
		call perror("receive error - ")
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

/* interpret error code from INETerror into user friendly string */
perror:
	arg str
	error = socket_errtxt(INETerror)
	say "Could not" str "because" error"!"
	return
