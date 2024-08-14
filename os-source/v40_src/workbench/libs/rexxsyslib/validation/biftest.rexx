/* Test Suite for the Built-In Functions    */
/* trace i */
test.0 = 'Not OK';test.1 = 'OK'
tempfile = 'ram:tempfile'

/* Test the ABBREV() function   */
test =         abbrev('short','sh') & ~abbrev('short','shr')
test = test &  abbrev('short','')   & ~abbrev('short','sh',3)
say 'ABBREV()' test.test

test = abs(123.3) = 123.3 & abs(-32) = 32
parse value abs(-1) with val
test = test & val == '1'
say 'ABS()' test.test

/* Test the ADDRESS() function */
saveaddr = address()
address 'Foo'
test = address() == 'Foo';
address saveaddr
say 'ADDRESS()' test.test

test =        func('one','two',,'four')
test = test & noargs()
say 'ARG()' test.test

test = b2c('01000001') == '41'x & b2c('') = ''
say 'B2C()' test.test

test =        bitand('abc','ABC')   == 'ABC'
test = test & bitand('ffff'x,'abc') == 'ab '
test = test & bitand('ABC',,'fe'x)  == '@BB'
test = test & bitand(copies('A',33000),,'fe'x)  == copies('@',33000)
say 'BITAND()' test.test

test =        bitchg('00001111'b,4) == '00011111'b
test = test & bitchg('0ff0'x,0)     == '0ff1'x
say 'BITCHG()' test.test

test =        bitclr('00001111'b,0) == '00001110'b
test = test & bitclr('0ff0'x,0)     == '0ff0'x
say 'BITCLR()' test.test

test =        bitor('ABC','   ')  == 'abc'
test = test & bitor('ABC',' ')    == 'abc'
test = test & bitor('ABC',,'21'x) == 'acc'
say 'BITOR()' test.test

test =        bitset('00001111'b,0) == '00001111'b
test = test & bitset('0ff0'x,0)     == '0ff1'x
say 'BITSET()' test.test

test =        bittst('00001111'b,0) == 1
test = test & bittst('0ff0'x,0)     == 0
say 'BITTST()' test.test

test =        bitxor('ABC','abc')    == '   '
test = test & bitxor('0ff0'x,,'ff'x) == 'f00f'x
say 'BITXOR()' test.test

long = copies('abc',5000)
test =        bitcomp('01010001'b,'01010001'b) = -1
test = test & bitcomp('01010001'b,'01000001'b) = 4
test = test & bitcomp('01'x||long,'00'x||long) = 3*5000*8
say 'BITCOMP()' test.test

test = c2b('0f'x) == '00001111' & c2b('') = ''
say 'C2B()' test.test

test =        c2d('31'x)     = 49    & c2d('ff81'x)   = 65409
test = test & c2d('81'x,1)   = -127  & c2d('81'x,2)   = 129
test = test & c2d('F081'x,2) = -3967 & c2d('F081'x,1) = -127
say 'C2D()' test.test

test = c2x('3132'x) == '3132' & c2x('') == ''
say 'C2X()' test.test

test =        center('123',5)     == ' 123 ' & centre('123',5) == ' 123 '
test = test & center('123',2)     == '12'    & center('123',1) == '2'
test = test & center('123',5,'-') == '-123-'
test = test & center('',5)        == '     ' & center('123',0) == ''
say 'CENTER()' test.test

test =        compare(long,long) == 0
test = test & compare(long||'a',long||'b') = 3*5000+1
say 'COMPARE()' test.test

test =        compress('This is a test') == 'Thisisatest'
test = test & compress(' -123.4 ',' -+') == '123.4'
say 'COMPRESS()' test.test

test =        copies('123',3) == '123123123'
test = test & copies('123',0) == '' & copies('',3) == ''
say 'COPIES()' test.test

test =        d2c(9)      == '09'x & d2c(129)    == '81'x
test = test & d2c(129,1)  == '81'x & d2c(129,2)  == '0081'x
test = test & d2c(257,1)  == '01'x & d2c(12,0)   == ''
test = test & d2c(-127,1) == '81'x & d2c(-127,2) == 'ff81'x
say 'D2C()' test.test

test =        d2x(9)      == '9'   & d2x(129)    == '81'
test = test & d2x(129,1)  == '1'   & d2x(129,4)  == '0081'
say 'D2X()' test.test

