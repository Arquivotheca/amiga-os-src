/* tests the instructiion keywords */
signal on syntax

address
address 'Foo'
address value 'Foo'12

do for 5
   end
do i=1 to 10 by 2.3
   end
do i=1 while I <= 9
   end
do i=1 until i > 9
   end
i=0
do forever
   i = i + 1
   if i > 3 then leave
   end

numeric form scientific
numeric form engineering
numeric fuzz 3-1
numeric digits 9+2

if 1 then
   nop

options results
options no results
options no cache
options cache
options resident
options no resident
options prompt "Hi!> "
options failat 10

signal value 'cont'
cont:
signal on  error
signal off error
signal on  failure
signal off failure
signal on  ioerr
signal off ioerr
signal on  halt
signal off halt
signal on  break_c
signal off break_c
signal on  break_d
signal off break_d
signal on  break_e
signal off break_e
signal on  break_f
signal off break_f

trace 12
trace a
trace all
trace b
trace background
trace c
trace commands
trace e
trace errors
trace l
trace labels
trace n
trace normal
trace r
trace results
trace o
trace off

select
   when 1 then nop
   end

trace s
trace scan
exit

syntax:
   say "Error in line" sigl
   exit 5
