/* tests the DATE() and TIME() functions */

call time(r)
say 'Military:' time() ' Civil:' time(c)
say 'Hours:' time(h) ' Minutes:' time(m) ' Seconds:' time(s)
say 'Elapsed:' time(e)
say ''
say 'Normal:' date(n) ' Month:' date(m) ' Weekday:' date(w)
say 'Europe:' date(e) ' USA:' date(u) ' Ordered:' date('o') ' Sort:' date(s)
say 'Base:' date('b') ' Century:' date('c') ' Internal:' date('I')
say 'Julian:' date('j') ' Days:' date('d')
