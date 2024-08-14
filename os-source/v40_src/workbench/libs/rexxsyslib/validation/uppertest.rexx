/* Validation tests for the upper instruction */
signal on syntax
message  = "Error in UPPER instruction"
continue = 'badnews'

test.0='Not OK';test.1='OK'
rc.0=5;rc.1=0
say "Testing UPPER instruction"

a='aBc';b = 'bbb'
upper a b
test = a == 'ABC' & b == 'BBB'
say "Variable value test" test.test
pass = test

/* test on literal values */
drop a
upper a
test = symbol('a') == 'LIT'
say "Literal value test" test.test
pass = pass & test

/* test with compound variables */
drop abc. def.
abc.1='one';abc.2='two'
def. = 'stem';def.1='leaf'
upper abc. def. abc.1 def.1
test =        symbol('abc.') == 'LIT' & abc.1 == 'ONE'
test = test & def. == 'STEM'          & def.1 == 'STEM'
say "Compound variable test" test.test
pass = pass & test

/* pathological case: uninitialized compound variable */
index = 'abc'
upper abc.index
test = abc.index == 'ABC.abc'
say "Uninitialized compound variable test" test.test
pass = pass & test

signal on syntax
message  = 'Missing variable trapped'
continue = 'next1'
upper       /* missing variable */

next1:
signal on syntax
message  = 'Invalid variable trapped'
continue = 'next2'
upper 'foo' /* invalid variable */

next2:
say "UPPER instruction" test.pass
exit rc.pass

syntax:
   say message':  RC='rc '"'errortext(rc)'"'
   signal value continue

badnews:
   exit 5
