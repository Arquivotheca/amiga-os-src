
// (C) Copyright 1979 Tripos Research Group
//     University of Cambridge
//     Computer Laboratory
// (C) Copyright 1985 All Rights Reserved METACOMCO plc
//     26 Portland Square
//     Bristol

SECTION "M68CG6"

GET "CGHDR"

// Class bits:
//      q  b     w  m  cr r  r7 r6 r5 r4 r3 r2 r1 r0
//   0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15

LET class(a) = VALOF
$( LET k, n = h1!a, h2!a
// LET junk = VALOF IF debug>7 DO backtrace()

   LET bits = regscontaining(k, n)

   IF debug>5 DO
      writef("REGSCONTAINING(%N,%N) %X4*N", k, n, bits)

   SWITCHON k INTO
   $( DEFAULT:
   // CASE K.LVLOC:   CASE K.LOCSH:
   // CASE K.LVGLOB:  CASE K.GLOBSH:
   // CASE K.LVLAB:   CASE K.LABSH:
   // CASE K.LVLAB:
   // CASE K.LVLABSH:
                  ENDCASE

$<EXT
      CASE k.ext:
$>EXT
      CASE k.glob:
      CASE k.loc:
      CASE k.lab:
      CASE k.ir7:
      CASE k.ir6:
      CASE k.ir5:
      CASE k.ir4:
      CASE k.ir3:
      CASE k.ir2:
      CASE k.ir1:
      CASE k.ir0: bits := bits | c.m
                  ENDCASE


      CASE k.numb:IF n=0 DO          bits := bits | c.z
                  IF -8<=n<=8 DO     bits := bits | c.q
                  IF -128<=n<=127 DO bits := bits | c.b
                  bits := bits | c.w
                  ENDCASE

      CASE k.reg: bits := bits | c.r+c.cr
   $)

   IF debug>5 DO
      writef("CLASS(%N,%N) %X4*N", h1!a, h2!a, bits)
   RESULTIS bits
$)

AND choosereg(regs) = VALOF
$( IF debug>5 DO
      writef("CHOOSEREG(%X4)*N", regs)
   FOR r = r1 TO r7 DO
       UNLESS (regs>>r&1)=0 RESULTIS r
   IF (regs&1)=0 DO bug(5)
   RESULTIS r0
$)

/*
AND dbg(s,a,b,c,d) BE
$( LET o=output()
   selectoutput(sysout)
   writef(s,a,b,c,d)
   newline()
   selectoutput(o)
$)
*/

