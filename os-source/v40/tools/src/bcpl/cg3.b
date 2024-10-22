
// (C) Copyright 1979 Tripos Research Group
//     University of Cambridge
//     Computer Laboratory
// (C) Copyright 1985 All Rights Reserved METACOMCO plc
//     26 Portland Square
//     Bristol

SECTION "M68CG3"

GET "CGHDR"


// compiles code to deal with any pending op
LET cgpendingop() BE

$(1 LET pendop = pendingop
    LET f, r= ?, ?

    pendingop := s.none

    SWITCHON pendop INTO
    $(sw DEFAULT:cgerror("BAD PNDOP %N",pendop)
                 RETURN

         CASE s.abs:
                 r := movetoanyr(arg1)
                 loadt(k.numb, 0)
                 f := cgcmp(f.bge,cgdyadic)
                 gen(f+2)       // BGE   *+4
                 genr(f.neg, r) // NEG.L Dr
                 lose1(k.reg, r)
                 forgetr(r)
                 RETURN

         CASE s.neg:
         CASE s.not:
                 r := movetoanyr(arg1)
                 genr((pendop=s.neg->f.neg,f.not), r)
                 forgetr(r)
         CASE s.none:
                 RETURN


         CASE s.eq: CASE s.ne:
         CASE s.ls: CASE s.gr:
         CASE s.le: CASE s.ge:
         CASE s.feq: CASE s.fne:
         CASE s.fls: CASE s.fgr:
         CASE s.fle: CASE s.fge:
         CASE s.lle: CASE s.lge:
         CASE s.lls: CASE s.lgr:
                 // comparisons are ARG2 <op> ARG1
              $( LET arg3 = arg2
                 loadt(h1!arg1, h2!arg1)
                 h1!arg2, h2!arg2 := h1!arg3, h2!arg3
                 h1!arg3, h2!arg3 := k.numb, FALSE
                 // select and initialise a register
                 // for the result
                 r := movetoanyr(arg3)
                 f := cgcmp(compbfn(condbfn(pendop)),cgdyadic)
                 gen(f+2)       // Bcc   *+4
                 genr(f.not, r) // NOT.L Dr
                 forgetr(r)
            stack(ssp-2)
                 RETURN
              $)

         CASE s.eqv:
         CASE s.neqv:
                 cgdyadic(fns.eor, TRUE, FALSE)
                 IF pendop=s.eqv DO
                 $( // just in case its not already in
                    // a register
                    r := movetoanyr(arg2)
                    genr(f.not, r)
                    forgetr(r)
                 $)
                 ENDCASE

         CASE s.minus:
                 cgdyadic(fns.sub, FALSE, FALSE)
                 ENDCASE

         CASE s.plus:
                 cgdyadic(fns.add, TRUE, FALSE)
                 ENDCASE

// Floating point operations added 8/12/82 TJK

         CASE s.fminus: CASE s.fdiv:
                 cgfloatdyadic((pendop=s.fminus->f.fsub,f.fdiv), FALSE)
                 ENDCASE

         CASE s.fplus: CASE s.fmult:
                cgfloatdyadic((pendop=s.fplus->f.fadd,f.fmul), TRUE)
                ENDCASE

         CASE s.fneg:
                 movetor(arg1,r1)
                 genfpcall( f.fneg )
                 forgetr(r1)
                 RETURN

         CASE s.div:
         CASE s.rem:
         CASE s.mult:
         TEST inlinemult & pendop = s.mult
         THEN $( inline.mult(); ENDCASE $)
         ELSE
$<MDFIX
//****************************************************
//start *** temporary fix until MUL DIV S/Rs are ready
         $( LET k = ssp - 2
            LET gn = gn.mul
            IF pendop=s.div DO gn := gn.div
            IF pendop=s.rem DO gn := gn.rem
            store(0, k-1)
            movetor(arg1, r2)     // left operand
            movetor(arg2, r1)     // right operand
            loadt(k.numb, 4*k)    // stack frame size
            movetor(arg1, r0)
            loadt(k.glob, gn)     // MUL DIV or REM function
            movetoa(rb)
            genea(f.jsr, m.2s, 0) // call the function
            stack(k)              // reset the stack
            loadt(k.reg, r1)
            forgetall()
            RETURN
         $)
//end ***** temporary fix until MUL DIV S/Rs are ready
//****************************************************
$>MDFIX
$<MDFIX'
         $(
                 LET k = ssp - 2
                 store(0,k-1)
                 movetor(arg1, r2)     // left op
                 movetor(arg2, r1)     // right op
                 genea(f.jsr, m.5s,    // call function
                       (pendop=s.mult -> sr.mul, sr.div))
                 stack(k)
                 loadt( k.reg, (pendop = s.rem -> r2, r1))
                 forgetall()
                 RETURN
         $)
$>MDFIX'

         CASE s.logor:
                 cgdyadic(fns.or, TRUE, FALSE)
                 ENDCASE

         CASE s.logand:
                 cgdyadic(fns.and, TRUE, FALSE)
                 ENDCASE

         CASE s.lshift:
         CASE s.rshift:
                 r := movetoanyr(arg2)
                 TEST h1!arg1=k.numb & 1<=h2!arg1<=8
                 THEN genshkr((pendop=s.lshift ->
                               f.lslkr,f.lsrkr), h2!arg1, r)
                 ELSE $( LET s = movetoanyr(arg1)
                         genrr((pendop=s.lshift ->
                                f.lslrr, f.lsrrr), s, r)
                      $)
                 forgetr(r)
                 ENDCASE

    $)sw

    stack(ssp-1)
    IF debug>6 DO dboutput(3)
$)1


