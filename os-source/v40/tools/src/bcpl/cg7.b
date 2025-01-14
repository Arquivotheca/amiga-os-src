//***************************************************************************
//* Contains code for TRIPOS, AMIGADOS, UNIX4, UNIX5 and Assembly Back Ends *
//***************************************************************************

SECTION "M68CG7"

GET "CGHDR"


// Amiga Object file descriptors

MANIFEST
$(
   hunk.pu     =  999
   hunk.name   = 1000
   hunk.code   = 1001
   hunk.ext    = 1007
   hunk.symbol = 1008
   hunk.end    = 1010

   ext.abs     =   2
   ext.ref32   = 129
   ext.ref16   = 131
$)


LET outputsection(last) BE
$(
   selectoutput(codestream)

   totalcodesize := totalcodesize + stvp/4

   TEST  altobj  THEN
      assemblymodule(last)
   ELSE
   $(
      SWITCHON objectmodule INTO
      $(
         DEFAULT         : bug(99)                            ; ENDCASE
$<MCC
         CASE o.tripos   : triposhunk(last)                   ; ENDCASE
         CASE o.mtripos  :
         CASE o.amigados : amigahunk(last)                    ; ENDCASE
         CASE o.unix4.2  : unix4.2hunk(last)                  ; ENDCASE
         CASE o.unix5.0  : unix5.0hunk(last)                  ; ENDCASE
         CASE o.gst      : gsthunk(last)                      ; ENDCASE
         CASE o.tos      : toshunk(last)                      ; ENDCASE
$>MCC
$<ATARI
         CASE o.gst      : gsthunk(last)                      ; ENDCASE
         CASE o.tos      : toshunk(last)                      ; ENDCASE
$>ATARI
$<AMIGA
         CASE o.amigados : amigahunk(last)                    ; ENDCASE
$>AMIGA
$<IMPERIAL
         CASE o.unix5.0  : unix5.0hunk(last)                  ; ENDCASE
$>IMPERIAL
$<RELEASE
         CASE o.tripos   : triposhunk(last)                   ; ENDCASE
$>RELEASE
      $)
   $)

   selectoutput(verstream)
$)


AND dboutput(lev) BE
$(1
    IF lev>3 DO
    $( LET p = rlist
       writes("*NRLIST:  ")
       UNTIL p=0 DO
       $( writef("%N:L%N ", h2!p, h3!p)
          p := !p
    $)
    $)

    IF lev>2 DO
    $( writes("*NSLAVE: ")
       FOR r = r0 TO r7 DO
       $( LET p = slave!r
          IF p=0 LOOP
          writef("   R%N= ", r)
          UNTIL p=0 DO
          $( wrkn(h2!p, h3!p)
             p := !p
          $)
       $)
    $)

    IF lev>1 DO
    $( writes("*NSTACK: ")
       FOR p=tempv TO arg1 BY 3 DO wrkn(h1!p,h2!p)
    $)

    writef("*NOP=%I3/%I3  SSP=%N LOC=%N*N",
           op,pendingop,ssp,stvp)
$)1


AND wrkn(k,n) BE
$(1 LET s = VALOF SWITCHON k INTO
    $( DEFAULT:          RESULTIS "?"
       CASE k.numb:      RESULTIS "N%N"
       CASE k.loc:       RESULTIS "P%N"
       CASE k.glob:      RESULTIS "G%N"
       CASE k.lab:       RESULTIS "L%N"
       CASE k.locsh:     RESULTIS "PS%N"
       CASE k.globsh:    RESULTIS "GS%N"
       CASE k.labsh:     RESULTIS "LS%N"
       CASE k.lvloc:     RESULTIS "@P%N"
       CASE k.lvglob:    RESULTIS "@G%N"
       CASE k.lvlab:     RESULTIS "@L%N"
       CASE k.lvlocsh:   RESULTIS "@PS%N"
       CASE k.lvglobsh:  RESULTIS "@GS%N"
       CASE k.lvlabsh:   RESULTIS "@LS%N"
       CASE k.reg:       RESULTIS "R%N"
       CASE k.ir0:       RESULTIS "(R0,%N)"
       CASE k.ir1:       RESULTIS "(R1,%N)"
       CASE k.ir2:       RESULTIS "(R2,%N)"
       CASE k.ir3:       RESULTIS "(R3,%N)"
       CASE k.ir4:       RESULTIS "(R4,%N)"
       CASE k.ir5:       RESULTIS "(R5,%N)"
       CASE k.ir6:       RESULTIS "(R6,%N)"
       CASE k.ir7:       RESULTIS "(R7,%N)"
    $)
    writef(s,n)
    wrch('*S')
$)1


