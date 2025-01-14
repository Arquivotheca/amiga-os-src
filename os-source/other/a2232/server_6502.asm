;This is the 6502 code to run the multiport board
;it was converted from the MOS records by mos_to_dcb
; <hi offset>,<low offset>,<num bytes>,<data...>
    SECTION main,CODE
    XDEF server_object
server_object:

 dc.b $2,$0,$f,$8f,$12,$3,$4c,$a4,$3,$64,$1a,$f7,$20,$7f,$2a,$f,$a5,$2a,$85
 dc.b $2,$10,$f,$1f,$a5,$0,$a4,$1,$a2,$20,$20,$a,$3e,$64,$2a,$a5,$3e,$f0,$11
 dc.b $2,$20,$f,$a0,$4,$b1,$0,$48,$9,$c,$91,$0,$68,$91,$0,$7f,$17,$2,$c6
 dc.b $2,$30,$f,$3e,$a6,$24,$a4,$25,$e4,$24,$d0,$f8,$c4,$25,$d0,$f4,$86,$1b,$84
 dc.b $2,$40,$f,$1c,$a0,$2,$b1,$0,$85,$19,$bf,$19,$6,$29,$f8,$85,$19,$80,$4a
 dc.b $2,$50,$f,$29,$7,$48,$a0,$0,$b1,$0,$20,$0,$3e,$d0,$4,$f7,$18,$87,$11
 dc.b $2,$60,$f,$7a,$d0,$13,$3f,$31,$10,$c5,$33,$d0,$4,$b7,$3b,$80,$2c,$c5,$32
 dc.b $2,$70,$f,$d0,$4,$37,$3b,$80,$24,$a4,$23,$a6,$22,$e8,$d0,$7,$c8,$c4,$37
 dc.b $2,$80,$f,$90,$2,$a4,$36,$e4,$1b,$d0,$8,$c4,$1c,$d0,$4,$b7,$1a,$80,$a
 dc.b $2,$90,$f,$92,$22,$f7,$23,$86,$22,$84,$23,$87,$18,$38,$a5,$1b,$e5,$22,$aa
 dc.b $2,$a0,$f,$a5,$1c,$e5,$23,$10,$5,$18,$65,$37,$e5,$36,$a8,$d0,$23,$8a,$f0
 dc.b $2,$b0,$f,$20,$e4,$34,$b0,$1c,$f7,$3d,$87,$18,$f,$31,$6,$f7,$3a,$87,$3b
 dc.b $2,$c0,$f,$80,$2e,$2f,$31,$2b,$a0,$4,$b1,$0,$29,$f7,$91,$0,$a7,$3b,$80
 dc.b $2,$d0,$f,$1f,$7f,$3d,$1c,$c4,$35,$90,$18,$77,$3d,$f,$31,$6,$e7,$3a,$7
 dc.b $2,$e0,$f,$3b,$80,$d,$2f,$31,$a,$a0,$4,$b1,$0,$9,$8,$91,$0,$27,$3b
 dc.b $2,$f0,$f,$a5,$19,$29,$7,$5,$1a,$5,$21,$85,$21,$6f,$19,$6,$c7,$3b,$c7
 dc.b $3,$0,$f,$21,$80,$2,$47,$3b,$a9,$1,$2c,$0,$7c,$f0,$6,$e7,$3b,$e7,$21
 dc.b $3,$10,$f,$80,$2,$67,$3b,$2c,$2,$7c,$f0,$4,$d7,$3b,$80,$2,$57,$3b,$a5
 dc.b $3,$20,$f,$3b,$85,$20,$29,$50,$aa,$c5,$3c,$f0,$6,$85,$3c,$f7,$18,$87,$11
 dc.b $3,$30,$f,$a0,$2,$b1,$0,$85,$19,$4f,$19,$6b,$a5,$3b,$29,$20,$25,$31,$d0
 dc.b $3,$40,$f,$63,$a5,$3b,$29,$8,$25,$31,$d0,$5b,$64,$3f,$7f,$3a,$6,$77,$3a
 dc.b $3,$50,$f,$a5,$33,$80,$33,$6f,$3a,$6,$67,$3a,$a5,$32,$80,$2a,$7f,$2e,$6
 dc.b $3,$60,$f,$77,$2e,$a5,$2f,$80,$21,$a6,$28,$a4,$29,$e4,$28,$d0,$f8,$c4,$29
 dc.b $3,$70,$f,$d0,$f4,$e4,$26,$d0,$d,$c4,$27,$d0,$9,$f,$13,$27,$f7,$18,$87
 dc.b $3,$80,$f,$11,$80,$21,$f7,$3f,$b2,$26,$a0,$0,$91,$0,$e7,$3f,$7f,$3f,$14
 dc.b $3,$90,$f,$a4,$27,$a6,$26,$e8,$d0,$7,$c8,$c4,$39,$90,$2,$a4,$38,$f7,$27
 dc.b $3,$a0,$f,$86,$26,$84,$27,$9f,$12,$3,$4c,$48,$5,$64,$1a,$f7,$40,$7f,$4a
 dc.b $3,$b0,$f,$f,$a5,$4a,$85,$1f,$a5,$2,$a4,$3,$a2,$40,$20,$a,$3e,$64,$4a
 dc.b $3,$c0,$f,$a5,$5e,$f0,$11,$a0,$4,$b1,$2,$48,$9,$c,$91,$2,$68,$91,$2
 dc.b $3,$d0,$f,$7f,$17,$2,$c6,$5e,$a6,$44,$a4,$45,$e4,$44,$d0,$f8,$c4,$45,$d0
 dc.b $3,$e0,$f,$f4,$86,$1b,$84,$1c,$a0,$2,$b1,$2,$85,$19,$bf,$19,$6,$29,$f8
 dc.b $3,$f0,$f,$85,$19,$80,$4a,$29,$7,$48,$a0,$0,$b1,$2,$20,$0,$3e,$d0,$4
 dc.b $4,$0,$f,$f7,$18,$97,$11,$7a,$d0,$13,$3f,$51,$10,$c5,$53,$d0,$4,$b7,$5b
 dc.b $4,$10,$f,$80,$2c,$c5,$52,$d0,$4,$37,$5b,$80,$24,$a4,$43,$a6,$42,$e8,$d0
 dc.b $4,$20,$f,$7,$c8,$c4,$57,$90,$2,$a4,$56,$e4,$1b,$d0,$8,$c4,$1c,$d0,$4
 dc.b $4,$30,$f,$b7,$1a,$80,$a,$92,$42,$f7,$43,$86,$42,$84,$43,$97,$18,$38,$a5
 dc.b $4,$40,$f,$1b,$e5,$42,$aa,$a5,$1c,$e5,$43,$10,$5,$18,$65,$57,$e5,$56,$a8
 dc.b $4,$50,$f,$d0,$23,$8a,$f0,$20,$e4,$54,$b0,$1c,$f7,$5d,$97,$18,$f,$51,$6
 dc.b $4,$60,$f,$f7,$5a,$87,$5b,$80,$2e,$2f,$51,$2b,$a0,$4,$b1,$2,$29,$f7,$91
 dc.b $4,$70,$f,$2,$a7,$5b,$80,$1f,$7f,$5d,$1c,$c4,$55,$90,$18,$77,$5d,$f,$51
 dc.b $4,$80,$f,$6,$e7,$5a,$7,$5b,$80,$d,$2f,$51,$a,$a0,$4,$b1,$2,$9,$8
 dc.b $4,$90,$f,$91,$2,$27,$5b,$a5,$19,$29,$7,$5,$1a,$5,$41,$85,$41,$6f,$19
 dc.b $4,$a0,$f,$6,$c7,$5b,$c7,$41,$80,$2,$47,$5b,$a9,$2,$2c,$0,$7c,$f0,$6
 dc.b $4,$b0,$f,$e7,$5b,$e7,$41,$80,$2,$67,$5b,$2c,$2,$7c,$f0,$4,$d7,$5b,$80
 dc.b $4,$c0,$f,$2,$57,$5b,$a5,$5b,$85,$40,$29,$50,$aa,$c5,$5c,$f0,$6,$85,$5c
 dc.b $4,$d0,$f,$f7,$18,$97,$11,$a0,$2,$b1,$2,$85,$19,$4f,$19,$6b,$a5,$5b,$29
 dc.b $4,$e0,$f,$20,$25,$51,$d0,$63,$a5,$5b,$29,$8,$25,$51,$d0,$5b,$64,$5f,$7f
 dc.b $4,$f0,$f,$5a,$6,$77,$5a,$a5,$53,$80,$33,$6f,$5a,$6,$67,$5a,$a5,$52,$80
 dc.b $5,$0,$f,$2a,$7f,$4e,$6,$77,$4e,$a5,$4f,$80,$21,$a6,$48,$a4,$49,$e4,$48
 dc.b $5,$10,$f,$d0,$f8,$c4,$49,$d0,$f4,$e4,$46,$d0,$d,$c4,$47,$d0,$9,$1f,$13
 dc.b $5,$20,$f,$27,$f7,$18,$97,$11,$80,$21,$f7,$5f,$b2,$46,$a0,$0,$91,$2,$e7
 dc.b $5,$30,$f,$5f,$7f,$5f,$14,$a4,$47,$a6,$46,$e8,$d0,$7,$c8,$c4,$59,$90,$2
 dc.b $5,$40,$f,$a4,$58,$f7,$47,$86,$46,$84,$47,$af,$12,$3,$4c,$ec,$6,$64,$1a
 dc.b $5,$50,$f,$f7,$60,$7f,$6a,$f,$a5,$6a,$85,$1f,$a5,$4,$a4,$5,$a2,$60,$20
 dc.b $5,$60,$f,$a,$3e,$64,$6a,$a5,$7e,$f0,$11,$a0,$4,$b1,$4,$48,$9,$c,$91
 dc.b $5,$70,$f,$4,$68,$91,$4,$7f,$17,$2,$c6,$7e,$a6,$64,$a4,$65,$e4,$64,$d0
 dc.b $5,$80,$f,$f8,$c4,$65,$d0,$f4,$86,$1b,$84,$1c,$a0,$2,$b1,$4,$85,$19,$bf
 dc.b $5,$90,$f,$19,$6,$29,$f8,$85,$19,$80,$4a,$29,$7,$48,$a0,$0,$b1,$4,$20
 dc.b $5,$a0,$f,$0,$3e,$d0,$4,$f7,$18,$a7,$11,$7a,$d0,$13,$3f,$71,$10,$c5,$73
 dc.b $5,$b0,$f,$d0,$4,$b7,$7b,$80,$2c,$c5,$72,$d0,$4,$37,$7b,$80,$24,$a4,$63
 dc.b $5,$c0,$f,$a6,$62,$e8,$d0,$7,$c8,$c4,$77,$90,$2,$a4,$76,$e4,$1b,$d0,$8
 dc.b $5,$d0,$f,$c4,$1c,$d0,$4,$b7,$1a,$80,$a,$92,$62,$f7,$63,$86,$62,$84,$63
 dc.b $5,$e0,$f,$a7,$18,$38,$a5,$1b,$e5,$62,$aa,$a5,$1c,$e5,$63,$10,$5,$18,$65
 dc.b $5,$f0,$f,$77,$e5,$76,$a8,$d0,$23,$8a,$f0,$20,$e4,$74,$b0,$1c,$f7,$7d,$a7
 dc.b $6,$0,$f,$18,$f,$71,$6,$f7,$7a,$87,$7b,$80,$2e,$2f,$71,$2b,$a0,$4,$b1
 dc.b $6,$10,$f,$4,$29,$f7,$91,$4,$a7,$7b,$80,$1f,$7f,$7d,$1c,$c4,$75,$90,$18
 dc.b $6,$20,$f,$77,$7d,$f,$71,$6,$e7,$7a,$7,$7b,$80,$d,$2f,$71,$a,$a0,$4
 dc.b $6,$30,$f,$b1,$4,$9,$8,$91,$4,$27,$7b,$a5,$19,$29,$7,$5,$1a,$5,$61
 dc.b $6,$40,$f,$85,$61,$6f,$19,$6,$c7,$7b,$c7,$61,$80,$2,$47,$7b,$a9,$4,$2c
 dc.b $6,$50,$f,$0,$7c,$f0,$6,$e7,$7b,$e7,$61,$80,$2,$67,$7b,$2c,$2,$7c,$f0
 dc.b $6,$60,$f,$4,$d7,$7b,$80,$2,$57,$7b,$a5,$7b,$85,$60,$29,$50,$aa,$c5,$7c
 dc.b $6,$70,$f,$f0,$6,$85,$7c,$f7,$18,$a7,$11,$a0,$2,$b1,$4,$85,$19,$4f,$19
 dc.b $6,$80,$f,$6b,$a5,$7b,$29,$20,$25,$71,$d0,$63,$a5,$7b,$29,$8,$25,$71,$d0
 dc.b $6,$90,$f,$5b,$64,$7f,$7f,$7a,$6,$77,$7a,$a5,$73,$80,$33,$6f,$7a,$6,$67
 dc.b $6,$a0,$f,$7a,$a5,$72,$80,$2a,$7f,$6e,$6,$77,$6e,$a5,$6f,$80,$21,$a6,$68
 dc.b $6,$b0,$f,$a4,$69,$e4,$68,$d0,$f8,$c4,$69,$d0,$f4,$e4,$66,$d0,$d,$c4,$67
 dc.b $6,$c0,$f,$d0,$9,$2f,$13,$27,$f7,$18,$a7,$11,$80,$21,$f7,$7f,$b2,$66,$a0
 dc.b $6,$d0,$f,$0,$91,$4,$e7,$7f,$7f,$7f,$14,$a4,$67,$a6,$66,$e8,$d0,$7,$c8
 dc.b $6,$e0,$f,$c4,$79,$90,$2,$a4,$78,$f7,$67,$86,$66,$84,$67,$bf,$12,$3,$4c
 dc.b $6,$f0,$f,$90,$8,$64,$1a,$f7,$80,$7f,$8a,$f,$a5,$8a,$85,$1f,$a5,$6,$a4
 dc.b $7,$0,$f,$7,$a2,$80,$20,$a,$3e,$64,$8a,$a5,$9e,$f0,$11,$a0,$4,$b1,$6
 dc.b $7,$10,$f,$48,$9,$c,$91,$6,$68,$91,$6,$7f,$17,$2,$c6,$9e,$a6,$84,$a4
 dc.b $7,$20,$f,$85,$e4,$84,$d0,$f8,$c4,$85,$d0,$f4,$86,$1b,$84,$1c,$a0,$2,$b1
 dc.b $7,$30,$f,$6,$85,$19,$bf,$19,$6,$29,$f8,$85,$19,$80,$4a,$29,$7,$48,$a0
 dc.b $7,$40,$f,$0,$b1,$6,$20,$0,$3e,$d0,$4,$f7,$18,$b7,$11,$7a,$d0,$13,$3f
 dc.b $7,$50,$f,$91,$10,$c5,$93,$d0,$4,$b7,$9b,$80,$2c,$c5,$92,$d0,$4,$37,$9b
 dc.b $7,$60,$f,$80,$24,$a4,$83,$a6,$82,$e8,$d0,$7,$c8,$c4,$97,$90,$2,$a4,$96
 dc.b $7,$70,$f,$e4,$1b,$d0,$8,$c4,$1c,$d0,$4,$b7,$1a,$80,$a,$92,$82,$f7,$83
 dc.b $7,$80,$f,$86,$82,$84,$83,$b7,$18,$38,$a5,$1b,$e5,$82,$aa,$a5,$1c,$e5,$83
 dc.b $7,$90,$f,$10,$5,$18,$65,$97,$e5,$96,$a8,$d0,$23,$8a,$f0,$20,$e4,$94,$b0
 dc.b $7,$a0,$f,$1c,$f7,$9d,$b7,$18,$f,$91,$6,$f7,$9a,$87,$9b,$80,$2e,$2f,$91
 dc.b $7,$b0,$f,$2b,$a0,$4,$b1,$6,$29,$f7,$91,$6,$a7,$9b,$80,$1f,$7f,$9d,$1c
 dc.b $7,$c0,$f,$c4,$95,$90,$18,$77,$9d,$f,$91,$6,$e7,$9a,$7,$9b,$80,$d,$2f
 dc.b $7,$d0,$f,$91,$a,$a0,$4,$b1,$6,$9,$8,$91,$6,$27,$9b,$a5,$19,$29,$7
 dc.b $7,$e0,$f,$5,$1a,$5,$81,$85,$81,$6f,$19,$6,$c7,$9b,$c7,$81,$80,$2,$47
 dc.b $7,$f0,$f,$9b,$a9,$8,$2c,$0,$7c,$f0,$6,$e7,$9b,$e7,$81,$80,$2,$67,$9b
 dc.b $8,$0,$f,$2c,$2,$7c,$f0,$4,$d7,$9b,$80,$2,$57,$9b,$a5,$9b,$85,$80,$29
 dc.b $8,$10,$f,$50,$aa,$c5,$9c,$f0,$6,$85,$9c,$f7,$18,$b7,$11,$a0,$2,$b1,$6
 dc.b $8,$20,$f,$85,$19,$4f,$19,$6b,$a5,$9b,$29,$20,$25,$91,$d0,$63,$a5,$9b,$29
 dc.b $8,$30,$f,$8,$25,$91,$d0,$5b,$64,$9f,$7f,$9a,$6,$77,$9a,$a5,$93,$80,$33
 dc.b $8,$40,$f,$6f,$9a,$6,$67,$9a,$a5,$92,$80,$2a,$7f,$8e,$6,$77,$8e,$a5,$8f
 dc.b $8,$50,$f,$80,$21,$a6,$88,$a4,$89,$e4,$88,$d0,$f8,$c4,$89,$d0,$f4,$e4,$86
 dc.b $8,$60,$f,$d0,$d,$c4,$87,$d0,$9,$3f,$13,$27,$f7,$18,$b7,$11,$80,$21,$f7
 dc.b $8,$70,$f,$9f,$b2,$86,$a0,$0,$91,$6,$e7,$9f,$7f,$9f,$14,$a4,$87,$a6,$86
 dc.b $8,$80,$f,$e8,$d0,$7,$c8,$c4,$99,$90,$2,$a4,$98,$f7,$87,$86,$86,$84,$87
 dc.b $8,$90,$f,$cf,$12,$3,$4c,$34,$a,$64,$1a,$f7,$a0,$7f,$aa,$f,$a5,$aa,$85
 dc.b $8,$a0,$f,$1f,$a5,$8,$a4,$9,$a2,$a0,$20,$a,$3e,$64,$aa,$a5,$be,$f0,$11
 dc.b $8,$b0,$f,$a0,$4,$b1,$8,$48,$9,$c,$91,$8,$68,$91,$8,$7f,$17,$2,$c6
 dc.b $8,$c0,$f,$be,$a6,$a4,$a4,$a5,$e4,$a4,$d0,$f8,$c4,$a5,$d0,$f4,$86,$1b,$84
 dc.b $8,$d0,$f,$1c,$a0,$2,$b1,$8,$85,$19,$bf,$19,$6,$29,$f8,$85,$19,$80,$4a
 dc.b $8,$e0,$f,$29,$7,$48,$a0,$0,$b1,$8,$20,$0,$3e,$d0,$4,$f7,$18,$c7,$11
 dc.b $8,$f0,$f,$7a,$d0,$13,$3f,$b1,$10,$c5,$b3,$d0,$4,$b7,$bb,$80,$2c,$c5,$b2
 dc.b $9,$0,$f,$d0,$4,$37,$bb,$80,$24,$a4,$a3,$a6,$a2,$e8,$d0,$7,$c8,$c4,$b7
 dc.b $9,$10,$f,$90,$2,$a4,$b6,$e4,$1b,$d0,$8,$c4,$1c,$d0,$4,$b7,$1a,$80,$a
 dc.b $9,$20,$f,$92,$a2,$f7,$a3,$86,$a2,$84,$a3,$c7,$18,$38,$a5,$1b,$e5,$a2,$aa
 dc.b $9,$30,$f,$a5,$1c,$e5,$a3,$10,$5,$18,$65,$b7,$e5,$b6,$a8,$d0,$23,$8a,$f0
 dc.b $9,$40,$f,$20,$e4,$b4,$b0,$1c,$f7,$bd,$c7,$18,$f,$b1,$6,$f7,$ba,$87,$bb
 dc.b $9,$50,$f,$80,$2e,$2f,$b1,$2b,$a0,$4,$b1,$8,$29,$f7,$91,$8,$a7,$bb,$80
 dc.b $9,$60,$f,$1f,$7f,$bd,$1c,$c4,$b5,$90,$18,$77,$bd,$f,$b1,$6,$e7,$ba,$7
 dc.b $9,$70,$f,$bb,$80,$d,$2f,$b1,$a,$a0,$4,$b1,$8,$9,$8,$91,$8,$27,$bb
 dc.b $9,$80,$f,$a5,$19,$29,$7,$5,$1a,$5,$a1,$85,$a1,$6f,$19,$6,$c7,$bb,$c7
 dc.b $9,$90,$f,$a1,$80,$2,$47,$bb,$a9,$10,$2c,$0,$7c,$f0,$6,$e7,$bb,$e7,$a1
 dc.b $9,$a0,$f,$80,$2,$67,$bb,$2c,$2,$7c,$f0,$4,$d7,$bb,$80,$2,$57,$bb,$a5
 dc.b $9,$b0,$f,$bb,$85,$a0,$29,$50,$aa,$c5,$bc,$f0,$6,$85,$bc,$f7,$18,$c7,$11
 dc.b $9,$c0,$f,$a0,$2,$b1,$8,$85,$19,$4f,$19,$6b,$a5,$bb,$29,$20,$25,$b1,$d0
 dc.b $9,$d0,$f,$63,$a5,$bb,$29,$8,$25,$b1,$d0,$5b,$64,$bf,$7f,$ba,$6,$77,$ba
 dc.b $9,$e0,$f,$a5,$b3,$80,$33,$6f,$ba,$6,$67,$ba,$a5,$b2,$80,$2a,$7f,$ae,$6
 dc.b $9,$f0,$f,$77,$ae,$a5,$af,$80,$21,$a6,$a8,$a4,$a9,$e4,$a8,$d0,$f8,$c4,$a9
 dc.b $a,$0,$f,$d0,$f4,$e4,$a6,$d0,$d,$c4,$a7,$d0,$9,$4f,$13,$27,$f7,$18,$c7
 dc.b $a,$10,$f,$11,$80,$21,$f7,$bf,$b2,$a6,$a0,$0,$91,$8,$e7,$bf,$7f,$bf,$14
 dc.b $a,$20,$f,$a4,$a7,$a6,$a6,$e8,$d0,$7,$c8,$c4,$b9,$90,$2,$a4,$b8,$f7,$a7
 dc.b $a,$30,$f,$86,$a6,$84,$a7,$df,$12,$3,$4c,$d8,$b,$64,$1a,$f7,$c0,$7f,$ca
 dc.b $a,$40,$f,$f,$a5,$ca,$85,$1f,$a5,$a,$a4,$b,$a2,$c0,$20,$a,$3e,$64,$ca
 dc.b $a,$50,$f,$a5,$de,$f0,$11,$a0,$4,$b1,$a,$48,$9,$c,$91,$a,$68,$91,$a
 dc.b $a,$60,$f,$7f,$17,$2,$c6,$de,$a6,$c4,$a4,$c5,$e4,$c4,$d0,$f8,$c4,$c5,$d0
 dc.b $a,$70,$f,$f4,$86,$1b,$84,$1c,$a0,$2,$b1,$a,$85,$19,$bf,$19,$6,$29,$f8
 dc.b $a,$80,$f,$85,$19,$80,$4a,$29,$7,$48,$a0,$0,$b1,$a,$20,$0,$3e,$d0,$4
 dc.b $a,$90,$f,$f7,$18,$d7,$11,$7a,$d0,$13,$3f,$d1,$10,$c5,$d3,$d0,$4,$b7,$db
 dc.b $a,$a0,$f,$80,$2c,$c5,$d2,$d0,$4,$37,$db,$80,$24,$a4,$c3,$a6,$c2,$e8,$d0
 dc.b $a,$b0,$f,$7,$c8,$c4,$d7,$90,$2,$a4,$d6,$e4,$1b,$d0,$8,$c4,$1c,$d0,$4
 dc.b $a,$c0,$f,$b7,$1a,$80,$a,$92,$c2,$f7,$c3,$86,$c2,$84,$c3,$d7,$18,$38,$a5
 dc.b $a,$d0,$f,$1b,$e5,$c2,$aa,$a5,$1c,$e5,$c3,$10,$5,$18,$65,$d7,$e5,$d6,$a8
 dc.b $a,$e0,$f,$d0,$23,$8a,$f0,$20,$e4,$d4,$b0,$1c,$f7,$dd,$d7,$18,$f,$d1,$6
 dc.b $a,$f0,$f,$f7,$da,$87,$db,$80,$2e,$2f,$d1,$2b,$a0,$4,$b1,$a,$29,$f7,$91
 dc.b $b,$0,$f,$a,$a7,$db,$80,$1f,$7f,$dd,$1c,$c4,$d5,$90,$18,$77,$dd,$f,$d1
 dc.b $b,$10,$f,$6,$e7,$da,$7,$db,$80,$d,$2f,$d1,$a,$a0,$4,$b1,$a,$9,$8
 dc.b $b,$20,$f,$91,$a,$27,$db,$a5,$19,$29,$7,$5,$1a,$5,$c1,$85,$c1,$6f,$19
 dc.b $b,$30,$f,$6,$c7,$db,$c7,$c1,$80,$2,$47,$db,$a9,$20,$2c,$0,$7c,$f0,$6
 dc.b $b,$40,$f,$e7,$db,$e7,$c1,$80,$2,$67,$db,$2c,$2,$7c,$f0,$4,$d7,$db,$80
 dc.b $b,$50,$f,$2,$57,$db,$a5,$db,$85,$c0,$29,$50,$aa,$c5,$dc,$f0,$6,$85,$dc
 dc.b $b,$60,$f,$f7,$18,$d7,$11,$a0,$2,$b1,$a,$85,$19,$4f,$19,$6b,$a5,$db,$29
 dc.b $b,$70,$f,$20,$25,$d1,$d0,$63,$a5,$db,$29,$8,$25,$d1,$d0,$5b,$64,$df,$7f
 dc.b $b,$80,$f,$da,$6,$77,$da,$a5,$d3,$80,$33,$6f,$da,$6,$67,$da,$a5,$d2,$80
 dc.b $b,$90,$f,$2a,$7f,$ce,$6,$77,$ce,$a5,$cf,$80,$21,$a6,$c8,$a4,$c9,$e4,$c8
 dc.b $b,$a0,$f,$d0,$f8,$c4,$c9,$d0,$f4,$e4,$c6,$d0,$d,$c4,$c7,$d0,$9,$5f,$13
 dc.b $b,$b0,$f,$27,$f7,$18,$d7,$11,$80,$21,$f7,$df,$b2,$c6,$a0,$0,$91,$a,$e7
 dc.b $b,$c0,$f,$df,$7f,$df,$14,$a4,$c7,$a6,$c6,$e8,$d0,$7,$c8,$c4,$d9,$90,$2
 dc.b $b,$d0,$f,$a4,$d8,$f7,$c7,$86,$c6,$84,$c7,$ef,$12,$3,$4c,$7c,$d,$64,$1a
 dc.b $b,$e0,$f,$f7,$e0,$7f,$ea,$f,$a5,$ea,$85,$1f,$a5,$c,$a4,$d,$a2,$e0,$20
 dc.b $b,$f0,$f,$a,$3e,$64,$ea,$a5,$fe,$f0,$11,$a0,$4,$b1,$c,$48,$9,$c,$91
 dc.b $c,$0,$f,$c,$68,$91,$c,$7f,$17,$2,$c6,$fe,$a6,$e4,$a4,$e5,$e4,$e4,$d0
 dc.b $c,$10,$f,$f8,$c4,$e5,$d0,$f4,$86,$1b,$84,$1c,$a0,$2,$b1,$c,$85,$19,$bf
 dc.b $c,$20,$f,$19,$6,$29,$f8,$85,$19,$80,$4a,$29,$7,$48,$a0,$0,$b1,$c,$20
 dc.b $c,$30,$f,$0,$3e,$d0,$4,$f7,$18,$e7,$11,$7a,$d0,$13,$3f,$f1,$10,$c5,$f3
 dc.b $c,$40,$f,$d0,$4,$b7,$fb,$80,$2c,$c5,$f2,$d0,$4,$37,$fb,$80,$24,$a4,$e3
 dc.b $c,$50,$f,$a6,$e2,$e8,$d0,$7,$c8,$c4,$f7,$90,$2,$a4,$f6,$e4,$1b,$d0,$8
 dc.b $c,$60,$f,$c4,$1c,$d0,$4,$b7,$1a,$80,$a,$92,$e2,$f7,$e3,$86,$e2,$84,$e3
 dc.b $c,$70,$f,$e7,$18,$38,$a5,$1b,$e5,$e2,$aa,$a5,$1c,$e5,$e3,$10,$5,$18,$65
 dc.b $c,$80,$f,$f7,$e5,$f6,$a8,$d0,$23,$8a,$f0,$20,$e4,$f4,$b0,$1c,$f7,$fd,$e7
 dc.b $c,$90,$f,$18,$f,$f1,$6,$f7,$fa,$87,$fb,$80,$2e,$2f,$f1,$2b,$a0,$4,$b1
 dc.b $c,$a0,$f,$c,$29,$f7,$91,$c,$a7,$fb,$80,$1f,$7f,$fd,$1c,$c4,$f5,$90,$18
 dc.b $c,$b0,$f,$77,$fd,$f,$f1,$6,$e7,$fa,$7,$fb,$80,$d,$2f,$f1,$a,$a0,$4
 dc.b $c,$c0,$f,$b1,$c,$9,$8,$91,$c,$27,$fb,$a5,$19,$29,$7,$5,$1a,$5,$e1
 dc.b $c,$d0,$f,$85,$e1,$6f,$19,$6,$c7,$fb,$c7,$e1,$80,$2,$47,$fb,$a9,$40,$2c
 dc.b $c,$e0,$f,$0,$7c,$f0,$6,$e7,$fb,$e7,$e1,$80,$2,$67,$fb,$2c,$2,$7c,$f0
 dc.b $c,$f0,$f,$4,$d7,$fb,$80,$2,$57,$fb,$a5,$fb,$85,$e0,$29,$50,$aa,$c5,$fc
 dc.b $d,$0,$f,$f0,$6,$85,$fc,$f7,$18,$e7,$11,$a0,$2,$b1,$c,$85,$19,$4f,$19
 dc.b $d,$10,$f,$6b,$a5,$fb,$29,$20,$25,$f1,$d0,$63,$a5,$fb,$29,$8,$25,$f1,$d0
 dc.b $d,$20,$f,$5b,$64,$ff,$7f,$fa,$6,$77,$fa,$a5,$f3,$80,$33,$6f,$fa,$6,$67
 dc.b $d,$30,$f,$fa,$a5,$f2,$80,$2a,$7f,$ee,$6,$77,$ee,$a5,$ef,$80,$21,$a6,$e8
 dc.b $d,$40,$f,$a4,$e9,$e4,$e8,$d0,$f8,$c4,$e9,$d0,$f4,$e4,$e6,$d0,$d,$c4,$e7
 dc.b $d,$50,$f,$d0,$9,$6f,$13,$27,$f7,$18,$e7,$11,$80,$21,$f7,$ff,$b2,$e6,$a0
 dc.b $d,$60,$f,$0,$91,$c,$e7,$ff,$7f,$ff,$14,$a4,$e7,$a6,$e6,$e8,$d0,$7,$c8
 dc.b $d,$70,$f,$c4,$f9,$90,$2,$a4,$f8,$f7,$e7,$86,$e6,$84,$e7,$4c,$98,$3e,$78
 dc.b $d,$80,$f,$a9,$0,$5b,$3,$a0,$1,$2b,$a2,$ff,$9a,$a3,$0,$a2,$0,$74,$0
 dc.b $d,$90,$f,$e8,$d0,$fb,$a9,$7f,$8d,$1a,$7c,$a9,$8,$8d,$1c,$7c,$8d,$1e,$7c
 dc.b $d,$a0,$f,$9c,$14,$7c,$9c,$4,$7c,$9c,$6,$7c,$a9,$3,$8d,$c,$7c,$a9,$0
 dc.b $d,$b0,$f,$8d,$e,$7c,$a9,$17,$8d,$1e,$7c,$9c,$10,$7c,$a9,$44,$85,$1,$a0
 dc.b $d,$c0,$f,$2,$91,$0,$a0,$4,$a9,$2,$85,$2b,$91,$0,$a0,$6,$a9,$1e,$85
 dc.b $d,$d0,$f,$2c,$91,$0,$a9,$9,$85,$31,$a9,$11,$85,$32,$a9,$13,$85,$33,$a9
 dc.b $d,$e0,$f,$fa,$85,$2d,$a9,$a,$85,$34,$a9,$2,$85,$35,$a9,$14,$85,$37,$a9
 dc.b $d,$f0,$f,$10,$85,$36,$85,$23,$85,$25,$a9,$32,$85,$39,$a9,$30,$85,$38,$85
 dc.b $e,$0,$f,$29,$85,$27,$a9,$4c,$85,$3,$a0,$2,$91,$2,$a0,$4,$a9,$2,$85
 dc.b $e,$10,$f,$4b,$91,$2,$a0,$6,$a9,$1e,$85,$4c,$91,$2,$a9,$9,$85,$51,$a9
 dc.b $e,$20,$f,$11,$85,$52,$a9,$13,$85,$53,$a9,$fa,$85,$4d,$a9,$a,$85,$54,$a9
 dc.b $e,$30,$f,$2,$85,$55,$a9,$18,$85,$57,$a9,$14,$85,$56,$85,$43,$85,$45,$a9
 dc.b $e,$40,$f,$34,$85,$59,$a9,$32,$85,$58,$85,$49,$85,$47,$a9,$54,$85,$5,$a0
 dc.b $e,$50,$f,$2,$91,$4,$a0,$4,$a9,$2,$85,$6b,$91,$4,$a0,$6,$a9,$1e,$85
 dc.b $e,$60,$f,$6c,$91,$4,$a9,$9,$85,$71,$a9,$11,$85,$72,$a9,$13,$85,$73,$a9
 dc.b $e,$70,$f,$fa,$85,$6d,$a9,$a,$85,$74,$a9,$2,$85,$75,$a9,$1c,$85,$77,$a9
 dc.b $e,$80,$f,$18,$85,$76,$85,$63,$85,$65,$a9,$36,$85,$79,$a9,$34,$85,$78,$85
 dc.b $e,$90,$f,$69,$85,$67,$a9,$5c,$85,$7,$a0,$2,$91,$6,$a0,$4,$a9,$2,$85
 dc.b $e,$a0,$f,$8b,$91,$6,$a0,$6,$a9,$1e,$85,$8c,$91,$6,$a9,$9,$85,$91,$a9
 dc.b $e,$b0,$f,$11,$85,$92,$a9,$13,$85,$93,$a9,$fa,$85,$8d,$a9,$a,$85,$94,$a9
 dc.b $e,$c0,$f,$2,$85,$95,$a9,$20,$85,$97,$a9,$1c,$85,$96,$85,$83,$85,$85,$a9
 dc.b $e,$d0,$f,$38,$85,$99,$a9,$36,$85,$98,$85,$89,$85,$87,$a9,$64,$85,$9,$a0
 dc.b $e,$e0,$f,$2,$91,$8,$a0,$4,$a9,$2,$85,$ab,$91,$8,$a0,$6,$a9,$1e,$85
 dc.b $e,$f0,$f,$ac,$91,$8,$a9,$9,$85,$b1,$a9,$11,$85,$b2,$a9,$13,$85,$b3,$a9
 dc.b $f,$0,$f,$fa,$85,$ad,$a9,$a,$85,$b4,$a9,$2,$85,$b5,$a9,$24,$85,$b7,$a9
 dc.b $f,$10,$f,$20,$85,$b6,$85,$a3,$85,$a5,$a9,$3a,$85,$b9,$a9,$38,$85,$b8,$85
 dc.b $f,$20,$f,$a9,$85,$a7,$a9,$6c,$85,$b,$a0,$2,$91,$a,$a0,$4,$a9,$2,$85
 dc.b $f,$30,$f,$cb,$91,$a,$a0,$6,$a9,$1e,$85,$cc,$91,$a,$a9,$9,$85,$d1,$a9
 dc.b $f,$40,$f,$11,$85,$d2,$a9,$13,$85,$d3,$a9,$fa,$85,$cd,$a9,$a,$85,$d4,$a9
 dc.b $f,$50,$f,$2,$85,$d5,$a9,$28,$85,$d7,$a9,$24,$85,$d6,$85,$c3,$85,$c5,$a9
 dc.b $f,$60,$f,$3c,$85,$d9,$a9,$3a,$85,$d8,$85,$c9,$85,$c7,$a9,$74,$85,$d,$a0
 dc.b $f,$70,$f,$2,$91,$c,$a0,$4,$a9,$2,$85,$eb,$91,$c,$a0,$6,$a9,$1e,$85
 dc.b $f,$80,$f,$ec,$91,$c,$a9,$9,$85,$f1,$a9,$11,$85,$f2,$a9,$13,$85,$f3,$a9
 dc.b $f,$90,$f,$fa,$85,$ed,$a9,$a,$85,$f4,$a9,$2,$85,$f5,$a9,$2c,$85,$f7,$a9
 dc.b $f,$a0,$f,$28,$85,$f6,$85,$e3,$85,$e5,$a9,$3e,$85,$f9,$a9,$3c,$85,$f8,$85
 dc.b $f,$b0,$7,$e9,$85,$e7,$c6,$14,$4c,$98,$3e
 dc.b $3e,$0,$f,$d0,$7,$1f,$19,$4,$f7,$1a,$17,$19,$60,$85,$1d,$84,$1e,$6f,$1f
 dc.b $3e,$10,$f,$16,$a0,$4,$b5,$b,$91,$1d,$a0,$6,$b5,$c,$91,$1d,$b5,$1b,$29
 dc.b $3e,$20,$f,$f7,$95,$0,$95,$1b,$95,$1c,$2f,$1f,$8,$a0,$4,$b1,$1d,$9,$1
 dc.b $3e,$30,$f,$91,$1d,$1f,$1f,$8,$a0,$4,$b1,$1d,$29,$fe,$91,$1d,$f,$1f,$4
 dc.b $3e,$40,$f,$b5,$d,$95,$1e,$5f,$1f,$3,$20,$57,$3e,$4f,$1f,$3,$20,$82,$3e
 dc.b $3e,$50,$f,$3f,$1f,$3,$20,$6e,$3e,$60,$a9,$0,$95,$0,$95,$1,$95,$e,$95
 dc.b $3e,$60,$f,$1a,$95,$1b,$95,$1c,$95,$1d,$20,$6e,$3e,$20,$82,$3e,$60,$b5,$4
 dc.b $3e,$70,$f,$b4,$5,$d5,$4,$d0,$f8,$48,$98,$7a,$d5,$5,$d0,$f1,$94,$2,$95
 dc.b $3e,$80,$f,$3,$60,$b5,$8,$b4,$9,$d5,$8,$d0,$f8,$48,$98,$7a,$d5,$9,$d0
 dc.b $3e,$90,$f,$f1,$94,$6,$95,$7,$74,$1e,$60,$77,$17,$ad,$12,$7c,$f0,$18,$9c
 dc.b $3e,$a0,$f,$12,$7c,$f7,$17,$a6,$18,$f0,$f,$a5,$e,$f0,$b,$c6,$e,$d0,$7
 dc.b $3e,$b0,$f,$86,$10,$64,$18,$9c,$0,$70,$4c,$0,$2,$40,$8d,$0,$80,$40,$40
 dc.b $3e,$c0,$0,$40
 dc.b $3f,$c0,$f,$43,$6f,$70,$79,$72,$69,$67,$68,$74,$20,$31,$39,$38,$39,$20,$43
 dc.b $3f,$d0,$f,$6f,$6d,$6d,$6f,$64,$6f,$72,$65,$2d,$41,$6d,$69,$67,$61,$2c,$20
 dc.b $3f,$e0,$f,$49,$6e,$63,$2e,$20,$41,$6c,$6c,$20,$52,$69,$67,$68,$74,$73,$20
 dc.b $3f,$f0,$f,$52,$65,$73,$65,$72,$76,$65,$64,$8,$0,$ba,$3e,$7f,$d,$bb,$3e
 dc.b $0,$0


 ds.w 0


;237 records loaded.
  END
