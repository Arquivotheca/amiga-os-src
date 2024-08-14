/* */
trace i

do i=-2 to 2
   say i abs(i) i**2
   end i

trace ?
parse value '123xx456xx789' with a +3 pattern +2 b (pattern) c
say a b c pattern
