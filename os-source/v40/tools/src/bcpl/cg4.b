
// (C) Copyright 1979 Tripos Research Group
//     University of Cambridge
//     Computer Laboratory
// (C) Copyright 1985 All Rights Reserved METACOMCO plc
//     26 Portland Square
//     Bristol

SECTION "M68CG4"

GET "CGHDR"

LET cgdyadic(fns, swappable, mem2) = VALOF

// MEM2 is TRUE if the function is CMP or if an
// assignment is being compiled (in which case the
// function is ADD, SUB, AND or OR).  The destination
// of the assignment is represented by ARG2.
// If MEM2 is FALSE no memory location may be changed,
// and the result of the operation will be
// represented (on return) by ARG2.

// If SWAPPABLE is TRUE the operands may be swapped if
// there is some advantage in doing so.

// The result is SWAPPED if the operands were swapped,
// and NOTSWAPPED otherwise.  SSP is not altered.
// FNS is a vector of 9 elements

// 0  FT.QR  ADDQ SUBQ                     #q,ea   GENQEA
// 1  FT.QM  ADDQ SUBQ                     #q,ea   GENQEA
// 2  FT.RR  ADD  SUB  CMP  AND  OR  EOR   ea,Dn   GENREA
// 3  FT.RM  ADD  SUB       AND  OR  EOR   Dn,ea   GENREA
// 4  FT.IR  ADDI SUBI CMPI ANDI ORI EORI  #ww,Dn  GENWEA
// 5  FT.IM  ADDI SUBI CMPI ANDI ORI EORI  #ww,ea  GENWEA
// 6  FT.MR  ADD  SUB  CMP  AND  OR        ea,Dn   GENREA
// 7  FT.ZR            TST                         GENEA
// 8  FT.ZM            TST                         GENEA

// Empty entries have value -1 indicating that
// that version of the function does not
// exist.
// The register slave is updated appropriately.
$( LET drcl = c.r
   IF fns=fns.cmp DO drcl := c.cr

   arg1cl := class(arg1)
   IF arg1cl=0 DO
   $( movetoanycr(arg1)
      LOOP
   $)
   arg2cl := class(arg2)
   IF arg2cl=0 DO
   $( movetoanycr(arg2)
      LOOP
   $)

   IF arg1cl=c.m=arg2cl DO
   $( // both unslaved memory operands
      // put the source in a register
      movetoanyr(arg1)
      LOOP
   $)

   IF arg1cl=c.b+c.w DO
   $( // if unslaved byte sized but not quick
      // put in register
      movetoanyr(arg1)
      LOOP
   $)

   IF arg2cl=c.b+c.w DO
   $( // if unslaved byte sized but not quick
      // put in register
      movetoanyr(arg2)
      LOOP
   $)


   fntab := fns

   IF try(mem2, drcl) RESULTIS notswapped

   IF swappable DO
   $( swapargs()
      IF try(mem2, drcl) RESULTIS swapped
      swapargs()
   $)

// we have failed to compile anything this
// time round, so let us try to simplify
// the operands and try again.

   IF fns=fns.sub & NOT mem2 & (arg2cl&c.r)=0 DO
   $( // make SUB ...,Dn possible
      movetoanyr(arg2)
      LOOP
   $)

   IF (arg2cl & c.w) \= 0 & (NOT mem2 | fns=fns.cmp) DO
   $( movetoanyr(arg2)  // mem2 = FALSE
      LOOP
   $)

   UNLESS (arg1cl & c.w) = 0 DO
   $( movetoanyr(arg1)
      LOOP
   $)

   IF (arg2cl & c.r) = 0 & (NOT mem2 | fns=fns.cmp) DO
   $( movetoanyr(arg2)  // mem2 = FALSE
      LOOP
   $)

   IF (arg1cl & c.r) = 0 DO
   $( movetoanyr(arg1)
      LOOP
   $)

   bug(1)
   RESULTIS notswapped
$) REPEAT