/* test the DATATYPE() function   */
test =        datatype(3)   == 'NUM' & datatype(-3.14) == 'NUM'
test = test & datatype('X') == 'CHAR'
test = test & datatype('AbC',A)  & ~datatype('a+-',a)
test = test & datatype('101',B)  & ~datatype('121',B)
test = test & datatype('abc',L)  & ~datatype('Abc',L)
test = test & datatype('ABC',m)  &  datatype('abc',m) & datatype('AbC','m')
test = test & datatype('1.3',n)  & ~datatype('abc',n)
test = test & datatype('ab.',S)  & ~datatype('A+B',S)
test = test & datatype('ABC',U)  & ~datatype('aBc',U)
test = test & datatype('123',W)  & ~datatype('1.1',W) & ~datatype('A','w')
test = test & datatype('0FE',X)  & ~datatype('afg',X)
say 'DATATYPE()' test.test

parse value date() with day month year .
test =          left(date(M),3)   == month
test = test & substr(date(U),4,2) == day & substr(date(U),7,2) = year-1900
test = test & substr(date(E),1,2) == day & substr(date(U),7,2) = year-1900
test = test & date(w,date('i')+7) == date(w)
test = test & date(N,19880325,S)  == '25 Mar 1988'
say 'DATE()' test.test

/* Test the DELSTR() function */
test =        delstr('This is a test',6)   == 'This '
test = test & delstr('This is a test',6,2) == 'This  a test'
test = test & delstr(' ',10) == ' ' & delstr('',1) == ''
say 'DELSTR()' test.test

/* Test the DELWORD() function */
test =        delword('This is a test ',3)   == 'This is '
test = test & delword('This is a test ',3,1) == 'This is test '
test = test & delword('This is a test ',5)   == 'This is a test '
test = test & delword('',1) == '' & delword(' ',1) == ' '
say 'DELWORD()' test.test

savedigits = digits()
numeric digits savedigits-2
test = digits() = savedigits-2
numeric digits savedigits
say 'DIGITS()' test.test

test = errortext(1) == 'Program not found'
say 'ERRORTEXT()' test.test

addr = getspace(16)
test =        export(addr,'Try this',16,'+') = 16
test = test & import(addr,16) == 'Try this++++++++'
call freespace(addr,16)
say 'EXPORT()' test.test

call writefile tempfile,'"a test"'
test = exists(tempfile) & ~exists('ram:foofoo.temp')
say 'EXISTS()' test.test

test =        find('Now is the time','is the') = 2
test = test & find('Now is the time','when')   = 0
test = test & find('Now is the time','')       = 1
test = test & find('1 2 3 4 2 3 5','2 3 5')    = 5
say "FIND()" test.test

saveform = form()
numeric form engineering
test =        form() == 'ENGINEERING'
interpret 'numeric form' saveform
test = test & form() == saveform
say 'FORM()' test.test

addr = getspace(16)
call freespace(addr,16)
test = 1
say 'FREESPACE()' test.test

numeric fuzz 3
test = fuzz() == '3'
numeric fuzz 0
test = test & fuzz() = 0
say 'FUZZ()' test.test

test = hash('A') = 65
say 'HASH()' test.test

addr = getspace(16)
test =        export(addr,'Try this',16) = 16
test = test & import(addr) == 'Try this'
call freespace(addr,16)
say 'IMPORT()' test.test

/* Test the INDEX() function   */
test =        index('abcabc','c')   = 3
test = test & index('abcabc','c',4) = 6
test = test & index('abc','c',9)    = 0
say 'INDEX()' test.test

test =        insert('123','abcdef',3)    == 'abc123def'
test = test & insert('123','abcdef',0)    == '123abcdef'
test = test & insert('123','abc',6,4,'+') == 'abc+++123+'
say 'INSERT()' test.test

/* Test the LASTPOS() function   */
test =        lastpos('b','abcabc'   ) = 5
test = test & lastpos('bc','abcabc',3) = 2
say 'LASTPOS()' test.test

test =        left('123',2) == '12'   & left('123',0)     == ''
test = test & left('123',4) == '123 ' & left('123',4,'+') == '123+'
say 'LEFT()' test.test

/* Test the LENGTH() function   */
test = length('123') = 3 & length('') = 0
say 'LENGTH()' test.test

test =        lines() = 0  /* nothing stacked?  */
/* test the lines() function */

test = (lines() = 0)
if open(input,'con://300/100/Input/c') then do
   say "Enter a line in the input window,"
   say "then hit enter here"
   pull .
   test = test & (lines(input) > 0)
   call close input
   end
say "LINES()" test.test

/*
push '123'                 /* stack a line      */
test = test & lines() = 1
if test then pull .
test = test & lines() = 0  /* back to nothing?  */
say "LINES()" test.test
*/

