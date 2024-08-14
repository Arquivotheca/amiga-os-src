/* Tests for variables */
test.0 = 'Not OK';test.1 = 'OK';
rc.0=5;rc.1=0

test = (12abc == '12ABC')
say 'Fixed symbols' test.test
pass = test

a = '123'; test = (a == '123')
drop a;test = test & a == 'A'
say 'Simple variables' test.test
pass = pass & test

a.  = 0;a.1 = '123';a.2 = '456';drop b.
test = (a.1 == '123') & (a.2 == '456') & (a.3 == 0) & (b.1.c == 'B.1.C')
a. = 0;test = test & a.1 = 0;
null = ''; test = test & abc.1.null.2 == 'ABC.1..2'
say 'Compound variables' test.test
pass = pass & test

drop a.;a.1 = 'abc';b. = 0;i = 1;val  = 123.4;
test =        symbol('ab ') == 'BAD'
test = test & symbol('abc') == 'LIT'
test = test & symbol('val') == 'VAR'
test = test & symbol('a.i') == 'VAR'
test = test & symbol('a.j') == 'LIT'
say 'SYMBOL()' test.test
pass = pass & test

test =        value('val') == 123.4
test = test & value('a.i') == 'abc'
test = test & value('a.2') == 'A.2'
test = test & value('a.j') == 'A.J'
test = test & value('b.' ) == '0'
say 'VALUE()' test.test
pass = pass & test

/* check for SIGNAL ON NOVALUE */
signal on novalue
message  = "NOVALUE trapped"
continue = 'first'

drop test
say test  /* uninitialized */
pass = 0
say "NOVALUE not trapped"

first:
signal on novalue
message  = "NOVALUE not cleared in function"
continue = 'second'
call func
test = result == 'OK'
pass = pass & test
signal 'third'

Second:
pass = 0

third:
say "Variables" test.pass
exit rc.pass

novalue:
   say message
   signal value continue

func:
   signal off novalue   /* reset interrupt */
   return OK            /* return literal  */
