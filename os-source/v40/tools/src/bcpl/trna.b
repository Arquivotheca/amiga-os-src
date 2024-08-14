SECTION "TRNA"
GET "LIBHDR"
GET "BCPHDR"
GET "TRNHDR"

LET nextparam() = VALOF
    $( paramnumber := paramnumber + 1
       RESULTIS paramnumber  $)

AND trnerrwrite(n, x) BE
       $(
          writef("%S near line %N", (n<100 -> "Warning:", "Error"),
                                       trnline REM 100000)
          TEST trnline >= 100000 DO
          $(  LET n = trnline/100000      // sequence number of GET file
              LET p = getchain            // chain of GET nodes in reverse order
              FOR i = 1 TO ngets-n DO p := !p
                                          // P now points to correct node
              writef(" of GET file %S:  ", p+5)
          $)
          OR
              writes(":  ")
          trnmessage(n, x)

          TEST n<100 DO
              IF warncount = warnmax DO
                  writes("Further warnings suppressed*N")
          OR
              IF reportcount>=reportmax
                  writes("Too many errors - compilation abandoned*N")
       $)


AND transreport(n, x) BE
$(
    IF n < 0 DO $( n := -n; reportmax := 0 $)
    TEST n<100 DO
    $(  // codes less than 100 are warnings
        warncount := warncount+1
        IF warncount > warnmax RETURN
    $)
    OR
    $(  // others are errors
        reportcount := reportcount+1
    $)

    selectoutput(sysopt)
        IF ocode & ocodestream=sysopt DO $( newline(); ocount:=0 $)
        IF quiet & NOT prsource & reportcount+warncount=1 &
               getbyte(sectionname, 0)  NE  0 DO
            writef("Section *"%S*"*N", sectionname)
    trnerrwrite(n, x)
    UNLESS ocodestream = 0 DO selectoutput(ocodestream)
    IF reportcount>=reportmax
    DO $(
        rc := 20
        longjump(err.p,err.l)
    $)
$)

AND trnmessage(n, x) BE
$(  LET fatal, prname, prconst = FALSE, TRUE, FALSE

    LET s = VALOF
    SWITCHON n INTO
    $( DEFAULT: writes("Compiler error  "); writen(n); newline()
$<TOS
                userexit("Hit <RETURN> to continue")
$>TOS
                RETURN

$<TRIPOS
       CASE 150: RESULTIS "BREAK detected in TRN"
$>TRIPOS
$<MTRIPOS
       CASE 150: RESULTIS "BREAK detected in TRN"
$>MTRIPOS
$<AMIDOS
       CASE 150: RESULTIS "BREAK detected in TRN"
$>AMIDOS
$<TOS
       CASE 150: RESULTIS "BREAK detected in TRN"
$>TOS
       CASE 141: RESULTIS "Too many cases"
       CASE 104: RESULTIS "Illegal use of BREAK, LOOP or RESULTIS"
       CASE 101:
       CASE 105: RESULTIS "Illegal use of CASE or DEFAULT"
       CASE 106: prconst := TRUE
                 RESULTIS "Two cases with same constant"
       CASE 144: fatal := TRUE
                 RESULTIS "Too many globals"
       CASE 142: prname := TRUE
                  RESULTIS "Name declared twice"
       CASE 145: fatal := TRUE
                 RESULTIS "Workspace too small"
       CASE 121:
       CASE 115: prname := TRUE
                 RESULTIS "Name not declared"
       CASE 116: prname := TRUE
                 RESULTIS "Dynamic free variable used"
       CASE 117:CASE 118:
                 RESULTIS "Error in constant expression"
       CASE 119: prname := TRUE
                RESULTIS "Name not declared manifest"
       CASE 120:
                 RESULTIS "Divide by zero in constant"
       CASE 147: RESULTIS "Error in expression"
       CASE 110:CASE 112:
                 RESULTIS "LHS and RHS do not match"
       CASE 109:CASE 113:
                 RESULTIS "Ltype expression expected"
$<MACRO
       CASE 134: RESULTIS "Too many macro calls - use M option"
       CASE 133: RESULTIS "Wrong number of arguments to macro call"
       CASE 135: RESULTIS "Unknown macro argument (compiler error)"
       CASE 136: RESULTIS "Expression found where macro name expected"
       CASE 137: RESULTIS "Macro Routine called as Function"
       CASE 138: RESULTIS "Macro Function called as Routine"
$>MACRO
$<EXT
       CASE 139: RESULTIS "Illegal Declaration of EXTERNAL name"
$>EXT
       CASE  91: RESULTIS "RETURN used in body of function"
       CASE  92: RESULTIS "Value of VALOF block may be undefined"
       CASE  81: RESULTIS "Constant condition in IF, UNLESS or TEST"
       CASE  82: RESULTIS "Constant condition in WHILE or UNTIL"
    $)

   writes(s)
   IF prname & (x NE 0) & h1!x=s.name writef(" - %S", x+2)
   IF prconst writef(" - %N", x)
   newline()

$<TOS
   UNLESS cont.errs DO
     UNLESS userexit("Hit <RETURN> to continue, *'A*' to abort")
        DO fatal := TRUE
$>TOS

   IF fatal DO reportmax := 0
$)

