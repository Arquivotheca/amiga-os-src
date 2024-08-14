// BCPL-ARGS
// (C) Copyright 1979 Tripos Research Group
//     University of Cambridge
//     Computer Laboratory
// (C) Copyright 1985 All Rights Reserved METACOMCO plc
//     26 Portland Square
//     Bristol
// (C) Copyright 1988 Commodore-Amiga Inc.


SECTION "ARGS"

GET "LIBHDR"
GET "BCPHDR"


LET call.args( nc ) BE
$(
    LET argv   = VEC 100
    LET errarg = "Bad args*N"
    LET errstr = "Run out of store*N"
    LET errfil = "Can't open %S*N"
    LET initcond = ""            // not "$TRIPOS" as before
    LET oldoutput = output()
    LET ctll = 1
    LET version = 1
    LET release = 10
    LET objbin = "bin"
    LET objo   = "o"
    LET objobj = "obj"
    LET argsres = ?
    LET optobj = ?

$<MCC
    LET args    = "FROM,TO,OCODE/K,VER/K,OPT/K,LIB/K,OBJ/K,INC/K"
$>MCC
$<AMIGA
    LET args    = "FROM,TO,OCODE/K,VER/K,OPT/K,LIB/K,INC/K"
$>AMIGA

    rc := 0
    fpsimglob := -1
    transchars := FALSE
    sourcestream, ocodestream, codestream := 0, 0, 0
    verstream := oldoutput

    argsres := rdargs(args, argv, 100)
    IF argsres = 0 THEN
    $( writes(errarg)
       GOTO fail
    $)

    UNLESS argv!3=0 DO
    $( verstream := findoutput(argv!3)
       IF verstream=0 DO
       $( writef(errfil,argv!3)
          GOTO fail
       $)
       selectoutput(verstream)
    $)

    UNLESS argv!0=0 DO
    $( LET from = argv!0
       LET len  = from%0
       sourcestream := findinput(from)
       IF sourcestream=0 DO
       $( LET oresult2 = result2
          LET suffix = "b"
          LET l1,l2 = from%0,suffix%0
          LET ofrom = from

          FOR i=1 TO l1 DO
            argsres%i := from%i
          argsres%(l1+1) := '.'
          FOR i=1 TO l2 DO
            argsres%(l1+1+i) := suffix%i
          argsres%0 := l1+l2+1
          from := argsres
          len  := from%0
          argsres := argsres+(l1+l2+1)/bytesperword + 1

          sourcestream := findinput(from)
          IF sourcestream = 0 THEN
          $( result2 := oresult2
             writef(errfil,ofrom)
             GOTO fail
          $)
       $)
       fromfile := getvec(len/bytesperword)
       IF fromfile=0 DO
       $( writes(errstr)
          GOTO fail
       $)
       FOR i = 0 TO len DO fromfile%i := from%i
    $)

    ocode := argv!2\=0
    TEST ocode THEN
    $(
        LET ocodename = argv!2
        LET ocodelen = ocodename%0
        ocodefile := getvec(ocodelen/bytesperword)
        IF ocodefile=0 DO
        $(
           writes(errstr)
           GOTO fail
        $)
        FOR i = 0 TO ocodelen DO ocodefile%i := ocodename%i

        UNLESS sourcestream = 0 DO
        $(
           ocodestream := findoutput(ocodefile)
           IF ocodestream=0 DO
           $( writef(errfil,ocodefile)
              GOTO fail
           $)
        $)
    $)
    ELSE
      IF sourcestream = 0 THEN
      $( writef("No source file specified*N")
         GOTO fail
      $)

    // set up defaults

    savespacesize := bytesperword=4 -> 3,2 // Works for most 16 & 32 bit BCPLs!

    // compiler options
    numocode := TRUE
    workspacesize := 40000
    naming := FALSE          // variable names are not put in ocode
    trnoptimise := TRUE      // optimise unless told otherwise
    pptrace := FALSE
    ptree := FALSE
    prsource := FALSE
    forceuppercase := FALSE
    quiet := FALSE
    maxglobals := 200
    lispopt := FALSE
$<MACRO
    macmax := 25
$>MACRO

// cg options
    cglisting := FALSE
    cgnaming := FALSE
    callcounting := FALSE
    profcounting := FALSE
    stkchking := FALSE
    restricted := FALSE
    jumpopt := FALSE
    altobj := FALSE
    cg.a := -1  // god knows why I have to do this !!
    cg.b := 0
    inlinemult  := FALSE
    globexternals := FALSE
    extlowbound := 9999
    dumpsymbols := FALSE

// May need to do floating point for non-tripos
// 68010 and so destination chip must be an option

$<MCC
$<UNIX4
    chip := c.m68010
$>UNIX4
$>MCC
$<AMIGA
    chip := c.m68000
$>AMIGA

    FOR i = 0 TO 9 DO datvec%i := "         "%i

// Set up default object module type

$<MCC
$<UNIX4
    objectmodule := o.unix4.2
    optobj := objo
$>UNIX4
$<UNIX5
    objectmodule := o.unix5.0
    optobj := objo
$>UNIX5
$>MCC
$<AMIGA
    objectmodule := o.amigados
    optobj := objobj
$>AMIGA

$<MCC
    UNLESS argv!6 = 0 DO
    $( LET ch = (argv!6)%1
       SWITCHON capitalch(ch) INTO
       $(
          DEFAULT : writef("Unknown object module option *'%C*'*N", ch)
                    ENDCASE
          CASE 'A': objectmodule := o.amigados; optobj := objobj
                    ENDCASE
          CASE 'G': objectmodule := o.gst; optobj := objbin
                    ENDCASE
          CASE 'J': objectmodule := o.tos; optobj := objo
                    ENDCASE
          CASE 'T': objectmodule := o.tripos; optobj := objobj
                    ENDCASE
          CASE 'M': objectmodule := o.mtripos; optobj := objobj
                    ENDCASE
          CASE 'U': objectmodule := o.unix4.2; optobj := objo
                    ENDCASE
          CASE 'V': objectmodule := o.unix5.0; optobj := objo
                    ENDCASE
       $)
    $)
$>MCC

    TEST argv!5 THEN
    $( LET s = argv!5
       FOR i=0 TO s%0 DO libraryname%i := s%i
    $)
    ELSE
       libraryname := 0

    ctll := initcond%0 + 1
    FOR i = 0 TO ctll-1 DO condtaglist%i := initcond%i

    // Set up default name for object module
    IF optobj & (argv!1 = 0) THEN
    $( LET safe = argsres
       UNLESS argsres = 0 DO
         argsres := defaultname(fromfile,"b",argsres,optobj,argv+100-argsres)
       IF argsres THEN argv!1 := safe
    $)

    UNLESS argv!1 = 0 DO  // Output file specified
    $( LET name = argv!1
       FOR i=0 TO name%0 DO
         nc%i := name%i
       codestream := findoutput(name)
       IF codestream=0 DO
       $( writef(errfil,name)
          GOTO fail
       $)
       name.codestream := nc    // Indicates we've opened a code file
    $)

    UNLESS argv!4=0 DO
    $( LET opts = argv!4
       LET i = 1
       LET switch = TRUE

       LET rdn(opts,lvi) = VALOF
        $( LET n = 0
           LET i = !lvi
           LET ch = opts%i
           WHILE i<=opts%0 & '0'<=ch<='9' DO
           $( n := n*10+ch-'0'
              i := i+1
              ch := opts%i
           $)
           !lvi := i
           RESULTIS n
        $)

       WHILE i<=opts%0 DO
       $( LET optch = capitalch(opts%i)
          i := i + 1
          SWITCHON optch INTO
          $(
             DEFAULT:  writef("Unknown option %C ignored*N",optch)

             CASE '$':
               condtaglist % ctll := ','
               ctll := ctll + 1
               $(
                  condtaglist % ctll := optch
                  ctll := ctll + 1
                  optch := capitalch(opts%i)
                  i := i + 1
                  IF tagchartable%optch = 0 ENDCASE
               $) REPEAT

             CASE ',': CASE ';':                      ENDCASE

             CASE '-': switch := FALSE;               LOOP

             CASE '**': altobj := switch;             ENDCASE
             CASE '#': dumpsymbols := switch;         ENDCASE
$<MCC
             CASE '%': chip := (chip = c.m68000) ->
                       c.m68010, c.m68000;            ENDCASE
$>MCC
             CASE 'A': cg.a := rdn(opts,@i);          ENDCASE
             CASE 'B': cg.b := rdn(opts,@i);          ENDCASE
             CASE 'C': stkchking := switch;           ENDCASE
             CASE 'D': numocode := switch;            ENDCASE
             CASE 'E': errormax := rdn(opts,@i);      ENDCASE
             CASE 'F': pptrace := switch;             ENDCASE
             CASE 'G': maxglobals := rdn(opts,@i);    ENDCASE
             CASE 'H': inlinemult := switch;          ENDCASE
             CASE 'I': informmax := rdn(opts,@i);     ENDCASE
             CASE 'J': jumpopt := switch;             ENDCASE
             CASE 'K': callcounting := switch;        ENDCASE
             CASE 'L': prsource := switch;            ENDCASE
$<MACRO
             CASE 'M': macmax := rdn(opts,@i);        ENDCASE
$>MACRO
             CASE 'N': naming := switch;              ENDCASE
             CASE 'O': trnoptimise := switch;         ENDCASE
             CASE 'P': profcounting := switch;        ENDCASE
             CASE 'Q': quiet := switch;               ENDCASE
             CASE 'R': restricted := switch;          ENDCASE
             CASE 'S': savespacesize := rdn(opts,@i); ENDCASE
             CASE 'T': ptree := switch;               ENDCASE
             CASE 'U': forceuppercase := switch;      ENDCASE
             CASE 'V': cgnaming := switch;            ENDCASE
             CASE 'W': workspacesize := rdn(opts,@i); ENDCASE
$<MCC
             CASE 'X': extlowbound := rdn(opts,@i)
                       TEST (objectmodule = o.amigados) |
                            (objectmodule = o.gst)      |
                            (objectmodule = o.tos) THEN
                          globexternals := TRUE
                       ELSE
                          writes("Externals only valid with Amiga/Atari modules*N")
                       ENDCASE
             CASE 'Y': fpsimglob := rdn(opts,@i)
                       ENDCASE
$>MCC
             CASE 'Z': lispopt := switch;             ENDCASE
          $)

          switch := TRUE
       $)

    $)

    // If the global number for fpsim hasn't been
    // specially assigned using the 'y' option, do so according
    // to the object module being generated

    IF fpsimglob = -1 THEN
       fpsimglob := VALOF
          SWITCHON objectmodule INTO
          $( DEFAULT:
             CASE o.tripos   :
             CASE o.mtripos  : RESULTIS -1
             CASE o.amigados : RESULTIS amigafpsim
             CASE o.gst      :
             CASE o.tos      : RESULTIS atarifpsim
             CASE o.unix4.2  :
             CASE o.unix5.0  : RESULTIS unixfpsim
          $)

    triposfp := (fpsimglob = -1)
   
    condtaglist%ctll := ':'
    condtaglist%0 := ctll

$<AMIGA
    makefilelist( argv!6 )
$>AMIGA

$<MCC
    makefilelist( argv!7 )
$>MCC

    UNLESS sourcestream=0 DO
    $(
       writef("BCPL V%N.%N", version, release )
$<MCC
       writef( " (%S)", VALOF
          SWITCHON objectmodule INTO
          $( DEFAULT:
             CASE o.tripos   : RESULTIS "TRIPOS"
             CASE o.mtripos  : RESULTIS "MTRIPOS"
             CASE o.unix4.2  : RESULTIS "UNIX4.2"
             CASE o.unix5.0  : RESULTIS "UNIX5.0"
             CASE o.amigados : RESULTIS "AMIGADOS"
             CASE o.gst      : RESULTIS "BIN"
             CASE o.tos      : RESULTIS "TOS"
          $)  )
$>MCC
       unless argv!4=0 do writef(" Options %S",argv!4)
       newline()
       selectinput(sourcestream)
       linecount := 1
    $)

    RETURN

fail:
    rc := 20

$)