test = max(12,-3,5,32.123 ) = 32.123
say 'MAX()' test.test

test = min(12,-3,5,32.123 ) = -3
say 'MIN()' test.test

/* Test the POS() function */
test =        pos('2','123456')   = 2
test = test & pos('2','123123',3) = 5
test = test & pos('3','123123',6) = 6
test = test & pos('3','123123',100000) = 0
say 'POS()' test.test

/* Test the OVERLAY() function */
test =        overlay('new','oldstring',4) == 'oldnewing'
test = test & overlay('123','abcdef',5)    == 'abcd123'
test = test & overlay('123','abc',1)       == '123'
test = test & overlay('123','abc',5,5,'+') == 'abc+123++'
say 'OVERLAY()' test.test

/* Test the PRAGMA() function */
currdir = pragma('D','')
test    = test & (currdir == pragma('D','ram:'))
savedir = pragma('D',currdir)

savepri = pragma('P',-5)
test = test & pragma('P',savepri) = -5
test = test & datatype(pragma('Id'),x)
savestack = pragma('Stack',8888)
test = test & (pragma('Stack',savestack) == 8888)

if open(window1,'con://300/100/window1') then do
   if pragma('*',window1) then do
      if open(window2,'*') then do
         test = test & (writech(window2,'Howdy!') = length('Howdy!'))
         call close window2
         end
      end
   call close window1
   end

call pragma 'W','None'
test = test & ~open(temp,'foovol:')
call pragma 'W'
say 'PRAGMA()' test.test

call writefile tempfile,"This is a test"
test = open(temp,tempfile)
if test then do
   test =        readch(temp,0) == ''
   test = test & readch(temp)   == 'T'
   test = test & readch(temp,99999) = 'his is a test'||'0A'x
   call close temp
   end
say "READCH()" test.test

test = open(temp,tempfile)
if test then do
   test = readln(temp) == 'This is a test'
   call close temp
   end
say "READLN()" test.test

test =        reverse('esrever') == 'reverse'
test = test & reverse('r')       == 'r' & reverse('') == ''
say 'REVERSE()' test.test

test =        right('123',2) == '23'   & right('123',0)     == ''
test = test & right('123',4) == ' 123' & right('123',4,'+') == '+123'
say 'RIGHT()' test.test

string = 'This is a test' || '0a'x || "Line 2"
call writefile tempfile,string
test = open(temp,tempfile)
if test then do
   test =        seek(temp,0,'end')     = length(string || '0a'x)
   test = test & seek(temp,0,'current') = seek(temp,0,'end')
   test = test & seek(temp,0,'begin')   = 0

   call readch temp,length('This is')
   test = test & seek(temp,0,'current') = length('This is')

   call seek temp,length(' a'),'current'
   test = test & readch(temp,5) == ' test'

   call seek temp,0,'begin'
   call readln temp
   test = test & seek(temp,0,'current') = length('This is a test'||'0a'x)
   call close temp
   end
say "SEEK()" test.test

test =        show(f) == "STDIN STDOUT"
test = test & find(show('l'),"REXX") > 0
if show('l','rexxsupport.library') then
   test = test & show('p') == showlist('p')
say "SHOW()" test.test

test =        sign(123) = 1  & sign(0) = 0
test = test & sign(-.1) = -1 & sign(-123) = -1
say "SIGN()" test.test

test = sourceline(2) == '/* trace i */'
test = test & sourceline() = 491
say 'SOURCELINE()' test.test

test =        space('This is a test',2)     == 'This  is  a  test'
test = test & space('This is a test',0)     == 'Thisisatest'
test = test & space('This is a test',1,'+') == 'This+is+a+test'
say 'SPACE()' test.test

addr = getspace(16)
test = length(storage(addr,'Try this',16,'+')) = 16
test = test & storage(addr,'Try this',16) == 'Try this++++++++'
call freespace(addr,16)
say 'STORAGE()' test.test

test =        strip('  abc  ',,  )     == 'abc'
test = test & strip('  abc  ','L')     == 'abc  '
test = test & strip('  abc  ','T')     == '  abc'
test = test & strip('++abc  ','B','+') == 'abc  '
say 'STRIP()' test.test

test = substr('123456',3)              == '3456'
test = test & substr('123456',3,2)     == '34'
test = test & substr('123456',3,5,'+') == '3456+'
say 'SUBSTR()' test.test

test =        subword('Now is the time  ',2) == 'is the time'
test = test & subword('Now is the time',2,2) == 'is the'
test = test & subword('Now is the time',5)   == ''
say "SUBWORD()" test.test

