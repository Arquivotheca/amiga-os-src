/* tests the support library functions */
test.0='Not OK';test.1='OK'

if ~show('l','rexxsupport.library') then
   if ~addlib('rexxsupport.library',0,-30) then
      do
         say "no library!"
         exit 5
         end

addr = allocmem(4000)
test = addr ~== NULL()
if test then call freemem(addr,4000)
say "ALLOCMEM()" test.test

test = baddr('00000001'x) == '00000004'x
say "BADDR()" test.test

secs = time(s)
call delay 125
test = time(s) >= secs + 100/50
say "DELAY()" test.test

address command 'echo >vd0:fubar "a test"'
test =         exists('vd0:fubar') &  delete('vd0:fubar')
test = test & ~exists('vd0:fubar') & ~delete('vd0:fubar')
say "DELETE()" test.test

addr = allocmem(4000)
test = addr ~== '00000000'x
if test then call freemem(addr,4000)
say "FREEMEM()" test.test

test =        forbid() = 0
test = test & forbid() = forbid() - 1
test = test & permit() > 0 & permit() = permit() + 1
say "FORBID()" test.test

test =        next('00000004'x) == '00000676'x
test = test & import(next('00000676'x,10)) == 'exec.library'
say "NEXT()" test.test

test = null() == '00000000'x
say "NULL()" test.test

test =         makedir('vd0:level1')
test = test &  word(statef('vd0:level1'),1) = 'DIR'
test = test &  delete('vd0:level1')
say "MAKEDIR()" test.test

test = offset('00000000'x,10) == '0000000A'x
say "OFFSET()" test.test

test =        forbid() = 0 & forbid() =  1
test = test & permit() = 0 & permit() = -1
say "PERMIT()" test.test

address command 'echo >vd0:fubar "a test"'
test =         rename('vd0:fubar','vd0:foo2') & ~exists('vd0:fubar')
test = test & ~rename('vd0:fubar','vd0:foo2') & delete('vd0:foo2')
say "RENAME()" test.test

test =        find(upper(showdir('devs:')),'MOUNTLIST') >= 1
test = test & showdir('vd0:') == translate(showdir('vd0:',,';'),,';')
say "SHOWDIR()" test.test

test =        showlist('l','exec.library')
test = test & showlist('l') == translate(showlist('l',,';'),,';')
test = test & showlist('l','exec.library',,'A') == import('00000004'x,4)
test = test & showlist('Assigns')   ~== ''
test = test & showlist('Devices')   ~== ''
test = test & showlist('Handlers')  ~== ''
test = test & showlist('MemList')   ~== ''
test = test & showlist('Resources') ~== ''
test = test & showlist('Volumes')   ~== ''
test = test & showlist('Waiting')   ~== ''
call showlist('I');call showlist('S');call showlist('T')
say "SHOWLIST()" test.test

/* check the status of a file ...               */
status = statef('slftest.rexx')
test = word(status,1) == 'FILE'
say "STATEF()" test.test

test = openport('MYPORT') & ~openport('MYPORT')
test1 = 0;test2 = 0;test3 = 0;test4 = 0;test5 = 0;test6 = 0;test7 = 0
if test then do
   test1 = getpkt('MYPORT') == NULL()
   cmd = '"address MYPORT;hello;say rc;call delay 50"'
   if left(address(),4) = 'WSH_' then
      ''cmd '>con:0/0/640/60/new &'
   else address command 'run rx >con:0/0/640/60/New' cmd

   test2 = waitpkt('MYPORT')     /* wait for a packet */

   pkt = getpkt('MYPORT')
   test3 = pkt ~== '0000 0000'x
   if test3 then do
      test4 = typepkt(pkt,'Command') & typepkt(pkt,'A') = 0
      test5 = getarg(pkt) == 'HELLO'
      test6 = reply(pkt,5)
      test7 = closeport('MYPORT')
      end
   end
say "CLOSEPORT()" test.test7
say "GETARG()" test.test5
say "GETPKT()" test.test3
say "OPENPORT()" test.test
say "REPLY()" test.test6
say "TYPEPKT()" test.test4
say "WAITPKT()" test.test2