AND dummy() BE RETURN

AND compileae(x) BE
$(1 LET d = newvec(maxglobals*2)
    LET k = newvec(256)
    LET l = newvec(256)

$<MACRO
    mvec  := newvec(macmax)
    mvecp := -1
$>MACRO

    dvec, dvecs, dvece, dvecp := treep, -4, -4, -4

    globdecl, globdecls, globdeclt := d, 0, maxglobals*2

    casek, casel, casep, caset, caseb := k, l, 0, 256, -1
    endcaselabel, defaultlabel := -1, -1

    resultlabel, breaklabel, looplabel := -1, -1, -1
    returnlabel, finishlabel := -1, 0
    inproc := FALSE
    accessible := FALSE
    err.p, err.l := level(), err.lab

    trnunused := dvec+dvecs-obufp

    currentbranch := x
    UNLESS ocode THEN writeop, wrn, wrc, endocode :=
                           putbytes, putbytes, dummy, dummy
    IF numocode THEN writeop := wrn

    ocount := 0

    paramnumber := 0
    ssp := savespacesize
    UNLESS x=0 THEN
    $(  LET t = h1!x
        UNLESS t=s.section | t=s.needs THEN BREAK
        out1(t)
        outstring(h2!x+1)
        x := h3!x
    $) REPEAT
    out2(s.stack, ssp)
    trnblockbody(x)
    complab(finishlabel)
    IF accessible DO out1(s.finish)
    out2(s.global, globdecls/2)

    $( LET i = 0
       UNTIL i=globdecls DO
          $( outn(globdecl!i)
             outl(globdecl!(i+1))
             i := i + 2
          $)
    $)

err.lab:
    endocode()
$)1


