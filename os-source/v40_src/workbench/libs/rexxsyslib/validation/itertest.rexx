/* Test for the ITERATE instruction */
signal on syntax
message  = "Error in ITERATE instruction"
continue = 'badnews'

test.1=OK;test.0='Not OK'
rc.0=5;rc.1=0
say "Testing the ITERATE instruction"

test = 1
do i=1 to 5
   if i=3 then iterate
   test = test & i ~= 3
   end
say "Loop test" test.test
pass = test

signal on syntax
message  = 'Bad context trapped'
continue = 'next1'
iterate     /* no loop */

next1:
signal on syntax
message  = 'Invalid loop trapped'
continue = 'next2'
do
   iterate
   end

next2:
say "ITERATE instruction" test.pass
exit rc.pass

syntax:
   say message':  RC='rc '"'errortext(rc)'"'
   signal value continue

badnews:
   exit 5