// try to compile an instruction for
// ARG2 op ARG1
// the result is TRUE iff successful
// FNTAB holds the function codes for op
// ARG1CL and ARG2CL are already setup
AND try(mem2, drcl) =
   match(ft.qr,         c.q,   c.r)    |
    match(ft.rr,         c.cr,  drcl)   |
    mem2 & match(ft.qm,  c.q,   c.m)    |
    match(ft.mr,         c.m,   drcl)   |
    mem2 & match(ft.rm,  c.cr,  c.m)    |
    match(ft.zr,         c.z,   drcl)   |
    mem2 & match(ft.zm,  c.z,   c.m)    |
    // only use immediate instructions
    // if the constant is larger than a byte
    match(ft.ir,         c.w,   drcl)   |
    mem2 & match(ft.im,  c.w,   c.m)    -> TRUE, FALSE

// Compile an instruction if the operands match
// the required classifications CL1 and CL2 and
// the corresponding function code exists.
// If the destination is a register that is
// updated, FORGETR is called and ARG2 updated
// appropriately.
// FNTAB will contain function code variations
// for one of the following:
//    ADD  SUB  CMP  AND  OR  EOR
// The state of the register slave is updated
// if any instruction is compiled.
AND match(ft.entry, cl1, cl2) = VALOF
$( LET f = fntab!ft.entry
   LET k1, n1, k2, n2 = ?, ?, ?, ?
   LET s, r = ?, ?

   IF debug>5 DO
      writef("MATCH(%N,%X4, %X4) ARG1CL=%X4 ARG2CL=%X4*N",
              ft.entry, cl1, cl2, arg1cl, arg2cl)

   IF f=-1 |
      (arg1cl & cl1) \= cl1 |
      (arg2cl & cl2) \= cl2 RESULTIS FALSE

   IF cl1=c.w DO
      // check that the constant is larger than a byte
      UNLESS (arg1cl & c.b) = 0 RESULTIS FALSE

   // The match was successful so compile
   // an instruction.
   k1, n1 := h1!arg1, h2!arg1
   k2, n2 := h1!arg2, h2!arg2

   IF cl1=c.cr DO s := choosereg(arg1cl&c.regs)
   IF cl2=c.cr DO r := choosereg(arg2cl&c.regs)
   IF cl2=c.r  DO r := n2 & 7

   SWITCHON ft.entry INTO
   $( CASE ft.qr: // the function is ADDQ or SUBQ
           IF n1=0 ENDCASE
           IF n1<0 DO n1, f := -n1, f.addq+f.subq-f
           genqea(f, n1, m.00+r, 0)
           ENDCASE

      CASE ft.qm: // the function is ADDQ or SUBQ
           IF n1=0 ENDCASE
           IF n1<0 DO n1, f := -n1, f.addq+f.subq-f
           formea(k2, n2)
           genqea(f, n1, ea.m, ea.d)
           r := -1
           ENDCASE

      CASE ft.zr: // the function is CMP (actually TST)
           genea(f, m.00+r, 0)
    ENDCASE

      CASE ft.zm: // the function is CMP (actually TST)
           formea(k2, n2)
           genea(f, ea.m, ea.d)
           r := -1
           ENDCASE

      CASE ft.rr:
           TEST f=f.eor
           THEN genrea(f, s, m.00+r, 0)
           ELSE genrea(f, r, m.00+s, 0)
           ENDCASE

      CASE ft.rm:
           formea(k2, n2)
           genrea(f, s, ea.m, ea.d)
        r := -1
           ENDCASE

      CASE ft.ir:
           genwr(f, n1, r)
           ENDCASE

      CASE ft.im:
           formea(k2, n2)
           genwea(f, n1, ea.m, ea.d)
           r := -1
           ENDCASE

      CASE ft.mr:
           formea(k1, n1)
           genrea(f, r, ea.m, ea.d)
           ENDCASE

      DEFAULT: bug(5)
   $)

   UNLESS fntab=fns.cmp TEST r>=0
   THEN forgetr(r)
   ELSE forgetvar(k2, n2)

   IF debug>5 DO dboutput(3)

   RESULTIS TRUE
$)
// cgfloatdyadic performs similar functions to cgdyadic,
// but uses the simulated instruction set implemented
// by line 1111 emulation. The value fns is either
// fns.cmp as in cgdyadic - in which case the instruction
// generated is FCMP or FTST as appropriate - or the
// instruction itself. As all these functions can only
// use registers r1 & r2 the code is simpler than in cgdyadic.
// The operands may be swapped if swappable is true. Returns
// swapped or notswapped.

