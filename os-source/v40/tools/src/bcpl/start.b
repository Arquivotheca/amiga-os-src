// (C) Copyright 1979 Tripos Research Group
//     University of Cambridge
//     Computer Laboratory
// (C) Copyright 1985 All Rights Reserved METACOMCO plc
//     26 Portland Square
//     Bristol
// (C) Copyright 1988 Commodore-Amiga Inc.

SECTION "BCP"

GET "LIBHDR"
GET "BCPHDR"
GET "SYNHDR"

MANIFEST
$(
   err.corrupt  = 1
   err.memory   = 2
   err.notfound = 3
   err.internal = 4
$)


LET start() BE
 $(
    LET oldoutput = output()
    LET v1  = VEC 14
    LET sn  = VEC 3
    LET ctl = VEC 64
    LET v2  = VEC 64
    LET nc  = VEC 64
    LET pt  = VEC 20
    LET tpglob = highest.global   // reference highest global
$<MCC
    LET nl  = VEC 20
$>MCC

$<TOS
    LET ov  = VEC 20     // Space for name of where
    ovhome  := ov        // overlays live
    sysin   := input()   // Get handle on keyboard
$>TOS
$<MCC
    libraryname := nl
$>MCC

    // Initialise 6 'critical' globals in case of 'tidyup'
    workvec      := 0
    fromfile     := 0
    ocodefile    := 0
    ocodestream  := 0
    sourcestream := 0
    codestream   := 0

    name.codestream := 0    // no code file yet

    sysout := oldoutput
    informmax := 10
    errormax := 10
    datvec := v1
    sectionname := sn
    condtaglist := ctl
    ctl%0 := 0
    getp := 0
    errors := FALSE
    uppercase := capitalch
    verstream := oldoutput
    insyn := FALSE

    for i=0 to 64 do
      v2!i := 0
    tagchartable := v2  // chars acceptable in tags
    for i=0 TO 20 do
      pt!i := 0
    plisttree := pt

    FOR j = '0' TO '9' DO putbyte(tagchartable, j, j)
    FOR j = 'a' TO 'z' DO putbyte(tagchartable, j, j)
    FOR j = 'A' TO 'Z' DO putbyte(tagchartable, j, j)
    putbyte( tagchartable, '.', '.')
    putbyte( tagchartable, '_', '_')

    call.args( nc )

    UNLESS rc=0 GOTO fail

    IF forceuppercase DO
    $(
        uppercasestring(ctl)
        FOR i = 'a' TO 'z' DO tagchartable%i := uppercase(i)
    $)

    TEST workspacesize >= 5000
    THEN
    $(
        LET tos = ?
        workvec := getvec(workspacesize)
        IF workvec = 0 DO
        $(
            writef("Cannot get workspace*N")
            rc := 20
            GOTO fail
        $)
        FOR i = 0 TO workspacesize DO workvec!i := -1
        obufp, obufb := workvec, 0
        workbase := workvec
        worktop := workbase + workspacesize
        UNLESS sourcestream = 0 DO
        $(
           comp()
           UNLESS ocodestream = 0 DO
           $(
              selectoutput(ocodestream)
              endwrite()
              ocodestream := 0
           $)
        $)
        selectoutput(verstream)

        IF errors DO rc := 20

        UNLESS [codestream = 0] | [rc > 0] DO codegen()

        UNLESS codestream = 0 DO
        $( LET temp = codestream
           codestream := 0
           selectoutput(temp)
           endwrite()
        $)

        selectoutput(verstream)
    $)
    ELSE
    $(
        writes("Workspace too small - use W option*N")
        rc := 20
    $)

fail:
    UNLESS verstream = oldoutput DO
    $( LET temp = verstream
       verstream := 0
       selectoutput(temp)
       endwrite()
    $)

    tidyup(rc)
 $)

