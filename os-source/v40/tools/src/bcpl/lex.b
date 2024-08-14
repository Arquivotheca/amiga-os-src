
// (C) Copyright 1979 Tripos Research Group
//     University of Cambridge
//     Computer Laboratory
// (C) Copyright 1985 All Rights Reserved METACOMCO plc
//     26 Portland Square
//     Bristol

SECTION "LEX"
GET "LIBHDR"
GET "BCPHDR"
GET "SYNHDR"

LET value(char) = VALOF
$(  char := uppercase(char)
    RESULTIS
            ('0'<=char) LOGAND (char<='9') -> char-'0',
            ('A'<=char) LOGAND (char<='Z') -> char-'A'+10,
               500
$)

LET readcomment(term) BE
    $(  rch()
        $(  IF iscc() THEN linecount := linecount + 1
            IF ch='**' THEN
            $(  rch(); UNLESS ch=term THEN LOOP
                rch(); RETURN
            $)
            IF ch=endstreamch THEN caereport(63, term)
            rch()
        $) REPEAT
    $)

AND iscc() = ch='*N' LOGOR ch='*P' LOGOR ch=13  //*** '*C'

AND islayout() = ch='*S' LOGOR ch='*T' LOGOR iscc()

LET nextsymb() BE
$(
$<TRIPOS
    IF testflags(1) DO
    $(
        rc := 20
        caereport(-20)
    $)
$>TRIPOS
$<MTRIPOS
    IF testflags(1) DO
    $(
        rc := 20
        caereport(-20)
    $)
$>MTRIPOS
$<AMIDOS
    IF testflags(1) DO
    $(
        rc := 20
        caereport(-20)
    $)
$>AMIDOS
$<TOS
    IF testflags(1) DO
    $(
        rc := 20
        caereport(-20)
    $)
$>TOS
    nsymb()

    $(  SWITCHON symb INTO
        $(
          DEFAULT:
            RETURN

          CASE s.lcond:
            condskip()
            LOOP

          CASE s.rcond:
            UNLESS tagp>=10 DO
            $(  LET tcond = ch='*''
                LET thistag = tcond -> -wordnode, wordnode
                IF tcond DO rch()
                TEST thistag=tagstack!tagp DO
                    tagp := tagp-1
                OR
                    TEST tagp=0 DO caereport(73, thistag) OR
                         caereport(75, thistag, tagstack!tagp)
            $)
            nsymb()
            LOOP

          CASE s.condname:
            condset()
         /* LOOP */
        $)
    $) REPEAT

$)

AND condskip() BE
$(  LET tag = wordnode
    LET ttag = h1!tag=-s.name   // truth value of tag
    LET tcond = ch='*''         // truth value required for skipping

    IF tcond DO rch()           // skip prime

    UNLESS tagp >= 10 DO
    $(  LET thistag = tcond -> -tag, tag
        tagp := tagp+1
        UNLESS tagp = 10 DO tagstack!tagp := thistag
    $)
    IF ttag NEQV tcond DO
    $(  nsymb()
        RETURN
    $)

    skipping := TRUE

    $(  IF iscc() DO linecount := linecount + 1
        TEST ch = '$' DO
        $(  rch()

            IF ch = '>' DO
            $(
               // set SYMB as if tag bracket had been read by
               // NSYMB, so NEXTSYMB will be able to process it

               symb := s.rcond

               rdtag('$')
               lookupword()
               IF wordnode=tag DO    // if tags match
               $(  skipping := FALSE
                   RETURN
               $)
           $)
       $)
       OR  rch()
    $)  REPEATUNTIL ch = endstreamch

    skipping := FALSE
    tagp := tagp-1
    caereport(71, (tcond->-tag, tag))

$)

AND condset() BE
$(  LET tag = wordnode

    nsymb()
    UNLESS symb=s.ass DO caereport(72)

    // set H1!TAG to reflect truth value of expr

    h1!tag :=  evalcond(0)  -> -s.name, s.name

    IF symb=s.semicolon DO nsymb()
$)

