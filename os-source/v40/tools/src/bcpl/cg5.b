
// (C) Copyright 1979 Tripos Research Group
//     University of Cambridge
//     Computer Laboratory
// (C) Copyright 1985 All Rights Reserved METACOMCO plc
//     26 Portland Square
//     Bristol

SECTION "M68CG5"

GET "CGHDR"

LET cgrv(op) BE

$(1 LET r = 0

    IF pendingop=s.minus & h1!arg1=k.numb DO
             pendingop, h2!arg1 := s.plus, -h2!arg1

    IF pendingop=s.plus &
        (h1!arg1=k.numb | h1!arg2=k.numb)

    THEN $( LET arg = arg2
            LET n = h2!arg1
            IF h1!arg2=k.numb DO arg, n := arg1, h2!arg2
            UNLESS op DO n := 4*n
            IF -128<=n<=127 DO
            $( pendingop := s.none
               TEST op THEN
                  r := movetoanyr(arg)
               ELSE
                  r := movetoanyrsh(arg)
               lose1(k.ir0+r, n)
               RETURN
            $)
         $)

    cgpendingop()
    TEST op THEN
       r := movetoanyr(arg1)
    ELSE
       r := movetoanyrsh(arg1)
    h1!arg1, h2!arg1 := k.ir0+r, 0
$)1


AND cgglobal(n) BE
$( cgstatics()
   code2(0)
   FOR i = 1 TO n DO
   $( code2(rdgn())
      code2(labv!rdl())
   $)
   code2(maxgn)
$)


AND cgentry(n,l) BE
$( cnop()
   cgname(s.entry,n)
   setlab(l)
   forgetall()
   incode := TRUE
   countflag := callcounting
$)


AND cgsave(n) BE
$( FOR r = r1 TO r4 DO
   $( LET s = 3+r-r1
      IF s>=n BREAK
      remem(r, k.loc, s)
   $)

   initstack(n)
$)
/*
AND dbg(s,a,b,c,d) BE
$( LET o= output()
   selectoutput(sysout)
   writef(s,a,b,c,d)
   newline()
   selectoutput(o)
$)
*/

// function or routine call
AND cgapply(op,k) BE
$( LET sa1 = k+3
   LET sa4 = k+6

$<EXT
   IF h1!arg1 = k.ext THEN
   $( cgextapply(op,k)
      RETURN
   $)
$>EXT

   cgpendingop()

   // store args 5,6,...
   store(sa4+1, ssp-2)

   // now deal with non-args
   FOR t = tempv TO arg2 BY 3 DO
   $( IF h3!t>=k BREAK
      IF h1!t>=k.reg DO storet(t)
   $)

   // move args 1-4 to arg registers
   FOR t = arg2 TO tempv BY -3 DO
   $( LET s = h3!t
      LET r = s-k-2
      IF s<sa1 BREAK
      IF s<=sa4 & isfree(r) DO movetor(t,r)
   $)
   FOR t = arg2 TO tempv BY -3 DO
   $( LET s = h3!t
      LET r = s-k-2
      IF s<sa1 BREAK
      IF s<=sa4 DO movetor(t,r)
   $)

   // deal with args not in SS
   FOR s = sa1 TO sa4 DO
   $( LET r = s-k-2
      IF s>=h3!tempv BREAK
      IF regusedby(arg1)=r DO movetor(arg1,r7)
      loadt(k.loc,s)
      movetor(arg1,r)
      stack(ssp-1)
   $)

   loadt(k.numb, 4*k)
   movetor(arg1, r0) // put the stack inc in R0
   stack(ssp-1)

   TEST globexternals & h1!arg1 = k.glob & h2!arg1 >= extlowbound THEN
   $( addglobexternalref(stvp+2,h2!arg1)
      arg1!h2 := 0
      // Ensure we generate  movea.l 0(A2),A4
      geneaea(f.movel, m.5g, 0, m.10+4, 0)
   $)
   ELSE
      movetoa(rb)      // MOVE <arg1>,B

   genea(f.jsr, m.2s, 0)  // JSR (S)
   forgetall()
   stack(k)
   IF op=s.fnap DO loadt(k.reg,r1)
$)

