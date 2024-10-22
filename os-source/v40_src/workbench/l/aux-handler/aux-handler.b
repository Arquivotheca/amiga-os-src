section "serhand"

get "bcpl/LIBHDR"
get "bcpl/IOHDR"
get "bcpl/MANHDR"
get "bcpl/FH3MANIFESTS"
get "bcpl/exec_lib.hdr"

// A version of the console handler which uses the serial line,
// not the window system.


MANIFEST $(
bufmask    = #XFF
delcharlen = 3

ctrl.c = 'C'-'@'
ctrl.d = 'D'-'@'
ctrl.e = 'E'-'@'
ctrl.f = 'F'-'@'
ctrl.x = 'X'-'@'
eof    =  #X1C      // Actually ^\

   IO.ctlchar = 48     // Long
   IO.rbuflen = 52     // Long
   IO.wbuflen = 56     // Long
   IO.baud    = 60     // Long
   IO.readlen = 76     // Byte
   IO.writelen= 77     // Byte
   IO.stopbits= 78     // Byte
   IO.serflags= 79     // Byte
   IO.sersize = 21     // Size in longs

$)


LET start(parm.pkt) BE
$( 
   LET ebuf = VEC 63          // 255 char buffer
   LET readp, tailp, echop, headp = 0, 0, 0, 0
   LET raw = parm.pkt!pkt.arg2 NE 0 // FALSE
   LET charbuf = VEC 1
   LET outbuf = (charbuf<<2)+4
   LET output.pkt = 0
   LET BSpend = 0
   LET EOFpend = FALSE
   LET EOLpend = FALSE
   LET current.input, current.output = 0, 0
   LET usecount = 0
   LET ignoring = false
   LET clienttaskid = parm.pkt!pkt.taskid
   LET openstring = parm.pkt!pkt.arg1

   LET delchar   = ("*B*S*B"<<2)+1

   LET timerpkt  = VEC pkt.type
   LET timerIOB  = VEC IO.size
   LET timeoutpkt = 0

   LET inpkt = VEC pkt.res1
   LET outpkt= VEC pkt.res1
   LET IOB    = VEC IO.sersize
   LET IOBO   = VEC IO.sersize

   LET devname = "serial.device*X00"

//selectoutput(findoutput("**"))
//writef("AUX-HANDLER: Invoked*N")


   IF OpenDevice( IOB, devname, 0, 0 ) = 0 THEN
   $( returnpkt(parm.pkt,FALSE,error.InvalidWindow)
      return
   $)

   copyvec(IOB,IOBO,IO.sersize)
   outpkt!pkt.type := act.write

   // Set up initial read and send it out.
   inpkt!pkt.type := act.read
   SetIO( IOB, IOC.read, charbuf, 1 );


   timerpkt!pkt.type := action.timer

   // Finished with parameter packet...send back...
   returnpkt( parm.pkt, TRUE )


   // This is the main repeat loop waiting for an event

   $( LET p = taskwait()
      LET ch = 0
      LET printlen, printbuf = 0, ?

      SWITCHON p!pkt.type INTO
      $(

      CASE act.findinput:     // Open
      CASE act.findoutput:
      $( LET scb = p!pkt.arg1
         scb!scb.id := TRUE   // Interactive
	 IF usecount=0 THEN SendIO(IOB, inpkt ) // // First open
         usecount := usecount+1
         returnpkt(p,TRUE)
         LOOP
      $)

      CASE act.end:          // Close
         usecount := usecount-1
rpkt:    returnpkt(p,TRUE)
         LOOP

      CASE act.read:      // A returning msg containing a character
      $( LET nheadp = (headp+1) & bufmask
         IF IOB%IO.error = 0 THEN 
         $( LET ch = charbuf%0        // Extract character
	    IF ctrl.c <= ch <= ctrl.f THEN       // Always set flags
	    $( setflags( clienttaskid, 1<<(ch-ctrl.c) )
	       UNLESS raw BREAK
	    $)
	    UNLESS raw DO
	    $(
	       IF ignoring THEN
	       $(
		  ignoring := (#X20 <= ch <= #X3F)
		  break
               $)

               SWITCHON ch INTO     // Handle special cases
               $( CASE #X9B: 
		     ignoring := TRUE
	             break	
		  CASE eof:
	             EOFpend := TRUE
		     tailp := headp
		     BREAK
	          CASE '*C' : 
                     ch := '*N'
                     tailp := nheadp
                     ENDCASE
                  CASE '*B' :
                  CASE ctrl.x:
                  $( IF headp=tailp BREAK
                     IF echop=headp THEN
                     $( echop := (echop-1) & bufmask
                        BSpend := BSpend+1
                     $)
                     headp := (headp-1) & bufmask
                  $) REPEATUNTIL ch='*B'
                  BREAK
               $)
	    $)
	    IF nheadp=readp BREAK     // No room in buffer
	    IF raw THEN
	    $( echop := (echop+1) & bufmask
               tailp := nheadp
	    $)
            ebuf%nheadp := ch         // Into echo buffer
            headp := nheadp           // Bump ptr
            BREAK
         $) REPEAT
//         SetIO( IOB, IOC.read, charbuf, 1 );
         SendIO( IOB, inpkt )
         ENDCASE
      $)

      CASE act.write:     // A returning msg after write complete
         IF output.pkt > 0 THEN qpkt( output.pkt )    // Return to client
         output.pkt := 0
         ENDCASE

      CASE 'R':            // A read request
	 clienttaskid := p!pkt.taskid   // Set my client only if reading
         append(@current.input, p)
         ENDCASE

      CASE 'W':            // A write request
         append(@current.output, p)
         ENDCASE

      CASE action.waitchar:
      $( LET period = p!pkt.arg1
	 UNLESS readp=tailp DO		     // 6Nov85 K
	 $( returnpkt( p, TRUE )             // Character already available
	    LOOP
         $)
	 IF period=0 THEN
	 $( returnpkt( p, FALSE )            // since fell thru above: FALSE
            LOOP
         $)
	 // Must make a timer request
	 OpenDevice( timerIOB, "timer.device*X00", 0, 0)
	 timeoutpkt := p
         SetIO( timerIOB, IOC.addrequest, ?, period REM 1000000)
	 timerIOB!IO.secs.p := period / 1000000
	 SendIO( timerIOB, timerpkt )
	 LOOP
      $)

      CASE action.timer:
	 UNLESS timeoutpkt=0 DO
	 $( returnpkt( timeoutpkt, FALSE )
	    timeoutpkt := 0
	 $)
	 CloseDevice( timerIOB )
	 LOOP

      CASE action.discinfo:
      $( LET v = p!pkt.arg1
	 clearvec(v,discinfo.size)
	 v!discinfo.type:= raw -> type.rawcon, type.con
	 v!discinfo.vol := 0
	 v!discinfo.inuse := IOB<<2 // Extra for Kodiak 24Feb86
	 GOTO rpkt
      $)


      CASE act.sc.mode:            // Extra item 9 July 86
         raw := p!pkt.arg1
	 GOTO rpkt

      DEFAULT:
         returnpkt(p,FALSE,Error.ActionNotKnown)
      $)

      // At this point something has happened. We must see
      // what we are now free to do.
      // First, see if we can complete any outstanding input request
      // First case: See if an EOF pending
      TEST readp=tailp THEN   
         IF EOFpend & current.input NE 0 THEN
	 $( LET pkt = current.input
	    current.input := !pkt
	    pkt!pkt.link := pkt!pkt.res1
	    returnpkt( pkt, 0 )
	    EOFpend := FALSE
	 $)
      // Second case: Input pending; no pending read but a timeout waiting
      ELSE TEST current.input=0 THEN
	 UNLESS timeoutpkt=0 DO        // Handle timeout request
         $( exec( e.AbortIO, ?, ?, ?, timerIOB<<2 )
            returnpkt( timeoutpkt, TRUE )
            timeoutpkt := 0
         $)
      // Third case: Only valid when read pending and data ready
      ELSE
      $( LET pkt = current.input
         LET buf = pkt!pkt.arg2
         LET end = buf+pkt!pkt.arg3
	 LET ptr = buf
         current.input := !pkt
         pkt!pkt.link := pkt!pkt.res1        // Restore link ptr
         UNTIL buf >= end | readp=tailp DO
         $( readp :=(readp+1) & bufmask
            0%buf := ebuf%readp
            buf   := buf+1
         $)
	 ptr := buf-ptr
         returnpkt( pkt, ptr)
      $)


      // Next, see if any output is required

      WHILE output.pkt=0 DO 
      $( 
         // Character typed requires reflection
         UNLESS echop=headp DO
         $( echop := (echop+1) & bufmask
            ch := ebuf%echop
            BREAK
         $)

         // A BS was typed and we must delete last char typed
         IF BSpend>0 THEN
         $( BSpend := BSpend-1
            printbuf, printlen := delchar, delcharlen
            BREAK
         $)

         // The last character was *N and we must provide *C
         IF EOLpend THEN
         $( EOLpend := FALSE
            ch := '*C'
            BREAK
         $)

         // A line is waiting to be output. We don't print this
         // if the user is halfway through a line
         UNLESS current.output=0 | headp NE tailp DO
         $( output.pkt := current.output
            printbuf, printlen := output.pkt!pkt.arg2, output.pkt!pkt.arg3
            current.output := !output.pkt
            output.pkt!pkt.link := output.pkt!pkt.res1 // Restore link ptr
            output.pkt!pkt.res1 := printlen
            IF printlen <= 0 THEN
            $( qpkt( output.pkt )
               output.pkt := 0
               LOOP
            $)
         $)

         BREAK

      $)

      // If ch is nonzero then place it in output buffer

      UNLESS ch=0 DO
      $( printbuf, printlen := outbuf, 1
         0%printbuf := ch
      $)

      // If printlen is nonzero then print that many chars
      // from printbuf. 

      IF printlen>0 THEN
      $( IF 0%(printlen+printbuf-1)='*N' THEN 
            EOLpend := TRUE
         SetIO( IOBO, IOC.write, ?, printlen )
         putlong( IOBO, IO.data, printbuf )
         SendIO( IOBO, outpkt )
         UNLESS output.pkt>0 DO output.pkt := -1
      $)
   $) REPEATUNTIL (usecount <= 0) & current.output=0 & output.pkt=0

   // Termination
   CloseDevice( IOBO )

$)

AND append( lvp, p ) BE
$( UNTIL !lvp=0 DO lvp := !lvp
   !lvp := p
   p!pkt.res1 := !p
   !p := 0
$)
