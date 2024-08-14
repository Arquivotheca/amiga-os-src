// (C) Copyright 1979 tripos Research Group
//     University of Cambridge
//     Computer Laboratory
// (C) Copyright 1985 All Rights Reserved METACOMCO plc
//     26 Portland Square
//     Bristol

SECTION "M68CG1"

GET "CGHDR"

// enter with workspace pointing to a suitable area
// and cgworksize set to its size
// the ocode input is set up and calls on rdn will return bytes from it.


LET call.cg() BE
  $( let maxused = 0
     let v = VEC 2

     fileposition := v       
     err.p, err.l := level(), stop.label

     switchspace, tempfile := 0,0

     writes("Code generator entered*N")

     progsize       := 0
     totalcodesize  := 0
     globxrefs      := 0
     globxrefstail  := 0
     globxdefs      := 0
     globxdefstail  := 0
     globxrefnumber := 0
     sectionnumber  := 0
     sectionoffset  := 0
     idlist         := 0
     symbols        := 0
     symbolstail    := 0
$<EXT
     extrnsymbols   := 0
$>EXT

     debugout := verstream
     traceloc, debug := cg.a, cg.b

     outputheader()
     op := rdn()

     maxused := cgsects(workspace, cgworksize)

     writef("%N%% workspace used*N",maxused*100/cgworksize)
     writef("Program size = %N bytes*N", progsize)

     collapse(0)

stop.label:
     RETURN


  $)



AND collapse(n) BE
  $(
     IF switchspace \= 0 THEN
       freevec(switchspace)
     rc := n
     longjump(err.p, err.l)
  $)



AND cgsects(workvec, vecsize) = VALOF
$(  LET maxused = 0
    UNTIL op=0 DO
$(1 LET p = workvec
    LET thisused = 0
    tempv := p
    p := p+3*100 // room for 100 SS items
    tempt := p-3 // highest legal value for ARG1
    procstk, procstkp := p, 0
    p := p+20
    slave := p  // for the slave info about R0 to R7
    p := p + 8
    dp := workvec+vecsize
    labv := p
    paramnumber := (dp-p)/10+10
    p := p+paramnumber
    FOR lp = labv TO p-1 DO !lp := -1
    stv := p
    stvp := 0
    initdatalists()
    initftables()
    initslave()
    freelist := 0
    incode := FALSE
    countflag := FALSE
    maxgn := 0
    maxlab := 0
    maxssp := 0
    procbase := 0
    datalabel := 0
    initstack(3)
    code2(0)
    TEST op=s.section
      THEN $( cgname(s.section,rdn())
              op := rdn()
           $)
      ELSE cgname(s.section,0)
    scan()
    op := rdn()
    stv!0 := stvp/4   //  size of section in words
    outputsection(op=0)
    thisused := stv + stvp/4 - dp + vecsize
    if thisused > maxused then maxused := thisused
    progsize := progsize + stvp

$)1
    RESULTIS maxused
$)

// read in an OCODE label
AND rdl() = VALOF
$( LET l = rdn()
   IF maxlab<l DO
   $( maxlab := l
      checkparam()
   $)
   RESULTIS l
$)

// read in a global number
AND rdgn() = VALOF
$( LET g = rdn()
   IF maxgn<g DO maxgn := g
   RESULTIS g
$)


// generate next label parameter
AND nextparam() = VALOF
$( paramnumber := paramnumber-1
   checkparam()
   RESULTIS paramnumber
$)


AND checkparam() BE IF maxlab>=paramnumber DO
$( cgerror("Too many labels - use the /Wn option to increase workspace")
   collapse(20)
$)


AND cgerror(mess, a) BE
$( writes("*NERROR: ")
   writef(mess,a)
   newline()
$<TOS
   userexit("Hit <RETURN> to continue")
$>TOS
$)

AND bug(n) BE
$( writef("COMPILER BUG %N*N", n)
   dboutput(4)
// backtrace()
   writes("Continuing ...*N")
$)


// Ouput any file header necessary for the system linker.
// For TRIPOS and AMIGADOS, each section is output as a whole hunk
// so we don't need to output a header, but for unix an perhaps
// others we shall have to.
// Note that if 'altobj' is true we're going to produce an assembly
// module, so there's no need to output a linker specific header.

$<MCC
AND outputheader() BE  UNLESS  altobj  DO
$(
   LET oldout = output()

   selectoutput( codestream )

   SWITCHON objectmodule INTO
   $(
      DEFAULT           : bug( 99 )                   ; ENDCASE
      CASE o.tripos     : triposheader()              ; ENDCASE
      CASE o.mtripos    :
      CASE o.amigados   : amigaheader()               ; ENDCASE
      CASE o.unix4.2    : unix4.2header()             ; ENDCASE
      CASE o.unix5.0    : unix5.0header()             ; ENDCASE
      CASE o.gst        : gstheader()                 ; ENDCASE
      CASE o.tos        : tosheader()                 ; ENDCASE
   $)

   selectoutput( oldout )
$)
$>MCC

