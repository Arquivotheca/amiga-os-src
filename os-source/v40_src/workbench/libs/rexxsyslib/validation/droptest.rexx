/* Validation tests for the DROP instruction */
signal on syntax
message  = "Error in DROP instruction"
continue = 'badnews'
test.0='Not OK';test.1='OK'
rc.0=5;rc.1=0

/* drop a variable */
a='123';b=123
pass = symbol('a') == 'VAR'
drop a b
pass = pass & symbol('a') == 'LIT' & symbol('b') == 'LIT'

/* drop a compound variable */
a.=0;a.1=1;a.2=2
drop a.1
pass = pass & a.1 = 'A.1' & a.2 = 2

/* drop a stem variable */
drop a.
pass = pass & a. == 'A.' & a.2 == 'A.2'

message  = "Missing variable trapped"
continue = next1
drop        /* missing variable */

next1:
signal on syntax
message  = "Invalid variable trapped"
continue = 'next2'
drop 'abc'  /* invalid variable */

next2:
say "DROP instruction" test.pass
exit rc.pass

syntax:
   say message':  RC='rc '"'errortext(rc)'"'
   signal value continue

badnews:
   exit 5