// Now for all the conditional code

// Produce code to go through a 68000 assembler
// The standard UNIX assembler doen't use standard nmemomics
// hence the difference
$<UNIX'
AND objword(w) BE writef("*TDC.L  $%X8*N", w)
$>UNIX'
$<UNIX
AND objword(w) BE writef("*T.long  0x%X8*N", w)
$>UNIX

// Produce an assembly module to go through
// the assembler of the system we're on.

AND assemblymodule(last) BE
$(
    FOR p=0 TO stvp/4-1 DO
    $( IF p REM 8 = 0 DO newline()
       objword(stv!p)
    $)

$<UNIX'
    IF last THEN
       writes( "*N*TEND*N" )
$>UNIX'
$<UNIX
    newline()         // No end directive for unix assembler
$>UNIX

$)

$<ATARI'
$<AMIGA'
$<IMPERIAL'
AND triposhunk(last) BE
$(
   LET hu, size, en = t.hunk, stvp/4, t.end
   writewords(@ hu, 1)
   writewords(@ size, 1)
   writewords(stv, size)
$<EXT
   IF (extrnsymbols NE 0) THEN
   $( writeword(1005)
      triposxrefs()
      writeword(0)
   $)
$>EXT
   IF last DO writewords(@ en, 1)
$)

$<EXT
// XREFs to 'C'
AND triposxrefs() BE
$( LET p = extrnsymbols

   UNTIL p = 0 DO
   $( LET ptr   = p!e.refs
      LET name  = @(p!e.name)
      LET rp    = ptr
      LET nrefs = 0

      UNTIL rp=0 DO
      $( nrefs := nrefs+1
         rp := rp!er.link
      $)

      UNLESS nrefs=0 DO
      $(
         LET buff   = VALOF
           $( dp := dp - 2
              checkspace()
              RESULTIS dp
           $)
         LET len = (name%0 <= 7) -> name%0, 7

         buff!0,buff!1 := 0,0
         buff%0 := 129    // ext.ref
         FOR i=1 TO len DO buff%i := name%i
         writewords(buff,2)
         writeword( nrefs )
         UNTIL ptr=0 DO
         $( writeword(ptr!er.loc)  // output the reference
            ptr := ptr!er.link     // get next ref
         $)
         p!e.refs, p!e.refstail := 0,0   // Throw all refs away
      $)
      p := p!e.link
   $)

   extrnsymbols := 0      // Throw away all xrefs
$)
$>EXT
$>IMPERIAL'
$>AMIGA'
$>ATARI'

$<ATARI'
$<AMIGA'
$<RELEASE'
AND unix5.0hunk(last) BE
$(
   writewords( stv, stvp/4 )   // write out the text

   IF  last  THEN
   $( LET posn = 4
      LET totalbytes = totalcodesize<<2

      mypoint(codestream)

      writewords( @totalbytes, 1 )
   $)
$)
$>RELEASE'
$>AMIGA'
$>ATARI'

$<ATARI'
$<RELEASE'
$<IMPERIAL'

$<AMIGA
AND writeword(w) = writewords(@w,1)
$>AMIGA

AND amigahunk() BE
$(
   writeword(hunk.pu)  // a program unit
   writeword( 0 )      // .. with no name

   IF libraryname NE 0 THEN
   $(
      LET buff   = VEC 10
      LET len    = libraryname%0
      LET lwords = (len+3)/4

      FOR i=0 TO lwords DO buff!i := 0
      FOR i=0 TO len-1 DO buff%i := libraryname%(i+1)

      writeword(hunk.name)     // hunk.name
      writeword( lwords )   // length (long word)
      FOR i=0 TO lwords-1 DO
      writeword(buff!i)
   $)
         
   writeword(hunk.code)      // hunk.code
   writeword( stvp/4)
   writewords( stv, stvp/4 )

$<EXT'
   IF globexternals THEN
$>EXT'
$<EXT
   IF globexternals | (extrnsymbols NE 0) THEN
$>EXT
   $( writeword(hunk.ext)  // Start external block
      amigaglobalxdefs()
      amigaglobalxrefs()
$<EXT
      amigaxrefs()
$>EXT
      writeword(0)     // End external block
   $)

   IF dumpsymbols THEN
   $( LET buff = VEC 3
      LET dest = @(buff!1)
      buff!0 := 2      // Length of name
      buff!3 := 0      // End of symbol Block
      writeword(hunk.symbol)   // Hunk Symbol
      UNTIL symbols= 0 DO
      $( LET name = @(symbols!sym.name)
         buff!1,buff!2,buff!3 := 0,0,symbols!sym.val
         FOR i=1 TO name%0 DO
           dest%(i-1) := name%i
         writewords(buff,4)
         symbols := symbols!sym.link
      $)
      writeword(0)
      symbolstail := 0   // Throw away list
   $)

   writeword(hunk.end)
$)

