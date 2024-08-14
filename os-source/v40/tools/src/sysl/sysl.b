get "LIBHDR"


// This command links together modules produced in Amiga format to make
// a DOS image file. It is used as follows:

// SYSL.BIN root.om file1 [file2.. filen] TO outputfile [BINARY] [ROM]

// The input files must be Amiga format modules.

// The outputfile will be a set of Srecords which can be downloaded,
// or a binary object file which can be transferred directly to a DOS
// disc and installed using the command DF-INSTALL. If the keyword BINARY
// is given then the latter format is used; if the keyword is omitted
// then Srecords are produced.

// Two versions may be created using the RAM or the ROM address
// Quoting the ROM keyword produces the ROM version.

// Version 1.3       TJK       March 11 1985
// Version 1.4       TJK       March 16 1985  Loads intuition
// Version 1.5       NAK       May 5 1985  new intuition address
// Version 1.6       TJK       May 23 1985 No intuition, ROM option
// Version 1.7       TJK       May 29 1985 Warning if 48K limit exceeded
// Version 1.7       NAK       another address change
// Version 1.8       TJK       Tidies within Klib require offset changes
// Version 1.9       TJK       Start address now an argument



global $( addr.offset: ug
          binary: ug + 1 
	  initial.offset: ug+2
       $)


let loadseg(filename) = valof
$( let list = 0
   let liste = @list
   let valid = FALSE
   let bcplize = FALSE
   let os = input()
   let is = findinput(filename)
   if is=0 resultis 0

   selectinput(is)

   $( let type, size, space, temp = ?, ?, ?, ?
      let n = readwords(@type,1)
      if n <= 0 break
      valid := false
      if type=1011 then
      $(
	 bcplize := TRUE	// this is an .ld module
	 readwords(@temp,1)	// Assume no resident libraries
	 readwords(@size,1)	// Assume primary node: this is # of hunks
	 readwords(@temp,1)	// assume zero
	 readwords(@temp,1)	// assume size-1
	 for n=1 to size do
	    readwords(@temp,1)	// toss all hunk sizes
	 readwords(@type,1)
      $)
      if type=999 then
      $(
         readwords(@type,1)     // Assume no PUname
         readwords(@type,1)
      $)
      if type=1001 then
      $(
         unless readwords(@size,1)=1 break
         if size>0 then
         $(
	    if bcplize then size := size+1
            space := getvec(size)
            if space=0 break
            !space := 0
            !liste := space
            liste := space
	    test bcplize then
	    $(
	       space!1 := size
	       unless readwords(space+2,size-1)=size-1 break
	    $)
	    else
	    $(
               unless readwords(space+1,size)=size break
	    $)
         $)
      $)
      if type=1010 then
      $(
	 valid := true
      $)
   $) repeat
   endread()
   selectinput(os)
   resultis valid -> list, 0
$)

let unloadseg(s) be until s=0 do $( let x = !s; freevec(s); s := x $)

let start() be
$( let argv = vec 100
   let rootseg = 0
   let segvec = VEC 10
   let outs = ?
   let seglist = ?
   let chain = ?
   let maxseg = 0
   let terms = output()
   let final.offset = ?
   let size, maxsize = ?,?


   if rdargs("From/a,,,,,,,,to/a/k,binary/s,start/a/k,stop/a/k",argv,100)=0 then
   $( writes("Bad args*N")
      stop(10)
   $)

   initial.offset := readhex(argv!10)
   final.offset   := readhex(argv!11)
   addr.offset := initial.offset
   chain       := initial.offset >> 2   // BCPL start ptr

   outs := findoutput(argv!8)
   if outs=0 then
   $( writef("Unable to open %S for output*N",argv!8)
      stop(10)
   $)

   binary := argv!9 NE 0

   for i=0 to 7 do
   $( let seg = ?
      if argv!i = 0 break
      maxseg := i
      seg := loadseg(argv!i)
      if seg=0 then
      $( writef("Unable to load %S*N",argv!i)
         break
      $)
      if i=0 then
      $(
	  seglist := seg+3		// Seglist in memory
      $)
      segvec!i := seg
      seglist!(i+1) := chain
      chain := chain + sizeof(seg)
   $)

   // Now write it all out, including segment links

   for i=0 to maxseg do writeseg(segvec!i,outs,argv!i)
   selectoutput(outs)
   terminate.srec()
   endwrite()
   selectoutput(terms)
   size := addr.offset - initial.offset
   maxsize := final.offset - initial.offset
   writef("Sizes: $%X4 available - $%X4 used - $%X4 free*N",
	   maxsize,size,maxsize-size)
   if initial.offset + size > final.offset then
      writes("WARNING: Size budget exceeded*N")
$)

and sizeof(seg) = valof
$( let size = 0
   until seg=0 do $( size := size + seg!1 + 1; seg := !seg $)
   resultis size
$)

and writeseg(seg,outs,name) be
   until seg=0 do
   $( let link = seg!0   // Previous link
      let s = output()
      unless link=0 do seg!0 := (addr.offset>>2) + seg!1 + 1
      writef("$%X8: Length %X4 bytes [%S]*N",
              addr.offset, seg!1<<2, name)
      selectoutput(outs)
      writehunk( seg )
      selectoutput(s)
      freevec( seg )
      seg := link
   $)

and writehunk(cvec) be test binary then
$( writewords( cvec, cvec!1+1 )
   addr.offset := addr.offset + (cvec!1+1) * 4
$)
else
$( LET base = ?
   LET addr = ?
    $(  LET size  =  cvec!1+1
        LET bvec  =  ?
        LET top   =  ?

        base := 0
        top := base + size
        addr := base*4 + addr.offset
        bvec := cvec + base

        FOR  i = base  TO  top-1  BY  8  DO
        $(
           LET nwords  =  top - i
           LET nbytes  =  (nwords > 8  ->  8, nwords) * 4
           LET length  =  4 + nbytes
           LET cs      =  length + (addr & #XFF) + ((addr >> 8)  & #XFF) +
                                                 ((addr >> 16) & #XFF)

           writef( "S2%X2", length )
           writehex( addr, 6 )

           FOR  j = 0  TO  nbytes - 1  DO
           $(
               LET byte = bvec % j

               cs  :=  cs + byte
               writehex( byte, 2 )
           $)
      
           writef( "%X2*N", NOT cs )

           addr  :=  addr + nbytes
           bvec  :=  bvec + (nbytes / 4)
        $)
    $)
    addr.offset := addr
$)

and terminate.srec() be test binary then
$( let n = 0
   writewords(@n, 1)
$) else
    $(
        LET cs    =  4
        AND addr  =  initial.offset

        writes( "S804" )

        FOR  i = 16  TO  0  BY  -8  DO
        $(
           LET byte  =  (addr >> i) & #XFF

           writehex( byte, 2 )
           cs  :=  cs + byte
        $)

        writef( "%X2*N", NOT cs )
    $)

and readhex(s) = valof
$( let res = 0
   for i = 1 to s%0 do
   $( let nib = readnib(s%i)
      res := (res<<4) + nib
   $)
   resultis res 
$)

and readnib(ch) = valof
$(
   unless validhex(ch) do writef("Not hex: %C*N",ch)
   resultis (ch>='A') -> ch-'A'+10, ch-'0'
$)

and validhex(ch) = '0'<=ch<='9' | 'A'<=ch<='F'
