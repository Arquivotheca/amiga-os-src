/* Tests the LEAVE instruction */
signal on syntax
message  = "Error in ITERATE instruction"
continue = 'badnews'

test.1=OK;test.0='Not OK'
rc.0=5;rc.1=0
say "Testing the LEAVE instruction"

do i=1 to 5
   if i=3 then leave
   end
test = i == 3
pass = test

do i=1 to 5
   if i=3 then leave i
   end
test = i == 3
pass = pass & test

do i=1 to 5
   do j=1 to i
      do k=j to 5
         if i=2 & k=3 then leave i
         end k
      if j = i-1 then leave
      end j 
   end i
test = i=2 & k=3
pass = pass & test

do i=1 to 5
   interpret 'do j=1 to 5;if j=3 then leave;end'
   end
test = i = 6
pass = pass & test

signal on syntax
message  = 'Bad context trapped'
continue = 'next1'
leave       /* no loop */

next1:
signal on syntax
message  = 'Invalid loop trapped'
continue = 'next2'
do
   leave
   end

next2:
say "LEAVE instruction" test.pass
exit rc.pass

syntax:
   say message':  RC='rc '"'errortext(rc)'"'
   signal value continue

badnews:
   exit 5
