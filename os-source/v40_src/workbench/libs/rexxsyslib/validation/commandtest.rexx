/* Tests commands and SIGNAL ON ERROR/FAILURE */
test.0='Not OK';test.1='OK'
rc.0=5;rc.1=0
options failat 10 /* set failure level */

'quit 0'
pass = rc = 0

/* make sure NOVALUE doesn't hit RC */
quit rc
pass = pass & rc = 0

signal on error
continue = 'next1'
'quit 5'
test = 0    /* error */

next1:
pass = pass & test & rc = 5
signal on failure
continue = 'next2'
'quit 10'
test = 0

next2:
pass = pass & test & rc = 10

/* Test commands with OPTIONS RESULTS */
options results
result = 'a value'
'quit 0'
pass = pass & symbol(result) == 'LIT'

/* Test command results */
address REXX 'quit.rexx' 'A Value'
pass = pass & result == 'A Value'

/* Test command inhibition */
trace !
'quit 0'
pass = pass & result == 'A Value'

say "Commands" test.pass
exit rc.pass

error:
   say "Trapped ERROR"
   test = 1
   signal value continue

failure:
   say "Trapped FAILURE"
   test = 1
   signal value continue