$<EXT
// move a SS item A onto the SP stack
AND movetosp(a) BE
$( LET k, n = h1!a, h2!a
   LET cl,r = ?,?

   cl := class(a)
   // dbg("MOVETOSP: cl = %N", cl)

   IF cl=0 SWITCHON h1!a INTO
   $( CASE k.lvlocsh:
      CASE k.lvloc: n := n-3
      CASE k.lvglobsh:
      CASE k.lvglob:n := 4*n // convert to byte address
                 $( LET oldn = H2!a
                    LET ms = m.10 + (k&7)  // (P) or (G)
                    TEST n=0 THEN
                       r := movetoanyr(a)
                    ELSE
                    $( h1!a, h2!a := k.numb, n
                       r := movetoanyr(a)
                       // compile   ADD P,Dr  or   ADD G,Dr
                       genrea(fns.add!ft.mr, r, ms, 0)
                    $)
                    n := oldn  // Restore n for remem
                 $)
        shret:   IF (k&k.sh)=0 DO
                     genshkr(f.lsrkr, 2, r)
                 geneaea(f.movel,m.00+r,0,m.4sp,0)
                 GOTO ret

      CASE k.lvlabsh:
      CASE k.lvlab: r := movetoanyr(a)
                    geneaea(f.movel,m.00+r,0,m.4sp,0)
                    GOTO ret

      CASE k.locsh:
      CASE k.globsh:
      CASE k.labsh: h1!a := h1!a - k.sh
                    r := movetoanyr(a)
                    genshkr(f.lslkr, 2, r)
                    geneaea(f.movel, m.00+r, 0, m.4sp, 0)
                    GOTO ret

      CASE k.lvext: formea(k.lvext,n)
                    genrea(f.lea, rl, ea.m, ea.d)
                    numbinl := 0  // value in L unknown
                    geneaea(f.movel, m.1l, 0, m.4sp, 0)
                    GOTO exit

      CASE k.ext:   formea(k.ext,n)
                    geneaea(f.movel, ea.m, ea.d, m.4sp, 0)
                    GOTO exit

      DEFAULT:      bug(19)

   $)

   UNLESS (cl & c.cr) = 0 DO // value in a register
   $( LET s = choosereg(cl & c.regs)
      geneaea(f.movel, m.00+s, 0, m.4sp, 0)
      GOTO exit
   $)

   formea(k, n)
   geneaea(f.movel, ea.m, ea.d, m.4sp, 0)
   GOTO exit

ret:
   forgetr(r)
   remem(r, k, n)
   h1!a, h2!a := k.reg, r
exit:
   RETURN
$)

