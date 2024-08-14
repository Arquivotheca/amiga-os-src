/*rx
 *  at.rexx --- simulate UNIX at command.  For use with CyberCron.
 *  By Loren J. Rittle (l-rittle@uiuc.edu)   Tue May  5 01:47:11 1992
 *  Updated to work with any AmigaOS shell - Wed May  6 03:32:00 1992
 *
 *  Copyright © 1992  Loren J. Rittle
 *  Use as you will, just document your changes and keep my copyright
 *  notice intact.  Feel free to mail enhancements to me.
 *
 *  This is not fancy yet - only handles 1 - 4 digit form of
 *  time specification.  1, 2 digits means hour only. 3, 4
 *  digits means 1 or 2 digits of hour and 2 digits of minute.
 *  This means that 'at' jobs can only be added for the period
 *  of the next 24 hours.
 *
 *  Usage:
 *    at [-q[ac-z]] time
 *    <shell script>
 *    <EOF>
 *
 *    at -qb
 *    <shell script>
 *    <EOF>
 *
 *    at -l
 *      [for list current 'at' jobs]
 *	not implemented yet.
 *
 *    at -r <task name as given at issue time, or from list>
 *      [remove an 'at' job as shown in list]
 *	not implemented yet.
 */

/* USER MODIFIABLE DEFAULT, EDIT TO TASTE */

/* AT_FILES should be somewhere that only at writes to.
   T: is not a suitable location, IMHO.  In the future, at.rexx
   and some startup code magic might know how to requeue jobs
   that should have been run while power was off or after a crash
   occurred.  To prepare for this, AT_FILES should be on a real
   file system. */
AT_FILES = 'usr:spool/at/'

/* NO GENERAL USER MODIFIABLE PARTS BELOW THIS COMMENT. */

/* Default queue to place jobs in.  Remember queue 'b' is special. */
QUEUE = 'a'

HOUR = '00'
MINUTE = '00'

options results

parse arg option line xline

if left(option, 2) == '-q' then
  do
    if length(option) ~= 3 then
      do
	say 'at: invalid ''-q'' parameter'
	exit 10
      end
    QUEUE = right(option, 1)
    if pos(QUEUE, xrange('a','z')) == 0 then
      do
	say 'at: invalid queue name,' QUEUE
	exit 10
      end
  end
else
  parse arg line xline

if QUEUE == 'b' then
  do
    if line ~= '' then
      do
	say 'at: time can''t be given for use with b[atch] queue'
	exit 10
      end
    HOUR = '*'
    MINUTE = '*'
  end
else
  do
    if line == '' then
      do
	say 'at: time must be given for use with' QUEUE 'queue'
	exit 10
      end
    if length(line) > 4 | xline ~= '' | ~datatype(line, 'N') then
      do
	say 'at: "'line'" is not a valid time, must be of form: H, HH, HMM, HHMM'
	exit 10
      end
    if length(line) > 2 then
      do
	MINUTE = right(line, 2)
	HOUR = left(line, length(line) - 2)
      end
    else
      HOUR = line
    if MINUTE < 0 | MINUTE > 59 | HOUR < 0 | HOUR > 23 then
      do
	say 'at: "'line'" is not in the range of a valid time'
	exit 10
      end
  end

if ~open('id','id:','R') then
  do
    say 'at: requires the ID: device to function properly'
    exit 10
  end
ID = readln('id')
call close('id')


if QUEUE == 'b' then
  scriptfile = AT_FILES'at.'ID'.'QUEUE
else
  scriptfile = AT_FILES'at.'ID'.'QUEUE'.'HOUR||MINUTE

if ~open('scriptfile', scriptfile, 'W') then
  do
    say 'at: can''t open' scriptfile 'for output'
    exit 10
  end
call writeln('scriptfile', 'cd' '22'x||pragma('D')||'22'x)
call writeln('scriptfile', 'stack' pragma('S', 4000))
if left(address(), 4) == 'WSH_' then
  do
    /*
     *  If we have a WShell under us, then try to propagate
     *  local environment variables.  If you don't, I'm not
     *  sorry for you, read comments below. :-)
     */
    'set | execio stem VARS.'
    do i = 1 to VARS.0
      if word(VARS.i, 1) ~= 'process' then
	call writeln('scriptfile', 'set' VARS.i)
    end
  end
else
  say 'warning: WShell not present under your ARexx, local environment variables can''t be propagated'
do forever
  line = readln(stdin)
  if eof(stdin) then
    leave
  call writeln('scriptfile', line)
end
call writeln('scriptfile', 'run <nil: >nil: delete' scriptfile)
call close('scriptfile')

address command 'protect' scriptfile '+s'

USER = getenv('USER')
if left(address(), 4) == 'WSH_' then
  do
    /*
     *  If we have a WShell under us, then try to get a local
     *  environment variable with the name USER to override
     *  the global USER setting.
     *  If you don't have a WShell under you, then why not? :-)
     *  Basically, you are less than human if you don't have a WShell
     *  under you.  If everyone had a WShell under their ARexx,
     *  I wouldn't have to write such weird ARexx code.  I hate you
     *  if you don't WShell under your ARexx, cause you make my life
     *  Hell!  Does anyone read these comments, or I'm I wasting
     *  my time here? :-)
     */
    /* AmigaDOG braindamage: */
    'get USER >nil:'
    if rc == 0 then
      'get USER | execio var USERX'
    else
      USERX = ''
    if USERX ~= '' then
      USER = USERX
  end
else
  say 'warning: WShell not present under your ARexx, local environment variable USER can''t be checked'

if USER == '' then
  do
    say 'warning: USER environment variable not set, mail will be sent to root'
    USER = 'root'
  end

address CYBERCRON 'ADD_EVENT :MAILUSER' USER ':EXECONCE :OBEYQUEUE' QUEUE MINUTE HOUR '* * *' scriptfile

DATE = date('i')
TIME = time()
say 'warning: commands will be executed using your UserShell'
say 'job' scriptfile 'at' left(date('w', DATE), 3) date('m', DATE) left(date('e', DATE), 2) TIME left(date('s', DATE), 4)
