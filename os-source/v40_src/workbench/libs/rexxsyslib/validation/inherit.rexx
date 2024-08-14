/* Tests inherited values */
/* trace r */
test.0 = "Not OK"
test.1 = "OK"

options prompt "OK? (Y or N) "
options failat 10
numeric digits 10
numeric fuzz 2
saveaddr = address()

test = check()
test = test & digits() == 10 & fuzz() == 2
test = test & address() == saveaddr
pull answ
test = test & upper(answ) = 'Y'
signal on failure
'quit 9'
signal done

failure:test = 0

done:
say "Inherited values" test.test
exit 

check:
test = digits() == 10 & fuzz() == 2
signal on failure
'quit 9'
test = test & rc == 9
numeric digits 8
numeric fuzz 2
options prompt "Func? "
options failat 5
address command
return test

failure: return 0