AND evalcond(n) = VALOF
$(
    LET a = evalbcond()

    LET b, c, p = 0, 0, 0

    $(  LET op = symb
        SWITCHON op INTO
        $(
          DEFAULT:
            RESULTIS a

          CASE s.logand: p := 23; ENDCASE
          CASE s.logor:  p := 22; ENDCASE
          CASE s.eqv:
          CASE s.neqv:   p := 21; ENDCASE

          CASE s.cond:
            IF n >= 13 RESULTIS a
            b := evalcond(0)
            UNLESS symb=s.comma DO caereport(30)
            c := evalcond(0)
            a := a -> b, c
            LOOP
        $)

        IF n>=p RESULTIS a

        b := evalcond(p)

        SWITCHON op INTO
        $(
          CASE s.logand: a := a &    b; LOOP
          CASE s.logor:  a := a |    b; LOOP
          CASE s.eqv:    a := a EQV  b; LOOP
          CASE s.neqv:   a := a NEQV b /* LOOP */
        $)
    $) REPEAT
$)

AND evalbcond() = VALOF
$(
    nsymb()

    SWITCHON symb INTO
    $(
      DEFAULT:
        caereport(72)

      CASE s.true: nsymb(); RESULTIS TRUE
      CASE s.false:nsymb(); RESULTIS FALSE

      CASE s.condname:
        $(  LET r = (h1!wordnode)=-s.name
            nsymb()
            RESULTIS r
        $)

      CASE s.lparen:
        $(  LET r = evalcond(0)
            UNLESS symb=s.rparen DO caereport(15)
            RESULTIS r
        $)

      CASE s.not:
        RESULTIS NOT evalcond(24)
    $)
$)

