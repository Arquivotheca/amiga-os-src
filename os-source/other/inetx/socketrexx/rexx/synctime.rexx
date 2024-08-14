/*
 * synctime: synchronize time to host using "daytime" service
 *
 * Note: since the daytime service doesn't specify a date format (see RFC867)
 * this program will not work with all hosts.  Hack the parse line below if
 * your host does it differently from BSD/Sun Unix.
 */
arg host
if host == "" then do
	say "usage: synctime hostname"
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
	call addlib 'socketrexx',-1
end

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

datebuf = ""
do forever
	drop buf
	cnt = socket_recv(s, buf, 1024)
	if cnt < 0 then do
		say "Receive error -" socketerrtxt(INETerror)
		call socket_close(sock)
		exit 20
	end
	if cnt == 0 | length(datebuf) > 512 then break
	datebuf = datebuf || buf
end
call socket_close(s)
datebuf = strip(strip(datebuf, 'B', 'd'x), 'B', 'a'x) /* strip \r\n */

JAN.days = 31
FEB.days = 29	/* we're not that picky to catch leap year errors! */
MAR.days = 31
APR.days = 30
MAY.days = 31
JUN.days = 30
JUL.days = 31
AUG.days = 31
SEP.days = 30
OCT.days = 31
NOV.days = 30
DEC.days = 31

/* Unix date format: Sun Oct 28 18:36:21 1990 */
parse var datebuf with day month daynum hour ":" min ":" sec year junk
if year > 1900 & year < 2000 then year = year - 1900
if year < 0 | year > 99 then beefdate()
month = upper(month)	/* should validate month */
if daynum <= 0 | daynum > month.days then beefdate()
if hour < 0 | hour > 23 then beefdate()
if min < 0 | min > 59 then beefdate()
if sec < 0 | sec > 59 then beefdate()

cmd = "C:Date" daynum'-'month'-'year
cmd = cmd || hour':'min':'sec
say "Setting date with command:" cmd
address command cmd
exit

beefdate:
	say "Got bad date from server:" datebuf
	exit 10
