/*
 * sendmail: a simple, stupid SMTP server.  Probably not entirely RFC 788
 * conformant and definitely needs more sophisticated address parsing.
 * Two required directories:
 *	mail:		- user mail boxes
 * 	mail:queue	- incoming mail queued here
 * The messages are dumped into MAIL:queue/tf#, where # is the job number
 * of the incoming mail.  More work has to be done to deliver the messages
 * into a mailbox.
**
**  06-Feb-91 dlarson
**	Added movemail() to move tmp files into user's mbox files.
**	Note that this is like /usr/spool/username on unix, not like
**	mbox on unix.
**
**  04-Feb-91 dlarson
**	There must now exist a directory in MAIL: for each user who can
**	receive mail on this host, and the directory names must match
**	the user names.  Users should include "root" and "postmaster"
**
**  31-Jan-91 dlarson
**	changed smtpeof from '"." || CRLF' to '"."LF' because gets()
**	strips CR's.  Replaced unnecessary concatenations with space
**	and abbuttal operators.
**
*/

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
end
call addlib 'socketrexx',-1

CR = 'd'x
LF = 'a'x
CRLF = CR || LF
maildir = "MAIL:"
mailqueue = maildir"queue"

/* mail dispatch loop */
if exists(maildir) ~= 1 then do
	say "Volume '"maildir"' must be defined; exiting!"
	exit 20
end
if exists(mailqueue) ~= 1 then do
	say "Directory '"mailqueue"' must be present; exiting!"
	exit 20
end

call recvjob(args)

exit 0



recvjob:	/* RECVJOB: SMTP protocol processing session */
	parse arg s mesgname hostaddress junk

	readbuf = ""	/* read ahead buffer */
	sender = ""
	recipient = ""
	myname = socket_gethostname()

	gr = "220 HELO" myname "SMTP service ready at" Date() Time()
	call puts(gr)

	/* main parsing loop: get command line, uppercase command, execute */
	do forever
		line = gets()
		if line == "" then QUITcmd("")
		line = strip(strip(line, 'B', LF), 'B', CR)
		parse var line cmd cmdarg

		cmd = upper(cmd)
		select
			when cmd == "HELO" then call HELOcmd(cmdarg)
			when cmd == "MAIL" then call MAILcmd(cmdarg)
			when cmd == "RCPT" then call RCPTcmd(cmdarg)
			when cmd == "DATA" then call DATAcmd(cmdarg)
			when cmd == "RSET" then call RSETcmd(cmdarg)
			when cmd == "NOOP" then call puts("200 OK")
			when cmd == "QUIT" then call QUITcmd(cmdarg)
			when cmd == "HELP" then call HELPcmd(cmdarg)
			otherwise call puts("500 Command not recognized")
		end
	end
	return

HELOcmd:	/* HELO command - pretty much a nop; should authenticate sender address */
	parse arg hostname junk
	gr = "250" myname "Hello" hostname "["hostaddress"]"
		", pleased to meet you"
	call puts(gr)
	return

MAILcmd: expose sender	/* MAIL command - specify sender addr */
	parse arg from ":" sr
	if upper(from) ~= "FROM" then do
		call puts("501 Syntax error")
		return
	end
	if sender ~== "" then do
		call puts("503 sender already specified")
		return
	end
	sender = sr
	call puts("250" sr "... sender OK")
	return

RCPTcmd: expose recipient	/* RCPT command - specify recipient(s) */
	parse arg tovar ":" rcip
	if upper(tovar) ~= "TO" then do
		call puts("501 Syntax error")
		return
	end
	parse value addr2namehost(rcip) with user host
/* check to see if user has a directory in "mail:"  */
	if exists(maildir || user) ~= 1 then do
		call puts("550" rcip "... User unknown")
		return
	end
	recipient = user recipient
	call puts("250" rcip "... Recipient OK")
	return

DATAcmd:	/* DATA command - receive data; forces delivery of mail */
	if sender == "" then do
		call puts("503 Need MAIL command")
		return
	end
	if recipient == "" then do
		call puts("503 Need RCPT command")
		return
	end
	call acceptmail()
	return

RSETcmd:	/* RESET command - reset connect state */
	sender = ""
	recipient = ""
	call puts("250 Reset state")
	return

QUITcmd:	/* QUIT command - end session */
	call puts("221" myname "closing session - G'day!")
	call socket_close(s)
	exit 0

