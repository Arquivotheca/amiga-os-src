/* Tests the SIGNAL ON FAILURE instruction */
trace c
options failat 10
options results
signal on error

do i=0 to 15 by 5
   'quit' i
   say "I="i "RC="rc
   end
exit 5

error:
   say "Trapped ERROR RC=" rc
