/* Examples of the IF instruction */
signal on syntax
message  = "Error in IF instruction"
continue = 'badnews'

test.0='NotOK';test.1='OK'
rc.0=5;rc.1=0

if 2 >= 5
   then
      test = 0
   /* a comment line */
   else test = 1
pass = test

if 2 >= 5 then
   test = 0
   /* a comment line */
   else test = 1
say "Inline THEN" test.test
pass = pass & test

test = 0
if 2 > 1 then  /* first test */

   if 3 > 2 then test = 1
            else nop

pass = pass & test

if 2 < 1
   then if 3 > 2 then test = 0
                 else nop
   else test = 1
pass = pass & test

if 0 then do for 2
   test = 0
   end
   else test = 1
pass = pass & test

sum = 0
if 1 then
   do for 2
      sum = sum + 1
      end
   else nop
test = sum == 2
pass = pass & test

signal on syntax
message  = 'Missing test trapped'
continue = 'next1'
if
   then nop

next1:
signal on syntax
message  = 'Missing THEN trapped'
continue = 'next2'
if 1
   else nop

next3:
say "IF instruction" test.pass
exit rc.pass

syntax:
   say message':  RC='rc '"'errortext(rc)'"'
   signal value continue

badnews:
   say "Error in IF instruction"
   exit 5

next2:
signal on syntax
message  = 'Incomplete ELSE trapped'
continue = 'next3'
if 1 then
   nop
   else