AND amigaglobalxdefs() BE
$(
    // Output a 16 bit absolute XDEF for each global
    // defined in the current section

    LET  ptr    = stv + (stvp/4) - 3  
    LET  globno = ptr!0
    LET  offset = ptr!1
    LET  buff   = VEC 1
   
    buff ! 0 := #X474C4F42    // GLOB

    //  First the XDEFs

    UNTIL  offset = 0  DO
    $(
        //  Absolute XDEF, name length 2 words

        IF globno >= extlowbound THEN
        $(
           writeword( (ext.abs<<24) | 2 )

           //  Name = GLOBnnnn
 
           FOR i=3 TO 0 BY -1 DO
           $(
              (buff+1)%i := '0' + (globno REM 10)
              globno := globno/10
           $)
        
           writeword( buff!0 )
           writeword( buff!1 )
           writeword( (ptr!0)*4 )
        $)

        ptr := ptr - 2
        globno := ptr!0
        offset := ptr!1
    $)
$)

AND amigaglobalxrefs() BE
$(
    LET  buff   = VEC 1
    LET  globno = ?
   
    buff ! 0 := #X474C4F42    // GLOB

    UNTIL globxrefs = 0 DO
    $(
        LET  ptr   = globxrefs!xref.link
        LET  gn    = globxrefs!xref.globno
        LET  nrefs = 1
        LET  p     = ?

        //  First, go down the list counting the
        //  number of refs for this particular global

        UNTIL ptr=0 DO
        $(
           IF ptr!2 = gn THEN nrefs := nrefs + 1
           ptr := ptr!0
        $)
 
        writeword( (ext.ref16<<24) | 2 )   // 16 bit XREF

        globno := gn
        FOR i=3 TO 0 BY -1 DO
        $(
           (buff+1)%i := '0' + (globno REM 10)
           globno := globno/10
        $)

        // Name and type
        writeword( buff!0 )
        writeword( buff!1 )

        // Number of refs
        writeword( nrefs )

        // Each of the refs
        p   := @globxrefs
        ptr := globxrefs   

        UNTIL ptr = 0 DO
        $(
           TEST ptr!xref.globno = gn THEN
           $( 
              writeword(ptr!xref.location)  // output the reference
              p!xref.link := ptr!xref.link  // unhook ref from list
              ptr := p!xref.link            // get next ref
           $)
           ELSE
           $( // Simply go on to next one in list
              p   := ptr
              ptr := p!xref.link
           $)
        $)
    $)
$)

$<EXT
// XREFs to 'C'
AND amigaxrefs() BE
$( LET p = extrnsymbols

   UNTIL p = 0 DO
   $( LET ptr   = p!e.refs
      LET name  = @(p!e.name)
      LET rp    = ptr
      LET nrefs = 0

      UNTIL rp=0 DO
      $( nrefs := nrefs+1
         rp := rp!er.link
      $)

      UNLESS nrefs=0 DO
      $( LET lwords = (name%0-1)/bytesperword + 1
         LET buff   = VALOF
           $( dp := dp - lwords
              checkspace()
              RESULTIS dp
           $)

         writeword( (ext.ref32<<24) | lwords )

         buff!(lwords-1) := 0
         FOR i=1 TO name%0 DO buff%(i-1) := name%i
         writewords(buff,lwords)

         writeword( nrefs )

         UNTIL ptr=0 DO
         $( writeword(ptr!er.loc)  // output the reference
            ptr := ptr!er.link     // get next ref
         $)

         p!e.refs, p!e.refstail := 0,0   // Throw all refs away
      $)

      p := p!e.link
   $)

   extrnsymbols := 0      // Throw away all xrefs
$)
$>EXT
$>IMPERIAL'
$>RELEASE'
$>ATARI'

