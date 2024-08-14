/* Test the numeric and numeric digits capabilities */
test.0='Not OK';test.1='OK'
rc.0=5;rc.1=0

/* Check default digits and fuzz */
test = digits() = 9 & fuzz() = 0

test = test & 1/3 == '0.333333333'

/* test exponential notation at reduced precision */
numeric digits 5
test = test & digits() = 5
test = test & 54321*54321 == '2.9508E+9'

/* check assignment of fuzz */
numeric fuzz 1
test = test & fuzz() = 1

/* Check default digits and fuzz */
numeric digits
numeric fuzz
test = test & digits() = 9 & fuzz() = 0

/* test invalid fuzz */
signal on syntax
label = 'cont1'
message = "Invalid FUZZ trapped"
numeric fuzz 9  /* force error */
test = 0

cont1:
/* test invalid digits */
signal on syntax
label = 'cont2'
message = "Invalid DIGITS trapped"
numeric fuzz 5
numeric digits 5  /* force error */
test = 0

cont2:
/* test invalid digits */
signal on syntax
label = 'cont3'
message = "Invalid FORM trapped"
numeric form foo
test = 0

cont3:
signal on syntax

/* Test engineering notation */
numeric form engineering

test = test & (+1.2345E+11 == '123.45E+9')
test = test & (+1.2345E-11 == '12.345E-12')

/* restore digits and fuzz */
numeric fuzz
numeric digits
test = test & digits() = 9 & fuzz() = 0

/* test fuzz in comparisons */
test = test & ~(4.99999999 = 5) &  (4.99999999 < 5) 
numeric fuzz 1
test = test &  (4.99999999 = 5) & ~(4.99999999 < 5) 
test = test & ~(4.9999999  = 5) &  (4.9999999  < 5) 
numeric fuzz 2
test = test &  (4.9999999  = 5) & ~(4.9999999  < 5) 

say "Numerics" test.test
exit rc.test

syntax:
   say message "RC=" rc
   signal value label
