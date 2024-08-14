    IFGE    INFOLEVEL-50
    SPUTMSG  50,<'[%s]ZText:'>
    MOVEM.L A0-A6,-(SP)
    PUTMSG  50,<'  A0:%lx 1:%lx 2:%lx 3:%lx 4:%lx 5:%lx 6:%lx'>
    LEA     7*4(SP),SP
    MOVEM.L D0-D7,-(SP)
    PUTMSG  50,<'  D0:%lx 1:%lx 2:%lx 3:%lx 4:%lx 5:%lx 6:%lx 7:%lx'>
    LEA     8*4(SP),SP
    ENDC