// function or routine call
AND cgextapply(op,k) BE
$( LET sa1 = k+3
   LET sa4 = k+6
   LET argcount = 0

   // dbg("CGEXTAPPLY: *"%S*"", @((h2!arg1)!e.name))

   cgpendingop()

   // First Save P which may get corrupted
   gen(#X2F09)     // MOVE.L  A1,-(SP)

   // store args 5,6,...
   FOR p = arg1 TO tempv BY -3 DO
      IF ssp-2>= h3!p >=sa4+1 DO
      $( argcount := argcount+1
         movetosp(p)
      $)

   // move args 1-4 onto SP stack
   FOR t = arg2 TO tempv BY -3 DO
   $( LET s = h3!t
      IF s<sa1 BREAK
      IF s<=sa4 THEN
      $( argcount := argcount+1
         movetosp(t)
      $)
   $)

   // Save non-args to avoid corruption
   FOR t = tempv TO arg2 BY 3 DO
   $( IF h3!t>=k BREAK
      IF h1!t>=k.reg DO storet(t)
   $)

   // deal with args not in SS
   FOR s = sa1 TO sa4 DO
   $( LET r = s-k-2
      IF s>=h3!tempv BREAK
      loadt(k.loc,s)
      movetosp(arg1)
      argcount := argcount+1
      stack(ssp-1)
   $)

   formea(arg1!h1,arg1!h2)
   genea(f.jsr, ea.m, ea.d)  // JSR external
   // Pop arguments from stack
   UNLESS argcount=0 DO
     TEST argcount<=2 THEN
        gen(#X508F + (((argcount<<2) REM 8) << 9))  // ADDQ.L #n,SP
     ELSE
     $( gen(#X4FEF)           // LEA.L  n(SP),SP
        gen(argcount<<2)
     $)
   gen(#X225F)               // MOVE.L (SP)+,A1
   gen(#X91C8)               // SUBA.L A0,A0

   forgetall()
   stack(k)
   IF op=s.fnap THEN
   $( gen(#X2200)            // MOVE.L D0,D1
      loadt(k.reg,r1)
   $)
$)
$>EXT

AND cgreturn(op) BE
$( cgpendingop()
   IF op=s.fnrn DO
   $( movetor(arg1,r1)
      stack(ssp-1)
   $)
$<JUMPS
   IF jumpopt & [stvp = lastlabpos]       // i.e. if any jumps come here
   DO $(
      LET p = rlist
      //writef("labelled return*N")
      UNTIL p = 0 DO
      $( // see whether ref is here and is unconditional branch
         IF [p ! h3 = lastlab] & [stv%(p!h2 - 2) = (f.bra >> 8)]
         DO $(
            //writef("Ret: op = %X2 pos = %X4*N",stv%(p!h2-2), p!h2-2)
            setword(f.jmp | m.2r, p!h2 - 2)   // JMP (R) in place of branch
            setword(f.nop, p!h2)              // NOP to make it tidy
         $)
         p := !p
      $)
   $)
$>JUMPS
   genea(f.jmp, m.2r, 0)     // JMP (R)
   initstack(ssp)
$)


// used for OCODE operators JT and JF
AND cgcondjump(b,l) BE
$(1 LET bfn = condbfn(pendingop)
    LET fpcmp = pendingop >= s.feq   // TRUE if floating point comparison
    IF bfn=0 DO
    $( cgpendingop()
       loadt(k.numb,0)
       bfn := f.bne
    $)
    pendingop := s.none
    store(0,ssp-3)
    UNLESS b DO bfn := compbfn(bfn)
    // Check for floating point
    bfn := cgcmp(bfn, (fpcmp -> cgfloatdyadic, cgdyadic))
    genb(bfn,l)
    stack(ssp-2)
    countflag := profcounting
$)1


// Compile code to set the condition code to reflect
// the result of <arg2> rel <arg1>.

AND cgcmp(f,fun) =
    fun(fns.cmp, TRUE, TRUE) = notswapped -> f,
    f=f.blt -> f.bgt,
    f=f.bgt -> f.blt,
    f=f.ble -> f.bge,
    f=f.bge -> f.ble,
    f=f.blle -> f.blge,
    f=f.blge -> f.blle,
    f=f.blls -> f.blgr,
    f=f.blgr -> f.blls,
    f


//***** Used to be SECTION cg6 ************

// compiles code for SWITCHON
// N = no. of cases
// D = default label
AND cgswitch(v,m) BE
$(1 LET n = m/2
LET d = rdl()
    casek, casel := v-1, v+n-1

    // read and sort (K,L) pairs
    FOR i = 1 TO n DO
    $( LET a = rdn()
       LET l = rdl()
       LET j = i-1
       UNTIL j=0 DO
       $( IF a > casek!j BREAK
          casek!(j+1) := casek!j
          casel!(j+1) := casel!j
          j := j - 1
       $)
       casek!(j+1), casel!(j+1) := a, l
    $)

    cgpendingop()
    store(0, ssp-2)
    movetor(arg1,r1)

    // care with overflow !
    TEST 2*n-6 > casek!n/2-casek!1/2

    THEN lswitch(1, n, d)

    ELSE $( bswitch(1, n, d)

            genb(f.bra, d)
         $)

    stack(ssp-1)
$)1


// binary switch
AND bswitch(p, q, d) BE TEST q-p>6

      THEN $( LET m = nextparam()
              LET t = (p+q)/2
              loadt(k.numb,casek!t)
              genb(cgcmp(f.bge,cgdyadic), m)
              stack(ssp-1)
              bswitch(p, t-1, d)
              genb(f.bra,d)
              setlab(m)
              forgetall()
              incode := TRUE
              genb(f.beq,casel!t)
              bswitch(t+1, q, d)
           $)

      ELSE FOR i = p TO q DO
           $( loadt(k.numb,casek!i)
              genb(cgcmp(f.beq,cgdyadic),casel!i)
              stack(ssp-1)
           $)



// label vector switch
AND lswitch(p,q,d) BE
$(1 LET l = nextparam()
    LET dl = labv!d

    loadt(k.numb,casek!p)
    cgdyadic(fns.sub, FALSE, FALSE)
    genb(f.blt,d)
    stack(ssp-1)

    loadt(k.numb,casek!q-casek!p)
    genb(cgcmp(f.bgt,cgdyadic),d)
    stack(ssp-1)

    genshkr(f.lslkr,1,r1)
    geneaea(f.movew,m.73,extd(r1,6),m.1l,0) // MOVE.W 6(PC,R1),L
    genea(f.jmp, m.6b, exta(rl,0))          // JMP    0(B,L)
    incode := FALSE
    // now compile the label vector table in-line
    IF dl=-1 DO dl := stvp + 2 * (casek!q-casek!p+1)
    FOR k=casek!p TO casek!q TEST casek!p=k
        THEN $( code(labv!(casel!p)-procbase)
                p := p+1
             $)
        ELSE code(dl-procbase)
$)1


AND condbfn(op) = VALOF SWITCHON op INTO
$( CASE s.eq: CASE s.feq:  RESULTIS f.beq
   CASE s.ne: CASE s.fne:  RESULTIS f.bne
   CASE s.gr: CASE s.fgr:  RESULTIS f.bgt
   CASE s.le: CASE s.fle:  RESULTIS f.ble
   CASE s.ge: CASE s.fge:  RESULTIS f.bge
   CASE s.ls: CASE s.fls:  RESULTIS f.blt
   CASE s.lle: RESULTIS f.blle
   CASE s.lge: RESULTIS f.blge
   CASE s.lls: RESULTIS f.blls
   CASE s.lgr: RESULTIS f.blgr
   DEFAULT:    RESULTIS 0
$)

AND compbfn(bfn) = bfn=f.beq -> f.bne,
                   bfn=f.bne -> f.beq,
                   bfn=f.blt -> f.bge,
                   bfn=f.bge -> f.blt,
                   bfn=f.bgt -> f.ble,
                   bfn=f.ble -> f.bgt,
                   bfn=f.blls -> f.blge,
                   bfn=f.blge -> f.blls,
                   bfn=f.blle -> f.blgr,
                   bfn=f.blgr -> f.blle,
                   bug(4)

AND genb(bfn, l) BE IF incode DO
$( LET a = labv!l
$<JUMPS
   LET labelled = [ stvp = lastlabpos]
   LET retdone  = FALSE
$>JUMPS

   TEST a<0

   // label is unset?
   THEN $(
           //writef("Genb: pos = %x4, bfn = %X4, ref to lab %n*N",stvp, bfn, l)
           gen(bfn)   // compile 2 word branch instruction
           rlist := getblk(rlist, stvp, l) // make ref to L
           code(-stvp)
        $)

   // no, the label was set
   ELSE $(
$<JUMPS
      IF jumpopt DO
      $(jopt
      LET instop = (stv%a)<<8     // the operand of the destination instruction
      TEST (instop & #XF000) = f.bra     // is destination a branch?
      THEN $(bra
         LET dest = stv%(a+1)     // short branch hop, zero if long
         IF (dest & #X80) \= 0 DO dest := dest | #XFF00  // sign extend
         IF dest = 0 DO dest := (stv%(a+2)<<8) + stv%(a+3)  // form long offset
         IF (dest & #X8000) \= 0 DO dest := dest | #XFFFF0000  // sign extend
         dest := dest + a + 2  // form byte address of dest
         IF instop = f.bra | instop = bfn  // if BRA or same condition
         DO $(
            //writef("To branch: pos = %X4, op = %x4, dest = %X8*N",a, instop, dest)
            TEST dest = 0     // dest will be zero if branch not bound yet
            THEN $(           // if un-bound, install a reference to the label
               LET p = rlist
               //writef("unbound*N")
               gen(bfn)       // a 2 word jump
               UNTIL p = 0 DO // there must be a ref for the branch
                  TEST p!h2 = a+2 // if this is it...
                  THEN $(
                     rlist := getblk(rlist, stvp, p!h3)
                     code(-stvp)
                     a := -1   // flag jump as unbound
                     //writef("ref to lab %n, at %X4*N",p!h3, stvp-2)
                     GOTO done   // a GOTO in the compiler !!!
                  $)
                  ELSE p := !p
               //writef("No ref found!!*N")
            $)
            ELSE a := dest    // go to destination of branch
         $)
      $)bra
      ELSE IF [bfn = f.bra] & [instop = #X4E00] & [stv%(a+1) = #XD6]
      THEN $(ret               // BRA to return
         //writef("return*N")
         genea(f.jmp, m.2r, 0)  // put out a return instruction here
         retdone := TRUE        // we have generated a return
         GOTO done          // avoid next bit
      $)ret
      // ELSE nothing special
      $)jopt
$>JUMPS

      //writef("Genb: pos = %X4, bfn = %X4, dest = %X4*N",stvp,bfn,a)
      TEST stvp-a > 127
      //  back jump too far for short branch
      THEN $( gen(bfn)   // compile 2 word branch
              code(a-stvp)
           $)
      // it can be a short backward jump
      ELSE gen(bfn|(a-stvp-2 & #XFF))
   $)

$<JUMPS
done:
   IF jumpopt & labelled  DO // is this instruction labelled ?
   $(
      LET p = rlist
      //writef("labelled*N")
      UNTIL p = 0 DO  // check for any forward jumps to here
      $(
         IF lastlab = p!h3     // if this is reference to the last label set
         DO $(
            LET instop = stv%(p!h2 - 2)<<8  // get op of jump
            //writef("op = %X4, pos = %X4*N",instop, (p!h2)-2)
            TEST retdone & instop = f.bra  // if unconditional bra to return
            THEN $(
               //writef("replacing branch with return*N")
               setword(f.jmp | m.2r, p!h2 - 2)   // JMP (R) in place of bra
               setword(f.nop, p!h2)              // NOP to make it tidy
            $)
            ELSE $(
               IF [instop = bfn] |                // if source is same condition
                  [ [(instop&#XF000) = f.bra] & [bfn = f.bra] ] // or Bcc to BRA
               THEN $(                               // we must do something
                  TEST a < 0   // if forward ref
                  THEN $(
                     rlist := getblk(rlist, p!h2, l) // make ref for other jump
                     setword(-(p!h2), p!h2)
                     //writef("adding ref: pos = %X4, lab = %N*N",p!h2,l)
                  $)
                  ELSE $(    // change branch to our destination
                     setword(a - p!h2, p!h2)
                     //writef("setting branch: to %n, at %X4*N",a-p!h2,p!h2)
                  $)
               $)
            $)
         $)
         p := !p
      $)
   $)
$>JUMPS

   IF bfn=f.bra DO incode := FALSE
$)

AND setword(val, addr) BE
$(
      stv%addr     := val>>8
      stv%(addr+1) := val
$)

