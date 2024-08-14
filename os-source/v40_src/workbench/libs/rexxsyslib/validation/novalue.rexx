/* Tests SIGNAL ON NOVALUE */
/* trace r */
signal on novalue

call func
say "RESULT=" result 

say test
say "No trap"
exit 5

Second:
say NOVALUE OK
exit 0

func:
   signal off novalue
   return OK

novalue:
   say "Trapped Novalue RC=" rc
   signal second
   exit 0
