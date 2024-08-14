/***********************************************************************
**    (C) Copyright 1980  TRIPOS Research Group                       **
**      University of Cambridge Computer Laboratory                   **
**    (C) Copyright 1985  METACOMCO plc                               **
**      26 Portland Square, Bristol, England                          **
**
                 #######   ##        ########  #######
                 ########  ##        ########  ########
                 ##    ##  ##           ##     ##    ##
                 #######   ##           ##     #######
                 ##    ##  ##           ##     ##    ##
                 ##    ##  ##           ##     ##    ##
                 ########  ########  ########  ########
                 #######   ########  ########  #######

************************************************************************
**     BCPL Library for UNIX                                          **
***********************************************************************/



// Modified 12 June 1980 by BJK: New interface for operations
//                               involving filenames
// 68000 version - TJK Apr 82 onwards
// UNIX Version PJF 1984 onwards

SECTION "BLIB"

GET "LIBHDR"
GET "SYSHDR"


LET unpackstring(s, v) BE
    FOR i = s%0 TO 0 BY -1 DO v!i := s%i


AND packstring(v, s) = VALOF
$( LET n = v!0 & 255
   LET size = n/bytesperword
   FOR i = 0 TO n DO s%i := v!i
   FOR i = n+1 TO (size+1)*bytesperword-1 DO s%i := 0
   RESULTIS size
$)


AND readn() = VALOF
 $( LET sum, ch = 0, 0
    AND neg = FALSE

l:  ch := rdch()
    UNLESS '0'<=ch<='9' DO
       SWITCHON ch INTO
      $( DEFAULT:    unrdch()
                     result2 := -1
                     RESULTIS 0
          CASE '*S':
          CASE '*T':
          CASE '*N': GOTO l

          CASE '-':  neg := TRUE
          CASE '+':  ch := rdch()
    $)

    WHILE '0'<=ch<='9' DO
    $( sum := 10*sum + ch - '0'
       ch := rdch() $)

    IF neg DO sum := -sum
    unrdch()
    result2 := 0
    RESULTIS sum
 $)


AND newline() BE wrch('*N')


AND writed(n, d) BE
 $( LET t = VEC 10
    AND i, k = 0, -n
    IF n<0 DO d, k := d-1, n
    t!i, k, i := -(k REM 10), k/10, i+1 REPEATUNTIL k=0
    FOR j = i+1 TO d DO wrch('*S')
    IF n<0 DO wrch('-')
    FOR j = i-1 TO 0 BY -1 DO wrch(t!j+'0')
 $)

AND writen(n) BE writed(n, 0)

AND writehex(n, d) BE
 $( IF d>1 DO writehex(n>>4, d-1)
    wrch((n&15)!TABLE
         '0','1','2','3','4','5','6','7',
         '8','9','A','B','C','D','E','F' )
 $)

AND writeoct(n, d) BE
 $( IF d>1 DO writeoct(n>>3, d-1)
    wrch((n&7)+'0')
 $)

AND writes(s) BE
    FOR i = 1 TO s%0 DO wrch(s%i)

AND writet(s, n) BE
 $( writes(s)
    FOR i = 1 TO n-s%0 DO wrch('*S')
 $)

AND writeu(n, d) BE
 $( LET m = (n>>1)/5
    UNLESS m=0 DO
    $( writed(m, d-1)
       d := 1 $)
    writed(n-m*10, d)
 $)

