/* tests the PARSE instruction */
test.0 = 'Not OK';test.1 = 'OK'
rc.0=5;rc.1=0

parse numeric digits fuzz form .
test = digits = digits() & fuzz = fuzz() & form = form()
say "PARSE NUMERIC" test.test
pass = test

parse version name vers cpu math video freq .
test = name == 'ARexx'
say "PARSE VERSION" test.test
pass = pass & test

parse upper source mode flag name resolved ext host .
test = mode == 'COMMAND' & ~flag & name == 'PARSETEST' & ext == 'REXX'
say "PARSE SOURCE" test.test
pass = pass & test

parse upper value 'abc DeF' with first second
test =        first == 'ABC' & second == ' DEF'
parse       value 'abc DeF' with first second
test = test & first == 'abc' & second == ' DeF'
say "PARSE UPPER" test.test
pass = pass & test

parse value '123xx456' with a 'xx' b
test =        (a == '123') & (b == '456')
parse value ' 123  456  789 ' with a b c
test = test & (a == '123') & (b == '456') & (c == '  789 ')
say "PARSE VALUE" test.test
pass = pass & test

string = 'xx123xx456 789'
parse var string pattern +2 a (pattern) b pattern
test =        (a == '123') & (b == '456') & pattern = ' 789'
parse value '1x2y' with a 'x' b 'yy' c
test = test & (a == '1') & (b == '2y') & (c == '')
parse value 'abc' with a 'abcd' b c
test = test & (a == 'abc') & (b == '') & (c == '')
parse value 'abcdef' with a 'd' b 'efg' c
test = test & (a == 'abc') & (b == 'ef') & (c == '')
parse value 'abc' with a '' -1 b '' c
test = test & (a == 'abc') & (b == 'c') & (c == '')
parse value '1abc2' with '1' a '2' b
test = test & (a == 'abc') & (b == '')
parse value 'x' with a 'x' b
test = test & (a == '') & (b == '')
parse value 'arg (extra ' with a b c '(' d
test = test & (a == 'arg') & (b == '') & (c == '') & d = 'extra '
say "PARSE with patterns" test.test
pass = pass & test

string = '062 123456 '
parse var string 1 pos 3 length +1 =pos a +length b 1 all
test = (a == '23') & (b == '456 ') & (all == '062 123456 ')
say "PARSE VAR" test.test
pass = pass & test

test = func('one two ','second',)
say "PARSE ARG" test.test
pass = pass & test

/* test a few special cases */
parse value 'abc' with 1 a +1 b +5 -3 c +2
test =        (a == 'a') & (b == 'bc') & (c == 'ab')
parse value 'abc' with 65537 a
test = test & (a == '')
say "miscellaneous PARSE" test.test
pass = pass & test

push 'a test'
if lines() > 0 then do
   parse pull instring
   test = (instring == 'a test')
   end
else test = 0
say "PARSE PULL" test.test
pass = pass & test

say 'PARSE instruction' test.pass
exit rc.pass

func:
   parse arg word1 word2 .,second,third
   return word1 == 'one' & word2 == 'two' & second == 'second' & third == ''