AND comp() BE
$(
    $(
        workbase := obufp+1 // set workbase to just above top of ocode
        worktop := workvec+workspacesize
        treep := worktop

        warncount, warnmax := 0, informmax
        reportcount, reportmax := 0, errormax
        !sectionname := 0
        insyn := TRUE

        $(
$<OVLAID
$<OVLOAD
            LET a = formtree()
$>OVLOAD
$<OVLOAD'
            LET a = overlay( "bcplsyn.ovl", glob.formtree )
$>OVLOAD'
$>OVLAID
$<OVLAID'
            LET a = formtree()
$>OVLAID'

            IF reportcount > 0 DO errors := TRUE

            IF rc > 0      // fatal failure
            DO $(
               endread()
               sourcestream := 0
               RETURN
            $)
            insyn := FALSE

            UNLESS sourcestream = 0 DO
                TEST rdch() = ENDSTREAMCH
                THEN $( endread(); sourcestream := 0 $)
                ELSE unrdch()

            IF a = 0
            $(
                writes("No program to compile*N")
                errors := TRUE
                RETURN
            $)

            UNLESS quiet DO writef("%n%% workspace used*N",
            ((worktop-treep)+(workbase-obufp-1))*100/(worktop-obufp))

            IF ptree THEN $( plist(A, 0, 20); newline() $)

            UNLESS ocodestream = 0 DO selectoutput(ocodestream)

            workbase := obufp + 1    // dispose of SYN's workspace

            compileae(a)

            IF reportcount > 0 DO errors := TRUE

            IF rc > 0      // if the compileae failed
            DO $(
               endread()
               sourcestream := 0
            $)

            selectoutput(verstream)
            UNLESS quiet DO writef("OCODE size = %n bytes*N",
                                  (obufp-workvec)*bytesperword + obufb)
        $)

    $) REPEATUNTIL sourcestream = 0
$)