AND nsymb() BE
$(1 nlpending := FALSE
next:
    IF pptrace THEN wrch(ch)

    SWITCHON ch INTO

    $(  CASE '*P':
        CASE '*N': linecount := linecount + 1
                   nlpending := TRUE  || ignorable characters
        CASE '*T':
        CASE '*S': rch() REPEATWHILE ch='*S'
                   GOTO next

        CASE '0':CASE '1':CASE '2':CASE '3':CASE '4':
        CASE '5':CASE '6':CASE '7':CASE '8':CASE '9':
            symb := s.number
            readnumber(10)
            IF ch='.' | ch='E' | ch='e' THEN readfloat()
            RETURN

        CASE 'a':CASE 'b':CASE 'c':CASE 'd':CASE 'e':
        CASE 'f':CASE 'g':CASE 'h':CASE 'i':CASE 'j':
        CASE 'k':CASE 'l':CASE 'm':CASE 'n':CASE 'o':
        CASE 'p':CASE 'q':CASE 'r':CASE 's':CASE 't':
        CASE 'u':CASE 'v':CASE 'w':CASE 'x':CASE 'y':
        CASE 'z':
        CASE 'A':CASE 'B':CASE 'C':CASE 'D':CASE 'E':
        CASE 'F':CASE 'G':CASE 'H':CASE 'I':CASE 'J':
        CASE 'K':CASE 'L':CASE 'M':CASE 'N':CASE 'O':
        CASE 'P':CASE 'Q':CASE 'R':CASE 'S':CASE 'T':
        CASE 'U':CASE 'V':CASE 'W':CASE 'X':CASE 'Y':
        CASE 'Z':
            rdtag(getbyte(tagchartable, ch))
            symb := lookupword()
                SWITCHON symb INTO
                $(  CASE s.get:
                        performget(); GOTO next

                    CASE s.rem:
                    CASE s.logand:CASE s.logor:CASE s.eqv:CASE s.neqv:
                        GOTO checkassx

                    DEFAULT:
                        RETURN
                $)

        CASE '$':
             rch()
             SWITCHON ch INTO
             $(
               DEFAULT:
                 caereport(91)

               CASE '(': symb := s.lsect;    ENDCASE
               CASE ')': symb := s.rsect;    ENDCASE
               CASE '<': symb := s.lcond;    ENDCASE
               CASE '>': symb := s.rcond;    ENDCASE
               CASE '[': symb := s.lcode;    ENDCASE
               CASE ']': symb := s.rcode;    ENDCASE
               CASE '$': symb := s.condname /* ENDCASE */
             $)
             rdtag('$')
             lookupword()
             RETURN


        CASE '[':
        CASE '(': symb := s.lparen; GOTO l
        CASE ']':
        CASE ')': symb := s.rparen; GOTO l
        CASE '#':
            symb := s.number
            rch()
            IF ('0'<=ch) LOGAND (ch<='7') DO $( readnumber(8); RETURN  $)
            $(  LET x = uppercase(ch)
                IF x='B' DO $( rch(); readnumber(2); RETURN  $)
                IF x='O' DO $( rch(); readnumber(8); RETURN  $)
                IF x='X' DO $( rch(); readnumber(16); RETURN  $)
            $)

            nsymb()
            SWITCHON symb&(\becomesbit) INTO
            $(  DEFAULT: caereport(35)
                CASE s.plus:CASE s.minus:CASE s.mult:
                CASE s.div:CASE s.abs:CASE s.ls:CASE s.gr:
                CASE s.ge: CASE s.le: CASE s.ne: CASE s.eq:
                    symb := symb|flbit
                    RETURN
            $)

        CASE '?': symb := s.query; GOTO l
        CASE '+': symb := s.plus; GOTO checkass
        CASE ',': symb := s.comma; GOTO l
        CASE ';': symb := s.semicolon; GOTO l
        CASE '@': symb := s.lv; GOTO l
        CASE '&': symb := s.logand; GOTO checkass
        CASE '=': symb := s.eq; GOTO l
        CASE '!': symb := s.vecap; GOTO l
$<WORD'
        CASE '%': symb := s.byteap; GOTO l
$>WORD'
$<WORD
        CASE '%': rch()
            IF ch='%' DO $( symb := s.wordap; GOTO l  $)
            symb := s.byteap; RETURN
$>WORD
        CASE '**':symb := s.mult; GOTO checkass
        CASE '~': rch(); GOTO cnot

        CASE '/': rch()
            IF ch='\' DO $( symb := s.logand; GOTO checkass $)
            IF ch='**' THEN $( readcomment('/'); GOTO next $)
            UNLESS ch='/' THEN $( symb := s.div; GOTO checkassx $)
        comment:
            rch() REPEATUNTIL iscc() | ch=endstreamch
            GOTO next

        CASE '|':
            rch()
            IF ch='|' THEN GOTO comment
            UNLESS ch='**' THEN $( symb := s.logor; GOTO checkassx $)
            readcomment('|')
            GOTO next


        CASE '\': rch()
            IF ch='/' DO $( symb := s.logor; GOTO checkass  $)
            IF ch='**' THEN GOTO comment
        cnot:
            IF ch='=' THEN $( symb := s.ne; GOTO l  $)
            symb := s.not
            RETURN

        CASE '<': rch()
            IF ch='=' DO $( symb := s.le; GOTO l  $)
            IF ch='<' DO $( symb := s.lshift; GOTO l $)
            IF ch='>' THEN $( symb := s.cont; GOTO l $)
            symb := s.ls
            RETURN

        CASE '>': rch()
                 IF ch='=' DO $( symb := s.ge; GOTO l  $)
                 IF ch='>' DO $( symb := s.rshift; GOTO l  $)
                 symb := s.gr
                 RETURN

        CASE '-': rch()
                 IF ch='>' DO $( symb := s.cond; GOTO l  $)
                 symb := s.minus
                 GOTO checkassx

        CASE ':': rch()
            IF ch='=' THEN $( symb := s.ass; GOTO l $)
            symb := s.colon
            RETURN

        CASE '*'':CASE '*"':
             $(1 LET quote = ch
                 charp := 0

              $( rch()
                 IF ch=quote | charp=255 DO
                        $( UNLESS ch=quote DO caereport(95)
                           IF  ch='*'' DO
                               TEST charp=1 DO
                                   $( symb := s.number
                                      decval := charcode(decval)
                                      GOTO l  $)
                               OR caereport(201)
                           charv!0 := charp
                           wordsize := packstring(charv, wordv)
                           symb := s.string
                           GOTO l   $)


                 IF iscc() THEN caereport(34)

                 IF ch='**' DO
                    $( rch()
                       IF islayout() THEN
                       $(
                           $(  IF iscc() DO linecount := linecount + 1
                               rch()
                           $) REPEATWHILE islayout()
                           UNLESS ch='**' DO caereport(34)
                           LOOP
                       $)
                       $( LET x = uppercase(ch)
                          LET readdigits(b, n) = VALOF
                          $(  LET r, x = 0, ?
                              FOR i = 1 TO n DO
                              $(  rch()
                                  x := value(ch)
                                  IF x >= b DO caereport(101, ch)
                                  r := r*b + x
                              $)
                              RESULTIS r
                          $)
                          SWITCHON x INTO
                          $(
                              CASE 'X': ch := readdigits(16, 2); ENDCASE
                              CASE 'O': ch := readdigits(8, 3) & 255; ENDCASE
                              CASE 'T': ch := '*T'; ENDCASE
                              CASE 'S': ch := '*S'; ENDCASE
                              CASE 'N': ch := '*N'; ENDCASE
                              CASE 'E': ch := '*E'; ENDCASE
                              CASE 'C': ch :=  13;  ENDCASE  // which is CR
                              CASE 'B': ch := 8; ENDCASE    //**** '*B'
                              CASE 'P': ch := 12; ENDCASE   //**** '*P'
                              DEFAULT: caereport(203, ch)
                              CASE '*'': CASE '*"': CASE '**':
                          $) $)
                    $)

                 decval, charp := ch, charp+1
                 charv!charp := ch  $) REPEAT  $)1



       DEFAULT:    symb := 0
                   caereport(94)

       CASE endstreamch:
       CASE '.':
                    $( IF getp=0 DO
                            $(  UNLESS tagp=0 | tagp>=10 DO
                                $(  tagp := tagp - 1
                                    caereport(74, tagstack!(tagp+1))
                                $)
                                symb := s.end
                                TEST ch = endstreamch DO
                                $(  endread()
                                    sourcestream := 0
                                $)
                                OR
                                $(  UNTIL ch='*N' DO ch:=rdch()
                                    ch := '*N'
                                    linecount := linecount+1
                                    IF prsource DO newline()
                                $)
                                RETURN   $)

                       endread()
                       linecount, sourcestream, ch := getp!1, getp!2, getp!3
                       getp := getp!4
                       selectinput(sourcestream)
                       GOTO next  $)


       checkass:  rch()
       checkassx: IF ch=':' DO $( rch()
                                  UNLESS ch='=' DO caereport(57)
                                  ch := '?'  $)
                  UNLESS ch='?' RETURN
                  symb := symb + becomesbit

       l: rch()   $)  $)1



