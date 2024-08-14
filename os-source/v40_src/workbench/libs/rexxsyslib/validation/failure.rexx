/* Tests the SIGNAL ON FAILURE instruction */
trace c
options results
signal on failure
options failat 10

do i=0 to 15 by 5
   'quit' i
   say "I="i "RC="rc
   end
exit 5

failure:
   say "Trapped FAILURE RC=" rc