AND plist(x, n, d) BE
    $(1 LET size = 0
        LET v = plisttree

        IF x=0 DO $( writes("NIL"); RETURN  $)

        SWITCHON (h1!x) & 255 INTO
    $(  CASE s.number: writen(h2!x); RETURN

        CASE s.name: writes(x+2); RETURN

        CASE s.string: writef("*"%S*"", x+1); RETURN

        CASE s.manifest:CASE s.static:CASE s.global:
            $( LET y=h2!x
//               writes ("OP")
//               writen(h1!x)
               writeop(h1!x)
               UNTIL y=0 DO $(
                   FOR i = 1 TO 2 DO
                     $( newline()
                        FOR j=0 TO n-1 DO writes( v!j )
                        writes("  **-")
                        v!n := i=2->"  ","! "
                        plist(h1!(y+i), n+3, d)  $)
                   y:=h1!y $)
               newline()
               FOR j=0 TO n-1 DO writes( v!j )
               writes("**-")
               v!n :="  "
               plist(h3!x, n+1, d)
               RETURN
            $)

        CASE s.for:
                size := size + 2

        CASE s.cond:CASE s.fndef:CASE s.rtdef:
        CASE s.test:CASE s.constdef:
                size := size + 1

        CASE s.section: CASE s.byteap:
$<WORD
        CASE s.wordap:
$>WORD
        CASE s.vecap:CASE s.fnap:
        CASE s.mult:CASE s.div:CASE s.rem:CASE s.plus:CASE s.minus:
        CASE s.eq:CASE s.ne:CASE s.ls:CASE s.gr:CASE s.le:CASE s.ge:
        CASE s.lle: CASE s.lge: CASE s.lls: CASE s.lgr:
        CASE s.lshift:CASE s.rshift:CASE s.logand:CASE s.logor:
        CASE s.eqv:CASE s.neqv:CASE s.comma: CASE s.commawithline:
        CASE s.and:CASE s.valdef:CASE s.vecdef:
        CASE s.ass:CASE s.rtap:CASE s.colon:CASE s.if:CASE s.unless:
        CASE s.while:CASE s.until:CASE s.repeatwhile:
        CASE s.repeatuntil:
        CASE s.switchon:CASE s.case:CASE s.seq:CASE s.let:
$<MACRO CASE s.macro: $>MACRO
                size := size + 1

        CASE s.valof:CASE s.lv:CASE s.rv:CASE s.neg:CASE s.not:
        CASE s.table:CASE s.goto:CASE s.resultis:CASE s.repeat:
        CASE s.default: CASE s.code: CASE s.car: CASE s.cdr:
                size := size + 1

        CASE s.loop:
        CASE s.break:CASE s.return:CASE s.finish:CASE s.endcase:
        CASE s.true:CASE s.false:CASE s.query: CASE s.nil:
        DEFAULT:
                size := size + 1

                IF n=d DO $( writes("ETC")
                             RETURN  $)

//                writes ("OP")
//                writen(h1!x)
                writeop(h1!x)
                writef(" [%n]",x)
                FOR i = 2 TO size DO
                     $( newline()
                        FOR j=0 TO n-1 DO writes( v!j )
                        writes("**-")
                        v!n := i=size->"  ","! "
                        plist(h1!(x+i-1), n+1, d)  $)

    RETURN
    $)1

AND writeop(x) BE
$(
    $(1 LET s = VALOF $(
        SWITCHON x&255 INTO
        $(
           DEFAULT:
                    writes("OP")
                    writen(x)
                    RETURN

           CASE s.for:     RESULTIS "FOR"

           CASE s.cond:    RESULTIS "COND"
           CASE s.fndef:   RESULTIS "FUNCTION"
           CASE s.rtdef:   RESULTIS "ROUTINE"
           CASE s.test:    RESULTIS "TEST"
           CASE s.constdef:RESULTIS "CONSTDEF"

           // nodes with 2 descendants
//****           CASE S.FIX:     RESULTIS "FIX"
           CASE s.vecap:   RESULTIS "PLING"
$<WORD
           CASE s.wordap:  RESULTIS "WORD-PLING"
$>WORD
           CASE s.byteap:  RESULTIS "BYTE-PLING"
           CASE s.mult:    RESULTIS "MULT"
           CASE s.div:     RESULTIS "DIV"
           CASE s.rem:     RESULTIS "REM"
           CASE s.plus:    RESULTIS "PLUS"
           CASE s.minus:   RESULTIS "MINUS"
           CASE s.eq:      RESULTIS "EQ"
           CASE s.ne:      RESULTIS "NE"
           CASE s.ls:      RESULTIS "LS"
           CASE s.gr:      RESULTIS "GR"
           CASE s.le:      RESULTIS "LE"
           CASE s.ge:      RESULTIS "GE"
           CASE s.lle:     RESULTIS "LLE"
           CASE s.lge:     RESULTIS "LGE"
           CASE s.lls:     RESULTIS "LLS"
           CASE s.lgr:     RESULTIS "LGR"
           CASE s.lshift:  RESULTIS "LSHIFT"
           CASE s.rshift:  RESULTIS "RSHIFT"
           CASE s.logand:  RESULTIS "LOGAND"
           CASE s.logor:   RESULTIS "LOGOR"
           CASE s.eqv:     RESULTIS "EQV"
           CASE s.neqv:    RESULTIS "NEQV"
           CASE s.comma:   RESULTIS "COMMA"
           CASE s.commawithline:RESULTIS "COMMA"
           CASE s.and:     RESULTIS "AND"
           CASE s.valdef:  RESULTIS "VALDEF"
           CASE s.vecdef:  RESULTIS "VECDEF"
           CASE s.ass:     RESULTIS "ASSIGN"
           CASE s.colon:   RESULTIS "LABEL"
           CASE s.if:      RESULTIS "IF"
           CASE s.unless:  RESULTIS "UNLESS"
           CASE s.while:   RESULTIS "WHILE"
           CASE s.until:   RESULTIS "UNTIL"
           CASE s.repeatwhile: RESULTIS "REPEATWHILE"
           CASE s.repeatuntil: RESULTIS "REPEATUNTIL"
           CASE s.switchon:RESULTIS "SWITCHON"
           CASE s.case:    RESULTIS "CASE"
           CASE s.seq:     RESULTIS "SEQ"
           CASE s.let:     RESULTIS "LET"
           CASE s.fnap:    RESULTIS "FNCALL"
           CASE s.rtap:    RESULTIS "RTCALL"
           CASE s.section: RESULTIS "SECTION"
           CASE s.needs:   RESULTIS "NEEDS"
           CASE s.global:  RESULTIS "GLOBAL"
           CASE s.static:  RESULTIS "STATIC"
           CASE s.manifest:RESULTIS "MANIFEST"
$<MACRO    CASE s.macro:   RESULTIS "MACRO" $>MACRO

           // nodes with 1 descendant
           CASE s.valof:   RESULTIS "VALOF"
           CASE s.lv:      RESULTIS "LV"
           CASE s.abs:     RESULTIS "ABS"
           CASE s.neg:     RESULTIS "NEG"
           CASE s.not:     RESULTIS "NOT"
           CASE s.rv:      RESULTIS "RV"
           CASE s.table:   RESULTIS "TABLE"
           CASE s.goto:    RESULTIS "GOTO"
           CASE s.resultis:RESULTIS "RESULTIS"
           CASE s.repeat:  RESULTIS "REPEAT"
           CASE s.default: RESULTIS "DEFAULT"
           CASE s.code:    RESULTIS "CODE"
           CASE s.car:     RESULTIS "CAR"
           CASE s.cdr:     RESULTIS "CDR"

           // nodes with no descendants
           CASE s.loop:    RESULTIS "LOOP"
           CASE s.break:   RESULTIS "BREAK"
           CASE s.return:  RESULTIS "RETURN"
           CASE s.endcase: RESULTIS "ENDCASE"
           CASE s.query:   RESULTIS "QUERY"
           CASE s.true:    RESULTIS "TRUE"
           CASE s.false:   RESULTIS "FALSE"
           CASE s.finish:  RESULTIS "FINISH"
           CASE s.nil:     RESULTIS "NIL"
           $)
        $)

        LET v = VEC 20
        UNLESS (x&flbit)=0 DO wrch('#')
        writes(s)
        UNLESS (x&becomesbit)=0 DO writes("AB")
$)1
$)

AND uppercasestring(s) BE
   FOR i = 1 TO getbyte(s, 0)
   DO putbyte(s, i, uppercase(getbyte(s, i)))

AND charcode(ch) = ch

// Look in current directory and nowhere else
AND findget(name) = VALOF
$(
   LET stream = ?
   LET fname  =  VEC 256/bytesperword

   // Null string means get input file again
   IF name%0 = 0 RESULTIS findinput( fromfile )

   stream := findinput( name )

   IF stream = 0 THEN

   $(
      // Can't open the file, so look in each of the include directories
      LET ptr = dir.incl

      UNTIL  ptr = 0 | stream NE 0  DO
      $(
	 LET dir.name = ptr + 1
	 LET dirl = dir.name%0

	 FOR  i = 1  TO  name%0  DO
	    fname % (dirl + i + 1)  :=  name % i
	 fname%(dirl+1) := '/'
	 
	 FOR  i = 1  TO  dirl  DO
	    fname % i  :=  dir.name % i
	 
	 fname % 0 :=  dirl + name%0 + 1
	 stream    :=  findinput( fname )

	 ptr := ptr!0
      $)

   $)

   IF stream = 0 DO caereport(-88)

   RESULTIS stream
$)

AND codegen() BE
$(
   selectoutput(verstream)
//   writef("obufp = %n obufb = %n*N",obufp,obufb)
   TEST [(obufp-workvec)*bytesperword+obufb] > 0 //ocode in store ?
   THEN $(  // take ocode from store
      curp, curb := workvec, -1  // initial values for ocode pointers
      rdn := st.rdn
      ocodestream := 0 // no ocode stream
   $)
   ELSE $( // otherwise take it from the file
      ocodestream := findinput(ocodefile) // should not fail (!!)
      selectinput(ocodestream)
      rdn := ci.rdn
   $)
   naming := cgnaming

   workspace := obufp+10 // a small safety margin
   cgworksize := workvec+workspacesize-workspace

   call.cg()

   UNLESS ocodestream = 0 DO
   $(
      selectinput(ocodestream)
      endread()
      ocodestream := 0
   $)

$)

// read in OCODE operator or argument from current input
// argument may be of form Ln
AND ci.rdn() = VALOF
$( LET a, sign = 0, '+'
   LET ch = 0

   ch := rdch() REPEATWHILE
      ch='*S' | ch='*N' | ch='L'

   IF ch=endstreamch RESULTIS 0

   IF ch='-' DO $( sign := '-'
                   ch := rdch()
                $)

   WHILE '0'<=ch<='9' DO $( a := 10*a + ch - '0'
                            ch := rdch()
                         $)

   IF sign='-' DO a := -a
   RESULTIS a
$)

// reads an ococde operator or argument from the
// in-store buffer.
AND st.rdn() = VALOF
$(
   LET n = 0
   LET i = 0
   LET b = 0
$<AMIDOS'
   result2 := FALSE
$>AMIDOS'
$<AMIDOS
   result2(TRUE,FALSE)
$>AMIDOS
   $(
      b := rdbyte()
$<AMIDOS'
      IF result2 RESULTIS 0
$>AMIDOS'
$<AMIDOS
      IF result2(FALSE) RESULTIS 0
$>AMIDOS
      n := n | [(b & #X7F) << i]
      IF (b & #X80) = 0 BREAK
      i := i + 7
   $) REPEAT
   RESULTIS n
$)

AND rdbyte() = VALOF
$(
   curb := curb + 1
   IF curb >= bytesperword DO
   $(
      curb := 0
      curp := curp + 1
   $)
   IF [curp = obufp] & [curb = obufb] THEN
   $(
$<AMIDOS'
      result2 := TRUE
$>AMIDOS'
$<AMIDOS
      result2(TRUE,TRUE)
$>AMIDOS
      RESULTIS 0
   $)
   RESULTIS curp%curb
$)

AND newvec(n) = VALOF
$( treep := treep - n - 1
   IF treep <= workbase THEN caereport(-98)
   RESULTIS treep
$)
