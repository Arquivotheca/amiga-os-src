

        OPT     p=68020

            move.l  #9,d1
            move.w  #$8100,d0                            ; Average left and right channel
            add.w   #$0000,d0
            roxr.w  d1,d0
            extb.l  d0
            move.l  d0,$0
            rts
