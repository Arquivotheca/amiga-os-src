/* test for the SELECT instruction */
signal on syntax
message = "Unexpected error in SELECT"
resume  = 'badnews'
test.0='Not OK';test.1='OK'
rc.0=5;rc.1=0

cond. = 0
do i = 1 to 3
   select
      when i = 1 then
         cond.1 = i == 1
      when i = 2 then
         do 
            cond.2 = i == 2
         end
      otherwise
      cond.3 = i == 3
      end
   end i
pass = cond.1 & cond.2 & cond.3

test1:
message = "Invalid statement trapped"
resume  = 'Test2'
select
   when i = 1 then nop
   say hi
   otherwise nop
   end
say "Invalid statement not trapped"
pass = 0

Test2:
Test3:
signal on syntax
message = "Hanging else trapped"
resume  = 'test4'
select
   when 0 then nop
   otherwise
     if 0 then nop
     else
   end
say "Hanging ELSE not trapped"
pass = 0

Test5:
signal on syntax
message = "Missing OTHERWISE trapped"
resume  = 'AllOK'
signal on syntax
select
   when 0 then nop
   end
say "Missing OTHERWISE not trapped"
pass = 0

AllOK:
say "SELECT instruction" test.pass
exit rc.pass

syntax:
   say message":  RC="rc '"'errortext(rc)'"'
   signal value resume

badnews:
   exit 5

Test4:
signal on syntax
message = "Missing END trapped"
resume = 'test5'
select
   when 1 then nop
