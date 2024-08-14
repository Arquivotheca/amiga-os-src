
// (C) Copyright 1979 Tripos Research Group
//     University of Cambridge
//     Computer Laboratory
// (C) Copyright 1985 All Rights Reserved METACOMCO plc
//     26 Portland Square
//     Bristol

SECTION "M68CG2"

GET "CGHDR"

// initialise the simulated stack (SS)
LET initstack(n) BE
$( arg2, arg1 := tempv, tempv+3
   ssp := n
   pendingop := s.none
   h1!arg2, h2!arg2, h3!arg2 := k.loc, ssp-2, ssp-2
   h1!arg1, h2!arg1, h3!arg1 := k.loc, ssp-1, ssp-1
   IF maxssp<ssp DO maxssp := ssp
$)


// move simulated stack (SS) pointer to N
AND stack(n) BE
$(1 IF n>=ssp+4 DO
    $( store(0,ssp-1)
       initstack(n)
       RETURN
    $)

    WHILE n>ssp DO loadt(k.loc, ssp)

    UNTIL n=ssp DO
    $( IF arg2=tempv DO
       $( TEST n=ssp-1
          THEN $( ssp := n
                  h1!arg1,h2!arg1 := h1!arg2,h2!arg2
                  h3!arg1 := ssp-1
                  h1!arg2,h2!arg2 := k.loc,ssp-2
                  h3!arg2 := ssp-2
               $)
          ELSE initstack(n)
          RETURN
       $)

       arg1, arg2 := arg1-3, arg2-3
       ssp := ssp-1
    $)
$)1



// store all SS items from A to B in their true
// locations on the stack
AND store(a,b) BE
$( FOR p = tempv TO arg1 BY 3 DO
   $( LET s = h3!p
      IF s>b BREAK
      IF s>=a & h1!p>=k.reg DO storet(p)
   $)
   FOR p = tempv TO arg1 BY 3 DO
   $( LET s = h3!p
      IF s>b RETURN
      IF s>=a DO storet(p)
   $)
$)

$<EXT
// Read string from ocode stream and create new node in the
// lookup table if necessary. Return pointer to node.
AND extlookup() = VALOF
$( LET length = rdn()
   LET v = VEC 256/bytesperword
   LET ptr = extrnsymbols
   LET ename = ?

   FOR i=1 TO length DO v%i := rdn()
   v%0 := length

   // Search existing names
   UNTIL ptr = 0 DO
   $( LET matched = TRUE
      LET p = 0

      ename := @(ptr!e.name)
      UNTIL p > length DO
      $( IF v%p NE ename%p THEN
         $( matched := FALSE
            BREAK
         $)
         p := p+1
      $)

      IF matched THEN RESULTIS ptr

      ptr := ptr!e.link
   $)

   // Create new node
   ptr := newvec( e.name + length/bytesperword )
   ptr!e.link, ptr!e.refs, ptr!e.refstail := extrnsymbols, 0, 0
   extrnsymbols := ptr
   ename := @(ptr!e.name)

   FOR i=0 TO length DO ename%i := v%i

   RESULTIS ptr
$)