$<ATARI
AND outputheader() BE  UNLESS  altobj  DO
$(
   LET oldout = output()

   selectoutput( codestream )

   SWITCHON objectmodule INTO
   $(
      DEFAULT           : bug( 99 )                    ; ENDCASE
      CASE o.gst        : gstheader()                  ; ENDCASE
      CASE o.tos        : tosheader()                  ; ENDCASE
   $)

   selectoutput( oldout )
$)
$>ATARI

$<RELEASE
AND outputheader() BE RETURN
$>RELEASE

$<AMIGA
AND outputheader() BE RETURN   
$>AMIGA

$<IMPERIAL
AND outputheader() BE
$(
   LET oldout = output()
   selectoutput( codestream )
   unix5.0header()
   selectoutput( oldout )
$)
$>IMPERIAL


$<RELEASE'
$<ATARI'
$<AMIGA'
$<IMPERIAL'
// No need for header - each section ouput as a hunk
AND triposheader() BE RETURN
$>IMPERIAL'
$>AMIGA'
$>ATARI'
$>RELEASE'

$<ATARI'
$<IMPERIAL'
// No need for header - each section ouput as a hunk
AND amigaheader() BE RETURN
$>IMPERIAL'
$>ATARI'

$<IMPERIAL'
$<AMIGA'
$<ATARI'
$<RELEASE'
AND unix4.2header() BE  UNLESS altobj DO
$(
   LET header = TABLE #X107,  // OMAGIC (impure code)
                          0,  // size of 'TEXT' (bytes)
                          0,  // size of 'DATA'
                          0,  // size of 'BSS'
                         12,  // size of Symbol Table
                          0,  // size of 'TEXT' reloc info
                          0,  // size of 'DATA' reloc info
                          0   // Entry Point

   writewords( header, 1 )
   mynote( codestream )       // Remember so we can patch length later
   writewords( header+1, 7 )
$)
$>RELEASE'
$>ATARI'
$>AMIGA'
$>IMPERIAL'

$<AMIGA'
$<ATARI'
$<RELEASE'
AND unix5.0header() BE  UNLESS altobj DO
$(
   LET header = TABLE #X107,  // OMAGIC (impure code) - #X108 = pure
                          0,  // size of 'TEXT' (bytes)
                          0,  // size of 'DATA'
                          0,  // size of 'BSS'
                          0,  // size of Symbol Table
                          0,  // Entry Point
                          0,  // size of 'TEXT' reloc info
                          0   // size of 'DATA' reloc info

   writewords( header, 1 )
   mynote( codestream )
   writewords( header+1, 7 )
$)
$>RELEASE'
$>ATARI'
$>AMIGA'

$<AMIGA'
$<IMPERIAL'
$<RELEASE'
AND tosheader() BE
$(
    LET headervec  = VEC 6

    mynote( codestream )         // We'll rewind to here later
    writewords( headervec, 7 )   // Output 7 words of rubbish
$)
$>RELEASE'
$>IMPERIAL'
$>AMIGA'

$<IMPERIAL'
$<AMIGA'
$<RELEASE'
AND gstheader() BE
$<MCC
IF libraryname = 0 THEN
$>MCC
$(
   // Simply ouput code file name for now
   LET filename = name.codestream
   LET length = (filename%0 > 31) -> 31, filename%0
   LET p = length
$<TOS
   LET safewrch = wrch
   wrch := binwrch
$>TOS

   writef( "%C%C", gst.escape, gst.source )

   // work from back of name to find directory separator
$<TOS
    UNTIL p = 0 | filename%p = ':' | filename%p = '/' do p := p-1
$>TOS
$<TRIPOS
    UNTIL p = 0 | filename%p = ':' | filename%p = '.' do p := p-1
$>TRIPOS
$<MTRIPOS
    UNTIL p = 0 | filename%p = ':' | filename%p = '/' do p := p-1
$>MTRIPOS

    wrch( length-p )

    FOR  i=p+1  TO  length  DO  wrch( filename%i )

$<TOS
    wrch := safewrch
$>TOS
$)
$>RELEASE'
$>AMIGA'
$>IMPERIAL'


$<TRIPOS
AND mynote(stream) BE note(stream,fileposition)
$>TRIPOS
$<UNIX
AND mynote(stream) BE fileposition := note(stream)
$>UNIX
$<TOS
AND mynote(stream) BE  // botch for now!!
$( fileposition!0,fileposition!1,fileposition!2 := 1,0,0
$)
$>TOS
$<AMIDOS
AND mynote(stream) BE i.seek(input(),0,0)
$>AMIDOS
$<MTRIPOS
AND mynote(stream) BE seek(input(),0,0)
$>MTRIPOS


