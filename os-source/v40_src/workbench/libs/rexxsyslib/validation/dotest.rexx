/* Tests for DO instructions */
test.0='Not OK';test.1='OK'
rc.0=10;rc.1 = 0

do i=1 to 10
   nop
   if i = 3 then iterate
   if i > 4 then leave i
   test = (i ~= 3)
   end i
test = test & (i = 5)

sum = 0
do for 5
  sum = sum + 1
  end
test = test & (sum = 5)

sum = 0
do i=11 to 0 by -2
  sum = sum + i
  end
test = test & (sum = 11+9+7+5+3+1)

do i=1
   do j=i for 3
      if j > 10 then leave i
      end j
   end i
test = test & (i = 9) & (j = 11)

/* test the while loop */
iter = 1
do while test(iter)
   iter = 0
   end
test = test & iter = 0

/* test implicit loop count */
sum = 0
do 5+2
   sum = sum + 1
   end
test = test & sum = 7

/* test implicit loop count with WHILE */
sum = 0
do 5+2 while 1
   sum = sum + 1
   end
test = test & sum = 7

/* test initializer expression with FOR */
sum = 0
do init() for 5
   sum = sum + 1
   end
test = test & (sum = 5) & initvar

sum = 0
do i=1 to 5
   if i > 2 then
      do j = i to 5
      sum = sum + 2
      end j
   sum = sum + 1
   end i
test = test & sum = 17

/* test FOREVER keyword */
sum = 0
do forever
   sum = sum + 1
   if sum = 3 then break
   end
test = test & sum = 3

sum = 0
do forever while sum < 3
   sum = sum + 1
   end
test = test & sum = 3

sum = 0
do 5 forever
   sum = sum + 1
   end
test = test & sum = 5

signal on syntax
message = "Unmatched symbol trapped"
continue = 'next1'
do i=1 until i > 10;
   iterate j
   end
test = 0

next2:
signal on syntax
message = "Keyword conflict trapped"
continue = 'next3'
do i=1 forever
   nop
   end
test = 0

next3:
say 'DO instruction' test.test
exit rc.test

syntax:
say message': RC='rc '"'errortext(rc)'"'
signal value continue

test:
   return arg(1)

init:
   initvar = 1
   return "I'm back"


next1:
signal on syntax;
message  = "Missing END trapped"
continue = 'next2'
do i=1 until i > 5;
   nop
