/* Tests the procedure instruction */
test.0='Not OK';test.1='OK'

abc  = '123'
a.1  = 'one'
j    = 10
b.10 = 'ten'
k    = 11
b.11 = 'eleven'
c.1  = 'one'

/* invoke the procedure */
test = foo() & def = '456' 
test = test & (c.1 == 0)

/* now test error conditions */
signal on syntax
test = test & badproc()

where = 'done'
procedure   /* procedure in main program */
test = 0

done: say 'PROCEDURE' test.test
exit 0

foo: procedure expose abc def j a.1 b.j b.k k c.
def = '456'
test = (b.j == 'ten') & (b.k == 'B.11')
c. = 0
return test & (abc == '123') & (a.1 == 'one')

badproc:
   procedure expose     /* no symbols */
   return 0
   where: return 1

syntax:
   say "trapped syntax error"
   signal value where
