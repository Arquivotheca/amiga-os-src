SECTION "TRNB"
GET "LIBHDR"
GET "BCPHDR"
GET "TRNHDR"

LET jumpcond(x, b, l) BE
$(  scanexpr(x)
    jcond(x, b, l)
$)

AND evaltv(x) = VALOF
$(  SWITCHON h1!x INTO
    $(
      CASE s.not:   RESULTIS NOT evaltv(h2!x)

      CASE s.logand:
        UNLESS evaltv(h2!x) RESULTIS FALSE
        ENDCASE

      CASE s.logor:
        IF     evaltv(h2!x) RESULTIS TRUE
        ENDCASE

      DEFAULT:
        RESULTIS evalconst(x) -> TRUE, FALSE

    $)

    RESULTIS evaltv(h3!x)    // common code for S.LOGAND and S.LOGOR
$)

AND jcond(x, b, l) BE
$(jc LET sw = b
     LET op = h1!x

     IF (op & constbit) NE 0 DO
     $(  LET c = evaltv(x)

         IF b EQV c DO compjump(l)

         RETURN
     $)

     SWITCHON op INTO
     $(

        CASE s.not: jumpcond(h2!x, NOT b, l)
                    RETURN

        CASE s.logand: sw := NOT sw
        CASE s.logor:
         TEST sw THEN $( jumpcond(h2!x, b, l)
                         jumpcond(h3!x, b, l)  $)

                   OR $( LET m = nextparam()
                         jumpcond(h2!x, NOT b, m)
                         jumpcond(h3!x, b, l)
                         complab(m)  $)

         RETURN

        DEFAULT: load(x)
                 out2p(b -> s.jt, s.jf, l)
                 ssp := ssp - 1
                 RETURN     $)jc

AND transswitch(x) BE
    $(1 LET p, b, dl = casep, caseb, defaultlabel
        AND ecl = endcaselabel
        LET l = nextparam()
        endcaselabel := 0
        caseb := casep

        compjump(l)
        defaultlabel := 0
        trans(h3!x)
        IF accessible DO
        $(  IF endcaselabel=0 DO endcaselabel := nextparam()
            compjump(endcaselabel)
        $)

        complab(l)
        load(h2!x)
        IF defaultlabel=0 DO
        $(  IF endcaselabel=0 DO endcaselabel := nextparam()
            defaultlabel := endcaselabel
        $)
        out3p(s.switchon, casep-p, defaultlabel)

        FOR i = caseb TO casep-1 DO $( outn(casek!i)
                                       outl(casel!i)  $)

        ssp := ssp - 1
        accessible := FALSE
        complab(endcaselabel)
        endcaselabel := ecl
        casep, caseb, defaultlabel := p, b, dl   $)1

AND transfor(x) BE
     $( LET a, b = dvece, dvecs
        LET l, m = nextparam(), nextparam()
        LET bl, ll = breaklabel, looplabel
        LET k, n, i = 0, 0, 0
        LET step = 1
        LET s = ssp
        breaklabel, looplabel := m, 0

        trnline := 6!x

        TEST scanexpr(h4!x) DO
            k, n := s.ln, evalconst(h4!x)
        OR
        $(  k, n := s.lp, ssp
            ld(h4!x)
            out1(s.store)
        $)

        i := ssp
        addname(h2!x, s.local, i, s.for)
        dvece := dvecs
        load(h3!x)

        UNLESS h5!x=0 DO step := evalconst(h5!x)

        out2p(s.res, l)
        complab(l)
        out2(s.rstack, i)
        out1(s.store)
        ssp := i + 1
        out2(s.lp, i); out2(k, n); out1(step<0 -> s.ge, s.le)
        out2p(s.jf, m)

        decllabels(h6!x)
        trans(h6!x)
        UNLESS looplabel=0 DO complab(looplabel)

        out2(s.lp, i); out2(s.ln, step); out1(s.plus)
        out2p(s.res, l)

        breaklabel, looplabel, ssp := bl, ll, s
        out2(s.stack, ssp)
        complab(m)
        dropnames(b)
        dvece := a  $)


AND load(x) BE
$(  scanexpr(x)
    ld(x)
$)


  // SCANEXPR scans the tree of an expression in order to detect
  // portions which can be evaluated at compile time. Such portions
  // are marked (by oring a bit into the operator field). Subsequently,
  // LOAD will exploit these bits.

AND scanexpr(x) = VALOF
$(      // The boolean result of SCANEXPR is TRUE if the expression
        // can be evaluated at compile time
    LET op = ?
    LET r = TRUE

    IF x = 0 RESULTIS FALSE   // Should not occur

    op := h1!x & NOT constbit
    h1!x := op

    SWITCHON op INTO
    $(
      CASE s.true:
      CASE s.false:
      CASE s.number:
        ENDCASE

      CASE s.query:
      CASE s.string:
      CASE s.table:
      CASE s.valof:
      CASE s.fnap:
        RESULTIS FALSE

      CASE s.name:
        $(  LET t = h2!x

$<MACRO
            // see the comment in transname
            $(
               LET k, a = dvec!(t+1), dvec!(t+2)
               IF t = 0 RESULTIS FALSE
               UNLESS (k & disbit) = 0 DO
               $( t := dvec!(t+3); LOOP $)
               IF k = s.number RESULTIS TRUE
               UNLESS k = s.mpar RESULTIS FALSE
               FOR i = 0 TO mvecp DO
               $(
                  LET lwb = [i=mvecp->dvecs+4,mvec!(i+1)]
                  LET upb = mvec!i - 4
                  IF lwb <= t <= upb DO
                  $(
                     FOR j = lwb TO upb BY 4
                     DO dvec!(j+1) := dvec!(j+1) | disbit
                     r := scanexpr( a )
                     FOR j = lwb TO upb BY 4
                     DO dvec!(j+1) := dvec!(j+1) & ~disbit
                     RESULTIS r
                  $)
               $)
            $) REPEAT
$>MACRO
$<MACRO'
            IF t = 0 LOGOR dvec!(t+1) NE s.number RESULTIS FALSE
            RESULTIS TRUE
$>MACRO'

        $)

      CASE s.cond:
        r := r & scanexpr(h4!x)

      CASE s.byteap: CASE s.vecap:
$<WORD
      CASE s.wordap:
$>WORD
      CASE s.fdiv: CASE s.fminus:
      CASE s.fls: CASE s.fgr: CASE s.fle: CASE s.fge:
      CASE s.div: CASE s.rem: CASE s.minus:
      CASE s.ls: CASE s.gr: CASE s.le: CASE s.ge:
      CASE s.lshift: CASE s.rshift:
      CASE s.fmult: CASE s.fplus: CASE s.feq: CASE s.fne:
      CASE s.mult: CASE s.plus: CASE s.eq: CASE s.ne:
      CASE s.logand: CASE s.logor: CASE s.eqv: CASE s.neqv:
        r := r & scanexpr(h3!x)

//***      CASE S.FIX: CASE S.FLOAT:
      CASE s.abs: CASE s.fabs:
      CASE s.fneg: CASE s.neg: CASE s.not: CASE s.rv:
      CASE s.lv:
        r := r & scanexpr(h2!x)

        UNLESS r RESULTIS FALSE

        IF op = s.byteap | op = s.vecap |
$<WORD
           op = s.wordap |
$>WORD
           op = s.rv | op = s.lv RESULTIS FALSE

        ENDCASE

      DEFAULT: RESULTIS FALSE

    $)

    h1!x := op | constbit

    RESULTIS TRUE

$)
AND ld(x) BE
    $(1 IF x=0 DO $( transreport(148, currentbranch)
                     loadzero()
                     RETURN  $)

     $( LET op = h1!x

        TEST (op & constbit) NE 0 DO
        $(  // the expression has been marked as compile-time
            // evaluable - see SCANEXPR

            ssp := ssp + 1
            out2(s.ln, evalconst(x))
            RETURN
        $)
        OR
        SWITCHON op INTO
     $( DEFAULT: transreport(147, currentbranch)
                 loadzero()
                 RETURN

$<WORD
        CASE s.wordap:  op := s.getword; GOTO l1
$>WORD
        CASE s.byteap:  op := s.getbyte
        CASE s.fdiv:CASE s.fminus:
        CASE s.fls:CASE s.fgr:CASE s.fle:CASE s.fge:
        CASE s.div: CASE s.rem: CASE s.minus:
        CASE s.lle: CASE s.lge: CASE s.lls: CASE s.lgr:
        CASE s.ls: CASE s.gr: CASE s.le: CASE s.ge:
        CASE s.lshift: CASE s.rshift:
   l1:      ld(h2!x)
            ld(h3!x)
            out1(op)
            ssp := ssp - 1
            RETURN

        CASE s.fmult:CASE s.fplus:CASE s.feq:CASE s.fne:
        CASE s.vecap: CASE s.mult: CASE s.plus: CASE s.eq: CASE s.ne:
        CASE s.logand: CASE s.logor: CASE s.eqv: CASE s.neqv:
         $( LET a, b = h2!x, h3!x
            IF h1!a=s.name | h1!a=s.number DO
                               a, b := h3!x, h2!x
            ld(a)
            ld(b)
            IF op=s.vecap DO $( out1(s.plus); op := s.rv  $)
            out1(op)
            ssp := ssp - 1
            RETURN   $)

//****         CASE S.FIX:CASE S.FLOAT:
        CASE s.abs:CASE s.fabs:
        CASE s.fneg:CASE s.neg: CASE s.not: CASE s.rv:
        CASE s.car: CASE s.cdr:
            ld(h2!x)
            out1(op)
            RETURN

        CASE s.query: ssp := ssp + 1
                      out2(s.stack, ssp)
                      RETURN

        CASE s.true: CASE s.false: CASE s.nil:
            out1(op)
            ssp := ssp + 1
            RETURN

        CASE s.lv: loadlv(h2!x)
                   RETURN

        CASE s.number:
            out2(s.ln, h2!x)
            ssp := ssp + 1
            RETURN

        CASE s.string:
            out1(s.lstr); outstring(LV h2!x)
            ssp := ssp + 1
            RETURN

        CASE s.name:
             transname(x, s.lp, s.lg, s.ll, s.ln)
             ssp := ssp + 1
             RETURN

        CASE s.valof:
         $( LET rl = resultlabel
            LET a, b = dvecs, dvece
            decllabels(h2!x)
            resultlabel := nextparam()
            trans(h2!x)
            IF accessible DO transreport(92, x)
            complab(resultlabel)
            out2(s.rstack, ssp)
            ssp := ssp + 1
            dropnames(a)
            dvece := b
            resultlabel := rl
            RETURN   $)


        CASE s.fnap:
$<MACRO
         TEST ismacro( x ! h2 )
         THEN macrocall( x , s.fnap )
         ELSE
$>MACRO
         $( LET s = ssp
            ssp := ssp + savespacesize
            out2(s.stack, ssp)
            loadlist(h3!x)
            load(h2!x)
            out2(s.fnap, s)
            ssp := s + 1
         $)
         RETURN

        CASE s.cond:
         IF trnoptimise & scanexpr(h2!x) DO
         $(  TEST evaltv(h2!x) DO ld(h3!x) OR ld(h4!x)
             RETURN
         $)
         $( LET l, m = nextparam(), nextparam()
            jumpcond(h2!x, FALSE, m)
            ld(h3!x)
            out2p(s.res, l)
            ssp := ssp - 1
            complab(m)
            ld(h4!x)
            out2p(s.res, l)
            ssp := ssp - 1
            complab(l)
            out2(s.rstack, ssp)
            ssp := ssp + 1
            RETURN   $)

        CASE s.table:
         $( LET m = nextparam()
            compdatalab(m)
            x := h2!x
            WHILE h1!x=s.comma | h1!x=s.commawithline DO
                  $( IF h1!x=s.commawithline trnline := h4!x
                     out2(s.itemn, evalconst(h2!x))
                     x := h3!x   $)
            out2(s.itemn, evalconst(x))
            out2p(s.lll, m)
            ssp := ssp + 1
            RETURN  $)                         $)1


AND loadlv(x) BE
$(1 LET a2, a3 = 0, 0
    IF x=0 THEN GOTO err
    a2, a3 := h2!x, h3!x

    SWITCHON h1!x INTO
     $( DEFAULT:
        err:     transreport(113, currentbranch)
                 loadzero()
                 RETURN

        CASE s.name:
              transname(x, s.llp, s.llg, s.lll, 0)
              ssp := ssp + 1
              RETURN

        CASE s.rv:
            load(a2)
            RETURN

        CASE s.vecap:
            IF h1!a2=s.name THEN
                $( LET t = a2
                    a2 := a3; a3 := t
                $)
            load(a2)
            load(a3)
            out1(s.plus)
            ssp := ssp - 1
            RETURN
$)1


AND loadzero() BE $( out2(s.ln, 0)
                     ssp := ssp + 1  $)

AND loadlist(x) BE UNLESS x=0 DO
$(  LET op = h1!x
    UNLESS op=s.comma | op=s.commawithline DO $( load(x); RETURN  $)

    IF op=s.commawithline DO trnline := h4!x
    load(h2!x)
    x := h3!x
$) REPEAT

AND evalconst(x) = VALOF

$(  IF x=0 THEN RESULTIS 0

    $(  LET h1x, h2x, h3x = h1!x & NOT constbit, h2!x, h3!x

          h1!x := h1x
//        WRITEF("EVALCONST: H1X,H2X are %N,%N*N",H1X, H2X)  //****************
//        WRITEOP(H1X)
        SWITCHON h1x INTO

        $(  CASE s.number: RESULTIS h2x
            CASE s.true:   RESULTIS TRUE
            CASE s.false:  RESULTIS FALSE
            CASE s.query:  RESULTIS 0

            CASE s.neg: RESULTIS -evalconst(h2x)
            CASE s.not: RESULTIS NOT evalconst(h2x)
            CASE s.abs: RESULTIS ABS evalconst(h2x)
            CASE s.fneg:RESULTIS #- evalconst(h2x)
//****            CASE s.fabs:RESULTIS #ABS evalconst(h2x)
//****            CASE S.FIX: RESULTIS FIX EVALCONST(H2X)
//****            CASE S.FLOAT: RESULTIS FLOAT EVALCONST(H2X)

            CASE s.name:
            $(  LET t = h2!x
$<MACRO       $(             $>MACRO
                IF t = 0 DO
                $(  transreport(121, x)
                    RESULTIS 0
                $)
$<MACRO
                UNLESS (dvec!(t+1) & disbit) = 0 DO
                $( t := dvec!(t+3); LOOP $)
$>MACRO
                IF dvec!(t+1)=s.number THEN RESULTIS dvec!(t+2)
$<MACRO
                // see the comment in transname
                IF dvec!(t+1)=s.mpar THEN
                $(
                   LET k, a, r = dvec!(t+1), dvec!(t+2), ?
                   FOR i = 0 TO mvecp DO
                   $(
                      LET lwb = [i=mvecp->dvecs+4,mvec!(i+1)]
                      LET upb = mvec!i - 4
                      IF lwb <= t <= upb DO
                      $(
                         FOR j = lwb TO upb BY 4
                         DO dvec!(j+1) := dvec!(j+1) | disbit
                         r := evalconst( a )
                         FOR j = lwb TO upb BY 4
                         DO dvec!(j+1) := dvec!(j+1) & ~disbit
                         RESULTIS r
                      $)
                   $)
                $)
$>MACRO
                transreport(119, x)
                RESULTIS 0
$<MACRO
              $) REPEAT
$>MACRO
            $)

            DEFAULT: transreport(118, x); RESULTIS 0

            CASE s.cond:
                RESULTIS evaltv(h2x) -> evalconst(h3x), evalconst(h4!x)
            CASE s.mult: CASE s.div: CASE s.rem: CASE s.plus: CASE s.minus:
            CASE s.logand: CASE s.logor: CASE s.eqv: CASE s.neqv:
            CASE s.lshift: CASE s.rshift:
            CASE s.fmult: CASE s.fdiv: CASE s.fplus: CASE s.fminus:
            CASE s.gr: CASE s.ls: CASE s.ge: CASE s.le: CASE s.eq: CASE s.ne:
            CASE s.fgr:CASE s.fls:CASE s.fge:CASE s.fle:CASE s.feq:CASE s.fne:
        $)
        $(  LET e2 = evalconst(h2x)
            AND e3 = evalconst(h3x)

//            Writef("EVALCONST: %X8 %X8 %X8*N",H1X, E2,E3)
//            WRITEOP(H1X)
            SWITCHON h1x INTO

            $(  CASE s.mult:   RESULTIS e2*e3
                CASE s.div: CASE s.rem:
                    IF e3=0 DO
                    $( transreport(120, x); RESULTIS 0 $)
                    RESULTIS h1x=s.div -> e2/e3, e2 REM e3
                CASE s.plus:   RESULTIS e2+e3
                CASE s.minus:  RESULTIS e2-e3

                CASE s.logand: RESULTIS e2&e3
                CASE s.logor:  RESULTIS e2|e3
                CASE s.eqv:    RESULTIS e2 EQV e3
                CASE s.neqv:   RESULTIS e2 NEQV e3
                CASE s.lshift: RESULTIS e2 << e3
                CASE s.rshift: RESULTIS e2 >> e3

                CASE s.fmult:  RESULTIS e2 #* e3
                CASE s.fdiv:
                    IF e3=0 DO $( transreport(120, x); RESULTIS 0 $)
                               RESULTIS e2 #/ e3
                CASE s.fplus:  RESULTIS e2 #+ e3
                CASE s.fminus: RESULTIS e2 #- e3
                CASE s.gr:     RESULTIS e2 > e3
                CASE s.ls:     RESULTIS e2 < e3
                CASE s.ge:     RESULTIS e2 >= e3
                CASE s.le:     RESULTIS e2 <= e3
                CASE s.eq:     RESULTIS e2 = e3
                CASE s.ne:     RESULTIS e2 NE e3

                CASE s.fgr:    RESULTIS e2 #> e3
                CASE s.fls:    RESULTIS e2 #< e3
                CASE s.fge:    RESULTIS e2 #>= e3
                CASE s.fle:    RESULTIS e2 #<= e3
                CASE s.feq:    RESULTIS e2 #= e3
                CASE s.fne:    RESULTIS e2 #~= e3
                DEFAULT:       RESULTIS 0
            $)
        $)
    $)
$)

AND assign(x, y, n, ll, lr) BE
$(
    $(1 IF x=0 | y=0 DO
            $( transreport(110, currentbranch)
               BREAK  $)
       trnline := lr

        SWITCHON h1!x INTO
     $( CASE s.comma: CASE s.commawithline:
            UNLESS h1!y=s.comma | h1!y=s.commawithline DO
                       $( transreport(112, currentbranch)
                          BREAK   $)
            IF h1!x=s.commawithline DO ll := h4!x
            IF h1!y=s.commawithline DO lr := h4!y
            assign(h2!x, h2!y, n, ll, lr)
            x, y := h3!x, h3!y
            LOOP

        CASE s.name:
            IF n=s.stind THEN
            $(  load(y)
                trnline := ll
                transname(x, s.sp, s.sg, s.sl, 0)
                ssp := ssp-1
                BREAK
            $)
            trnline := ll; load(x)
            trnline := lr; load(y)
            out1(n)
            trnline := ll; transname(x, s.sp, s.sg, s.sl, 0)
            ssp := ssp - 2
            BREAK

        CASE s.rv: CASE s.vecap:
            load(y)
            trnline := ll; loadlv(x)
            IF n NE s.stind THEN out1(s.mod)
            out1(n)
            ssp := ssp - 2
            BREAK

        CASE s.byteap:
           TEST n NE s.stind THEN
           $(
               trnline := ll; load(x)
               trnline := lr; load(y)
               out1(n)
               ssp := ssp - 1
           $)
           ELSE load(y)
           trnline := ll; load(h2!x)
           load(h3!x)
           out1(s.putbyte)
           ssp:=ssp-3
           BREAK
$<WORD
        CASE s.wordap:
           TEST n NE s.stind THEN
           $( trnline := ll; load(x)
              trnline := lr; load(y)
              out1(n)
              ssp := ssp - 1
           $)
           ELSE load(y)
           trnline := ll; load(h2!x)
           load(h3!x)
           out1(s.putword)
           ssp:=ssp-3
           BREAK
$>WORD
       CASE s.nil:
           load(y)
           out1(s.stnil)
           ssp := ssp - 1
           BREAK

       CASE s.car: CASE s.cdr:
           load(y)
           load(h2!x)
           IF h1!x=s.cdr DO $(
              out2(s.ln,4); out1(s.plus) $)
           out1(s.stindb)
           ssp := ssp - 2
           BREAK

$<MACRO
       CASE s.fnap:
         IF ismacro( x ! h2 )
         $(
            LET dve = dvecentry( x ! h2 )
            LET mdef = dve ! h3
            LET args = mdef ! h3                  // argument list
            LET pars = x ! h3                     // repacement expressions
            LET b = dvecs
         
            UNLESS mdef!h1 = s.fndef DO $( transreport(137, x!h2); RETURN $)

            IF mvecp >= macmax
            DO $( transreport(134); RETURN $)       // too many macro calls
         
            mvecp := mvecp + 1                      // step on to next slot
            mvec!mvecp := dvecs+4                   // set lwb of prev macros args/locals
         
            // first define all the arguments
            UNLESS args = 0 DO
            $(
               LET op = args!h1
               LET pop = pars!h1
               IF op = s.name DO
               $( IF pop = s.comma | pop = s.commawithline DO transreport(133)
                  addname( args, s.mpar, pars, 0)
                  BREAK
               $)
               IF op=s.comma | op=s.commawithline DO
               $( IF op=s.commawithline DO trnline := h4!pars
                  UNLESS pop=s.comma | pop=s.commawithline DO transreport(133)
                  addname( args!h2 , s.mpar, pars!h2, 0)
                  args := h3!args
                  pars := h3!pars
                  LOOP
               $)
               transreport(133)
            $) REPEAT
         
            // that done, compile the macro code

            assign( mdef ! h4, y, n, ll, lr)
         
            dropnames(b)
            mvecp := mvecp - 1  // dispose of bounds
            BREAK
         $)
$>MACRO

        DEFAULT: transreport(109, currentbranch)
                 BREAK
    $)1 REPEAT

    trnline := lr
$)

/*
AND dbg(s,a,b,c,d) BE
$( LET o = output()
   selectoutput(sysout)
   writef(s,a,b,c,d)
   selectoutput(o)
$)
*/

AND transname(x, p, g, l, n) BE
    $(1 LET t = h2!x
$<MACRO
      $(rpt
$>MACRO
        LET k, a = dvec!(t+1), dvec!(t+2)

        IF t=0 DO $( transreport(115, x)
                     out2(g, 0)
                     RETURN  $)
$<MACRO
        UNLESS (k & disbit) = 0 DO // a disabled one, try again
        $( //writef("Skipping %s*N",x+2)
           t := dvec!(t+3); LOOP $)
$>MACRO

        SWITCHON k INTO
        $( CASE s.local: IF t>dvecp DO transreport(116, x)
                         out2(p, a); RETURN

           CASE s.global: out2(g, a); RETURN

           CASE s.label: out2p(l, a); RETURN

           CASE s.number: IF n=0 DO $( transreport(113, x)
                                       n := p  $)
                          out2(n, a); RETURN

$<EXT
           CASE s.external:
              $( LET e = (g=s.lg) -> s.lext,
                         (g=s.llg) -> s.llext,
                         (g=s.sg) -> s.sext, 999
                 LET s = (a=0) -> x+2,a+1
                 out1(e); outstring(s); RETURN
              $)
$>EXT

$<MACRO
           // The following code works as follows... Whenever a macro is
           // called a new entry is added to Mvec recording the value of
           // Dvecs+4 ( remember: Dvecs grows downward and is always 4 ahead ).
           // When a macro argument is encountered here Mvec is searched for
           // the pair of adjacent value between which its Dvec entry lies - in
           // the case of the current macro one of these values will be Dvecs+4.
           // Once these values have been found all Dvec entries between them
           // are disabled by setting the Disbit in the second (type) word of
           // the entry. The argument expression is now evaluated, and because
           // the macro argument and local definitions have been disabled it
           // will be evaluated 'outside' the macro.
           // Once this has been done the arguments are re-enabled.
           // The practice of disabling Dvec entries - while of surpassing
           // grubbiness - is necessary: consider what happens when a macro
           // is passed an argument which contains another macro call.
           CASE s.mpar:
                     FOR i = 0 TO mvecp DO
                     $(
                        LET lwb = [i=mvecp->dvecs+4,mvec!(i+1)]
                        LET upb = mvec!i - 4
                        //writef("lwb %n upb %n t %n*N",lwb,upb, t)
                        IF lwb <= t <= upb DO
                        $(
                           //writef("Disabling ")
                           FOR j = lwb TO upb BY 4
                           DO $( dvec!(j+1) := dvec!(j+1) | disbit
                                 //writef("%S ",(dvec!j)+2)
                           $)
                           //writef("*N")
                           IF p = s.lp THEN $( load( a ); ssp := ssp - 1 $)
                           IF p = s.llp THEN $( loadlv( a ) ; ssp := ssp - 1 $)
                           IF p = s.sp THEN $(
                              TEST a!h1 = s.name
                              THEN transname(a, p, g, l, n)
                              ELSE $(
                                 loadlv( a ) ; out1(s.stind) ; ssp := ssp - 1 $)
                           $)
                           FOR j = lwb TO upb BY 4
                           DO dvec!(j+1) := dvec!(j+1) & ~disbit
                           RETURN
                        $)
                     $)
                     transreport(135,x)
           CASE s.macro:
           DEFAULT:
                     selectoutput(sysout)
                     writes("Internal error in *'transname*'*N")
                     tidyup(20)
$>MACRO
        $)
$<MACRO
      $)rpt REPEAT
$>MACRO
    $)1

AND complab(l) BE
    UNLESS l=0 DO
    $(  out2p(s.lab, l)
        accessible := TRUE
    $)

AND compblab(l) BE
    UNLESS l=0 DO
    $(  out2p(s.blab, l)
        accessible := TRUE
    $)

AND compentry(n, l) BE
$(  LET s = n+2
    out3p(s.entry, s%0, l)
    accessible := TRUE
    FOR i = 1 TO s%0 DO outc(s%i)
    wrc('*S')
$)

AND compdatalab(l) BE out2p(s.datalab, l)

AND compjump(l) BE
$(  out2p(s.jump, l)
    accessible := FALSE
$)

AND out1(x) BE writeop(x)

AND out2(x, y) BE
    $( writeop(x)
       wrn(y) $)

AND out2p(x, y) BE
    $(
       writeop(x); wrc('L')
       wrn(y) $)

AND out3p(x, y, z) BE
    $(
       writeop(x)
       wrn(y); wrc('L')
       wrn(z) $)


AND outn(n) BE wrn(n)

AND outl(x) BE
    $( wrc('*S'); wrc('L'); wrn(x) $)

AND outc(x) BE wrn(charcode(x))

AND outstring(s) BE
$(  outn(s%0)
    FOR i = 1 TO s%0 DO outc(s%i)
    wrc('*S')
$)

AND writeop(x) BE
$(
    $(1 LET s= VALOF $( SWITCHON x&255 INTO
        $( DEFAULT: transreport(199, currentbranch)
                    RESULTIS "ERROR"

//****           CASE S.FIX:     RESULTIS "FIX"
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
           CASE s.lshift:  RESULTIS "LSHIFT"
           CASE s.rshift:  RESULTIS "RSHIFT"
           CASE s.logand:  RESULTIS "LOGAND"
           CASE s.logor:   RESULTIS "LOGOR"
           CASE s.eqv:     RESULTIS "EQV"
           CASE s.neqv:    RESULTIS "NEQV"

           CASE s.abs:     RESULTIS "ABS"
           CASE s.neg:     RESULTIS "NEG"
           CASE s.not:     RESULTIS "NOT"
           CASE s.rv:      RESULTIS "RV"

           CASE s.getbyte: RESULTIS "GETBYTE"
           CASE s.putbyte: RESULTIS "PUTBYTE"
$<WORD
           CASE s.putword: RESULTIS "PUTWORD"
           CASE s.getword: RESULTIS "GETWORD"
$>WORD
           CASE s.mod:     RESULTIS "MOD"

           CASE s.true:    RESULTIS "TRUE"
           CASE s.false:   RESULTIS "FALSE"

           CASE s.lp:      RESULTIS "LP"
           CASE s.lg:      RESULTIS "LG"
           CASE s.ln:      RESULTIS "LN"
           CASE s.lstr:    RESULTIS "LSTR"
           CASE s.ll:      RESULTIS "LL"

           CASE s.llp:     RESULTIS "LLP"
           CASE s.llg:     RESULTIS "LLG"
           CASE s.lll:     RESULTIS "LLL"

           CASE s.sp:      RESULTIS "SP"
           CASE s.sg:      RESULTIS "SG"
           CASE s.sl:      RESULTIS "SL"
           CASE s.stind:   RESULTIS "STIND"

           CASE s.jump:    RESULTIS "JUMP"
           CASE s.jt:      RESULTIS "JT"
           CASE s.jf:      RESULTIS "JF"
           CASE s.goto:    RESULTIS "GOTO"
           CASE s.lab:     RESULTIS "LAB"
           CASE s.blab:    RESULTIS "BLAB"
           CASE s.stack:   RESULTIS "STACK"
           CASE s.store:   RESULTIS "STORE"

           CASE s.entry:   RESULTIS "ENTRY"
           CASE s.save:    RESULTIS "SAVE"
           CASE s.fnap:    RESULTIS "FNAP"
           CASE s.fnrn:    RESULTIS "FNRN"
           CASE s.rtap:    RESULTIS "RTAP"
           CASE s.rtrn:    RESULTIS "RTRN"
           CASE s.startblock: RESULTIS "STARTBLOCK"
           CASE s.endblock: RESULTIS "ENDBLOCK"
           CASE s.endproc: RESULTIS "ENDPROC"
           CASE s.res:     RESULTIS "RES"
           CASE s.rstack:  RESULTIS "RSTACK"
           CASE s.finish:  RESULTIS "FINISH"
           CASE s.section:  RESULTIS "SECTION"
           CASE s.needs:    RESULTIS "NEEDS"
           CASE s.switchon:RESULTIS "SWITCHON"
           CASE s.global:  RESULTIS "GLOBAL"
$<EXT
           CASE s.external: RESULTIS "EXTERNAL"
$>EXT
           CASE s.datalab: RESULTIS "DATALAB"
           CASE s.iteml:   RESULTIS "ITEML"
           CASE s.itemn:   RESULTIS "ITEMN"   $) $)

        UNLESS (x&flbit)=0 DO wrc('#')
        FOR i = 1 TO s%0 DO wrc(s%i)
        wrc('*S')
        putbytes(x)
$)1
$)


AND wrn(n) BE $(
    LET t=VEC 10
    AND i,k = 0, -n
    putbytes(n)
    IF n<0 THEN k:=n
    t!i, k, i:=-(k REM 10), k/10, i+1 REPEATUNTIL k=0
    IF n<0 THEN wrc('-')
    FOR j=i-1 TO 0 BY -1 DO wrc(t!j+'0')
    wrc('*S')
$)

AND endocode() BE $( wrch('*N'); ocount := 0  $)


AND wrc(ch) BE $( ocount := ocount + 1
                  IF ocount>62 & ch='*S' DO
                            $( wrch('*N'); ocount := 0
                               RETURN  $)
                  wrch(ch)  $)

AND putbytes(n) BE

$(  LET k = n>>7
    TEST k=0 THEN wrbyte(n)
    OR $( wrbyte((n&127)+128)
          putbytes(k)
       $)
$)

AND wrbyte(n) BE $( putbyte(obufp, obufb, n)
                    obufb := obufb+1
                    IF obufb=bytesperword THEN
                         obufp, obufb := obufp+1, 0
                    checkworkspace()
                 $)

AND checkworkspace() BE
$(  LET slack = dvec+dvecs-obufp
    IF slack<trnunused DO
    $(  trnunused := slack
        IF trnunused <= 0 DO
        $(  transreport(145, currentbranch)
            longjump(err.p,err.l)
        $)
    $)
$)
