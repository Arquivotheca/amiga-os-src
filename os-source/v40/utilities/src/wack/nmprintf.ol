.id  "date" "Mar 27 1986  20:11:44"
.id  "target" "68000"
.id  "translator" "as68000"
.seg {$$seg0} %43 1 1 {}
.len %43 34
.org %43 0
.defg {_mnprintf} %47 %43 0 +
'23df'
0 {_holdRTS}%46 +:L
'4eb9'
0 {_printf}%44 +:L
'2f3c00000009'
'4eb9'
0 {_putchar}%45 +:L
'588f'
'2f39'
0 %46 +:L
'4e75'
