//***************************************************************************
//* Contains code for GST & ATARI Back Ends                                 *
//***************************************************************************

SECTION "M68CG8"

GET "CGHDR"
$<TRIPOS
GET "IOHDR"
$>TRIPOS
$<MTRIPOS
GET "IOHDR"
$>MTRIPOS


$<EXT
MANIFEST
$(
   rel.begin.longword = 5
   rel.undefined      = 4
$)
$>EXT

$<IMPERIAL'
$<AMIGA'
$<RELEASE'
LET gsthunk(last) BE
$(
$<TOS
    LET safewrch = wrch
    wrch := binwrch
$>TOS
$<MCC
    last := last | (libraryname NE 0)
    IF libraryname NE 0 THEN
    $( LET name = VEC 20
       LET llen = libraryname%0
       LET ptr  = stv + stvp/4 - 3
       LET sectno = (ptr!1 = 0) -> 0, ptr!0

       IF sectno = 0 THEN
       $( selectoutput(verstream)
          writef("No globals defined*N")
          RETURN
       $)

       FOR i=1 TO llen DO
         name%i := libraryname%i

       FOR i=3 TO 0 BY -1 DO
       $( name%(llen+i+1) := '0' + (sectno REM 10)
          sectno := sectno/10
       $)
       name%0 := llen+4

       selectoutput(verstream)
       writef("Creating Library Module %S*N", name )
       selectoutput(codestream)

       writef( "%C%C", gst.escape, gst.source )
       wrch( llen+4 )
       FOR  i=1  TO  llen+4 DO  wrch( name%i )
    $)
$>MCC

    sectionnumber := sectionnumber - 1   // Update section number
    globxrefstail := 0                   // Reset for next section

    gstsectno()     // section directive
    IF globexternals THEN
    $( gstglobalxdefs()      // declare xdefs for this section
       gstglobalxrefs()      // declare and output xrefs for this section
    $)
$<EXT
    xrefs := gstxrefs()
$>EXT
    gstcodesect()   // Code, relocation & xrefs

    IF last THEN
    $( wrch( gst.escape )
       wrch( gst.end )

       // If we're making a library, each
       // section should look like a separate module
       // So reset params for next section

       sectionnumber  := 0         // reset section id
       globxrefnumber := 0         // reset xref id
       sectionoffset  := 0         // reset section base address
       idlist         := 0         // Throw away previous definitions
$<EXT
       extrnsymbols   := 0         // Throw away all XREF symbols
$>EXT
    $)

$<TOS
    wrch := safewrch         // Restore wrch
$>TOS
$)

// Reloc section (a SECTION directive)
AND gstsectno() BE
$(
   LET name = "TEXT"

   // First Declare it
   writef( "%C%C", gst.escape, gst.define )
   wrch( (@sectionnumber)%2 )
   wrch( (@sectionnumber)%3 )
   writef( "%C%S", name%0, name )

   // Then start generating code for it
   writef( "%C%C", gst.escape, gst.section )
   wrch( (@sectionnumber)%2 )
   wrch( (@sectionnumber)%3 )
$)

// Output all the XDEF values
AND gstglobalxdefs() BE
$(
    // Output a 16 bit absolute XDEF for each global
    // defined in the current section

    LET ptr    = stv + (stvp/4) - 3
    LET globno = ptr!0
    LET offset = ptr!1
    LET buff   = VEC 1
    LET value  = globno*4
   
    buff ! 0 := #X474C4F42    // GLOB

    UNTIL  offset = 0  DO
    $(
       IF globno >= extlowbound THEN
       $(
          FOR i=3 TO 0 BY -1 DO
          $( (buff+1)%i := '0' + (globno REM 10)
             globno := globno/10
          $)

          // FB 06 <symbol> <value> <sectionid>
          writef( "%C%C", gst.escape, gst.xdef)
          wrch( 8 )                         // Length of symbol
          FOR i=0 TO 7 DO wrch(buff%i)
          FOR i=0 TO 3 DO wrch((@value)%i)  // The value
//          wrch((@sectionnumber)%2)          // Section in which declared
//          wrch((@sectionnumber)%3)
          wrch(0)
          wrch(0)                           // Absolute
       $)

       ptr    := ptr - 2
       globno := ptr!0
       offset := ptr!1
       value  := globno*4
    $)
$)

$<EXT
// Form all the 32 bit XREFs into an ordered list
AND makexlist( lvlist, refs, id ) BE
$(
   UNTIL refs = 0 DO
   $(
      LET next = refs!er.link
      refs!er.id   := id    // Insert ID
      refs!er.link := 0     // Zero link to next
      insert( lvlist, refs )
      refs := next
   $)
$)

