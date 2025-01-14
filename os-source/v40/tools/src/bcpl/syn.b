SECTION "SYN"
GET "LIBHDR"
GET "BCPHDR"
GET "SYNHDR"


LET unlist(p, n) BE
    $( IF p=treep DO treep := treep + n $)

AND list1(x) = VALOF
    $( LET p = newvec(0)
       p!0 := x
       RESULTIS p  $)

AND list2(x, y) = VALOF
     $( LET p = newvec(1)
        p!0, p!1 := x, y
        RESULTIS p   $)

AND list3(x, y, z) = VALOF
     $( LET p = newvec(2)
        p!0, p!1, p!2 := x, y, z
        RESULTIS p     $)

AND list4(x, y, z, t) = VALOF
     $( LET p = newvec(3)
        p!0, p!1, p!2, p!3 := x, y, z, t
        RESULTIS p   $)

AND list5(x, y, z, t, u) = VALOF
     $( LET p = newvec(4)
        p!0, p!1, p!2, p!3, p!4 := x, y, z, t, u
        RESULTIS p   $)

AND list6(x, y, z, t, u, v) = VALOF
     $( LET p = newvec(5)
        p!0, p!1, p!2, p!3, p!4, p!5 := x, y, z, t, u, v
        RESULTIS p  $)

AND list7(x, y, z, t, u, v, w) = VALOF
     $( LET p = newvec(6)
        p!0, p!1, p!2, p!3, p!4, p!5, p!6 := x, y, z, t, u, v, w
        RESULTIS p  $)

AND isccorend() = ch='*N' -> TRUE,
               ch='*P' -> TRUE,
               ch=endstreamch -> TRUE,
                          FALSE


AND caerrwrite(n, a, b, c, d) BE
           $( IF prsource & NOT isccorend() DO newline()
              TEST n>200 DO writes("Warning: ") OR writes("Syntax error ")
              writef("near line %N", linecount REM 100000)
              TEST getp  NE  0 DO
                  writef(" of GET file %S:  ", getp+5)
              OR
                  writes(":  ")
              caemessage(n, a, b, c, d)
              IF n>200 & warncount=warnmax DO
                  writes("*NFurther warnings suppressed")
              TEST prsource & linecount < 10000 DO
                  TEST isccorend() DO newline() OR writes("*N        ... ")
              OR TEST n>200 DO newline() OR wrchbuf()
                                 // no context for warnings

              IF reportcount>=reportmax DO
                      writes("*NToo many errors - compilation abandoned*N")  $)

AND caereport(n, a, b, c, d) BE
     $(
$<TOS
        LET abandon = ?
$>TOS
        IF ABS n = 20 DO
        $(
            writef("BREAK detected in SYN*N")
$<TOS
            userexit("Hit <RETURN> to continue")
$>TOS
            rc := 20
            longjump(err.p, err.l)
        $)
        TEST n>200 DO warncount := warncount+1
                   OR reportcount := reportcount+1
        IF n<0 THEN $( n := -n; reportmax := 0 $)

        IF n>200 & warncount>warnmax RETURN

        IF quiet & NOT prsource & reportcount+warncount=1 &
               getbyte(sectionname, 0)  NE  0 DO
            writef("Section *"%S*"*N", sectionname)

        caerrwrite(n, a, b, c, d)
$<TOS
        abandon := userexit("Hit <RETURN> to Continue, A to abort")
$>TOS
        IF n>100 RETURN
                  // N>100 if error recovery skipping not required
                  // this includes Warnings (N>200)

$<TOS
        IF abandon | (reportcount>=reportmax)
$>TOS
$<TOS'
        IF reportcount>=reportmax
$>TOS'
        DO $(        // if max errors, set rc and quit
            rc := 20
            longjump(err.p, err.l)
        $)

        nlpending := FALSE

        UNTIL symb=s.lsect | symb=s.rsect | symb=s.lcode | symb=s.rcode |
              symb=s.let | symb=s.and |
              symb=s.global | symb=s.static | symb=s.manifest |
$<EXT
              symb=s.external |
$>EXT
              symb=s.end | nlpending  | symb=s.semicolon
                    DO nextsymb()
        ignore(s.semicolon)
        longjump(rec.p, rec.l)   $)