HELPcmd:	/* HELP <topic> command - return helpful messages */
	parse arg topic junk
	if topic == "" then do
		HELP = ""
		HELP = HELP"214-Commands:"CRLF
		HELP = HELP"214-HELO MAIL RCPT DATA RSET NOOP QUIT HELP"CRLF
		HELP = HELP"214 For more info use HELP <topic>"
		call puts(HELP)
		return
	end

	topic = upper(topic)
	helpstr = ""
	select
		when topic == "MAIL" then do
			helpstr = helpstr"214-MAIL FROM: <sender>"CRLF
			helpstr = helpstr"214-  Specify the sender."CRLF
		end
		when topic == "RCPT" then do
			helpstr = helpstr"214-RCPT TO: <recipient>"CRLF
			helpstr = helpstr"214-  Specify the recipient.  Can be used any number of times."CRLF
		end
		when topic == "DATA" then do
			helpstr = helpstr"214-DATA"CRLF
			helpstr = helpstr"214-  Collect message, end with single dot." || CRLF
		end
		when topic == "RSET" then do
			helpstr = helpstr"214-RSET"CRLF
			helpstr = helpstr"214-  Reset session"CRLF
		end
		when topic == "NOOP" then do
			helpstr = helpstr"214-NOOP"CRLF
			helpstr = helpstr"214-  No operation."CRLF
		end
		when topic == "HELO" then do
			helpstr = helpstr"214-HELO <hostname>"CRLF
			helpstr = helpstr"214-  Introduce yourself.  I am an Amiga and like every user I meet!"CRLF
		end
		when topic == "HELP" then do
			helpstr = helpstr"214-HELP [ <topic> ]"CRLF
			helpstr = helpstr"214-  The HELP command gives help info."CRLF
		end
		when topic == "QUIT" then do
			helpstr = helpstr"214-QUIT"CRLF
			helpstr = helpstr"214-  Exit sendmail (SMTP)."CRLF
		end
		otherwise call puts("504 HELP topic unknown")
	end
	if helpstr ~= "" then do
		call puts(helpstr"214 End of HELP info")
	end
	return

acceptmail:	/* accept mail from client */
	tmpname = mailqueue"/tf"mesgname
	if open(TMP, tmpname, 'W') ~= 1 then do
		call puts("451 Requested action aborted: local error")
		return
	end
/* From:, To: -> control file? */

	call puts("354 Enter mail, end with '.' on a line by itself.")
/*  used to be crlf, but gets() strips 'CR':  */
	smtpeof = "."LF

	do forever
		line = gets()
		if line == smtpeof then break
		if pos(".", line) == 1 then
			line = substr(line, 2)
		line = strip(line, 'B', CR)	/* eat \r */
		call writech(TMP, line)
	end
	call puts("250 Mail accepted")
	call close(TMP)
        call movemail()
	return

gets: expose readbuf /* get a line of data from remote */
	line = ""
	do forever
		if readbuf == "" then do
			drop tmpbuf
			cnt = socket_recv(s, tmpbuf, 1024)
			if cnt <= 0 then return ""
			readbuf = tmpbuf
		end

		lfpos = pos(LF, readbuf)
		if lfpos == 0 then do
			line = line || readbuf
			readbuf = ""
		end
		else do
			line = line || left(readbuf, lfpos)
			lfpos = lfpos + 1
			if lfpos >= length(readbuf) then readbuf = ""
			else readbuf = substr(readbuf, lfpos)
			say "r:" line
			return line
		end
	end

perror:	/* Simple perror() function */
	parse arg str
	error = socket_errtxt(INETerror)
	say "Could not" str "because" error"!"
	return

puts: 	/* send "str" as a NETASCII line, ie terminated with <cr> <lf> */
	parse arg str
	say "s:" str
	call socket_send(s, str || CRLF)
	return

/* remove angle-brackets and return two words, the name of the
** user and the name of the host:
*/
addr2namehost: procedure
	parse arg str
	len = length(str)-2
	str = substr(str, 2, len)
	at  = pos("@", str)
	return(substr(str, 1, at-1) substr(str, at+1, len-at))

/*
**  use the recipient list to move mail from the temp dir to the
**  user directory(s)
*/

movemail:
	do while words(recipient) > 0
		parse value recipient with user recipient
		mbox = maildir||user"/mbox"
		ombox = mbox".old"
                scratch = mailqueue"/scratch"

		address command

		delete ombox
		join tmpname mbox "as" scratch
		rename mbox ombox
		rename scratch mbox
	end