// Definition of 32 bit XREFs
// Here we need throw out definitions of the xrefs and produce
// an ordered list of XREF locations.
AND gstxrefs() = VALOF
$(
   LET p = extrnsymbols
   LET list = 0

   UNTIL p = 0 DO
   $(
      LET name = @(p!e.name)

      // Define the name
      globxrefnumber := globxrefnumber+1
      writef( "%C%C", gst.escape, gst.define )
      wrch( (@globxrefnumber)%2 )         // id
      wrch( (@globxrefnumber)%3 )
      FOR i=0 TO name%0 DO wrch(name%i)   // name

      // Store the symbol id in the symbol node
      p!e.id := globxrefnumber

      makexlist( @list, p!e.refs, globxrefnumber )

      p!e.refs := 0
      p := p!e.link
   $)

   RESULTIS list
$)

AND insert( lvlist, node ) BE
$(
   UNTIL !lvlist = 0 DO
   $( IF  node!er.loc < (!lvlist)!er.loc THEN
         BREAK
      lvlist := !lvlist
   $)

   // Insert into list
   node!er.link := !lvlist
   (!lvlist) := node
$)
$>EXT

// Now DEFINE the XREFs (i.e. associate a number with each)
AND gstglobalxrefs() BE
$(
   LET ptr   = globxrefs
   LET buff  = VEC 1

   buff ! 0 := #X474C4F42    // GLOB

   UNTIL ptr=0 DO
   $(
      LET globno = (ptr!xref.globno) & #XFFFF
      LET id     = (ptr!xref.globno) >> 16

      IF id = 0 THEN    // Not previously defined in this section
      $(
         LET gn = globno

         // Check to see if defined in an earlier section

         id := Defined.Earlier( gn )

         TEST id = 0 THEN    // Not previously defined
         $(
            globxrefnumber  := globxrefnumber + 1   // Next ID

            FOR i=3 TO 0 BY -1 DO
            $( (buff+1)%i := '0' + (gn REM 10)
               gn := gn/10
            $)

            writef( "%C%C", gst.escape, gst.define )
            wrch( (@globxrefnumber)%2 )         // id
            wrch( (@globxrefnumber)%3 )
            wrch( 8 )                       // length
            FOR i=0 TO 7 DO wrch( buff%i )  // name

            // Store its ID
            ptr!xref.globno := (ptr!xref.globno) | (globxrefnumber << 16)

            // add to list of defined XREF ids
            $( LET temp = newvec(1)
               temp!0 := idlist
               temp!1 := ptr!xref.globno
               idlist := temp
            $)

            markdefined(ptr!xref.link,globno,globxrefnumber)
         $)
         ELSE
            markdefined(ptr,globno,id)
      $)

      ptr := ptr!xref.link            // ptr to next symbol
   $)
$)