AND procext(fn,arg) BE
$( LET esymbol = extlookup()
   // dbg("Ext Symbol: *"%S*"", @(esymbol!e.name))
   fn(arg,esymbol)
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
$>EXT

AND scan() BE

$(1 LET l, m = ?, ?

    IF traceloc>=0 DO
       TEST traceloc<=stvp<=traceloc+20
       THEN debug := 10
       ELSE debug := 0

    IF debug>0 DO dboutput(debug)

    SWITCHON op INTO

 $(sw DEFAULT:     cgerror("BAD OP %N", op)
                   ENDCASE

      CASE 0:      RETURN

      CASE s.debug:debug := rdn() // set the debug level
                   ENDCASE

      CASE s.lp:   loadt(k.loc,  rdn());  ENDCASE
      CASE s.lg:   loadt(k.glob, rdgn()); ENDCASE
      CASE s.ln:   loadt(k.numb, rdn());  ENDCASE
$<EXT
      CASE s.lext:  procext(loadt,k.ext); ENDCASE
      CASE s.llext: procext(loadt,k.lvext); ENDCASE
      CASE s.sext:  procext(storein,k.ext); ENDCASE
$>EXT
      CASE s.nil:  loadt(k.glob,nilglobalnumber); ENDCASE  // **TEMP**

      CASE s.lstr: cgstring(rdn());       ENDCASE

      CASE s.true: loadt(k.numb, TRUE);   ENDCASE
      CASE s.false:loadt(k.numb, FALSE);  ENDCASE

      CASE s.llp:  loadt(k.lvloc, rdn()); ENDCASE
      CASE s.llg:  loadt(k.lvglob,rdgn());ENDCASE

      CASE s.sp:   storein(k.loc, rdn()); ENDCASE
      CASE s.sg:   storein(k.glob,rdgn());ENDCASE
      CASE s.stnil: storein(k.glob,nilglobalnumber); ENDCASE

      CASE s.ll:
      CASE s.lll:
      CASE s.sl: $( LET l = rdl()
                    LET p = llist
                    UNTIL p=0 DO
                    $( IF l=h2!p BREAK
                       p := !p
                    $)
                    IF op=s.sl & p=0 DO
                    $( storein(k.lab, l)
                       ENDCASE
                    $)
                    IF op=s.lll & p=0 DO
                    $( loadt(k.lvlab, l)
                       ENDCASE
                    $)
                    IF op=s.ll TEST p=0
                    THEN $( loadt(k.lab, l)
                            ENDCASE
                         $)
                    ELSE $( loadt(k.lvlabsh, h3!p)
                            ENDCASE
                         $)
                    cgerror("Illegal use of static constant")
                    ENDCASE
                 $)

      CASE s.stindb: cgstind(TRUE); ENDCASE
      CASE s.stind:cgstind(FALSE);             ENDCASE

      CASE s.cdr:  loadt(k.numb,4)  // Simulate LN 4; PLUS ocode seq
                   cgpendingop()
                   pendingop := s.plus  // andthen treat as CAR
      CASE s.car:  cgrv(TRUE); ENDCASE
      CASE s.rv:   cgrv(FALSE);                ENDCASE

      CASE s.mult:CASE s.div:CASE s.rem:
      CASE s.plus:CASE s.minus:
      CASE s.eq: CASE s.ne:
      CASE s.ls:CASE s.gr:CASE s.le:CASE s.ge:
      CASE s.lle: CASE s.lge: CASE s.lls: CASE s.lgr:
      CASE s.lshift:CASE s.rshift:
      CASE s.logand:CASE s.logor:CASE s.eqv:CASE s.neqv:
      CASE s.not:CASE s.neg:CASE s.abs:
      CASE s.fplus: CASE s.fminus: CASE s.fmult: CASE s.fdiv: CASE s.fneg:
      CASE s.feq: CASE s.fne:
      CASE s.fls: CASE s.fgr: CASE s.fle: CASE s.fge:
                   cgpendingop()
                   pendingop := op
                   ENDCASE

      CASE s.endfor:
                   cgpendingop()
                   pendingop := s.le
                   cgcondjump(TRUE, rdl())
                   ENDCASE

      CASE s.jt:
      CASE s.jf:   l := rdl()
                   $( LET nextop = rdn()

                      IF nextop=s.jump DO
                      $( cgcondjump(op=s.jf, rdl())
                         GOTO jump
                      $)
                      cgcondjump(op=s.jt, l)
                      op := nextop
                      LOOP
                   $)

      CASE s.res:  cgpendingop()
                   store(0, ssp-2)
                   movetor(arg1,r1)
                   stack(ssp-1)

      CASE s.jump: cgpendingop()
                   store(0, ssp-1)
                   l := rdl()

      jump:        $( op := rdn() // deal with STACKs
                      UNLESS op=s.stack BREAK
                      stack(rdn())
                   $) REPEAT

                   UNLESS op=s.lab DO
                   $( genb(f.bra, l)
                      incode := FALSE
                      LOOP
                   $)
                   m := rdl()
                   UNLESS l=m DO
                   $( genb(f.bra, l)
                      incode := FALSE
                   $)
                   GOTO lab

      CASE s.blab: // BCPL label (for compat. with FE)
      CASE s.lab:  cgpendingop()
                   store(0, ssp-1)
                   m := rdl()

      lab:         setlab(m)
      // only compile code inside
                   // procedure bodies
                   incode := procstkp>0
                   countflag := profcounting
                   forgetall()
                   ENDCASE


      CASE s.goto: cgpendingop()
                   store(0, ssp-2)
                   TEST h1!arg1=k.lvlabsh
                   THEN genb(f.bra, h2!arg1)
                   ELSE $( movetoa(rl)
                           genea(f.jmp, m.2l, 0)
                        $)
                   incode := FALSE
                   stack(ssp-1)
                   ENDCASE

      CASE s.query:cgpendingop()
                   stack(ssp+1)
                   ENDCASE

      CASE s.stack:cgpendingop()
                   stack(rdn())
                   ENDCASE

      CASE s.store:cgpendingop()
                   store(0, ssp-1)
                   ENDCASE

      CASE s.entry:
                $( LET n = rdn()
                   LET l = rdl()
                   cgentry(n, l)
                   ENDCASE
                $)

      CASE s.save: IF procstkp>=20 DO
                   $( cgerror("PROC STACK OVF")
                      collapse(20)
                   $)
                   procstk!procstkp     := procbase
                   procstk!(procstkp+1) := maxssp
                   procbase := stvp
                   cgsave(rdn())
                   IF stkchking DO
                   $(
                      geneaea( f.movel, m.74, 0, m.00, 0)
                      genea(f.jsr,m.5s,sr.stkchk)
                      procstk!(procstkp+2) := stvp
//                      code2(0)
                      maxssp := ssp
                   $)
                   procstkp := procstkp+3
                   ENDCASE

      CASE s.fnap:
      CASE s.rtap: cgapply(op, rdn())
                   ENDCASE

      CASE s.rtrn:
      CASE s.fnrn: cgreturn(op)
                   incode := FALSE
                   ENDCASE

      CASE s.endproc:
          $( LET n = rdn()
             procstkp := procstkp-3
             IF stkchking  DO
             $( LET p = procstk!(procstkp+2) - 8
                LET bmssp = maxssp << 2   // form byte count
                FOR i = 0 TO 3 DO
                    stv%(p+i) := (@bmssp)%i
             $)
             maxssp   := procstk!(procstkp+1)
             procbase := procstk!procstkp
             IF procstkp=0 DO cgstatics()
             ENDCASE
          $)

      CASE s.rstack:
                   initstack(rdn())
                   loadt(k.reg, r1)
                   ENDCASE

      CASE s.finish:
             $( LET k = ssp
                stack(ssp+3)
                loadt(k.numb, 0)
                loadt(k.glob, gn.stop)
                cgapply(s.rtap, k)
                ENDCASE
             $)

      CASE s.switchon:
            $( LET n = 2 * rdn() + 1
               switchspace := getvec(n)
               IF switchspace = 0 THEN
                 $( cgerror("can't get workspace for SWITCHON")
                collapse(20)
                 $)
               cgswitch(switchspace, n)
               freevec(switchspace)
               switchspace := 0
               ENDCASE
            $)

      CASE s.getbyte:
      CASE s.putbyte:
                   cgbyteap(op)
                   ENDCASE
$<WORD
      CASE s.getword:
      CASE s.putword:
                   cgwordap(op)
                   ENDCASE
$>WORD
      // not fully implemented yet
      CASE s.needs:cgname(s.needs,rdn())
                   ENDCASE

      CASE s.global: cgglobal(rdn())
                     RETURN

      // DATALAB is always immediately followed
      // by either (1) one or more ITEMNs
      //        or (2) one ITEML
      CASE s.datalab: datalabel := rdl()
                      ENDCASE

      // ITEML is always immediately preceeded by
      // a DATALAB
      CASE s.iteml:llist := getblk(llist, datalabel, rdl())
    ENDCASE

      // ITEMN is always immediately preceeded by
      // a DATALAB or an ITEMN
      // CGITEMN sets DATALABEL to zero
      CASE s.itemn:cgitemn(rdn())
                   ENDCASE
 $)sw

    op := rdn()

$)1 REPEAT

