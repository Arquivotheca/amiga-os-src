/* Examples of functions in rexxsupport.library */
trace r

if ~show('L',"rexxsupport.library") then do
   if addlib('rexxsupport.library',0,-30,0) then
      say 'added rexxsupport.library'
   else do;
      say 'support library not available'
      exit 10
      end
   end

/* Allocate some memory                         */
block = allocmem(4000)

/* now release it ...                           */
if block ~= "0000 0000"x then
   say freemem(block,4000)

/* Delay for a while ...                        */
say 'Wait for a few ...'
call delay 100
say 'seconds'

/* check the status of a file ...               */
say statef(':rexx/support.rexx')
say statef('ram:')

/* Get a directory as a string of names         */
say showdir('ram:')
say showdir('ram:',,';')

/* print various system lists ...               */
say showlist('Assigns')       /* DOS assignments*/
say showlist('D')             /* EXEC Devices   */
say showlist('Handlers')      /* DOS devices    */
say showlist('I')             /* EXEC IntrList  */
say showlist('L')             /* EXEC Libraries */
say showlist('MemList')       /* EXEC MemList   */
say showlist('P')             /* EXEC Ports     */
say showlist('R')             /* EXEC Resources */
say showlist('S')             /* EXEC Semaphores*/
say showlist('T')             /* EXEC TaskReady */
say showlist('Volumes')       /* DOS volumes    */
say showlist('W')             /* EXEC TaskWait  */

/* Check for a specific node name               */
say showlist('A','FONTS')

/* Use a separator character                    */
say showlist('P',,';')

/* Open a public message port                   */
if ~openport('MYPORT') then exit 10

/* Spawn a process to send us a message!        */
pgm = 'run rx >con:0/0/640/60/New "address MYPORT;hello;say rc"'
address command pgm

/* Wait for the message packet                  */
if ~waitpkt('MYPORT') then do;
   say 'no packet sent??'
   exit
   end

/* Get the packet address as a 4-byte string    */
pkt = getpkt('MYPORT')        /* a packet?      */
say c2x(pkt)                  /* here it is     */

/* Inspect the command, then return an error    */
if (pkt ~= '0000 0000'x) then do;
   say getarg(pkt)            /* argument string*/
   call reply pkt,10          /* send it back   */
   end

/* close the port ...                           */
call closeport 'MYPORT'

exit