AND Defined.Earlier(gn) = VALOF
$( LET ptr = idlist

   UNTIL ptr = 0 DO
   $( LET val = ptr!1
      IF (val & #XFFFF) = gn THEN
        RESULTIS (val >> 16)
      ptr := ptr!0
   $)

   RESULTIS 0
$)

AND markdefined(ptr,gn,id) BE
$(
   UNTIL ptr = 0 DO
   $(
      IF (ptr!xref.globno) = gn THEN
        ptr!xref.globno := (ptr!xref.globno) | (id << 16)
      ptr := ptr!xref.link
   $)
$)

$<EXT'
// Output a section of code
AND gstcodesect() BE
$(
   LET next.xref = (globxrefs = 0) -> -10, globxrefs!xref.location

   location := 0
    
   UNTIL (location >= stvp) DO
     TEST (next.xref=location) THEN
     $( gstglobalxref()
        globxrefs := globxrefs!xref.link
        IF globxrefs NE 0 THEN
          next.xref := globxrefs!xref.location
     $)
     ELSE
     $( wcb(stv%location)
        location := location + 1
     $)
$)
$>EXT'
$<EXT
// Output a section of code
AND gstcodesect() BE
$(
   LET next.g = (globxrefs = 0) -> -10, globxrefs!xref.location
   LET next.x = (xrefs = 0) -> -10, xrefs!er.loc

   location := 0
    
   UNTIL (location >= stvp) DO
     TEST (next.g=location) THEN
     $( gstglobalxref()
        globxrefs := globxrefs!xref.link
        IF globxrefs NE 0 THEN
          next.g := globxrefs!xref.location
     $)
     ELSE TEST (next.x=location) THEN
     $( gstxref()
        xrefs := xrefs!er.link
        UNLESS xrefs=0 DO
          next.x := xrefs!er.loc
     $)
     ELSE
     $( wcb(stv%location)
        location := location + 1
     $)
$)
$>EXT

AND gstglobalxref() BE
$(
   LET size     = gst.trunc.word
   LET id       = (globxrefs!xref.globno) >> 16
   LET padbytes = 2
   LET trule    = size | gst.trunc.signed

   writef( "%C%C", gst.escape, gst.xref )

   // 32 bit value to be used in the calculations
   FOR i=1 TO padbytes DO wrch(0)
   FOR i=padbytes+1 TO 4 DO
   $( wrch(stv%location)
      location := location + 1
   $)

   writef("%C%C%C%C%C", (@trule)%3, gst.op.plus,
                        (@id)%2, (@id)%3, gst.escape )
$)

$<EXT
AND gstxref() BE
$(
   LET size     = gst.trunc.long
   LET id       = xrefs!er.id
   LET trule    = size | gst.trunc.unsigned | gst.trunc.lwrtrel 

   writef( "%C%C", gst.escape, gst.xref )

   // 32 bit value to be used in the calculations
   FOR i=1 TO 4 DO
   $( wrch(stv%location)
      location := location + 1
   $)

   writef("%C%C%C%C%C", (@trule)%3, gst.op.plus,
                        (@id)%2, (@id)%3, gst.escape )
$)
$>EXT

AND wcb( byte ) BE
$( wrch( byte )
   IF byte = #XFB THEN wrch( #XFB )
$)
$>RELEASE'
$>AMIGA'
$>IMPERIAL'

$<IMPERIAL'
$<AMIGA'
$<RELEASE'
// If producing a library, we need to make a separate
// code file for each section. Use libraryname supplied
// and take the first XDEF number to make filename.

$<MCC
AND createlibfile() = VALOF
$(
   LET name   = VEC 64
   LET llen   = libraryname%0
   LET sectno = ?
   LET stream = ?

   IF globxdefs = 0 THEN
   $( selectoutput(verstream)
      writef("No globals defined*N")
      RESULTIS 0
   $)

   sectno := globxdefs ! 2

   FOR i=1 TO llen DO
     name%i := libraryname%i

   FOR i=3 TO 0 BY -1 DO
   $( name%(llen+i+1) := '0' + (sectno REM 10)
      sectno := sectno/10
   $)
   name%0 := llen+4

   selectoutput(verstream)
   writef("Creating Library File %S*N", name )

   stream := findoutput(name)
   IF stream = 0 THEN
   $( selectoutput(verstream)
      writef("Could not open %S for output*N", name)
   $)

   RESULTIS stream
$)
$>MCC

AND toshunk(last) BE
$(
$<MCC
   LET stream = ?
   LET safecodestream = codestream
$>MCC

   tosglobalxdefs()     // Make some xdef nodes

$<MCC
   last := last | (libraryname NE 0)
   IF (libraryname NE 0) THEN
   $( stream := createlibfile()
      TEST stream = 0 THEN
         GOTO exit
      ELSE
      $( codestream := stream
         selectoutput( codestream )
         tosheader(?)  // reserve seven words in file
      $)
   $)
$>MCC

   writewords(stv,stvp/4)
   IF last THEN
   $( LET size = ?
      size := tossymbols()*14
      tosrelocation()
      tosheader(size)
$<MCC
      IF libraryname NE 0 THEN endwrite()
$>MCC
   $)

   // Place where next section begins
   sectionoffset := sectionoffset + stvp

$<MCC
exit:
   IF (libraryname NE 0) THEN     // reset all parameters
   $(
      codestream     := safecodestream
      sectionoffset  := 0
      globxdefs      := 0
      globxrefs      := 0
      globxdefstail  := 0
      globxrefstail  := 0
      totalcodesize  := 0
      globxrefnumber := 0
$<EXT
      extrnsymbols   := 0
$>EXT
   $)
$>MCC
$)

AND tosheader(symtablen) BE
$(
   LET magic     = #X601A
   LET headervec = VEC 7

   // Rewind the file
   mypoint(codestream)

   // Fill in the magic number and length counts.

   headervec ! 0 := magic << 16

   FOR i = 1 to 6 DO headervec!i := 0

   pbytes((headervec<<2)+2,4,totalcodesize<<2)
   pbytes((headervec<<2)+14,4,symtablen)

   writewords(headervec, 7)
$)

AND tosrelocation() BE
$(
   LET data, sdata, words    = ?, ?, ?
   LET address  = 0     // Number of reloc bytes output
   LET next.g = (globxrefs=0)-> -10, globxrefs!xref.location
$<EXT
   LET next.x = (xrefs = 0) -> -10, xrefs!er.loc
$>EXT
   LET textlen = totalcodesize << 2

   UNTIL address >= textlen DO           // for each word of code
   $(
      data := 7

      TEST (address = next.g) THEN
      $( LET id = ((globxrefs!xref.globno) >> 16) & #X7FFF
         /* dbg("XREF Address = %N Output gn = %N id = %N*N",
                    address, (globxrefs!xref.globno)&#XFFFF, id)*/
         data  := 4 | (id << 3)
         globxrefs := globxrefs!xref.link
         UNLESS globxrefs = 0 DO
           next.g := globxrefs!xref.location
         address := address + 2
$<TOS'
         wrch((@data)%2)
         wrch((@data)%3)
$>TOS'
$<TOS
         write16bits(data)
$>TOS
      $)
      ELSE
$<EXT
      TEST (address = next.x) THEN
      $(
         LET id = xrefs!er.id
         data := (rel.begin.longword<<16) | (id << 3) | rel.undefined
         xrefs := xrefs!er.link
         UNLESS xrefs=0 DO
           next.x := xrefs!er.loc
         address := address + 4
$<TOS'
         FOR i=0 TO 3 DO
           wrch((@data)%i)
$>TOS'
$<TOS
         writebytes(cos,@data,4)
$>TOS
      $)
      ELSE
$>EXT
      $(
         address := address + 2
$<TOS'
         wrch((@data)%2)
         wrch((@data)%3)
$>TOS'
$<TOS
         write16bits(data)
$>TOS
      $)
   $) // for each word of code
$)


// Make a node for each global defined in the current section

AND tosglobalxdefs() BE
$(
   // Output a 16 bit absolute XDEF for each global
   // defined in the current section
   LET  ptr    = stv + (stvp/4) - 3
   LET  globno = ptr!0
   LET  offset = ptr!1

   UNTIL offset = 0 DO
   $(
      IF globno >= extlowbound THEN
        addglobalxdef(globno)

      ptr := ptr - 2
      globno := ptr!0
      offset := ptr!1
   $)
$)

AND addglobalxdef(gn) BE
$(
   LET p   = ?
   LET gn1 = (globxrefnumber << 16) | gn | #X80000000

   p   := newvec(2)
   p!0 := 0
   p!1 := gn*4
   p!2 := gn

   globxrefnumber := globxrefnumber + 1

   TEST globxdefs = 0 THEN
     globxdefs := p
   ELSE
     globxdefstail!xref.link := p
   globxdefstail := p

   markasoutput(globxrefs,gn1)
$)
/*
AND dbg(s,a,b,c,d) be
$( let o = output()
   selectoutput(verstream)
   writef(s,a,b,c,d)
   selectoutput(o)
$)
*/

AND tossymbols() = VALOF
$(
   LET nsyms = 0
   LET ptr   = globxrefs
$<EXT
   LET list  = 0
$>EXT

   // First the xdefs
   UNTIL globxdefs = 0 DO
   $( LET id  = globxdefs!xref.globno
      tossymbol(#XE000,id,@nsyms)
      globxdefs := globxdefs!xref.link
   $)

   // Now the xrefs
   UNTIL ptr = 0 DO
   $( LET id  = ptr!xref.globno

      /* dbg("XREF %X8 gn = %N id = %N*N", ptr!xref.location,
             id&#XFFFF, id >> 16 ) */

      IF id > 0 THEN    // if we haven't yet declared this name..
      $( id := id | (globxrefnumber << 16)
         ptr!xref.globno := id
         tossymbol(#XA800,id,@nsyms)
         globxrefnumber := globxrefnumber + 1
         markasoutput(ptr!xref.link,id)
      $)   

      // Now remove the marker from this node...
      // ptr!xref.globno := (ptr!xref.globno) & #X7FFFFFFF

      // .. and continue
      ptr := ptr!xref.link
   $)

$<EXT
   ptr := extrnsymbols
   UNTIL ptr = 0 DO
   $( tosextsymbol(@(ptr!e.name),@nsyms)
      makexlist(@list,ptr!e.refs,globxrefnumber)
      globxrefnumber := globxrefnumber + 1
      ptr := ptr!e.link
   $)

   xrefs := list
$>EXT

   RESULTIS nsyms
$)


AND tossymbol(sysval,ginfo,p.num) BE
$(
   LET buff = VEC 1
   LET gn   = ginfo & #XFFFF
   LET id   = ginfo >> 16
   LET gn1  = (sysval = #XE000) -> gn*4, 0

   !p.num     := !p.num + 1    // Update symbol count

   buff ! 0 := #X474C4F42    // GLOB

   FOR i=3 TO 0 BY -1 DO
   $( (buff+1)%i := '0' + (gn REM 10)
      gn := gn/10
   $)
   
   mywritewords(buff,2)

$<TOS'
   wrch((@sysval)%2)
   wrch((@sysval)%3)
$>TOS'
$<TOS
   write16bits(sysval)
$>TOS

   mywritewords(@gn1,1)
$)

$<EXT
AND tosextsymbol(name,p.num) BE
$(
   LET buff     = VEC 1
   LET sysval   = #XA800
   LET val      = 0
   LET min(a,b) = a<b -> a,b

   !p.num := !p.num + 1      // Update symbol count

   buff!0,buff!1 := 0,0      // Pad with zeros

   FOR i=1 TO min(8,name%0) DO  // Copy in name
     buff%(i-1) := name%i
   
   mywritewords(buff,2)

$<TOS'
   wrch((@sysval)%2)
   wrch((@sysval)%3)
$>TOS'
$<TOS
   write16bits(sysval)
$>TOS

   mywritewords(@val,1)
$)
$>EXT

AND markasoutput(ptr,info) BE
$(
   LET gn = info & #XFFFF

   UNTIL ptr = 0 DO
   $(
      IF ((ptr!xref.globno)&#XFFFF) = gn THEN
      $( ptr!xref.globno := info
          
     //    UNLESS ptr!xref.link = 0 DO   // if not last in list..
         ptr!xref.globno := (ptr!xref.globno) | #X80000000  // mark as declared
      $)
      ptr := ptr!xref.link
   $)
$)
$>RELEASE'
$>AMIGA'
$>IMPERIAL'


$<TRIPOS
AND writeword( w ) BE writewords( @w, 1 )

AND mypoint(stream) BE
$( deplete()
   point(stream,fileposition)
$)

AND mywritewords(v,n) BE
$(
   LET nbytes = n << 2
   FOR i = 0 TO nbytes-1 DO wrch(v%i)
$)

AND deplete() BE
$(
   LET buf , pos = cos ! scb.buf , cos ! scb.pos
   LET lastchs = pos REM bytesperword

   IF pos > 0
   THEN
      TEST lastchs = 0
      THEN
      $(
         LET zero = 0
         writewords ( buf , pos / bytesperword )

         IF buf % ( pos - 1 ) < bytesperword THEN
            writewords ( @zero , 1 )
      $)
      ELSE
      $(
         buf % ( pos - lastchs + bytesperword - 1 ) := lastchs

         writewords ( buf , pos / bytesperword + 1 )
      $)
   cos ! scb.pos := -1      // So we don't flush the buffer again on close
$)
$>TRIPOS

$<MTRIPOS
AND writeword( w ) BE writewords( @w, 1 )

AND mypoint(stream) BE
$( deplete()
   seek(stream,-1,fileposition)
$)

AND mywritewords(v,n) BE
$(
   LET nbytes = n << 2
   FOR i = 0 TO nbytes-1 DO wrch(v%i)
$)

AND deplete() BE
$(
   LET buf , pos = cos ! scb.buf , cos ! scb.pos
   LET lastchs = pos REM bytesperword

   IF pos > 0
   THEN
      TEST lastchs = 0
      THEN
      $(
         LET zero = 0
         writewords ( buf , pos / bytesperword )

         IF buf % ( pos - 1 ) < bytesperword THEN
            writewords ( @zero , 1 )
      $)
      ELSE
      $(
         buf % ( pos - lastchs + bytesperword - 1 ) := lastchs

         writewords ( buf , pos / bytesperword + 1 )
      $)
   cos ! scb.pos := -1      // So we don't flush the buffer again on close
$)
$>MTRIPOS

$<UNIX
AND writeword( w ) BE writewords( @w, 1 )

AND mypoint(stream) BE point(stream,fileposition)

AND mywritewords(v,n) BE
$(
   LET nbytes = n << 2
   FOR i = 0 TO nbytes-1 DO wrch(v%i)
$)
$>UNIX

$<AMIDOS
AND writeword( w ) BE writewords( @w, 1 )
$>AMIDOS

$<TOS
AND writeword( w ) BE writewords( @w, 1 )

AND mypoint(stream) BE point(stream,fileposition)

AND mywritewords(v,n) BE writebytes(cos,v,n<<2)

AND write16bits(val) BE
$( val := val << 16
   writebytes(cos,@val,2)
$)
$>TOS


