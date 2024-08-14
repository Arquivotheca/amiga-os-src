/* test some operations */
test.0='Not OK';test.1='OK'
rc.0=5;rc.1=0
say "Testing REXX operations"

allok = 1
if 1     ~= 1         then allok = 0
if 1.0   ~= 1         then allok = 0
if 'abc' ~= '  abc  ' then allok = 0

if 2   <= 1   then allok = 0
if 2   <  1   then allok = 0
if 2.0 <= 1 then allok = 0
if 'b' <= a then allok = 0
if 2   <= 1 then allok = 0

if 2 >  3      then allok = 0
if 2 ~< 3      then allok = 0

if 3.1 <  2.15 then allok = 0
if 3.1 ~> 2.15 then allok = 0

if ~('abc'  == 'abc' )  then allok = 0
if  (' abc' == 'abc' )  then allok = 0
if  ('abc'  ~== 'abc' ) then allok = 0
if ~('abc'  ~== 'abc ') then allok = 0

test = allok
test = test & '123' = '  + 123    '
test = test & 12300 = '  + 123e2  '
say 'Comparisons' test.test
pass = test

/* Test the arithmetic operators */
test =        (2+3 = 5) & (2.0+3.1 == '5.1') & (2.0*3.0 == '6.00')
test = test & (5.0 - 9.0) == -4.0

if 2.1 - 1.1 ~== 1.0 then
   do
      test = 0
      say "subtraction inaccurate"
   end

test = test & (2*3 == '6') & (2.0*3.00 == '6.000')
test = test & -22*(0*-1) = 0

/* Test formatting of 0 */
test = test & (1.23*0.00 == '0') & +0.00 == '0'

test = test & (5.0/2 = 2.5)
numeric digits 12
test = test & length(2/3) == 14
numeric digits 9

/* Test integer division % operator */
test = test & (21%4 = 5 & 1%3 = 0)

/* Test remainder division // operator */
test = test & (32//31 = 1 & 7//3 = 1 & -12.5//5 = -2.5)

test = test & (2**3 = 8 & 2**-3 = .125)
test = test & (1**0 = 1 & 2**0  = 1 & 0**0 = 1)

test = test & (10**14 == '1E+14')
say 'Arithmetic' test.test
pass = pass & test

/* Now for the logical operators */
allok = 1

if ~1 ~= 0    then allok = 0
if ~0 ~= 1    then allok = 0

if (1 & 1) ~= 1 then allok = 0
if (0 & 1) ~= 0 then allok = 0
if (0 & 1) ~= 0 then allok = 0
if (0 & 0) ~= 0 then allok = 0

if (1 ^ 1) ~= 0 then allok = 0
if (0 ^ 1) ~= 1 then allok = 0
if (0 ^ 1) ~= 1 then allok = 0
if (0 ^ 0) ~= 0 then allok = 0

if (1 | 1) ~= 1 then allok = 0
if (0 | 1) ~= 1 then allok = 0
if (0 | 1) ~= 1 then allok = 0
if (0 | 0) ~= 0 then allok = 0
say 'Logicals' test.allok
pass = pass & test

/* Test the concatenation operators */
test =        ('abc'VALUE   == 'abcVALUE')
test = test & (VALUE'abc'   == 'VALUEabc')
test = test & ((VALUE)'abc' == 'VALUEabc')
test = test & ('abc'||'def' == 'abcdef'  )

test = test & ('abc' 'def' == 'abc def')
test = test & ('abc' ABC   == 'abc ABC')
test = test & (('abc')ABC  == 'abcABC' )
say 'Concatenation' test.test
pass = pass & test

say "Operations" test.pass
exit rc.pass