AND defaultname(root,rootsuf,newname,newsuf,length) = VALOF
$( LET len   = root%0
   LET rsufl = rootsuf%0
   LET nsufl = newsuf%0
   LET matched = FALSE
   LET lwords = ?
   LET p = 0

   LET sepchar = '.'

   // First find out whether the name we've been given
   // has the default suffix on it.
   IF len > rsufl THEN
   $( matched := TRUE
      FOR i=1 TO rsufl DO
        IF compch(root%(len-rsufl+i),rootsuf%i) NE 0 THEN
        $( matched := FALSE
           BREAK
        $)
   $)

   // If it contains the suffix, 'strip' it
   IF matched THEN
      len := len - rsufl - 1

   // Now copy the root name into the space provided
   // And append the new suffix to it.
   // First check to see that we have enough room.

   lwords := (len + nsufl + 1)/bytesperword + 1

   IF lwords > length THEN RESULTIS 0

   FOR i=1 TO len DO
   $( p := p+1
      newname%p := root%i
   $)
   p := p+1
   newname%p := sepchar
   FOR i=1 TO nsufl DO
   $( p := p+1
      newname%p := newsuf%i
   $)
   newname%0 := p

   RESULTIS newname+lwords
$)

AND insert.name( ptr, strstart, length ) = VALOF
$(
    LET space = getvec( length/BYTESPERWORD + 1 )

    space!0 := 0
    FOR  i = 1  TO  length  DO
	(space+1)%i := ptr%(strstart + i - 1)
    (space+1)%0 := length

    RESULTIS  space
$)

AND makefilelist( string )  BE
$(
    LET  listptr = @dir.incl

    listptr!0 := 0
    UNLESS  string = 0  DO
    $(
        LET  strl  = string%0
        LET  count = 1

        UNTIL  count >= strl  DO
        $(
            LET  stringstart = count
            LET  length = 0
            LET  ch = string%count

            WHILE  (ch EQ '+') | (ch EQ ',') | (ch EQ '*S') DO
            $(
                count  := count + 1

                IF  count > strl   BREAK

                ch     := string%count
            $)
	    stringstart := count
                
            WHILE  (ch NE '+') & (ch NE ',') & (ch NE '*S') DO
            $(
                length := length + 1
                count  := count + 1

                IF  count > strl   BREAK

                ch     := string%count
            $)
                
            listptr!0 := insert.name( string, stringstart, length ) 
            listptr   := listptr!0 

            count := count + 1
        $)
    $)
$)
