/* maketest.rexx */

blocksize = 240
b_count   = 12
b_extra   = 1

char1 = '-'
char2 = 'X'
char3 = '0'

s = ''
c = '|_'

do 32 ; s = s || c ; end
s = s || '|'

do x = 1 to 60 ; say s x ; end