AND inline.mult() BE
$(
   MANIFEST $(
      f.swap      = #X4840
      f.mulu      = #XC0C0
      f.addw      = #XD040
      f.clrw      = #X4240
      f.add       = #XD080
      f.lsll      = #XE188
   $)
   LET reg1, reg2, reg3, reg4 = ?,?,?,?
   LET k = ssp-2
   LET k1, n1, k2, n2 = ?,?,?,?

   //writef("Mult at %n arg1(%o2 %n) arg2(%o2 %n)*N",stvp,arg1!h1,arg1!h2,arg2!h1,arg2!h2)

   // IF we have a const * var, make sure arg1 is the constant
   IF arg2!h1 = k.numb DO swapargs()

   IF arg1!h1 = k.numb
   THEN $(
      LET shifts = 0
      LET const = arg1 ! h2
      // here arg1 is a constant, if it fits 16 bits or is a power of 2
      // we can compile a more efficient multiplication for this...

      // multiply by zero, result is zero i.e arg1
      IF const = 0
      DO $(
         //writef("zero*N")
         swapargs()
         forgetvar( arg2!h1, arg2!h2)
         RETURN
      $)

      // multiply by 1, result is arg2
      IF const = 1
      DO $(
         //writef("one*N")
         forgetvar(arg2!h1, arg2!h2)
         RETURN
      $)

      // now check for a power of 2 ( very common )
      FOR i = 0 TO 31 DO   // search for ls bit
         IF [(const>>i) & 1] = 1 $( shifts := i; BREAK $)
      FOR i = shifts+1 TO 31 DO // see if any more bits
         IF [(const>>i) & 1] = 1 $( shifts := -1; BREAK $)

      IF 1 <= shifts <= 8
      THEN $(  // power of 2 <= 256 - can use immediate shift count
         reg1 := movetoanyr( arg2 )
         //writef("short shift %n*N",reg1)
         gen( f.lsll + ((shifts & 7) << 9) + reg1 )
         forgetr( reg1 )
         RETURN
      $)
   
      IF shifts >= 9
      THEN $(  // power of 2 > 256 - must use register shift count
         reg1 := movetoanyr( arg2 )
         reg2 := findreg(0, reg1)
         //writef("long shift %n %n*N",reg1,reg2)
         geneaea(f.movel, m.74, shifts, m.00+reg2, 0)
         gen(f.lsll + #X0020 + (reg2 << 9) + reg1 )
         forgetr( reg1 )
         RETURN
      $)

      // now see if it fits 16 bits 
      IF 3 <= const <= 32767
      THEN $(
         reg1 := movetoanyr( arg2 )
         reg2 := movetoanyr( arg1 )
         reg3 := findreg(1, reg1, reg2)
         //writef("short %n %n %n*N",reg1,reg2,reg3)
         geneaea(f.movew, m.00+reg2, 0, m.00+reg3, 0 )
         gen( f.swap + reg2 )
         gen( f.mulu + ( reg2 << 9) + reg1 )
         gen( f.mulu + ( reg1 << 9) + reg3 )
         gen( f.swap + reg2 )
         genea( f.clrw, m.00+reg2, 0 )
         gen( f.add + (reg1 << 9) + reg2)
         forgetr( reg2 )
         forgetr( reg1 )
         RETURN
      $)
      // otherwise drop through to full multiply
   $)
   reg1 := movetoanyr( arg2 )
   reg2 := movetoanyr( arg1 )
   reg3 := findreg( 1, reg1, reg2)
   reg4 := findreg( 2, reg1, reg2, reg3)
   //writef("long %n %n %n %n*N",reg1,reg2,reg3,reg4)
   geneaea(f.movew, m.00|reg1, 0, m.00|reg3, 0)
   geneaea(f.movew, m.00|reg2, 0, m.00|reg4, 0)
   gen( f.swap + reg2 )
   gen( f.swap + reg1 )
   gen( f.mulu | ( reg2 << 9) | reg3 )
   gen( f.mulu | ( reg1 << 9) | reg4 )
   gen( f.mulu | ( reg3 << 9) | reg4 )
   genrea( f.addw, reg1, m.00|reg2, 0 )
   gen( f.swap | reg1 )
   genea( f.clrw, m.00|reg1, 0)
   gen( f.add | (reg1 << 9) | reg3 )
   forgetr( reg2 )
   forgetr( reg1 )
$)

AND findreg(upb, a, b, c) = VALOF
$(
   LET reglist = @a
   //writef("Findreg: %n %n %n %n*N",upb,a,b,c)

   // first look for an empty register
   FOR r = r0 TO r7 DO
   $(
      UNLESS isfree( r ) LOOP
      FOR i = 0 TO upb DO IF r = reglist!i GOTO loop1
      freereg(r)   // just in case
      forgetr(r)
      RESULTIS r
loop1:
   $)

   // no free registers - evict something from a used one
   FOR r = r0 TO r7 DO
   $(
      FOR i = 0 TO upb DO IF r = reglist!i GOTO loop2
      //writef("Found reg %n*N",r)
      freereg(r)
      forgetr(r)
      RESULTIS r
loop2:
   $)
   selectoutput(sysout)
   writes("Internal error in *'findreg*'*N")
   tidyup(20)
$)