$<ATARI'
$<AMIGA'
$<IMPERIAL'
$<RELEASE'
// Need to output a dummy symbol tabel at the end of
// a unix 4.2 output module
AND unix4.2hunk(last) BE
$(
   writewords( stv, stvp/4 )   // write out the text

$<EXT'
   IF last THEN
$>EXT'
$<EXT
   IF last THEN
   TEST extrnsymbols=0 THEN
$>EXT
   $(
      // Update the total size of the TEXT section and
      // produce the symbol table and name table which
      // has to go with a unit in 'a.out' format

      LET symboltable = TABLE #X00000004,   // Offset in name table
                              #X01000000,   // Its an XREF
                              #X00000000    // It's value is 0
      LET nametable   = TABLE #X0000000E,   // Length
                              #X5F626370,   // _bcp
                              #X6C6D6169,   // lmai
                              #X6E000000    // n
      LET posn = 4
      LET totalbytes = totalcodesize<<2

      // Output the symbol table followed by the name table
           
      writewords( symboltable, 3 )        // 3 words of symbol table
      writewords( nametable, 4 )          // Finally the name table

      // Now put in the file length
      // Position to rewind to was saved in 'fileposition'
      mypoint( codestream )

      writewords( @totalbytes, 1 )
   $)
$<EXT
   ELSE
   $( LET info = VEC 6
      LET symtabsize = ?
      LET textrelocsize = countxrefs()*8
      LET nametabsize = ?
      LET totalbytes = totalcodesize << 2

      FOR i=0 TO 6 DO info!i := 0

      symtabsize := Make.External.Ids(0)*12

      // First the reloc info
      unix4.2extrelocinfo()

      // Now produce the symbol Table
      nametabsize := unix4.2symboltable(4)

      // Now the name table
      unix4.2nametable(nametabsize)

      // Finally rewind and patch the header
      mypoint( codestream )
      info!0 := totalbytes
      info!3 := symtabsize
      info!5 := textrelocsize
      writewords( info, 7)
   $)
$>EXT
$)


$<EXT
AND countxrefs() = VALOF
$( LET res = 0
   LET ptr = extrnsymbols
   UNTIL ptr = 0 DO
   $( LET p = ptr!e.refs
      UNTIL p=0 DO
      $( res := res + 1
         p := p!er.link
      $)
      ptr:= ptr!e.link
   $)
   RESULTIS res
$)

AND Make.External.Ids(idnumber) = VALOF
$(
   LET p = extrnsymbols
   UNTIL p=0 DO
   $( p!e.id := idnumber
      idnumber := idnumber+1
      p := p!e.link
   $)
   RESULTIS idnumber
$)

AND unix4.2extrelocinfo() BE
$(
   // Reloc info for XREFS
   LET ptr = extrnsymbols

   UNTIL ptr = 0 DO
   $( LET refs = ptr!e.refs
      LET name = @(ptr!e.name)
                                 // r.length32 + r.extern
      LET info = ((ptr!e.id)<<8) +  #B01000000 + #B00010000

      UNTIL refs=0 DO
      $( writeword( refs!er.loc )
         writeword( info )
         refs := refs!er.link
      $)

      ptr := ptr!e.link
   $)
$)


AND unix4.2symboltable(offset,lvsize) = VALOF
$(
   LET endstring = 0
   LET t = extrnsymbols

   UNTIL t=0 DO
   $( LET name = @(t!e.name)
      LET info = (1 << 24)    // (n.ext<<24) + (n.other<<16) + n.desc

      writeword( offset )     // Offset in Name Table
      writeword( info )       // Info to go with symbol
      writeword( 0 )          // Value of Symbol

      offset := offset + name%0 + 1

      t := t!e.link            // Next name
   $)

   RESULTIS  offset
$)


AND unix4.2nametable(offset) BE
$( LET zero = 0
   LET p = extrnsymbols

   writeword(offset)   // Length of name table

   UNTIL p=0 DO
   $( LET name = @(p!e.name)
      LET length = name%0

      FOR i=1 TO length DO
        wrch(name%i)
      wrch(0)

      p := p!e.link
   $)

   wrch(0)   // Terminate Name Table with a zero
$)
$>EXT
$>RELEASE'
$>IMPERIAL'
$>AMIGA'
$>ATARI'

