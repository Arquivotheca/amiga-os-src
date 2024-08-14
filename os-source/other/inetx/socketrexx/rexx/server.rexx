/* server: simple server example.  Prints message, closes connection */

cr = 'd'x
lf = 'a'x
crlf = cr || lf
nul = '0'x

if ~show('L','rexxsupport.library') then do
	if ~addlib("rexxsupport.library",0,-30) then do
		say 'rexxsupport.library not found, exiting!'
		exit 20
	end
end
if ~showlist('P', 'socketrexx') then do
	say 'socket_rexx is not running, trying to start it.'
	address command
	'run >nil: inet:c/socketrexx'
	'waitforport socketrexx'
	if ~showlist('P', 'socketrexx') then do
		say 'Could not start socketrexx, exiting!'
		exit 20
	end
	call addlib 'socketrexx',-1
end

/*
 * Open a stream, ie byte sequenced, reliable delivery, socket
 */
s = socket_socket("inet", "stream")
if s < 0 then do
	call perror("create socket")
	exit 20
end

/*
 * Bind (ie name) the socket to local port address 2000.
 */
addr.domain = "inet"
addr.sin_addr = "0.0.0.0"
addr.sin_port = 2000
if socket_bind(s, addr) < 0 then do
	call perror("bind")
	call socket_close(s)
	exit 20
end

/*
 * Tell the socket to listen for incoming connection requests
 */
if socket_listen(s, 5) < 0 then do
	call perror("listen")
	call socket_close(s)
	exit 20
end

do forever
	drop peer

	/*
	 * Accept a connection request for processing
	 */
	new_s = socket_accept(s, peer)
	if new_s < 0 then do
		call perror("accept")
		call socket_close(s)
		exit 20
	end
	str = "Your address is" peer.sin_addr ":" peer.sin_port

	/*
	 * Tell the client which number to play in the lotto :-)
	 */
	str = str "-- and your lucky number is"
	str = str random(1, 1000, time('S')) crlf
	call socket_send(new_s, str)

	/*
	 * Close the socket.
	 */
	call socket_close(new_s)
end

/*
 * Simple perror() function
 */
perror:
	arg str

	error = socket_errtxt(INETerror)
	say "Could not" str "because" error "!"
	return