LET trans(x) BE
  $(tr
next:
 $( LET sw = FALSE
    AND h1x, h2x, h3x = 0, 0, 0
    IF x=0 RETURN
    currentbranch := x

$<UNIX'
    IF testflags(1) DO
    $(
        rc := 20
        transreport(-150)
    $)
$>UNIX'

    h1x, h2x, h3x := h1!x, h2!x, h3!x
    IF (h1x&becomesbit) NE 0 THEN
    $(  assign(h2x, h3x, h1x&\becomesbit, h4!x, h4!x); RETURN  $)

    SWITCHON h1x INTO
$(  DEFAULT: transreport(100, x); RETURN

    CASE s.let: CASE s.static:
    CASE s.manifest: CASE s.global:
$<MACRO
    CASE s.macro:
$>MACRO
$<EXT
    CASE s.external:
$>EXT
    $(  LET a, b, s, v = dvece, dvecs, ssp, vecssp
        trnblockbody(x)
        IF naming&inproc THEN $( out1(s.endblock); writenames(b) $)
        UNLESS ssp=s DO out2(s.stack, s)
        dropnames(b)
        dvece, ssp, vecssp := a, s, v
        RETURN
    $)

    CASE s.ass:
       assign(h2x, h3x, s.stind, h4!x, h4!x)
       RETURN

    CASE s.rtap:
$<MACRO
     TEST ismacro( h2x ) 
     THEN macrocall( x , s.rtap )
     ELSE
$>MACRO
     $( LET s = ssp
        trnline := h4!x
        ssp := ssp+savespacesize
        out2(s.stack, ssp)
        loadlist(h3x)
        load(h2x)
        out2(s.rtap, s)
        ssp := s
     $)
     RETURN

    CASE s.goto:
        trnline:=h3x
        load(h2x)
        out1(s.goto)
        accessible := FALSE
        ssp := ssp-1
        RETURN

    CASE s.colon:
        compblab(h4!x)
        trans(h3x)
        RETURN

    CASE s.unless: sw := TRUE
    CASE s.if:
        trnline := h4!x
        IF trnoptimise & scanexpr(h2x) DO
        $(
            transreport(81,0)
            IF evaltv(h2x) NEQV sw DO trans(h3x)
            RETURN
        $)
        $(  LET l = isjump(h3x)
            IF l>0 DO
            $(  jumpcond(h2x, NOT sw, l)
                RETURN
            $)
            l := nextparam()
            jumpcond(h2x, sw, l)
            trans(h3x)
            complab(l)
            RETURN
        $)

    CASE s.test:
        trnline := h5!x
        IF trnoptimise & scanexpr(h2x) DO
        $(
            transreport(81,0)
            trans(evaltv(h2x) -> h3x, h4!x)
            RETURN
        $)
        $(  LET l, m = isjump(h3x), ?
            IF l>0 DO
            $(  jumpcond(h2x, TRUE, l)
                trans(h4!x)
                RETURN
            $)
            m := isjump(h4!x)
            IF m>0 DO
            $(  jumpcond(h2x, FALSE, m)
                trans(h3x)
                RETURN
            $)
            l, m := nextparam(), nextparam()
            jumpcond(h2x, FALSE, l)
            trans(h3x)
            TEST accessible DO compjump(m) OR m := 0
            complab(l)
            trans(h4!x)
            complab(m)
            RETURN
        $)

    CASE s.loop:
        IF looplabel<0 DO transreport(104, x)
        IF looplabel=0 DO looplabel := nextparam()
        compjump(looplabel)
        RETURN

    CASE s.break:
        IF breaklabel<0 DO transreport(104, x)
        IF breaklabel=0 DO breaklabel := nextparam()
        compjump(breaklabel)
        RETURN

    CASE s.return: IF returnlabel<0 DO transreport(91, x)
                   out1(s.rtrn)
                   accessible := FALSE
                   RETURN

    CASE s.finish: out1(s.finish)
                   accessible := FALSE
                   RETURN

    CASE s.resultis:
        trnline:=h3x
        IF resultlabel<0 DO transreport(104, x)
        load(h2x)
        TEST resultlabel=0 DO out1(s.fnrn) OR out2p(s.res, resultlabel)
        accessible := FALSE
        ssp := ssp - 1
        RETURN

    CASE s.while: sw := TRUE
    CASE s.until:
     trnline:=h4!x
     IF trnoptimise & scanexpr(h2x) DO
     $(
        transreport(82,0)
        IF (sw NEQV evaltv(h2x)) RETURN
     $)
     $( LET l, m = nextparam(), nextparam()
        LET bl, ll = breaklabel, looplabel
        LET saveline = ?
        breaklabel, looplabel := 0, m
        compjump(m)
        complab(l)
        trans(h3x)
        complab(m)
        saveline := trnline
        trnline := h4!x
        jumpcond(h2x, sw, l)
        trnline := saveline
        complab(breaklabel)
        breaklabel, looplabel := bl, ll
        RETURN   $)

    CASE s.repeatwhile: sw := TRUE
    CASE s.repeatuntil:
    CASE s.repeat:
     $( LET l, bl, ll = nextparam(), breaklabel, looplabel
        breaklabel, looplabel := 0, 0
        complab(l)
        TEST h1!x=s.repeat
            THEN $( looplabel := l
                    trnline:=h3x
                    trans(h2x)
                    compjump(l)  $)
              OR $( trnline:=h4!x
                    trans(h2x)
                    complab(looplabel)
                    jumpcond(h3x, sw, l)  $)
        complab(breaklabel)
        breaklabel, looplabel := bl, ll
        RETURN   $)

    CASE s.case:
     trnline := h4!x
     $( LET l, k = nextparam(), evalconst(h2x)
        IF casep>=caset DO $( transreport(141, x)
                              RETURN  $)
        IF caseb<0 DO transreport(105, x)
        FOR i = caseb TO casep-1 DO
                    IF casek!i=k DO transreport(106, k)
        casek!casep := k
        casel!casep := l
        casep := casep + 1
        complab(l)
        trans(h3x)
        RETURN   $)

    CASE s.default:
        IF caseb<0 DO transreport(105, x)
        UNLESS defaultlabel=0 DO transreport(101, x)
        defaultlabel := nextparam()
        complab(defaultlabel)
        trans(h2x)
        RETURN

    CASE s.endcase: IF endcaselabel<0 DO transreport(105, x)
                    IF endcaselabel=0 DO endcaselabel := nextparam()
                    compjump(endcaselabel)
                    RETURN

    CASE s.switchon:
        trnline:=h4!x
        transswitch(x)
        RETURN

    CASE s.for: transfor(x)
                RETURN

    CASE s.code:
        $( LET a=h2!x
           WHILE h1!a=s.comma | h1!a=s.commawithline DO $(
               IF h1!a=s.commawithline trnline:=h4!a
               out2(s.code,evalconst(h2!a))
               a:=h3!a $)
           out2(s.code,evalconst(a))
           RETURN $)

    CASE s.seq:
        trans(h2x)
        x := h3x
        GOTO next        $)tr

AND isjump(x) = VALOF
$(  // tests command X to see if it can be represented as an
    // unconditional jump. Returns label number for jump, or
    // -1 if there is any difficulty.

    LET addr = ?
    SWITCHON h1!x INTO
    $(
      DEFAULT:  RESULTIS -1

      CASE s.loop:    addr := @looplabel;   ENDCASE
      CASE s.break:   addr := @breaklabel;  ENDCASE
      CASE s.endcase: addr := @endcaselabel;ENDCASE
      CASE s.return:  addr := @returnlabel; ENDCASE
      CASE s.finish:  addr := @finishlabel; ENDCASE
    $)
    IF !addr < 0 RESULTIS -1
    IF !addr=0 DO !addr := nextparam()
    RESULTIS !addr
$)

AND writenames(x) BE

$(  LET i, n = x, 0
    WHILE i>dvecs DO
    $(  UNLESS dvec!(i+1)=s.number THEN n := n+1
        i := i-4
    $)
    outn(n)
    WHILE x>dvecs DO
    $(  UNLESS dvec!(x+1)=s.number THEN
        $(  outstring(dvec!x + 2)
            outn(dvec!(x+1)-s.manifest)
            outn(dvec!(x+2))
        $)
        x := x-4
    $)
$)

AND trnblockbody(x) BE

$(1 IF naming & inproc THEN out1(s.startblock)
    UNTIL x=0 DO
    $(2 LET h2x = h2!x
        LET op  = h1!x
//        WRITEF("TRNBLOCKBODY: OP is %A(%A) H2X is %A(%A) X is %A*N",
//                OP, H1!X, H2X, H2!X, X)    //****************
        SWITCHON op INTO

        $(3 DEFAULT:
                decllabels(x); trans(x); RETURN

            CASE s.let:
                $(  LET s, s1, b = ssp, 0, dvecs
                    trnline := h4!x
                    declnames(h2x)
                    dvece := dvecs
                    vecssp := ssp
                    s1 := ssp
                    ssp := s
                    trnline := h4!x    // DECLNAMES changed it!
                    transdef(h2x)
                    UNLESS ssp=s1 DO transreport(110, x)
                    UNLESS ssp=vecssp DO $( ssp := vecssp
                                            out2(s.stack, ssp)  $)
                    out1(s.store)
                    ENDCASE
                $)

$<MACRO
            CASE s.macro:
               trnline := h4!x
               macdef(h2x)
               dvece := dvecs
               ENDCASE
$>MACRO

            CASE s.manifest:
            CASE s.static:
            CASE s.global:
$<EXT
            CASE s.external:
$>EXT
                $(  LET y = h2x
                    trnline := h4!x
                    UNTIL y=0 DO
                    $(  trnline := h4!y
$<EXT
                        TEST op=s.external THEN
                           addname(h2!y, op, h3!y, op)
                        ELSE
$>EXT
                        TEST op=s.static THEN
                        $(  LET m = nextparam()
                            addname(h2!y, s.label, m, s.static)
                            compdatalab(m)
                            out2(s.itemn, evalconst(h3!y))
                        $)
                        ELSE
                           addname(h2!y, (op=s.manifest->s.number,op),
                                          evalconst(h3!y), op)
                        y := h1!y
                        dvece := dvecs
                    $)
//     WRITES("All names added*N")  //****************
                    ENDCASE
                $)

        $)3
//        WRITEF("X=%A, H3!X=%A*N", X, H3!X)  //*************
        x := h3!x
    $)2
//    WRITES("leaving TRNBLOCKBODY")  //***************
$)1

/*
AND dbg(s,a,b,c,d) BE
$( let o = output()
   selectoutput(sysout)
   writef(s,a,b,c,d)
   selectoutput(o)
$)
*/

$<MACRO
AND macdef(x) BE
   UNTIL x = 0 DO
   $(
      SWITCHON h1!x INTO
      $(
      DEFAULT: transreport(102,0)

      CASE s.fndef:
      CASE s.rtdef:
         addname(h2!x, s.macro, x, s.macro)
         RETURN

      CASE s.and:
         macdef( h2!x )
         trnline := h4!x
         x := h3!x
      $)
   $)

AND ismacro( n ) = VALOF
$(
   TEST (n!h1 = s.name) THEN
   $( LET p = dvecentry( n )
      RESULTIS (p = 0) -> FALSE, (p ! 1 = s.macro)
   $)
   ELSE RESULTIS FALSE
$)

AND dvecentry( n ) = VALOF
$(
   LET d = dvec + ( n ! 1 )          // the dvec entry
   WHILE [d NE 0] & [(((d ! 1) & disbit) = disbit) | (d ! 1 = s.mpar)]
      TEST d ! 1 = s.mpar
      THEN TEST (d ! 2) ! 0 = s.name
           THEN d := dvec + ( d ! 2 ) ! 1
           ELSE transreport(-136)   // expression found where macro expected
      ELSE d := dvec ! ( d ! 3 )
   RESULTIS d
$)

// The following is the code for a call-by-name macro scheme, this is closest
// to the facilities provided by a textual macro pre-processor, but does not
// fit in with the BCPL view of procedures.
AND macrocall( x, type ) BE
$(
   LET dve = dvecentry( x ! h2 )
   LET mdef = dve ! h3
   LET args = mdef ! h3                  // argument list
   LET pars = x ! h3                     // repacement expressions
   LET b = dvecs

   IF type = s.rtap DO trnline := x!h4

   TEST type = s.rtap
   THEN UNLESS mdef!h1 = s.rtdef DO $( transreport(138, x!h2); RETURN $)
   ELSE UNLESS mdef!h1 = s.fndef DO $( transreport(137, x!h2); RETURN $)

   IF mvecp >= macmax
   DO $( transreport(134,x!h2); RETURN $)       // too many macro calls

   mvecp := mvecp + 1                      // step on to next slot
   mvec!mvecp := dvecs+4                   // set lwb of prev macros args/locals

   // first define all the arguments
   UNLESS args = 0 DO
   $(
      LET op = args!h1
      LET pop = pars!h1
      IF op = s.name DO
      $( IF pop = s.comma | pop = s.commawithline DO transreport(133,x!h2)
         addname( args, s.mpar, pars, 0)
         BREAK
      $)
      IF op=s.comma | op=s.commawithline DO
      $( IF op=s.commawithline DO trnline := h4!pars
         UNLESS pop=s.comma | pop=s.commawithline DO transreport(133,x!h2)
         addname( args!h2 , s.mpar, pars!h2, 0)
         args := h3!args
         pars := h3!pars
         LOOP
      $)
      transreport(133,x!h2)
   $) REPEAT

   // that done, compile the macro code

   TEST type = s.rtap
   THEN trans( mdef ! h4 )
   ELSE load ( mdef ! h4 )

   dropnames(b)
   mvecp := mvecp - 1  // dispose of bounds
$)
$>MACRO

LET declnames(x) BE
    UNTIL x=0 DO
    $(
       LET op = h1!x
       SWITCHON op INTO
       $(
       DEFAULT: transreport(102, currentbranch)
                 RETURN

       CASE s.vecdef: CASE s.valdef:
             decldyn(h2!x, op)
             RETURN

       CASE s.rtdef: CASE s.fndef:
             h5!x := nextparam()
             declstat(h2!x, h5!x, op)
             RETURN

       CASE s.and:
             declnames(h2!x)
             trnline := h4!x
             x := h3!x
             ENDCASE
       $)
    $)

AND decldyn(x, c) BE IF x NE 0 THEN     //*******BUGGY  BUGGY BUGGY!!!!!!
    $( LET op = h1!x
//***********       IF X=0 RETURN

       IF op=s.name DO
          $( addname(x, s.local, ssp, c)
             ssp := ssp + 1
             RETURN   $)

       IF op=s.comma | op=s.commawithline DO
          $( IF op=s.commawithline DO trnline := h4!x
             addname(h2!x, s.local, ssp, c)
             ssp := ssp + 1
             x := h3!x
             LOOP  $)

       transreport(103, x)
       RETURN                $) REPEAT

AND declstat(x, l, c) BE
$(1 LET t = h2!x
    LET m = ?

$<EXT
    IF [t \= 0] & [dvec!(t+1)=s.external] THEN
       transreport(139)
$>EXT
    IF [t \= 0] & [dvec!(t+1)=s.global] DO
    $( LET n = dvec!(t+2)
       IF globdecls >= globdeclt THEN
       $( transreport(144, x)
          RETURN
       $)
       globdecl!globdecls := n
       globdecl!(globdecls+1) := l
       globdecls := globdecls + 2
       RETURN
    $)

    m := nextparam()
    addname(x, s.label, m, c)
    compdatalab(m)
    out2p(s.iteml, l)
$)1


AND decllabels(x) BE
$(  LET b = dvecs
    LET saveline = trnline
    scanlabels(x)
    trnline := saveline
    dvece := dvecs
$)



AND addname(n, p, a, c) BE
$(  dvec!dvecs, dvec!(dvecs+1), dvec!(dvecs+2),
        dvec!(dvecs+3) := n, p, a, n!1
$<MACRO'
    UNLESS n!1=0 | n!1>dvece DO transreport(142, n)
$>MACRO'
$<MACRO
    $(
        LET d = n!1
        UNTIL d = 0 | d > dvece DO
        $(
            UNLESS ((dvec ! d ! 1) & disbit) = disbit DO transreport(142,n)
            d := dvec ! d ! 3
        $)
    $)
$>MACRO
    n!1 := dvecs
    dvecs := dvecs-4
    checkworkspace()
$)


AND dropnames(s) BE
    UNTIL dvecs=s DO
    $(  dvecs := dvecs+4
        $(  LET n = dvec!dvecs
            n!1 := dvec!(dvecs+3)
        $)
    $)


AND scanlabels(x) BE
    $( LET sw = FALSE
       IF x=0 RETURN

       SWITCHON h1!x INTO
    $( DEFAULT: RETURN

       CASE s.colon:
            trnline := h5!x
            h4!x := nextparam()
            declstat(h2!x, h4!x, s.colon)

       CASE s.unless: CASE s.until: sw := TRUE
       CASE s.if: CASE s.while:
            IF trnoptimise & scanexpr(h2!x) DO
                          // don't scan command if it won't be compiled
                IF evaltv(h2!x) EQV sw RETURN
       CASE s.switchon: CASE s.case:
            scanlabels(h3!x)
            RETURN

       CASE s.seq:
            scanlabels(h3!x)

       CASE s.repeat:
       CASE s.repeatwhile: CASE s.repeatuntil: CASE s.default:
            scanlabels(h2!x)
            RETURN

       CASE s.test:
            IF trnoptimise & scanexpr(h2!x) DO
            $(  TEST evaltv(h2!x) DO scanexpr(h3!x) OR scanexpr(h4!x)
                RETURN
            $)
            scanlabels(h3!x)
            scanlabels(h4!x)
            RETURN   $)   $)


AND transdef(x) BE UNLESS x=0 DO
    $(1 LET saveline=trnline
//        WRITEF("In TRANSDEF with %A*N",X)  //**********
        transdyndefs(x)
//        WRITEF("left TRANSDYNDEF*N")  //**********
        IF statdefs(x) DO
           $( LET l, s= nextparam(), ssp
              TEST accessible DO compjump(l) OR l := 0
              trnline := saveline
              transstatdefs(x)
//              WRITEF("left TRANSSTATDEF*N")  //**********
              ssp := s
              out2(s.stack, ssp)
              complab(l)  $)
//        WRITEF("leaving TRANSDEF*N")  //**********
$)1


AND transdyndefs(x) BE
$(
//        WRITEF("In TRANSDYNDEF with %A*N",X)  //**********
     IF x = 0 THEN RETURN

     SWITCHON h1!x INTO
     $( CASE s.and:
            transdyndefs(h2!x)
            trnline := h4!x
            x := h3!x
            LOOP

        CASE s.vecdef:
         $( LET n = evalconst(h3!x)
            out2(s.llp, vecssp)
            ssp := ssp + 1
            vecssp := vecssp + 1 + n
            RETURN  $)

        CASE s.valdef: loadlist(h3!x)
                       RETURN

        DEFAULT: RETURN  $)
$) REPEAT

AND transstatdefs(x) BE
$(
//        WRITEF("In TRANSSTATDEF with %A and %N*N",X,H1!X)  //**********
     IF x = 0 RETURN

     WHILE h1!x = s.and DO
     $(
        transstatdefs(h2!x)
        trnline := h4!x
        x := h3!x
     $)

     IF [h1!x = s.fndef] | [h1!x = s.rtdef] DO
      $(2 LET a, b, c = dvece, dvecs, dvecp
          AND bl, ll = breaklabel, looplabel
          AND rl, cb = resultlabel, caseb
          AND ecl = endcaselabel
          AND rtl = returnlabel
          AND fil = finishlabel
          AND ip = inproc
          inproc := TRUE
          breaklabel, looplabel := -1, -1
          resultlabel, caseb := -1, -1
          returnlabel := -1

          compentry(h2!x, h5!x)
          ssp := savespacesize

          dvecp := dvecs
          decldyn(h3!x, h1!x)
          dvece := dvecs
          decllabels(h4!x)

          finishlabel := 0

          out2(s.save, ssp)

          TEST h1!x=s.fndef DO
          $(  LET y = h4!x
              TEST h1!y = s.valof DO
              $(  // optimisation of resultis when body of
                  // function is a valof.
                  decllabels(h2!y)
                  resultlabel := 0  // SEE CASE S.RESULTIS IN TRANS
                  trans(h2!y)
                  IF accessible DO transreport(92, x)
              $)
              OR
              $(  load(y); out1(s.fnrn); accessible := FALSE $)
          $)
          OR
          $(  returnlabel := 0
              trans(h4!x)
              complab(returnlabel)
          $)

          IF accessible DO $( out1(s.rtrn); accessible := FALSE $)


          complab(finishlabel)
          IF accessible DO $( out1(s.finish); accessible := FALSE $)

          out1(s.endproc)
          TEST naming THEN writenames(b) OR outn(0)

          inproc := ip
          finishlabel := fil
          returnlabel := rtl
          endcaselabel := ecl
          breaklabel, looplabel := bl, ll
          resultlabel, caseb := rl, cb
          dropnames(b)
          dvece, dvecp := a, c
      $)2
$)

AND statdefs(x) = x = 0 -> FALSE,
                  h1!x=s.fndef | h1!x=s.rtdef -> TRUE,
                  h1!x NE s.and -> FALSE,
                  statdefs(h2!x) -> TRUE,
                  statdefs(h3!x)
