/**/

parse arg args

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
	call addlib 'socketrexx',-1
end

call main(args)

exit 0


main:
	s = socket_socket("inet", "stream")
	if s < 0 then do
		call perror("Create socket")
		exit 20
	end
	addr.domain = "inet"
	addr.sin_addr = "0.0.0.0"  /*  from any host */
	addr.sin_port = socket_getservbyname("smtp", "tcp")
	if socket_bind(s, addr) < 0 then do
		call perror("socket_bind")
		call socket_close(s)
		exit 20
	end

	if socket_listen(s, 5) < 0 then do
		call perror("socket_listen")
		call socket_close(s)
		exit 20
	end

	msg = 1
	do forever
		drop peer
		new_s = socket_accept(s, peer)
		if new_s < 0 then do
			call perror("socket_accept")
			call socket_close(s)
			return
		end
		cmd = "run >nil: smtpd.rexx" new_s msg
/*  to turn on logging add this to line above: ">>ram:mail.log" */
		cmd = cmd peer.sin_addr
		address command cmd
		msg = msg + 1
	end
	return


perror:	/* Simple perror() function */
	parse arg str
	error = socket_errtxt(INETerror)
	say "Could not" str "because" error"!"
	return
