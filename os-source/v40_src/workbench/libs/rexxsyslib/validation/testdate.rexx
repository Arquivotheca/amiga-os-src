/* */

/* 'date 01-jan-89'
start = date('i')

do i=1 to 365
   'date tomorrow'
   if newdate ~== date() then
      do
      say "Error! date=" date() "newdate=" newdate
      exit 5
      end
   end */

trace i
do i=1979 to 2099
   'date' '01-jan-'right(i,2)
   newdate = date('s')

   if newdate ~== date(s,i*10000+0101,'s') then
      do
      say "Error on date=" date()
      exit 5
      end
   end
