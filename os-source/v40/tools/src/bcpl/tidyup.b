
// (C) Copyright 1979 Tripos Research Group
//     University of Cambridge
//     Computer Laboratory
// (C) Copyright 1985 All Rights Reserved METACOMCO plc
//     26 Portland Square
//     Bristol

SECTION "TIDYUP"

GET "LIBHDR"
GET "BCPHDR"
$<TRIPOS
GET "IOHDR"
$>TRIPOS
$<MTRIPOS
GET "IOHDR"
$>MTRIPOS

LET tidyup(rc) BE
$(
   IF insyn DO       // only if in syn
   $(
      LET p = getp   // dispose of get stack if it exists
      UNTIL p = 0 DO
      $(
         closefile( @(p!2), selectinput, endread )
         p := p!4
      $)
   $)
   myfreevec(@workvec)

$<TRIPOS'
$<MTRIPOS'
   UNLESS ocodestream = 0 DO endstream(ocodestream)
$>MTRIPOS'
$>TRIPOS'
$<TRIPOS
   UNLESS ocodestream = 0 DO
   $( TEST ocodestream!scb.id = id.inscb THEN
        closefile(@ocodestream,selectinput,endread)
      ELSE
        closefile(@ocodestream,selectoutput,endwrite)
   $)
$>TRIPOS
$<MTRIPOS
   UNLESS ocodestream = 0 DO
   $( TEST ocodestream!scb.id = id.inscb THEN
        closefile(@ocodestream,selectinput,endread)
      ELSE
        closefile(@ocodestream,selectoutput,endwrite)
   $)
$>MTRIPOS

   closefile(@sourcestream,selectinput,endread)
   closefile(@codestream,selectoutput,endwrite)
   unless (name.codestream = 0) | (rc = 0) do
      deleteobj(name.codestream)
   myfreevec(@fromfile)
   myfreevec(@ocodefile)
$<UNIX
   if rc > 0 rc := 1
$>UNIX
   stop(rc)
$)

AND closefile(p.stream,fn1,fn2) BE UNLESS !p.stream = 0 DO
$(
   LET temp = !p.stream
   !p.stream := 0
   fn1(temp)
   fn2()
$)

AND myfreevec(p.ptr) BE UNLESS !p.ptr = 0 DO
$(
   LET temp = !p.ptr
   !p.ptr := 0
   freevec(temp)
$)