LET d(words) BE
    $(1 LET i, length = 1, 0

        $( LET ch = getbyte(words, i)
           TEST ch=',' | ch=':' DO
                            $( charv!0 := length
                               wordsize := packstring(charv, wordv)
                               lookupword()
                               TEST codep=0 DO
                               $(    // set conditional compilation tag to
                                     // true
                                   h1!wordnode := -s.name
                               $)
                               OR
                               $(  h1!wordnode := !codep
                                   codep := codep + 1
                               $)
                               IF ch=':' RETURN
                               length := 0  $)
                       ELSE $( length := length + 1
                               charv!length := ch  $)
           i := i + 1
        $) REPEAT
    $)1

LET declsyswords() BE
   $(1 codep := TABLE
          s.and,s.abs,
          s.be,s.break,s.by,
          s.case,
          s.do,s.default,
          s.eq,s.eqv,s.or,s.endcase,
          s.false,s.for,s.finish, //*** S.FLOAT,S.FIX,
          s.goto,s.ge,s.gr,s.global,s.get,
          s.if,s.into,
          s.let,s.lv,s.le,s.ls,s.logor,s.logand,s.loop,s.lshift,
          s.manifest,
          s.ne,s.not,s.neqv,s.needs,
          s.or,
          s.resultis,s.return,s.rem,s.rshift,s.rv,
          s.repeat,s.repeatwhile,s.repeatuntil,
          s.switchon,s.static,s.section,
          s.to,s.test,s.true,s.do,s.table,
          s.until,s.unless,
          s.vec,s.valof,s.while,
$<EXT'
          s.name
$>EXT'
$<EXT
          s.external,s.name
$>EXT

       //** fix and float removed from string below
       d("AND,ABS,*
         *BE,BREAK,BY,*
         *CASE,*
         *DO,DEFAULT,*
         *EQ,EQV,ELSE,ENDCASE,*
         *FALSE,FOR,FINISH,*
         *GOTO,GE,GR,GLOBAL,GET,*
         *IF,INTO,*
         *LET,LV,LE,LS,LOGOR,LOGAND,LOOP,LSHIFT:")

       d("MANIFEST,*
         *NE,NOT,NEQV,NEEDS,*
         *OR,*
         *RESULTIS,RETURN,REM,RSHIFT,RV,*
         *REPEAT,REPEATWHILE,REPEATUNTIL,*
         *SWITCHON,STATIC,SECTION,*
         *TO,TEST,TRUE,THEN,TABLE,*
         *UNTIL,UNLESS,*
         *VEC,VALOF,*
         *WHILE:")
$<EXT'
       d("$:")
$>EXT'
$<EXT
       d("EXTERNAL,$:")
$>EXT

        nulltag := wordnode

        IF lispopt DO
        $(
            codep := TABLE s.nil,s.car,s.cdr,s.lle,s.lge,s.lls,s.lgr
            d("NIL,CAR,CDR,LLE,LGE,LLS,LGR:")
        $)

        codep := 0
        d(condtaglist)     // conditional compilation tags from
                           // the parameter string
     $)1

AND lookupword() = VALOF

$(1
//        LET hashval = (wordv!0+wordv!wordsize >> 1) REM nametablesize
        LET hashval =
        VALOF $( LET res = wordv%0
                 FOR i = 1 TO res DO
                      res := (res*13 +
                              capitalch(wordv%i)) & #X7FFF
                 RESULTIS res REM nametablesize
              $)
        LET i = 0

        wordnode := nametable!hashval

/*
   ****** Used to be optional code for non-tripos versions

        UNTIL wordnode=0 \/ i>wordsize DO
           TEST wordnode!(i+2) = wordv!i
             THEN i := i+1
             ELSE wordnode, i := h2!wordnode, 0

   ****** End removal
*/

       UNTIL [wordnode = 0] | [compstring(wordnode+2, wordv) = 0]
       DO wordnode := wordnode ! h2

        IF wordnode=0 DO
          $( wordnode := newvec(wordsize+2)
             wordnode!0, wordnode!1 := s.name, nametable!hashval
             FOR i = 0 TO wordsize DO wordnode!(i+2) := wordv!i
             nametable!hashval := wordnode  $)

        RESULTIS h1!wordnode  $)1

LET rch() BE
    $( ch := rdch()

       IF prsource DO IF getp=0 /\ ch NE endstreamch DO
          $( UNLESS linecount=prline DO $( writef("%I4 %C ", linecount,
                                                 skipping -> '$', ' ')
                                           prline := linecount  $)
             wrch(ch)  $)

       chcount := chcount + 1
       chbuf!(chcount&63) := ch  $)

AND wrchbuf() BE
    $( writes("*N...")
       FOR p = chcount-63 TO chcount DO
                $( LET k = chbuf!(p&63)
                   UNLESS k=0 \/ k=endstreamch DO wrch(k)  $)
       newline()  $)


AND rdtag(x) BE
    $( charp, charv!1 := 1, x

        $(  rch()
            IF ch=endstreamch THEN BREAK
            x := getbyte(tagchartable, ch)
            IF x = 0 BREAK
            charp := charp+1
            charv!charp := x  $) REPEAT

       charv!0 := charp
       wordsize := packstring(charv, wordv) $)


AND performget() BE
$(  LET p = ?
    nextsymb()
    UNLESS symb = s.string DO caereport(89)
    p := newvec(wordsize+5)
$<UNIX'
    uppercasestring(wordv)    // Leave as lower case for unix GET's
$>UNIX'
    ngets := ngets+1
     // CHAIN THE NEW GET NODE FOR TRN
    p!0 := getchain
    getchain := p
     // COPY THE NAME
    FOR i = 0 TO wordsize DO p!(i+5) := wordv!i
     // STACK THE CURRENT STATE
    p!1, p!2, p!3, p!4 := linecount, sourcestream, ch, getp
     // TRY TO OPEN THE NEW STREAM
    sourcestream := findget(wordv) // findget is machine dependant
    IF sourcestream=0 THEN caereport(-88)
     // ESTABLISH NEW STATE
    linecount := 100000*ngets+1
    getp := p
    selectinput(sourcestream)
    rch()
$)

AND readnumber(base) BE

$( LET d = value(ch)
   decval := d
   IF d>=base DO caereport(33)

   $( rch()
      d := value(ch)
      IF d>=base RETURN
      decval := decval*base+d  $) REPEAT
$)

$<IMPERIAL'
AND readfloat() BE
$(  LET exp, n = 0, 0
    LET flten = FLOAT(10)
    decval := FLOAT(decval); symb := s.number
    IF ch='.' THEN
    $(  rch(); n := value(ch)
        IF n>=10 THEN BREAK
        exp := exp-1
        decval := decval #* flten #+ FLOAT(n)
    $) REPEAT
    IF ch='E' | ch='e' THEN
    $(  LET neg = FALSE
        rch(); IF ch='+' | ch='-' THEN  $( neg := ch='-'; rch()  $)
        n := decval
        readnumber(10)
        TEST neg THEN exp := exp - decval
                   OR exp := exp + decval
        decval := n
    $)
    WHILE exp NE 0 DO
    $(  TEST exp<0
        THEN  $(  decval := decval #/ flten; exp := exp + 1  $)
          OR  $(  decval := decval #* flten; exp := exp - 1  $)
    $)
$)
$>IMPERIAL'
$<IMPERIAL
AND readfloat() BE
$( LET safe = output()
   selectoutput(stdout)
   writes("*NNo Floating point*N")
   selectoutput(safe)
$)
$>IMPERIAL


