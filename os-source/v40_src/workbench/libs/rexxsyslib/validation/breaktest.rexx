/* Tests the BREAK instruction */
signal on syntax
message  = "Error in BREAK instruction"
continue = 'badnews'

test.0='Not OK';test.1='OK'
rc.0=5;rc.1=0

do i=1 to 10
   if i=3 then break
   end
pass = i == 3

do
   test = 1
   break
   test = 0
end
pass = pass & test

interpret 'a=1;b=2;break;a=2;b=3'
pass = pass & a=1 & b=2

signal on syntax
message  = 'Unexpected BREAK trapped'
continue = 'next1'
break

next1:
say "BREAK instruction" test.pass
exit rc.pass

syntax:
   say message':  RC='rc '"'errortext(rc)'"'
   signal value continue

badnews:
   exit 5