// form effective address in EA.M and EA.D
// If the address requires an offset that will not
// fit in a 16 bit word then code is compiled to
// put a suitable value in L.  The result is TRUE
// if this was done and FORMEA may not be called
// until EA.M AND EA.D have been used.
AND formea(k, n) = VALOF
$( LET x = k & 7  // P G or D0-D7

   ea.d := n

   SWITCHON k INTO
   $( DEFAULT:     bug(8)

$<EXT
      CASE k.ext:  // dbg("FORMEA: k.ext, *"%S*"", @(n!e.name))
                   ea.m, ea.d := #X80000000 | m.71, n
                   RESULTIS FALSE
      CASE k.lvext: // dbg("FORMEA: k.lvext, *"%S*"", @(n!e.name))
                   ea.m, ea.d := #X80000000 | m.71, n
                   RESULTIS FALSE
$>EXT
      CASE k.reg:  ea.m, ea.d := n, 0 // Dn direct
                   RESULTIS FALSE

      CASE k.numb: ea.m := m.74       // #w long immediate
                   RESULTIS FALSE

      CASE k.loc:  n    := n - 3
      CASE k.glob: ea.d := n * 4
                   UNLESS -32768<=ea.d<=32767 DO
                   $( ea.d :=  ea.d & #X00FFFFFF
                      n    := (ea.d & #X00FFFF00) + 128
                      UNLESS numbinl=n DO
// Another bug fix IJJH 10 Nov 82
                         geneaea(f.movel, m.74, n, m.10+rl, 0)
//                         genrea(f.lea, rl, m.74, n)
                      numbinl := n
     ea.m := m.6l
                      ea.d := exta(x, ea.d-n)
                      RESULTIS TRUE
                   $)
                   TEST ea.d=0
                   THEN ea.m := m.20 + x // (P)  or (G)
                   ELSE ea.m := m.50 + x // w(P) or w(G)
                   RESULTIS FALSE
      CASE k.ir7:
      CASE k.ir6:
      CASE k.ir5:
      CASE k.ir4:
      CASE k.ir3:
      CASE k.ir2:
      CASE k.ir1:
      CASE k.ir0:  // it is known that -128<=N<127
                   ea.m, ea.d := m.6z, extd(x, n)  // b(Z,Ri)
                   RESULTIS FALSE

      CASE k.lab:  ea.m := m.5b
                   RESULTIS FALSE
   $)
$)

AND initslave() BE FOR r = r0 TO r7 DO slave!r := 0

AND forgetr(r) BE UNLESS slave!r=0 DO
$( LET a = @slave!r
   UNTIL !a = 0 DO  a := !a
   !a := freelist
   freelist := slave!r
   slave!r := 0
$)

AND forgetall() BE
$(  FOR r = r0 TO r7 DO forgetr(r)
    numbinl := 0  // no known value in L
$)

AND remem(r, k, n) BE IF k<k.reg DO
    slave!r := getblk(slave!r, k, n)

AND moveinfo(s, r) BE UNLESS s=r DO
$( LET p = slave!s
   forgetr(r)
   UNTIL p=0 DO
   $( remem(r, h2!p, h3!p)
      p := !p
   $)
$)

// Forget the slave informationabout the
// variable (K, N).  If K>=K.IR0 all information
// about variables are lost.
// K is one of: K.LOC, K.GLOB, K.LAB or K.IRr
AND forgetvar(k, n) BE TEST k>=k.ir0
THEN forgetvars()
ELSE FOR r = r0 TO r7 DO
$( LET a = @slave!r

    $( LET p = !a
      IF p=0 BREAK
      TEST h3!p=n & (h2!p & k.notsh)=k
      THEN $( !a := !p   // free and unlink the item
              freeblk(p)
           $)
      ELSE a := p
   $) REPEAT
$)

AND forgetvars() BE FOR r = r0 TO r7 DO
$( LET a = @slave!r

   $( LET p = !a
      IF p=0 BREAK
      TEST h2!p <= k.labsh
      THEN $( !a := !p   // free and unlink the item
              freeblk(p)
           $)
      ELSE a := p
   $) REPEAT
$)

AND regscontaining(k, n) = VALOF
$( LET regset = 0

   IF k=k.reg RESULTIS 1<<n | c.cr+c.r

   FOR r = r0 TO r7 IF isinslave(r, k, n) DO
       regset := regset | (1<<r)| c.cr

   RESULTIS regset
$)

AND inregs(r, regs) =
    r<0 | (regs>>r & 1)=0 -> FALSE, TRUE

AND isinslave(r, k, n) = VALOF
$( LET p = slave!r

   UNTIL p=0 DO
   $( IF h2!p=k & h3!p=n RESULTIS TRUE
      p := !p
   $)

   RESULTIS FALSE
$)

AND regsinuse() = VALOF
$( LET regset = 0

   FOR t = tempv TO arg1 BY 3 DO
       IF h1!t>=k.reg DO
       $( LET r = h1!t & 7
          IF h1!t=k.reg DO r := h2!t
          regset := regset | (1<<r)
       $)
   RESULTIS regset
$)

AND regswithinfo() = VALOF
$( LET regset = 0
   FOR r = r0 TO r7 DO
       UNLESS slave!r=0 DO regset := regset | (1<<r)
   RESULTIS regset
$)


AND code(a) BE
$( stv%stvp     := a>>8
   stv%(stvp+1) := a
   stvp := stvp + 2
   IF debug>0 DO
      writef("CODE: %X4*N", a)
   checkspace()
$)

AND code2(a) BE
$( code(a>>16)
   code(a)
$)

// line up on full word boundary
AND cnop() BE IF (stvp&3)=2 DO code(f.nop)

AND addtoword(val, a) BE
$( val := val + (stv%a<<8) + stv%(a+1)
   stv%a     := val>>8
   stv%(a+1) := val
$)

// functions to form index extension words
AND extd(r, d) = #X0800 + ((r&7)<<12) + (d&#XFF)

AND exta(r, d) = #X8800 + ((r&7)<<12) + (d&#XFF)

$<EXT
AND addxref(symbol,loc) BE
$( LET p = newvec(er.size)
   // dbg("ADDXREF: *"%S*" loc %X8", @(symbol!e.name), loc)
   p!er.link, p!er.loc := 0, loc+sectionoffset
   TEST symbol!e.refs = 0 THEN
     symbol!e.refs := p
   ELSE
     (symbol!e.refstail)!er.link := p
   symbol!e.refstail := p
$)
$>EXT

// make an operand if required
AND genrand(m, d) BE
$( // dbg("GENRAND: m=%X8 d=%X8", m, d)
$<EXT
   IF m<0 THEN
   $( addxref(d,stvp)
      d := 0
   $)
$>EXT
   TEST (m & m.l)=0 THEN
   $( UNLESS (m&m.ww)=0 DO code(d>>16)
      UNLESS (m&m.w) =0 DO code(d)
   $)
   ELSE
   $( LET val = labv!d
      IF val=-1  DO
      $( rlist := getblk(rlist, stvp, d)
         val := 0
      $)
      code(val-procbase)
   $)
$)

// compile  single word instructions
AND gen(f) BE IF incode DO
$( insertcount()
   code(f)
$)

// compile  NEG ea  etc.
AND genea(f, m, d) BE IF incode DO
$( LET instr = f | (m&#77)
   insertcount()
   code(instr)
   genrand(m, d)
$)

// compile  MOVE.L  ea,ea  etc.
AND geneaea(f, ms, ds, md, dd) BE IF incode DO
$( LET instr = f | (ms&#77) | (md&7)<<9 | (md&#70)<<3
   insertcount()
   code(instr)
   genrand(ms, ds)
   genrand(md, dd)
$)

// compile  ADDQ.L  #q,ea  etc.
AND genqea(f, q, m, d) BE genrea(f, q&7, m, d)

// compile MOVEQ #b,Dn
AND genmoveq(b, r) BE gen(f.moveq | (r<<9) | (b&#XFF))

// compile  ADD.L Dn,ea   ADD.L ea,Dn  etc.
AND genrea(f, r, m, d) BE IF incode DO
$( LET instr = f | (m&#77) | (r<<9)
   insertcount()
   code(instr)
   genrand(m, d)
$)

// compile  SWAP Dn  etc.
AND genr(f, r) BE gen(f+r)

// compile  LSL Ds,Dr     etc.
AND genrr(f, s, r) BE gen(f | s<<9 | r)

// compile  LSL #q,Dn  etc.
AND genshkr(f, sk, r) BE genrr(f, sk&7, r)

// compile  ADDI.L  #w,Dr  etc.
AND genwr(f, w, r) BE genwea(f, w, m.00+r, 0)

// compile  ADDI.L  #w,ea  etc.
AND genwea(f, w, m, d) BE IF incode DO
$( LET instr = f | (m&#77)
   insertcount()
   code(instr)
   code2(w)
   genrand(m, d)
$)


// inserts a profile count
AND insertcount() BE IF countflag DO
$( countflag := FALSE
   cnop()
   genea(f.jsr, m.5s, sr.profile)
   code2(0)
$)


// set the label L to the current location
AND setlab(l) BE
$( LET a = @rlist
   UNLESS labv!l=-1 DO bug(15)
   labv!l := stvp
$<JUMPS
   lastlab := l         // record for genb
   lastlabpos := stvp
   //writef("Setlab: label = %n, pos = %X4*N",l,stvp)
$>JUMPS

   // fill in forward jump refs
   // and/or remove them from RLIST
   UNTIL !a=0 DO
   $( LET p = !a
      TEST l = h3!p
      THEN $(
          addtoword(stvp, h2!p)
$<JUMPS'
          !a := !p
          freeblk(p)
      $)
$>JUMPS'
$<JUMPS
          //writef("adjusting word by %n at %X4*N",stvp, h2!p)
          a := p
      $)
      ELSE TEST [labv!(h3!p) \= -1] & [labv!(h3!p) < stvp] // free any passed references
      THEN $(
              //writef("Freeing ref to label %n at %n*N",h3!p, h2!p)
              !a := !p
              freeblk(p)
      $)
      ELSE TEST [labv!(h3!p) = stvp] // a previous label put here
      THEN $(
         p!h3 := l    // make it a ref to this label instead (yuk)...
         a := p       // ...this is so we can handle multiple labels
      $)
$>JUMPS
      ELSE a := p
   $)
$)

AND addsymbol(s) BE
$( LET node = newvec(3)
   TEST symbolstail = 0 THEN
     symbols := node
   ELSE
     symbolstail!sym.link := node
   symbolstail := node

   node!sym.link, node!sym.val := 0, stvp
   FOR i=0 TO 7 DO
     (@(node!sym.name))%i := s%i
$)

// compiles names for S.ENTRY, S.SECTION, S.NEEDS
AND cgname(op,n) BE
$( LET v = VEC 4
   FOR i=0 TO 4 DO v!i := 0
   v%0 := op=s.entry->7,17
   FOR i=1 TO n DO
   $( LET c = rdn()
      IF i<=7 DO v%i := c
   $)
   IF dumpsymbols & (op = s.entry) THEN
      addsymbol(v)
   FOR i = n+1 TO 7 DO
       v%i := n=0->#X2A,#X20 // #X20 is ASCII '*S'
                             // #X2A is ASCII asterisk
   v%8 := #X20 // ASCII space
   FOR i = 1 TO datvec%0 DO v%(i+8) := datvec%i

   UNLESS op=s.needs DO IF naming THEN
   $( IF op=s.section DO code2(secword)
      FOR i = 0 TO op=s.entry->1,4  DO code2(v!i)
   $)
$)


AND cgstring(n) BE
$( LET w = n
   datalabel := nextparam()
   loadt(k.lvlab, datalabel)

   FOR i = 1 TO n|3 DO
   $( w := w<<8
      IF i<=n DO w := rdn() | w
      IF i REM 4 = 3 DO
      $( cgitemn(w)
         w := 0
      $)
   $)
$)

AND getblk(a, b, c) = VALOF
$( LET p = freelist
   TEST p=0
   THEN $( dp := dp-3
           p := dp
           checkspace()
        $)
   ELSE freelist := !p
   h1!p, h2!p, h3!p := a, b, c
   RESULTIS p
$)

AND freeblk(p) BE
$( !p := freelist
   freelist := p
$)

AND cgitemn(n) BE
$(  LET p = getblk(0, datalabel, n)
    datalabel := 0
    !nliste := p
    nliste := p
$)

// Compile static data.  It is only
// called at the outermost level
// There are no ITEML items since are regarded
// as constants so as to allow position independent
// code.  ITEML information is held on theLLIST

AND cgstatics() BE
$( cnop() // line up on a full word boundary

   UNTIL nlist=0 DO
   $( LET p = h1!nlist
      LET l = h2!nlist
      LET n = h3!nlist
      UNLESS l=0 DO setlab(l)
      code2(n)
      freeblk(nlist)
      nlist := p
   $)

   nliste := @nlist  // (NLIST=0 when we are finished)
$)



AND initdatalists() BE
$(  rlist   := 0        // for single word rel label refs
    llist := 0        // for the DATALAB ITEML mappings
    nlist   := 0        // for ITEMNs with their labels
    nliste  := @nlist
    needslist  := 0     // list of NEEDS directives
    needsliste := @needslist
$)


AND checkspace() BE
$(
    IF stv+stvp/4>dp DO
    $(  cgerror("Program too large - use Wn option to increase workspace")
    //    writef("STV = %N, DP = %N*N", stv, dp)
        collapse(20) 
    $)
    IF stvp > max.sect.size DO
    $(
        cgerror("Section greater than 32K bytes")
        collapse( 20 )
    $)
$)