drop val;
test =        symbol('+V+') == 'BAD'
test = test & symbol('val') == 'LIT'
val = 12;
test = test & symbol('test') == 'VAR'
say 'SYMBOL()' test.test

test = time() == time('N')
hours = time('h');mins  = time('m');secs  = time('s')
test = test & (hours >= 0 & hours < 23)
test = test & (mins  >= 0 & mins  < 24*60)
test = test & (secs  >= 0 & secs  < 24*60*60)
parse value time() time('c') with normal civil . 1 hours ':' mins ':' secs
test = test & length(normal) = 8
suffix = right(civil,2)
test = test & (hours < 12 & suffix == 'AM') | (suffix == 'PM')
say "TIME()" test.test

trace off;
test = trace('N') == 'O'
say 'TRACE()' test.test

test =        translate('abcDEFæÆğĞçÇ')           == 'ABCDEFÆÆĞĞÇÇ'
test = test & translate('123-abc+','00','-+')     == '1230abc0'
test = test & translate('123-abc+','0' ,'-+')     == '1230abc '
test = test & translate('123-abc+','0' ,'-+','=') == '1230abc='
say "TRANSLATE()" test.test

test =        trim('abc    ') == 'abc'
test = test & trim('  abc  ') == '  abc'
test = test & trim('       ') == '' & trim('') == ''
say "TRIM()" test.test

/* Test the TRUNC() function */
test = trunc('123.456') == '123' & trunc('123.456',4) == '123.4560'
test = test & trunc(1) == '1' & trunc(11) == '11' & trunc(111) == '111'
test = test & trunc(12,2) == '12.00' & trunc(12.0,2) == '12.00'
say 'TRUNC()' test.test

test =        upper('abc') == 'ABC' & upper('123XYZ') == '123XYZ'
test = test & upper('')    == ''
say 'UPPER()' test.test

val = 12;name = 'Bill';a.name = 'Me'
test =        value('val')     == '12'
drop val b.
test = test & value('val')     == 'VAL'
test = test & value('b.')      == 'B.' & value('b.1') == 'B.1'
test = test & value('a.name')  == 'Me' & value('b.name') == 'B.Bill'
say 'VALUE()' test.test

test =        verify('123456','0123456789')     = 0
test = test & verify('12 456','0123456789')     = 3
test = test & verify('abcd1f','0123456789','m') = 5
test = test & verify('abc1e3','0123456789',,4)  = 5
say "VERIFY()" test.test

test =        word('This is a test ',3) == 'a';
test = test & word('This is a test ',5) == '' & word('',1) == ''
say 'WORD()' test.test

test =        words('This is a test ') = 4;
test = test & words('') = 0
say 'WORDS()' test.test

test =        wordindex('This is a test ',2) = 6;
test = test & wordindex('This is a test ',5) = 0 & wordindex('',1) = 0
say 'WORDINDEX()' test.test

test =        wordlength('This is a test ',1) = 4;
test = test & wordlength('This is a test ',5) = 0 & wordlength('   ',1) = 0
say 'WORDLENGTH()' test.test

test = open(temp,tempfile,'w')
if test then do
   test = writech(temp,copies(abc,100)) = 300
   call close temp
   end
say "WRITECH()" test.test

test = open(temp,tempfile,'w')
if test then do
   test = writeln(temp,'This is a test') = 15
   call close temp
   end
say "WRITELN()" test.test

test =        x2c('aa81') == 'aa81'x
say "X2C()" test.test

test =        x2d('81',2)   = -127  & x2d('81',0)   = 0
test = test & x2d('f081',4) = -3967 & x2d('f081',3) = 129
test = test & x2d('f081',2) = -127  & x2d('f081',1) = 1
test = test & x2d('ffff')   = 65535 & x2d('ffff',4) = -1
say "X2D()" test.test

test =        xrange('01'x,'03'x) == '010203'x
test = test & xrange('fe'x)       == 'feff'x
test = test & xrange('00'x,'ff'x) == xrange()
say 'XRANGE()' test.test

exit 

noargs:
   return arg() = 0 & arg(1,o) & arg(2,o)

func:
   test = arg(1) == 'one' & arg(2) == 'two' & arg() = 4
   return test & arg(1,e) & ~arg(2,o) & ~arg(3,e) & arg(3,o)

writefile: procedure
   parse arg name,contents

   if open(TEMPFILE,name,'w') then
      do
         len = writeln(TEMPFILE,contents)
         call close TEMPFILE
      end
   else len = -1

   return len