AND formtree() =  VALOF
     $(1
        // allocate all work vectors at bottom of workspace
        chbuf := workbase 
        wordv := chbuf + 64
        charv := wordv + 101
        nametable := charv + 257
        tagstack := nametable + 102
        workbase := tagstack + 11  // set workbase for checking purposes

        chcount := 0
        FOR i = 0 TO 63 DO chbuf!i := 0
        skipping := FALSE

        ngets, getp, getchain := 0, 0, 0

        charp := 0

        nametablesize := 101
        FOR i = 0 TO 101 DO nametable!i := 0

        tagp := 0; tagstack!tagp := 0
                           // stack for checking conditional compilation
                           // bracket matches
        rec.p, rec.l := level(), l1
        err.p, err.l := level(), err.lab

        rch()

        declsyswords()
     l: nextsymb()

     l2:

    $(  LET rsprog(n) = VALOF
        $(  LET a = 0
            nextsymb(); a := rbexp()
            UNLESS h1!a=s.string DO caereport(89)
//            uppercasestring(a+1)
            IF n=s.section DO
            $(  sectionname!0, sectionname!1, sectionname!2 :=
                           a!1, a!2, a!3
                IF getbyte(sectionname, 0) > 8 DO
                    putbyte(sectionname, 0, 8)
                UNLESS prsource | quiet DO
                    writef("Section *"%S*"*N", sectionname)
            $)
            RESULTIS list3(n, a, rprog())
        $)

        AND rprog() =
                symb=s.needs -> rsprog(s.needs),
                                rdblockbody()

        LET a = symb=s.section -> rsprog(s.section),
                                rprog()
        UNLESS symb=s.end DO caereport(99)

        // finally, clear the cells in name nodes which hold
        // the hash chains. These cells are reused in TRN
        // to assist the lookup of declarations.

        FOR i = 0 TO nametablesize-1 DO
        $(  LET x = nametable+i
            LET y = !x
            UNTIL y = 0 DO $( !x := 0; x := y+h2; y := !x $)
        $)
        RESULTIS a
    $)
    l1: IF symb=s.rsect THEN nextsymb()
        IF symb=s.and THEN symb := s.let
        GOTO l2

    err.lab:
        RESULTIS 0  // simply returns with nil tree
$)1