AND writef(format, a, b, c, d, e, f, g, h, i, j, k) BE
$(1 LET t = @ a

    FOR p = 1 TO format%0 DO
    $(2 LET k = format%p

        TEST k='%'

          THEN $(3 LET f, arg, n = 0, t!0, 0
                   p := p + 1
                $( LET type = capitalch(format%p)
                   SWITCHON type INTO
                $( DEFAULT: wrch(type); ENDCASE

                   CASE 'S': f := writes; GOTO l
                   CASE 'T': f := writet; GOTO m
                   CASE 'C': f := wrch; GOTO l
                   CASE 'O': f := writeoct; GOTO m
                   CASE 'X': f := writehex; GOTO m
                   CASE 'I': f := writed; GOTO m
                   CASE 'N': f := writen; GOTO l
                   CASE 'U': f := writeu; GOTO m

                m: p := p + 1
                   n := format%p
                   n := '0'<=n<='9' -> n-'0',
                        10+n-'A'

                l: f(arg, n)

                   CASE '$':
                   t := t + 1
               $)3

          ELSE wrch(k)
    $)2
$)1


AND capitalch(ch) = 'a' <= ch <= 'z' -> ch + 'A' - 'a', ch


AND compch(ch1, ch2) = capitalch(ch1) - capitalch(ch2)


AND compstring(s1, s2) = VALOF
    $(
    LET lens1, lens2 = s1%0, s2%0
    LET smaller = lens1 < lens2 -> s1, s2

    FOR i = 1 TO smaller%0
    DO
        $(
        LET res = compch(s1%i, s2%i)

        UNLESS res = 0 RESULTIS res
        $)

     IF lens1 = lens2 RESULTIS 0

    RESULTIS smaller = s1 -> -1, 1
    $)



AND rdargs(keys, argv, size) = VALOF
 $( LET w = argv
    LET numbargs = ?

    !w := 0
    FOR p = 1 TO keys%0 DO
      $( LET kch = keys%p
         IF kch = '/' DO
           $( LET c = capitalch(keys%(p+1))
              IF c = 'A' THEN !w := !w | 1
              IF c = 'K' THEN !w := !w | 2
              IF c = 'S' THEN !w := !w | 4
              LOOP
           $)
         IF kch = ',' THEN
           $( w := w+1
              !w := 0
           $)
      $)
    w := w+1
    numbargs := w-argv

// At this stage, the argument elements of argv have been
// initialised to  0    -
//                 1   /A
//                 2   /K
//                 3   /A/K
//                 4   /S
//                 5   /S/A
//                 6   /S/K
//                 7   /S/A/K

    $( LET argno = -1
       LET wsize = size + argv - w

       SWITCHON rditem(w, wsize) INTO
       $( DEFAULT:
 err:     $( LET ch = ?
             ch := rdch() REPEATUNTIL ch='*E' | ch='*N' |
                        ch=';' | ch=endstreamch
             result2 := 120
             RESULTIS 0
          $)

          CASE 0:  // *N, *E, ;, endstreamch
             FOR i = 0 TO numbargs-1 DO
               $( LET a = argv!i
                  IF 0 <= a <= 7 THEN
                  TEST (a & 1) = 0 THEN
                    argv!i := 0
                   ELSE
                    GOTO err
               $)
             rdch()
             RESULTIS w

          CASE 1:  // ordinary item
             argno := findarg(keys, w)
             TEST argno>=0 THEN  // get and check argument
               TEST 4 <= argv!argno <= 7 THEN
                 $( // no value for key.
                    argv!argno := -1
                    LOOP
                 $)
               ELSE
                 $( LET item = rditem(w,wsize)
                    IF item = -2 THEN
                       item := rditem(w,wsize)
                    IF item <= 0 THEN
                       GOTO err
                 $)
             ELSE
               TEST rdch()='*N' & compstring("?", w)=0 THEN
                 $( // help facility
                    writef("%S: ", keys)
                    wrch(0)    // Flush buffer
                    ENDCASE
                 $)
               ELSE
                 unrdch()

          CASE 2:  // quoted item (i.e. arg value)
             IF argno<0 THEN
               FOR i = 0 TO numbargs-1 DO
                 SWITCHON argv!i INTO
                   $( CASE 0: CASE 1:
                        argno := i
                        BREAK
                      CASE 2: CASE 3:
                        GOTO err
                   $)
             UNLESS argno>=0 GOTO err

             argv!argno := w
             w := w + w%0/bytesperword + 1
       $)

    $) REPEAT
 $)


// Read an item from command line
// returns -2    "=" Symbol
//         -1    error
//          0    *N, *E, ;, endstreamch
//          1    unquoted item
//          2    quoted item
AND rditem(v, size) = VALOF
 $( LET p = 0
    LET pmax = (size+1)*bytesperword-1
    LET ch = ?
    LET quoted = FALSE

    FOR i = 0 TO size DO v!i := 0

    ch := rdch() REPEATWHILE ch='*S'

    IF ch='"' DO
    $( quoted := TRUE
       ch := rdch()
    $)

    UNTIL ch='*E' | ch='*N' | ch=endstreamch DO
    $( TEST quoted THEN
       $( IF ch='"' RESULTIS 2
          IF ch='**' DO
          $( ch := rdch()
             IF capitalch(ch)='E' DO ch := '*E'
             IF capitalch(ch)='N' DO ch := '*N'
          $)
       $)
       ELSE
          IF ch=';' | ch='*S' | ch='=' BREAK
       p := p+1
       IF p>pmax RESULTIS -1
       v%p := ch
       v%0 := p
       ch := rdch()
    $)

    unrdch()
    IF quoted RESULTIS -1
    TEST p=0 THEN
    $( IF ch='=' DO
       $( rdch()
          RESULTIS -2
       $)
       RESULTIS 0
    $)
    ELSE
       RESULTIS 1
 $)


AND findarg(keys, w) = VALOF  // =argno if found
                              // =-1 otherwise
  $( MANIFEST $( matching = 0; skipping = 1 $)

     LET state, wp, argno = matching, 0, 0

     FOR i = 1 TO keys % 0 DO
       $( LET kch = keys % i
          IF state = matching THEN
            $( IF (kch = '=' | kch= '/' | kch =',') &
                  wp = w % 0 THEN
                 RESULTIS argno
               wp := wp + 1
               UNLESS compch(kch,w % wp) = 0 THEN
                 state := skipping
            $)
          IF kch = ',' | kch = '=' THEN
            state,wp := matching,0
          IF kch=',' THEN
            argno := argno+1
       $)
     IF state = matching & wp = w % 0 THEN
       RESULTIS argno
     RESULTIS -1
  $)


AND getbyte(v,offset) = v%offset


AND putbyte(v,offset,byte) BE v%offset := byte


AND bst2cst(bstr,cstr) BE
$( LET len = bstr%0
   FOR i=1 TO len DO 0%(cstr+i-1) := bstr%i
   0%(cstr+len) := 0
$)


AND cst2bst(cstr,bstr) = VALOF
$( LET len = 0
   LET ch  = 0%cstr
   UNTIL ch = 0 DO
   $( len := len + 1
      IF len > 255 THEN
      $( bstr%0 := 255
         RESULTIS FALSE
      $)
      bstr%len := ch
      ch := 0%(cstr+len)
   $)
   bstr%0 := len
   RESULTIS TRUE
$)


AND transargs(argc,argv) BE
$(
   LET pos  = 0
   LET buf  = @(stdin!scb.buff)
   LET maxc = scb.buffsize << 2     // size of scb buffer in bytes

   result2 := FALSE       // Indicate failure
   argv    := argv >> 2   // make bcplptr

   FOR i=1 TO argc-1 DO
   $( LET cstr = argv!i         
      LET ch   = 0%cstr
      LET len  = 0

      $( IF pos >= maxc THEN
         $( stdin!scb.end := pos-1
            RETURN
         $)
         IF (ch = 0) THEN
         $( IF (i=argc-1) THEN BREAK
            ch := '*S'
         $)
         buf%pos := ch
         pos := pos + 1
         len := len + 1
         IF ch = '*S' THEN BREAK
         ch  := 0%(cstr+len)
      $) REPEAT
   $)

   // Ensure a '*N' is in the input stream
   TEST pos >= maxc THEN
   $( stdin!scb.end := pos-1
      RETURN
   $)
   ELSE
   $( buf%pos := '*N'
      pos     := pos + 1
   $)

   // Update the scb and return 'all OK'
   stdin!scb.end := pos
   result2 := TRUE
$)


// Library I/O for UNIX


AND findinput(string) = findstream(string, id.inscb)


AND findoutput(string) = findstream(string, id.outscb)


AND findstream(string, id) = VALOF
$(
   LET scb    = getvec(scb.upb)
   LET name   = VEC 64
   LET oflag  = (id = id.inscb) -> o.findinput, o.findoutput
   LET fildes = ?

   IF scb=0 THEN
   $( result2 := ENOMEM
      RESULTIS 0
   $)

   FOR i = 0 TO scb.upb DO scb!i := -1

   bst2cst(string,name<<2)
   fildes := open(name<<2,oflag,#O644)

   TEST fildes=-1 THEN
   $( freevec(scb)
      RESULTIS 0
   $)
   ELSE
   $( scb!scb.id, scb!scb.fildes := id, fildes
      scb!scb.pos, scb!scb.end   :=  0, -1
      RESULTIS scb
   $)
$)


AND selectinput(scb) BE
$( UNLESS scb=0 | scb!scb.id=id.inscb DO
      abort( err.BadInstream )
   cis := scb
$)


AND selectoutput(scb) BE
$( // First Flush current output stream
   UNLESS cos = 0 DO
      deplete(cos)

   UNLESS scb=0 | scb!scb.id=id.outscb DO
      abort( err.BadOutStream )

   cos := scb
$)


AND rdch() = VALOF
$( LET pos, end = cis!scb.pos, cis!scb.end
   IF pos>=end THEN   // Replenish buffer
   $( LET fildes = cis!scb.fildes
      LET buff   = (@(cis!scb.buff)) << 2
      end := read(fildes,buff,scb.buffsize<<2)
      cis!scb.end := end
      pos, cis!scb.pos := 0,0
   $)
   TEST pos >= end THEN
     RESULTIS endstreamch
   ELSE
   $( cis!scb.pos := pos+1
      RESULTIS (@(cis!scb.buff))%pos
   $)
$)


AND unrdch() = VALOF
$( LET pos = cis!scb.pos
   IF pos<=0 RESULTIS cis!scb.end=0
   // Attempt to 'UnRdch' past buffer origin.
   cis!scb.pos := pos-1
   RESULTIS TRUE
$)


AND wrch(ch) BE
$( LET pos = cos!scb.pos

   IF pos>=scb.buffsize<<2 DO
   $( deplete(cos) 
      pos := 0
   $)

   (@(cos!scb.buff))%pos := ch
   cos!scb.pos := pos+1

   IF (cos = stdout) &
      (ch='*N' | ch=0 | ch='*P' | ch='*C') THEN deplete(cos)
$)


AND deplete(stream) BE
$(
   LET res2   = result2
   LET buff   = (@(stream!scb.buff)) << 2
   LET pos    = stream!scb.pos
   LET fildes = stream!scb.fildes

   UNLESS stream!scb.id=id.outscb DO
      abort( err.BadOutStream )

   UNLESS pos = 0 DO
   $( stream!scb.pos := 0
      UNLESS write(fildes,buff,pos) = pos DO
         abort( err.WriteFailed )
   $)
            
   result2 := res2
$)


AND note( scb ) = VALOF
$( LET bytepos = ?

   IF scb = cos THEN
     deplete(scb) 

   bytepos := lseek( scb!scb.fildes, 0, 1 )

   UNLESS bytepos = -1 DO
     IF scb!scb.id = id.inscb THEN
        bytepos := bytepos - scb!scb.end + scb!scb.pos
 
   RESULTIS  bytepos
$) 


AND point( scb, n ) = VALOF
$( TEST scb!scb.id = id.outscb THEN
     deplete(scb)
   ELSE scb!scb.end := 0

   RESULTIS lseek( scb!scb.fildes, n, 0 )
$)
  

AND pointword( n ) = point( cis, n<<2 )


AND endread() = (cis = 0) -> 0, VALOF
$( LET res = endstream(cis)
   cis := 0
   RESULTIS res
$)


AND endwrite() = (cos = 0) -> 0, VALOF
$( LET res = ?
   UNLESS cos = 0 DO deplete(cos)
   res := endstream(cos)
   cos := 0
   RESULTIS res
$)

AND endstream(scb) = (scb = 0) -> 0, VALOF
$( LET res  = ?
   IF scb!scb.id = id.outscb THEN deplete(scb)
   res := close(scb!scb.fildes)
   freevec(scb)
   RESULTIS res
$)

AND input() = cis

AND output() = cos

AND writewords( v, n ) = VALOF
$( LET res = writebytes( v, n<<2 ) 
   RESULTIS (res = -1) -> -1, res >> 2
$)

AND readwords( v, n ) = VALOF
$( LET res = readbytes( v, n<<2 ) 
   RESULTIS (res = -1) -> -1, res >> 2
$)

AND writebytes( v, n ) = write( cos!scb.fildes, v<<2, n )

AND readbytes( v, n ) = read( cis!scb.fildes, v<<2, n )

AND deleteobj(name) = VALOF
$( LET v = VEC 64
   LET cname = v << 2
   bst2cst(name,cname)
   RESULTIS unlink(cname)
$)


// Code to deal with BCPL stack backtrace

AND abort(code) BE
$(
   LET stackptr = @CODE             // Current stack ptr
   
   selectoutput(stderr) 

   WRITES("*N****** BCPL ABORT ****** ")
   writef( VALOF 
     SWITCHON code INTO
     $( CASE 0:                RESULTIS "User Abort"
        CASE err.BadInStream:  RESULTIS "Bad Input Stream"
        CASE err.BadOutStream: RESULTIS "Bad Output Stream"
        CASE err.BadStream:    RESULTIS "Bad Stream Specified"
	CASE err.WriteFailed:  RESULTIS "Write Failure"
        DEFAULT :              RESULTIS "Error code = %n"
     $), code)
   writes("*N*N")

   backtrace( (stackptr!-3) >> 2, stackptr )

   unless code = 0 DO stop(1)
$)


AND backtrace(stackptr,oldsptr) BE
$( LET sbase   = stackbase >> 2              // Stackbase as BCPL ptr
   UNTIL stackptr = sbase DO
   $( LET codeptr = stackptr!-1
      LET argptr  = stackptr
      writef("%S  ", (codeptr-8)>>2)         // write out routines name
      FOR i = 1 TO 8 DO                      // write out arguments etc
      $( IF argptr >= oldsptr-3 BREAK        // reached next stack frame
         writearg(!argptr)
         argptr := argptr+1
      $)
      newline()
      oldsptr  := stackptr            // move to previous
      stackptr :=(stackptr!(-3))>>2   // stack frame
   $)
   writes("*NEnd of backtrace*N*N")
$)

// Print value in suitable fashion
AND writearg(arg) be
$( 
   TEST -100000000 < arg < 100000000 THEN
      writef(" %I9",arg)                 // write out as decimal
   ELSE 
   $( LET gn = (arg - undefined.global)/2
      TEST 0 < gn < 1000 THEN
         writef("   glob%I3",gn)         // write out unassigned global
      ELSE
         writef(" $%X8",arg)             // write out in hex
   $)
$)