AND cgfloatdyadic(fns, swappable) = VALOF
$( LET thisswapped = notswapped

   // Check if comparison with zero, in which case we can use FTST.
   if fns = fns.cmp then
   $( if (class(arg1) & c.z) NE 0 then  // r2 would be zero
      $( movetor(arg2,r1)
         // gen( f.ftst )
         genfpcall( f.ftst )
         resultis notswapped
      $)

      if (class(arg2) & c.z) NE 0 then  // r1 would be zero
      $( movetor(arg1,r1)
         // gen( f.ftst )
         genfpcall( f.ftst )
         resultis swapped
      $)

      // Must use FCMP
      fns := f.fcmp

   $)

   if swappable & (regusedby(arg1) = r1 | regusedby(arg2) = r2) then
   $( // Already in registers, but the wrong ones
      swapargs()
      thisswapped := swapped
   $)

   // Generate the code in the general case
   movetor(arg2,r1)
   movetor(arg1,r2)
   genfpcall( fns )

   unless fns = f.fcmp then forgetr(r1)
   resultis thisswapped
$)


AND genfpcall( fns ) BE
$(
    // Can't do TRIPOS 1111 emulator TRAPS for other systems
    // so this grotty piece of code has to be patched
    // in to interface to the FP library.
    // Basically we need to call a global without 
    // screwing the contents of any of the registers.

    LET SRins = (chip = c.m68010) -> #X42E7,    // MOVE  CCR,-(SP)
                                     #X40E7     // MOVE  SR,-(SP)
    TEST  triposfp  THEN
       gen( fns )
    ELSE
    $(
       gen( #X4EBA )     //      JSR address(PC)     ; Return address on stack
       gen( #X0002 )     // address:
       gen( SRins )      //      MOVE SR/CCR,-(SP)
       gen( #X2F2A )     //      MOVE.L G_FPSIM*4(A2),-(SP)  ; Get address of FPSIM

       TEST globexternals & (fpsimglob >= extlowbound) THEN
       $(
          addglobexternalref( stvp, fpsimglob )
          gen( 0 )
       $)
       ELSE 
          gen( fpsimglob*4 )

       gen( #X4E75 )     //      RTS                 ; JMP to FPSIM
       gen(  fns   )     //      DC.W  $F00?         ; Put in FP trap
                         //      ... fpsim will return here
    $)
$)

/*
AND dbg(s,a,b,c,d) BE
$( let o = output()
   selectoutput(verstream)
   writef(s,a,b,c,d)
   newline()
   selectoutput(o)
$)
*/

AND mygetblk(a,b,c) = VALOF
$(
   LET p = newvec(2)
   p!0,p!1,p!2 := a,b,c
   RESULTIS p
$)
  
AND addglobexternalref( loc, gn ) BE
$(
   LET p = mygetblk( 0, loc+sectionoffset, gn )

   TEST globxrefs = 0 THEN
      globxrefs := p
   ELSE
      globxrefstail!xref.link := p

   globxrefstail := p
$)


AND swapargs() BE
$( LET k, n, cl = h1!arg1, h2!arg1, arg1cl
   h1!arg1, h2!arg1, arg1cl := h1!arg2, h2!arg2, arg2cl
   h1!arg2, h2!arg2, arg2cl := k, n, cl
$)


AND initftables() BE
$( fns.add := TABLE
#X5080,#X5080,#XD080,#XD180,#X0680,#X0680,#XD080,    -1,    -1
//  QR     QM     RR     RM     IR     IM     MR     ZR     ZM

   fns.sub := TABLE
#X5180,#X5180,#X9080,#X9180,#X0480,#X0480,#X9080,    -1,    -1
//  QR     QM     RR     RM     IR     IM     MR     ZR     ZM

   fns.cmp := TABLE
    -1,    -1,#XB080,    -1,#X0C80,#X0C80,#XB080,#X4A80,#X4A80
//  QR     QM     RR     RM     IR     IM     MR     ZR     ZM

   fns.and := TABLE
    -1,    -1,#XC080,#XC180,#X0280,#X0280,#XC080,    -1,    -1
//  QR     QM     RR     RM     IR     IM     MR     ZR     ZM

   fns.or  := TABLE
    -1,    -1,#X8080,#X8180,#X0080,#X0080,#X8080,    -1,    -1
//  QR     QM     RR     RM     IR     IM     MR     ZR     ZM

   fns.eor := TABLE
    -1,    -1,#XB180,#XB180,#X0A80,#X0A80,    -1,    -1,    -1
//  QR     QM     RR     RM     IR     IM     MR     ZR     ZM

$)

AND movetoanyrsh(a) = VALOF
$( LET r = -1

   SWITCHON h1!a INTO
   $( CASE k.loc:
      CASE k.glob:
      CASE k.lab:
      CASE k.lvloc:
      CASE k.lvglob:
      CASE k.lvlab: h1!a := h1!a + k.sh
                    ENDCASE

      CASE k.numb:  h2!a := h2!a * 4
                    ENDCASE

      DEFAULT:      r := movetoanyr(a)
                    genshkr(f.lslkr, 2, r)
                    forgetr(r)
   $)

   IF r<0 DO r := movetoanyr(a)
   RESULTIS r
$)

// Get A in a data register for use as a
// source operand.
// No data registers will change before it is used
AND movetoanycr(a) = VALOF
$( LET cl = class(a)
   LET poss = cl & c.regs
   IF poss=0 RESULTIS movetoanyr(a)
   RESULTIS choosereg(poss)
$)

// move a SS item into any data register
AND movetoanyr(a) = VALOF
$( LET usedregs = regsinuse()
   LET poss = ?

   // is A already in a register?
   IF h1!a=k.reg RESULTIS h2!a

   // slaved registers that are free
   poss := class(a) & c.regs & NOT usedregs
   UNLESS poss=0 RESULTIS movetor(a, choosereg(poss))

   // suitable regs with no info that are free
   poss := c.regs & NOT (usedregs | regswithinfo())
   UNLESS poss=0 RESULTIS movetor(a, choosereg(poss))

   // suitable regs that are free
   poss := c.regs & NOT usedregs
   UNLESS poss=0 RESULTIS movetor(a, choosereg(poss))

   // If A is of form K.IRr
   // then move it to Dr.
   IF h1!a>=k.ir0 RESULTIS movetor(a, h1!a & 7)

   // all registers are in use
   // so free the oldest
   FOR t = tempv TO arg1 by 3 DO    // by 3 added 1Aug83 TJK
       IF regusedby(t)>=0 DO
       $( storet(t)
          BREAK
       $)
   // The situation is now better so try again
$) REPEAT


// move a SS item A into data register Dr

AND movetor(a,r) = VALOF
$( LET k, n = h1!a, h2!a
   LET cl = ?

   // is A already where required?
   IF k=k.reg & n=r RESULTIS r

   // free register R if necessary
   UNLESS regusedby(a)=r DO
   $( freereg(r)
      k, n := h1!a, h2!a
   $)

   cl := class(a)

   IF cl=0 SWITCHON h1!a INTO
   $( CASE k.lvlocsh:
      CASE k.lvloc: n := n-3
      CASE k.lvglobsh:
      CASE k.lvglob:n := 4*n // convert to byte address
                 $( LET oldn = H2!a
                    LET ms = m.10 + (k&7)  // (P) or (G)
                    TEST n=0
                    THEN geneaea(f.movel, ms, 0, m.00+r, 0)
                    ELSE $( h1!a, h2!a := k.numb, n
                            movetor(a, r)
                            // compile   ADD P,Dr  or   ADD G,Dr
                            genrea(fns.add!ft.mr, r, ms, 0)
                         $)
                    n := oldn  // Restore n for remem
                 $)
           shret:   IF (k&k.sh)=0 DO
                       genshkr(f.lsrkr, 2, r)
                    GOTO ret

      CASE k.lvlabsh:
      CASE k.lvlab: formea(k.lab, n)
                    genrea(f.lea, rl, ea.m, ea.d)
                    numbinl := 0  // value in L unknown
                    geneaea(f.movel, m.1l, 0, m.00+r, 0)
                    GOTO shret

      CASE k.locsh:
      CASE k.globsh:
      CASE k.labsh: h1!a := h1!a - k.sh
                    movetor(a, r)
                    genshkr(f.lslkr, 2, r)
                    GOTO ret

$<EXT
      CASE k.lvext: // dbg("MOVETOR: k.lvext, *"%S*"", @(n!e.name))
                    formea(k.lvext,n)
                    genrea(f.lea, rl, ea.m, ea.d)
                    numbinl := 0  // value in L unknown
                    geneaea(f.movel, m.1l, 0, m.00+r, 0)
                    GOTO ret

      CASE k.ext:   // dbg("MOVETOR: k.ext, *"%S*"", @(n!e.name))
                    formea(k.ext,n)
                    geneaea(f.movel, ea.m, ea.d, m.00+r, 0)
                    GOTO ret
$>EXT
      DEFAULT:      bug(9)

   $)

   UNLESS (cl & c.cr) = 0 DO // value already in a register
   $( LET s = choosereg(cl & c.regs)
      IF (cl>>r & 1) = 0 DO
      $( // move only if necessary
         geneaea(f.movel, m.00+s, 0, m.00+r, 0)
       moveinfo(s, r)
      $)
      GOTO ret
   $)

   UNLESS (cl & c.b) = 0 DO  // a byte constant
   $( genmoveq(n&#XFF, r)
      GOTO ret
   $)

   formea(k, n)
   geneaea(f.movel, ea.m, ea.d, m.00+r, 0)

ret: forgetr(r)
     remem(r, k, n)
     h1!a, h2!a := k.reg, r
     RESULTIS r
$)

// move ARG1 to Ar
AND movetoa(r) BE
$( LET k, n, s = h1!arg1, h2!arg1, -1
   LET cl = class(arg1)

   UNLESS (cl & c.cr) = 0 DO // value is in a data register
      s := choosereg(cl & c.regs)

   IF s=-1 SWITCHON k INTO
   $( CASE k.lvlocsh:
      CASE k.lvglobsh:
                    formea(k&7, n)
                    genrea(f.lea, r, ea.m, ea.d)
                    RETURN

      CASE k.lvlabsh:
                    formea(k.lab, n)
                    genrea(f.lea, r, ea.m, ea.d)
                    RETURN


      CASE k.numb:  // use a D register for a byte constant
                    UNLESS (cl & c.b)=0 ENDCASE
                   formea(k.numb, n)
// Bug fix IJJH 10 Nov 82
                    geneaea(f.movel, ea.m, ea.d, m.10+r, 0)
//                    genrea(f.lea, r, ea.m, ea.d)
                    RETURN

      CASE k.loc:
      CASE k.glob:
      CASE k.lab:   formea(k, n)
                    geneaea(f.movel, ea.m, ea.d, m.10+r, 0)
                    RETURN
$<EXT
      CASE k.ext:   formea(k, n)
                    geneaea(f.movel, ea.m, ea.d, m.10+r, 0)
                    RETURN
      CASE k.lvext: formea(k, n)
                    geneaea(f.lea, ea.m, ea.d, m.10+r, 0)
                    RETURN
$>EXT

      DEFAULT:

   $)

   IF s=-1 DO s := movetoanycr(arg1)

   // compile  MOVEA.L Ds,Ar
   geneaea(f.movel, m.00+s, 0, m.10+r, 0)
$)


// find which register, if any, is used by
// an SS item
AND regusedby(a) = VALOF
$( LET k=h1!a
   IF k<k.reg RESULTIS -1
   IF k=k.reg RESULTIS h2!a
   RESULTIS k-k.ir0
$)


AND isfree(r) = VALOF
$( FOR t=tempv TO arg1 BY 3 DO
      IF regusedby(t)=r RESULTIS FALSE
   RESULTIS TRUE
$)


// Free register R by storing the SS item (if any)
// that depends on it.
AND freereg(r) BE FOR t=tempv TO arg1 BY 3 DO
                    IF regusedby(t)=r DO
                    $( storet(t)
                       BREAK
                    $)


// store the value of a SS item in its true
// stack location
// it uses CGDYADIC and preserves PENDINGOP
AND storet(a) BE UNLESS h2!a=h3!a & h1!a=k.loc DO
$( LET ak, an, s = h1!a, h2!a, h3!a
   LET pendop = pendingop
   pendingop := s.none
   h1!a, h2!a := k.loc, s
   loadt(k.loc, s)
   loadt(ak, an)
   cgmove()
   stack(ssp-2)
   pendingop := pendop
$)


// load an item (K,N) onto the SS
AND loadt(k, n) BE
$( cgpendingop()
   TEST arg1=tempt
   THEN cgerror("SIMULATED STACK OVERFLOW")
   ELSE arg1, arg2 := arg1+3, arg2+3

   h1!arg1,h2!arg1,h3!arg1 := k,n,ssp
   ssp := ssp + 1
   IF maxssp<ssp DO maxssp := ssp
   IF debug>6 DO dboutput(3)
$)


// replace the top two SS items by (K,N)
AND lose1(k, n) BE
$( ssp := ssp - 1
   TEST arg2=tempv
   THEN $( h1!arg2,h2!arg2 := k.loc,ssp-2
           h3!arg2 := ssp-2
        $)
   ELSE arg1, arg2 := arg1-3, arg2-3
   h1!arg1, h2!arg1, h3!arg1 := k,n,ssp-1
$)


// On arrival here, if we're doing a%b then arg1 holds b and arg2 holds a.
// One small optimization is to cope with 0%b operations, which we do as
// 0(A0,Dn.L) where the data register holds b.

AND cgbyteap(op) BE
$(1 cgpendingop()
 $( LET r = ?
    LET i = ?
    TEST (h1!arg2 = k.numb) & (h2!arg2 = 0) THEN
    $( swapargs()               // Make arg2 the offset, arg1 = number 0
       r := movetoanyr(arg2)    // Get offset into the register
       i := 0                   // Make index zero
    $)
    ELSE
    $( r := movetoanyrsh(arg2)
       i := h2!arg1
       UNLESS h1!arg1=k.numb & -128<=i<=127 DO
       $( cgdyadic(fns.add, TRUE, FALSE)
          i := 0
       $)
    $)
    stack(ssp-1)
    // just to make certain
    r := movetoanyr(arg1)
    h1!arg1, h2!arg1 := k.ir0+r, i
    // ARG1 now represents the addressof the
    // byte in mode 6 addressible form.

    TEST op=s.getbyte
    THEN $( loadt(k.numb, 0)
            r := movetoanyr(arg1)
            formea(h1!arg2, h2!arg2)
            // byte assignment to a data register
            // does not extend the sign
            geneaea(f.moveb, ea.m, ea.d, m.00+r, 0)
            forgetr(r)
            lose1(k.reg, r)
         $)
    ELSE $( LET m, d = 0, 0
            TEST h1!arg2=k.loc | h1!arg2=k.glob
            THEN $( formea(h1!arg2, h2!arg2)
                    m, d := ea.m, ea.d + 3
                    IF m=m.2p DO m := m.5p
                    IF m=m.2g DO m := m.5g
                 $)
            ELSE m, d := m.00+movetoanycr(arg2), 0
            formea(h1!arg1, h2!arg1)
            // the address in EA.M and EA.D will not use L
            geneaea(f.moveb, m, d, ea.m,ea.d)
            forgetvars()
            stack(ssp-2)
         $)
 $)
$)1

$<WORD
// If we're doing a%%n, where -64 <= n <= 63, we can use the INDEX(A0,Dr.L)
// mode, otherwise we'll have to do 0(A0,Dr.L) where Dr = ((a<<1)+n)<<1.
AND cgwordap(op) BE
$( LET r,i = ?,?
   cgpendingop()
   i := h2!arg1 * 2
   TEST h1!arg1=k.numb & -128<=i<=127 THEN
      r := movetoanyrsh(arg2)         // Get byte pointer in register
   ELSE
   $( r := movetoanyr(arg2)           // Get long word ptr in register
      genshkr(f.lslkr,1,r)            // Make word pointer
      cgdyadic(fns.add, TRUE, FALSE)  // Add in word offset
      genshkr(f.lslkr,1,r)            // Now byte pointer
      i := 0
   $)
   stack(ssp-1)
   // just to make certain
   r := movetoanyr(arg1)
   h1!arg1, h2!arg1 := k.ir0+r, i
   // ARG1 now represents the addressof the
   // byte in mode 6 addressible form. i.e index(A0,Dr.L)

   TEST op=s.getword THEN
   $( loadt(k.numb, 0)
      r := movetoanyr(arg1)                      // MOVEQ  #0,Ds
      formea(h1!arg2, h2!arg2)
      // word assignment to a data register
      // does not extend the sign
      geneaea(f.movew, ea.m, ea.d, m.00+r, 0)    // MOVE.W i(A0,Dr.L),Ds
      forgetr(r)
      lose1(k.reg, r)
   $)
   ELSE
   $( LET m, d = 0, 0
      TEST h1!arg2=k.loc | h1!arg2=k.glob THEN   // Store in local or global
      $( formea(h1!arg2, h2!arg2)
         m, d := ea.m, ea.d + 2
         IF m=m.2p DO m := m.5p
         IF m=m.2g DO m := m.5g
      $)
      ELSE m, d := m.00+movetoanycr(arg2), 0

      formea(h1!arg1, h2!arg1)
      // the address in EA.M and EA.D will not use L
      geneaea(f.movew, m, d, ea.m,ea.d)
      forgetvars()
      stack(ssp-2)
   $)
$)
$>WORD

// compile code to move <arg1> to <arg2>
// where <arg2> represents a memory location
AND cgmove() BE
$( LET k, n = h1!arg2, h2!arg2
   LET m, d = -1, 0
   LET cl = class(arg1)

   UNLESS (cl&c.cr)=0 DO
      m := m.00+choosereg(cl&c.regs)
//++ Typo??      m := m.00+choosereg(cl, c.regs)

   IF m=-1 & (cl&c.b)\=0 DO
   $( IF h2!arg1=0 DO
      $( // use CLR instruction
         formea(k, n)
         genea(f.clr, ea.m, ea.d)
         forgetvar(k, n)
         RETURN
      $)
      // otherwise take advantage of MOVEQ
      m := m.00+movetoanycr(arg1)
   $)

   IF m=-1 & (cl&c.m+c.w)\=0 THEN
      UNLESS formea(h1!arg1, h2!arg1) DO
          // provided <arg1> address does not use L
          m, d := ea.m, ea.d

   IF m=-1 DO m := m.00+movetoanyr(arg1)

   formea(k, n)
   geneaea(f.movel, m, d, ea.m, ea.d)
   forgetvar(k, n)
   IF 0<=m<=7 DO remem(m, k, n) // M is D reg direct mode
$)


AND cgstind(op) BE
$( cgrv(op)
   swapargs()
   cgmove()
   stack(ssp-2)
$)


// store the top item of the SS in (K,N)
// K is K.LOC, K.GLOB or K.LAB
AND storein(k, n) BE
$(1 LET b = (h1!arg1=k & h2!arg1=n) -> 1,
            (h1!arg2=k & h2!arg2=n) -> 2, 0
    LET pendop = pendingop

    IF b=0 GOTO gencase

    pendingop := s.none
    SWITCHON pendop INTO

    $(2 DEFAULT:
        gencase: pendingop := pendop
                 cgpendingop()

        CASE s.none:
                 loadt(k, n)
                 swapargs()
                 cgmove()
                 ENDCASE

        CASE s.neg:
        CASE s.not:
                 UNLESS b=1 GOTO gencase
                 formea(k, n)
                 genea((pendop=s.neg -> f.neg, f.not),
                       ea.m, ea.d)
                 forgetvar(k, n)
                 stack(ssp-1)
                 RETURN

        CASE s.plus:
                 IF b=1 DO swapargs()
                 cgdyadic(fns.add, FALSE, TRUE)
                 ENDCASE

        CASE s.minus:
                 UNLESS b=2 GOTO gencase
                 cgdyadic(fns.sub, FALSE, TRUE)
                 ENDCASE

        CASE s.logor:
                 IF b=1 DO swapargs()
                 cgdyadic(fns.or,  FALSE, TRUE)
                 ENDCASE

        CASE s.logand:
                 IF b=1 DO swapargs()
                 cgdyadic(fns.and, FALSE, TRUE)
                 ENDCASE

    $)2
    stack(ssp-2)
$)1


