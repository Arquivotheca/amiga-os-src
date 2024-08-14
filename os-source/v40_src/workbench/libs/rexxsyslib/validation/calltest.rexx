/* Validation tests for the CALL instruction */
signal on syntax
message  = "Error in CALL instruction"
continue = 'badnews'

test.0='Not OK';test.1='OK'
rc.0=5;rc.1=0

call test1 'one','two'
pass = result

/* alternative syntax */
call test1('one','two')
pass = pass & result

call test2
pass = pass & result

/* attempt unknown function */
message  = "Unknown function trapped"
continue = next1
call test3

say "Error not trapped"
pass = 0

next1:
say "CALL instruction" test.pass
exit

syntax:
   say message
   signal value continue

badnews:
   exit 5

test1:
   return arg(1) == 'one' & arg(2) == 'two' & arg() = 2

test2:
   return arg() = 0