AND caemessage(n, a, b, c, d) BE
    $( LET wrtag(tag) BE
       $(  LET t = ABS tag + 2
           FOR i = 2 TO getbyte(t, 0) DO wrch(getbyte(t, i))
           IF tag < 0 DO wrch('*'')
       $)

       LET s = VALOF

         SWITCHON n INTO

         $( DEFAULT:  writen(n); RETURN

            CASE 88: writef("Input %S not provided for GET", wordv)
                     RETURN
            CASE 94: writef("Illegal character (hex %X2)", ch)
                     ch := '*S'
                     RETURN
$<TRIPOS
            CASE 20:RESULTIS "BREAK detected in SYN"
$>TRIPOS
$<MTRIPOS
            CASE 20:RESULTIS "BREAK detected in SYN"
$>MTRIPOS
$<AMIDOS
            CASE 20:RESULTIS "BREAK detected in SYN"
$>AMIDOS
$<TOS
            CASE 20:RESULTIS "BREAK detected in SYN"
$>TOS
            CASE 89: RESULTIS "String expected"
            CASE 91: RESULTIS "*"(*", *")*", *"<*", *">*" or *"$*" expected"
            CASE 95: RESULTIS "String too long"
            CASE 101:RESULTIS "Illegal digit (%C) after **X or **O"
            CASE 201:RESULTIS "Character constant too long, string assumed"
            CASE 203:RESULTIS "Unknown escape combination '**%C': '**' ignored"
            CASE 98: RESULTIS "Workspace too small, increase region"
            CASE 99: RESULTIS "Incorrect termination"

            CASE 8:CASE 40:CASE 43:
                     RESULTIS "Name expected"
            CASE 6: RESULTIS "*"$(*" expected"
            CASE 7: writes("*"$)"); wrtag(a)
                    writes("*" expected")
                    RETURN
            CASE 9: writes("*"$)*" found where *"$)"); wrtag(a);
                    writes("*" expected")
                    RETURN
            CASE 202:writes("*"$)"); wrtag(a)
                     writes("*" missing before *"$)"); wrtag(b)
                     writes("*""); RETURN
            CASE 32: RESULTIS "Error in expression"
            CASE 33: RESULTIS "Error in number"
            CASE 34: RESULTIS "Incorrect use of newline in a string"
            CASE 15:CASE 19:CASE 41: RESULTIS "*")*" missing"
            CASE 35: RESULTIS "Illegal floating-point operator"
            CASE 30: RESULTIS "*",*" missing"
            CASE 45: RESULTIS "*":*" or *"=*" expected"
            CASE 42: RESULTIS "*"=*" or *"BE*" expected"
            CASE 44: RESULTIS "*"=*" or *"(*" expected"
            CASE 50: RESULTIS "Error in label"
            CASE 51: RESULTIS "Error in command"
            CASE 54: RESULTIS "*"OR*" expected"
            CASE 57: RESULTIS "*"=*" expected"
            CASE 58: RESULTIS "*"TO*" expected"
            CASE 60: RESULTIS "*"INTO*" expected"
            CASE 61:CASE 62: RESULTIS "*":*" expected"
            CASE 63: RESULTIS "*"**%C*" missing"
            CASE 71: CASE 74:
                     writes("*"$>"); wrtag(a); writes("*" missing")
                     RETURN
            CASE 72: RESULTIS "Error in conditional compilation expression"
            CASE 73: writes("Unexpected *"$>"); wrtag(a); wrch('*"')
                     RETURN
            CASE 75: writes("*"$>"); wrtag(a)
                     writes("*" found where *"$>"); wrtag(b)
                     writes("*" expected")
                     RETURN
                       $)

         writef(s, a, b, c, d)
            $)

LET rdblockbody() = VALOF
$(  LET p, l = rec.p, rec.l
    LET a, b = 0, 0
    LET ptr = @ a

    $(
        LET ln = linecount

        rec.p, rec.l := level(), recover

        ignore(s.semicolon)

        SWITCHON symb INTO
        $(
          CASE s.manifest:
          CASE s.static:
          CASE s.global:
$<EXT
          CASE s.external:
$>EXT
            $(  LET op = symb
                nextsymb()
$<MACRO
                TEST symb \= s.lsect
                THEN $(
                  b := mdef()
                  rec.l := recoverm
               recoverm:
                  WHILE symb = s.and DO
                  $( LET ln2 = linecount
                     nextsymb()
                     b := list4(s.and, b, mdef(), ln2)
                  $)
                  !ptr := list4(s.macro, b, 0, ln )
                  ptr := @(h3!(!ptr))
                  LOOP
                $)
                ELSE
$>MACRO
                $(
                   !ptr := list4(op, rdsect(rdcdefs,op), 0, ln)
                   ptr := @(h3!(!ptr))
                $)
                LOOP
            $)


          CASE s.let:
            nextsymb()
            b := rdef()
          recover:
            WHILE symb=s.and DO
            $(  LET ln2 = linecount
                nextsymb()
                b := list4(s.and, b, rdef(), ln2)
            $)
            !ptr := list4(s.let, b, 0, ln)
            ptr := @(h3!(!ptr))
            LOOP

          DEFAULT:
            !ptr := rdseq()

            UNLESS symb=s.rsect \/ symb=s.end DO
                caereport(51)

            BREAK
        $)

    $) REPEAT

    rec.p, rec.l := p, l
    RESULTIS a
$)

AND rdseq() = VALOF
$(  LET a, b = 0, 0
    LET ptr = @ a

    $(  ignore(s.semicolon)

        IF symb=s.rsect | symb=s.end DO $( !ptr := 0; RESULTIS a $)

        b := rcom()

        IF symb=s.rsect | symb=s.end DO $( !ptr := b; RESULTIS a $)

        !ptr := list3(s.seq, b, 0)
        ptr := @(h3!(!ptr))
    $) REPEAT
$)


AND rdcdefs(arg) = VALOF
    $(1 LET a, b = 0, 0
        LET ptr = @ a
        LET p, l = rec.p, rec.l
        LET ln = linecount
        rec.p, rec.l := level(), recover

        $( b := rname()
           ln := linecount
$<EXT'
           TEST symb=s.eq | symb=s.colon THEN
             nextsymb()
           ELSE caereport(45)
           !ptr := list4(0, b, rexp(0), ln)
$>EXT'
$<EXT
           TEST arg=s.external THEN
           $( TEST symb=s.eq | symb=s.colon THEN
              $( nextsymb()
                 !ptr := list4(0, b, rexp(0), ln)
              $)
              ELSE   
                 !ptr := list4(0, b, 0, ln)
           $)
           ELSE
           $( TEST symb=s.eq | symb=s.colon THEN
                nextsymb()
              ELSE caereport(45)
              !ptr := list4(0, b, rexp(0), ln)
           $)
$>EXT
           ptr := @ h1!(!ptr)
  recover:
           ignore(s.semicolon)
        $) REPEATWHILE symb=s.name

        rec.p, rec.l := p, l
        RESULTIS a  $)1

AND rdsect(r,arg) = VALOF
    $(  LET tag, a = wordnode, 0
        LET p, l = rec.p, rec.l
        rec.p, rec.l := level(), recover
        checkfor(s.lsect, 6)
l99:    a := r(arg)
        rec.p, rec.l := p, l
        UNLESS symb=s.rsect DO caereport(7, tag)
        TEST tag=wordnode
             THEN nextsymb()
               OR TEST wordnode=nulltag DO
               $(  symb := 0
                   caereport(9, tag)  $)
               OR UNLESS tag=nulltag DO caereport(202, tag, wordnode)
        RESULTIS a

recover:  IF r = rdseq DO r := rdblockbody
          GOTO l99

                     $)


AND rdcode() = VALOF $(
        LET tag, a = wordnode, 0
        LET p, l = rec.p, rec.l
        rec.p, rec.l := level(), l99
        checkfor(s.lcode, 6)
l99:    a := rlist(rexp,l)
        rec.p, rec.l := p, l
        UNLESS symb=s.rcode DO caereport(7, tag)
        TEST tag=wordnode
             THEN nextsymb()
               OR TEST wordnode=nulltag DO
               $(  symb := 0
                   caereport(9, tag)  $)
               OR UNLESS tag=nulltag DO caereport(202, tag, wordnode)
        RESULTIS list2(s.code,a)
$)

AND rlist(f, l) = VALOF
$(  LET a = 0
    LET ptr = @ a

    $(  LET b = f(0)

        UNLESS symb =s.comma DO $( !ptr := b; RESULTIS a $)

        TEST l = linecount DO
            !ptr := list3(s.comma, b, 0)
        OR
        $(  !ptr := list4(s.commawithline, b, 0, linecount)
            l := linecount
        $)

        nextsymb()
        ptr := @(h3!(!ptr))

    $) REPEAT
$)

AND rnamelist(l) = rlist(rname, l)


AND rname() = VALOF
    $( LET a = wordnode
       checkfor(s.name, 8)
       RESULTIS a  $)

AND ignore(item) BE IF symb=item DO nextsymb()

AND checkfor(item, n) BE
      $( UNLESS symb=item DO caereport(n)
         nextsymb()  $)

LET rbexp() = VALOF
  $(1   LET a, op = 0, symb

        SWITCHON symb INTO

    $(  DEFAULT:
            caereport(32)

        CASE s.query:
            nextsymb(); RESULTIS list1(s.query)

        CASE s.true:
        CASE s.false:
        CASE s.name:
        CASE s.nil:
            a := wordnode
            nextsymb()
            RESULTIS a

        CASE s.string:
            a := newvec(wordsize+1)
            a!0 := s.string
            FOR i = 0 TO wordsize DO a!(i+1) := wordv!i
            nextsymb()
            RESULTIS a

        CASE s.number:
            a := list2(s.number, decval)
            nextsymb()
            RESULTIS a

        CASE s.lparen:
            nextsymb()
            a := rexp(0)
            checkfor(s.rparen, 15)
            RESULTIS a

        CASE s.valof:
            nextsymb()
            RESULTIS list2(s.valof, rcom())

        CASE s.vecap: op := s.rv
        CASE s.float:CASE s.fix:CASE s.abs:CASE s.fabs:
        CASE s.car: CASE s.cdr:
        CASE s.lv:
        CASE s.rv: nextsymb(); RESULTIS list2(op, rexp(35))

        CASE s.fplus:
        CASE s.plus: nextsymb(); RESULTIS rexp(34)

        CASE s.fminus: nextsymb(); RESULTIS list2(s.fneg, rexp(34))

        CASE s.minus: nextsymb()
                      a := rexp(34)
                      TEST h1!a=s.number
                          THEN h2!a := - h2!a
                            OR a := list2(s.neg, a)
                      RESULTIS a

        CASE s.not: nextsymb(); RESULTIS list2(s.not, rexp(24))

        CASE s.table:
                $( LET lc = linecount
                   nextsymb()
                   RESULTIS list2(s.table, rexplist(lc))
                $)

     $)1


AND rexp(n) = VALOF
    $(1 LET lc = linecount
        LET a = rbexp()

        LET b, c, p, q = 0, 0, 0, 0

  l: $( LET op = symb

        IF nlpending RESULTIS a

        SWITCHON op INTO
    $(b DEFAULT: RESULTIS a

        CASE s.lparen: nextsymb()
                       b := 0
                       UNLESS symb=s.rparen DO b := rexplist(lc)
                       checkfor(s.rparen, 19)
                       a := list3(s.fnap, a, b)
                       GOTO l

        CASE s.vecap: p := 40; GOTO lassoc
$<WORD
        CASE s.wordap: p := 38; GOTO lassoc
$>WORD
        CASE s.byteap: p := 36; GOTO lassoc

        CASE s.fmult: CASE s.fdiv:
        CASE s.rem:CASE s.mult:CASE s.div: p := 35; GOTO lassoc

        CASE s.fplus: CASE s.fminus:
        CASE s.plus:CASE s.minus: p := 34; GOTO lassoc

        CASE s.feq: CASE s.fne:
        CASE s.fle: CASE s.fge:
        CASE s.fls: CASE s.fgr:
        CASE s.lle: CASE s.lge:
        CASE s.lls: CASE s.lgr:
        CASE s.eq:CASE s.ne:
        CASE s.le:CASE s.ge:
        CASE s.ls:CASE s.gr:
            IF n GE 30 RESULTIS a

            $(r nextsymb()
                b := rexp(30)
                a := list3(op, a, b)
                TEST c=0 THEN c :=  a
                           OR c := list3(s.logand, c, a)
                a, op := b, symb
            $)r REPEATWHILE ((s.eq<=op) & (op<=s.ge)) |
                            ((s.feq<=op) & (op<=s.fge)) |
                            (s.lle <= op <= s.lgr)

            a := c
            GOTO l

        CASE s.lshift:CASE s.rshift: p, q := 25, 30; GOTO diadic

        CASE s.logand: p := 23; GOTO lassoc

        CASE s.logor: p := 22; GOTO lassoc

        CASE s.eqv:CASE s.neqv: p := 21; GOTO lassoc

        CASE s.cond:
                IF n GE 13 RESULTIS a
                nextsymb()
                b := rexp(0)
                checkfor(s.comma, 30)
                a := list4(s.cond, a, b, rexp(0))
                GOTO l


        lassoc: q := p

        diadic: IF n GE p RESULTIS a
                nextsymb()
                a := list3(op, a, rexp(q))
                GOTO l                     $)b     $)1

LET rexplist(l) = rlist(rexp, l)

LET rdef() = VALOF
    $(1 LET lc = linecount
        LET n = rnamelist(lc)
        SWITCHON symb INTO
     $( CASE s.lparen:
             $( LET a = 0
                nextsymb()
                UNLESS h1!n=s.name DO caereport(40)
                IF symb=s.name DO a := rnamelist(lc)
                checkfor(s.rparen, 41)

                IF symb=s.be DO
                     $( nextsymb()
                        RESULTIS list5(s.rtdef, n, a, rcom(), 0)  $)

                IF symb=s.eq DO
                     $( nextsymb()
                        RESULTIS list5(s.fndef, n, a, rexp(0), 0)  $)

                caereport(42)  $)

        DEFAULT: caereport(44)

        CASE s.eq:
                nextsymb()
                IF symb=s.vec DO
                     $( nextsymb()
                        UNLESS h1!n=s.name DO caereport(43)
                        RESULTIS list3(s.vecdef, n, rexp(0))  $)
                RESULTIS list3(s.valdef, n, rexplist(lc))  $)1

$<MACRO
LET mdef() = VALOF
$(
   LET lc = linecount
   LET n = rnamelist(lc)
   SWITCHON symb INTO
   $(
      CASE s.lparen:
      $(
         LET a = 0
         nextsymb()
         UNLESS h1!n = s.name DO caereport(40)
         IF symb = s.name DO a := rnamelist(lc)
         checkfor(s.rparen, 41)

         IF symb = s.be DO
         $(
            nextsymb()
            RESULTIS list5(s.rtdef, n, a, rcom(), 0)
         $)
         IF symb = s.eq DO
         $(
            nextsymb()
            RESULTIS list6( s.fndef, n, a, rexp(0), 0)
         $)
         caereport(42)
      $)
      DEFAULT: caereport(44)
   $)
$)
$>MACRO

LET rbcom() = VALOF
   $(1 LET a, b, op = 0, 0, symb
       LET l = linecount

        SWITCHON symb INTO
     $( DEFAULT: RESULTIS 0

        CASE s.name:CASE s.number:CASE s.string:
        CASE s.true:CASE s.false:CASE s.lv:CASE s.rv:CASE s.vecap:
        CASE s.lparen: CASE s.car: CASE s.cdr: CASE s.nil:
                a := rexplist(l)

                IF symb=s.ass | (symb&becomesbit) NE 0 THEN
                    $(  op := symb
                        nextsymb()
                        RESULTIS list4(op, a, rexplist(l), l)  $)

                IF symb=s.colon DO
                     $( UNLESS h1!a=s.name DO caereport(50)
                        nextsymb()
                        RESULTIS list5(s.colon, a, rbcom(), 0, l)  $)

                IF h1!a=s.fnap DO
                     $( LET b = h2!a
                        LET c = h3!a
                        unlist(a, 3)
                        RESULTIS list4(s.rtap, b, c, l)
                            $)

                caereport(51)
                RESULTIS a

        CASE s.goto:CASE s.resultis:
                nextsymb()
                RESULTIS list3(op, rexp(0), l)

        CASE s.if:CASE s.unless:
        CASE s.while:CASE s.until:
                nextsymb()
                a := rexp(0)
                ignore(s.do)
                RESULTIS list4(op, a, rcom(), l)

        CASE s.test:
                nextsymb()
                a := rexp(0)
                ignore(s.do)
                b := rcom()
                checkfor(s.or, 54)
                RESULTIS list5(s.test, a, b, rcom(), l)

        CASE s.for:
            $(  LET i, j, k = 0, 0, 0
                nextsymb()
                a := rname()
                checkfor(s.eq, 57)
                i := rexp(0)
                checkfor(s.to, 58)
                j := rexp(0)
                IF symb=s.by DO $( nextsymb()
                                   k := rexp(0)  $)
                ignore(s.do)
                RESULTIS list7(s.for, a, i, j, k, rcom(), l)  $)

        CASE s.loop:
        CASE s.break:CASE s.return:CASE s.finish:CASE s.endcase:
                a := wordnode
                nextsymb()
                RESULTIS a

        CASE s.switchon:
                nextsymb()
                a := rexp(0)
                checkfor(s.into, 60)
                b := rdsect(rdseq)
                IF b=0 RESULTIS 0
                RESULTIS list4(s.switchon, a, b, l)

        CASE s.case:
                nextsymb()
                a := rexp(0)
                checkfor(s.colon, 61)
                RESULTIS list4(s.case, a, rbcom(), l)

        CASE s.default:
                nextsymb()
                checkfor(s.colon, 62)
                RESULTIS list2(s.default, rbcom())

        CASE s.lcode:
                RESULTIS rdcode()

        CASE s.lsect:
                RESULTIS rdsect(rdblockbody)   $)1


AND rcom() = VALOF
$(1 LET a = rbcom()

    IF a=0 DO caereport(51)

    WHILE symb=s.cont DO
        $( LET b = 0
           nextsymb()
           b := rbcom()
           IF b=0 DO caereport(51)
           a := list3(s.seq, a, b)  $)

        WHILE symb=s.repeat | symb=s.repeatwhile |
                    symb=s.repeatuntil DO
                  $( LET op = symb
                     LET l = linecount
                     nextsymb()
                     TEST op=s.repeat
                         THEN a := list3(op, a, l)
                           OR a := list4(op, a, rexp(0), l)   $)

        RESULTIS a  $)1

