/* Validation tests for the END instruction */
signal on syntax
message  = "Error in END instruction"
continue = 'badnews'

/* test a simple loop */
do i=1 to 10
   end
say "Simple DO-END" OK

/* test a simple SELECT */
select
   when 1 then nop
   end
say "Simple SELECT-END" OK

do i=1 to 10
   do j=1 to 5
      nop
      end j
   end i
say "END <symbol>" OK

message  = "Unexpected END trapped"
continue = next1
   end   /* unexpected? */

next1:
signal on syntax
message  = "Unmatched symbol trapped"
continue = 'next2'
do i=1 to 10
   end j

next2:
signal on syntax
message  = "Invalid symbol trapped"
continue = 'next3'
do i=1 to 10
   end 'i'

next3:
signal on syntax
message  = "Improper nesting trapped"
continue = 'next4'
do i=1 to 10
   do j=1 to 5
      nop
      end i
   end

next4:
signal on syntax
message  = "Unexpected symbol trapped"
continue = 'next5'
select
   when 1 then nop
   end j

next5:
say "END instruction OK"
exit

syntax:
   say message
   signal value continue

badnews:
   exit 5
