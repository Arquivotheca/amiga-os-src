; Color map

SPRITE_COLORS:
	dc.w	$579
	dc.w	$fff
	dc.w	$9d
	dc.w	$7c
	dc.w	$5b
	dc.w	$cf
	dc.w	$111
	dc.w	$ccc


SPRITE_0_CTL:

	dc.b	$a0, $7d	; Y, X
	dc.b	$c0, $0		; VSTOP Lo, Hi

SPRITE_0_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$2,$3
	dc.w	$f,$8
	dc.w	$1f,$12
	dc.w	$1d,$17
	dc.w	$e,$12
	dc.w	$a,$16
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$e,$13
	dc.w	$1d,$17
	dc.w	$1f,$12
	dc.w	$f,$8
	dc.w	$2,$3
	dc.w	$0,$3
	dc.w	$0,$1
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_1_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_1_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$b,$0
	dc.w	$24,$0
	dc.w	$24,$0
	dc.w	$20,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$25,$0
	dc.w	$24,$0
	dc.w	$b,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$3,$0
	dc.w	$1,$0
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_2_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_2_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$100,$ff00
	dc.w	$ffc0,$40
	dc.w	$3e0,$fd20
	dc.w	$2e0,$3a0
	dc.w	$1c0,$7f28
	dc.w	$140,$ffac
	dc.w	$140,$ffae
	dc.w	$140,$f1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$1c0,$e12f
	dc.w	$2e0,$e3af
	dc.w	$3e0,$fd3f
	dc.w	$ffc0,$5f
	dc.w	$100,$ff1f
	dc.w	$0,$f03f
	dc.w	$0,$fffe
	dc.w	$0,$fffc
	dc.w	$0,$7ff8

SPRITE_3_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_3_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$80,$0
	dc.w	$80,$0
	dc.w	$340,$0
	dc.w	$90,$0
	dc.w	$7e98,$0
	dc.w	$fe1c,$0
	dc.w	$fe1e,$0
	dc.w	$f01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e09f,$0
	dc.w	$e09f,$0
	dc.w	$35f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$f03f,$0
	dc.w	$fffe,$0
	dc.w	$fffc,$0
	dc.w	$7ff8,$0

SPRITE_4_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_4_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_5_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_5_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_6_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_6_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$e00,$a00
	dc.w	$1400,$1a00
	dc.w	$2e00,$3200
	dc.w	$1800,$2400
	dc.w	$2e00,$32e0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1ae0
	dc.w	$1400,$1ae0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$e00,$bf0
	dc.w	$0,$1f0
	dc.w	$0,$1f0
	dc.w	$0,$1f0
	dc.w	$0,$e0

SPRITE_7_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_7_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$400,$0
	dc.w	$100,$0
	dc.w	$500,$0
	dc.w	$a00,$0
	dc.w	$4e0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1e0,$0
	dc.w	$1e0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$5f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$e0,$0

SPRITE_8_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_8_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$2,$3
	dc.w	$f,$8
	dc.w	$1f,$12
	dc.w	$1d,$17
	dc.w	$e,$12
	dc.w	$a,$16
	dc.w	$1c,$15
	dc.w	$0,$3
	dc.w	$0,$3
	dc.w	$0,$3
	dc.w	$2,$3
	dc.w	$f,$8
	dc.w	$1f,$12
	dc.w	$1d,$17
	dc.w	$e,$12
	dc.w	$a,$16
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$e,$13
	dc.w	$1d,$17
	dc.w	$1f,$12
	dc.w	$f,$8
	dc.w	$2,$3
	dc.w	$0,$3
	dc.w	$0,$1
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_9_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_9_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$b,$0
	dc.w	$24,$0
	dc.w	$24,$0
	dc.w	$20,$0
	dc.w	$9,$0
	dc.w	$3,$0
	dc.w	$3,$0
	dc.w	$3,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$b,$0
	dc.w	$24,$0
	dc.w	$24,$0
	dc.w	$20,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$25,$0
	dc.w	$24,$0
	dc.w	$b,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$3,$0
	dc.w	$1,$0
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_10_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_10_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$100,$ff00
	dc.w	$ffc0,$40
	dc.w	$3e0,$fd20
	dc.w	$2e0,$3a0
	dc.w	$1c0,$7f28
	dc.w	$140,$ffac
	dc.w	$140,$ffae
	dc.w	$140,$f1af
	dc.w	$1c0,$e12f
	dc.w	$2e0,$e3af
	dc.w	$3e0,$fd3f
	dc.w	$ffc0,$5f
	dc.w	$100,$ff1f
	dc.w	$0,$3f
	dc.w	$0,$7ffe
	dc.w	$0,$fffc
	dc.w	$0,$fff8
	dc.w	$0,$f000
	dc.w	$0,$e000
	dc.w	$0,$e000
	dc.w	$20,$ffe0
	dc.w	$fff0,$10
	dc.w	$20,$ffe0
	dc.w	$0,$f000
	dc.w	$0,$fffe
	dc.w	$0,$ffff
	dc.w	$0,$7ffe

SPRITE_11_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_11_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$80,$0
	dc.w	$80,$0
	dc.w	$340,$0
	dc.w	$90,$0
	dc.w	$7e98,$0
	dc.w	$fe1c,$0
	dc.w	$fe1e,$0
	dc.w	$f01f,$0
	dc.w	$e09f,$0
	dc.w	$e09f,$0
	dc.w	$35f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$3f,$0
	dc.w	$7ffe,$0
	dc.w	$fffc,$0
	dc.w	$fff8,$0
	dc.w	$f000,$0
	dc.w	$e000,$0
	dc.w	$e000,$0
	dc.w	$0,$0
	dc.w	$20,$0
	dc.w	$0,$0
	dc.w	$f000,$0
	dc.w	$fffe,$0
	dc.w	$ffff,$0
	dc.w	$7ffe,$0

SPRITE_12_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_12_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$2,$3
	dc.w	$f,$8
	dc.w	$1f,$12
	dc.w	$1d,$17
	dc.w	$e,$12
	dc.w	$a,$16
	dc.w	$1c,$15
	dc.w	$0,$3
	dc.w	$0,$3
	dc.w	$0,$3
	dc.w	$0,$1
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$1c,$14
	dc.w	$a,$16
	dc.w	$e,$12
	dc.w	$1d,$17
	dc.w	$1f,$12
	dc.w	$f,$8
	dc.w	$2,$3
	dc.w	$0,$3
	dc.w	$0,$1
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_13_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_13_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$b,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$0,$0
	dc.w	$9,$0
	dc.w	$3,$0
	dc.w	$3,$0
	dc.w	$3,$0
	dc.w	$1,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$8,$0
	dc.w	$0,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$b,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$3,$0
	dc.w	$1,$0
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_14_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_14_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$100,$ff00
	dc.w	$ffc0,$40
	dc.w	$3e0,$fd20
	dc.w	$2e0,$3a0
	dc.w	$1c0,$7f28
	dc.w	$140,$ffac
	dc.w	$140,$ffae
	dc.w	$140,$f1af
	dc.w	$1c0,$e12f
	dc.w	$2e0,$e3af
	dc.w	$be0,$cd3f
	dc.w	$1fc0,$105f
	dc.w	$be0,$d3f
	dc.w	$2e0,$3af
	dc.w	$1c0,$12e
	dc.w	$140,$1ac
	dc.w	$140,$1ae
	dc.w	$140,$1af
	dc.w	$1c0,$12f
	dc.w	$2e0,$3af
	dc.w	$3e0,$fd3f
	dc.w	$ffc0,$5f
	dc.w	$100,$ff1f
	dc.w	$0,$f03f
	dc.w	$0,$fffe
	dc.w	$0,$fffc
	dc.w	$0,$7ff8

SPRITE_15_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_15_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$80,$0
	dc.w	$80,$0
	dc.w	$340,$0
	dc.w	$80,$0
	dc.w	$7e98,$0
	dc.w	$fe1c,$0
	dc.w	$fe1e,$0
	dc.w	$f01f,$0
	dc.w	$e09f,$0
	dc.w	$e09f,$0
	dc.w	$c35f,$0
	dc.w	$89f,$0
	dc.w	$35f,$0
	dc.w	$9f,$0
	dc.w	$9e,$0
	dc.w	$1c,$0
	dc.w	$1e,$0
	dc.w	$1f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$35f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$f03f,$0
	dc.w	$fffe,$0
	dc.w	$fffc,$0
	dc.w	$7ff8,$0

SPRITE_16_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_16_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$1c,$14
	dc.w	$a,$16
	dc.w	$a,$16
	dc.w	$a,$16
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$e,$13
	dc.w	$1d,$17
	dc.w	$1f,$12
	dc.w	$f,$8
	dc.w	$2,$3
	dc.w	$0,$3
	dc.w	$0,$1
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_17_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_17_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$8,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$1,$0
	dc.w	$1,$0
	dc.w	$1,$0
	dc.w	$1,$0
	dc.w	$5,$0
	dc.w	$4,$0
	dc.w	$b,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$3,$0
	dc.w	$1,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_18_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_18_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$e0,$a0
	dc.w	$140,$1a0
	dc.w	$140,$1a0
	dc.w	$140,$1a0
	dc.w	$140,$c1ae
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$340,$ffaf
	dc.w	$ff40,$1af
	dc.w	$340,$ffaf
	dc.w	$140,$f1af
	dc.w	$140,$ffaf
	dc.w	$140,$ffaf
	dc.w	$140,$7faf
	dc.w	$140,$1af
	dc.w	$140,$1af
	dc.w	$140,$1af
	dc.w	$140,$1af
	dc.w	$140,$1af
	dc.w	$e0,$bf
	dc.w	$0,$1f
	dc.w	$0,$1f
	dc.w	$0,$1f
	dc.w	$0,$e

SPRITE_19_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_19_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$40,$0
	dc.w	$10,$0
	dc.w	$10,$0
	dc.w	$10,$0
	dc.w	$c01e,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$1f,$0
	dc.w	$21f,$0
	dc.w	$1f,$0
	dc.w	$f01f,$0
	dc.w	$fe1f,$0
	dc.w	$fe1f,$0
	dc.w	$7e1f,$0
	dc.w	$1f,$0
	dc.w	$1f,$0
	dc.w	$1f,$0
	dc.w	$1f,$0
	dc.w	$1f,$0
	dc.w	$5f,$0
	dc.w	$1f,$0
	dc.w	$1f,$0
	dc.w	$1f,$0
	dc.w	$e,$0

SPRITE_20_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_20_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$2,$3
	dc.w	$f,$8
	dc.w	$1f,$12
	dc.w	$1d,$17
	dc.w	$e,$12
	dc.w	$a,$16
	dc.w	$a,$17
	dc.w	$e,$13
	dc.w	$1d,$17
	dc.w	$1f,$12
	dc.w	$f,$8
	dc.w	$2,$3
	dc.w	$0,$3
	dc.w	$0,$1
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$1c,$14
	dc.w	$a,$16
	dc.w	$c,$12
	dc.w	$1f,$12
	dc.w	$f,$8
	dc.w	$2,$3
	dc.w	$0,$3
	dc.w	$0,$1
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_21_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_21_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$b,$0
	dc.w	$24,$0
	dc.w	$24,$0
	dc.w	$20,$0
	dc.w	$21,$0
	dc.w	$25,$0
	dc.w	$24,$0
	dc.w	$b,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$3,$0
	dc.w	$1,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$8,$0
	dc.w	$20,$0
	dc.w	$24,$0
	dc.w	$b,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$3,$0
	dc.w	$1,$0
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_22_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_22_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$20,$ffe0
	dc.w	$fff0,$10
	dc.w	$20,$ffe0
	dc.w	$0,$0
	dc.w	$0,$7ffe
	dc.w	$0,$ffff
	dc.w	$0,$fffe
	dc.w	$0,$f000
	dc.w	$0,$e000
	dc.w	$100,$ff00
	dc.w	$ffc0,$40
	dc.w	$3e0,$fd20
	dc.w	$2e0,$f3a0
	dc.w	$1c0,$ff28
	dc.w	$140,$ffac
	dc.w	$140,$7fae
	dc.w	$140,$1af
	dc.w	$140,$1af
	dc.w	$1c0,$12f
	dc.w	$2e0,$3af
	dc.w	$3e0,$fd3f
	dc.w	$ffc0,$5f
	dc.w	$100,$ff1f
	dc.w	$0,$e03f
	dc.w	$0,$fffe
	dc.w	$0,$fffc
	dc.w	$0,$7ff8

SPRITE_23_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_23_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$20,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$7ffe,$0
	dc.w	$ffff,$0
	dc.w	$fffe,$0
	dc.w	$f000,$0
	dc.w	$e000,$0
	dc.w	$80,$0
	dc.w	$80,$0
	dc.w	$340,$0
	dc.w	$f090,$0
	dc.w	$fe98,$0
	dc.w	$fe1c,$0
	dc.w	$7e1e,$0
	dc.w	$1f,$0
	dc.w	$1f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$35f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$e03f,$0
	dc.w	$fffe,$0
	dc.w	$fffc,$0
	dc.w	$7ff8,$0

SPRITE_24_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_24_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$2,$3
	dc.w	$f,$8
	dc.w	$1f,$12
	dc.w	$1d,$17
	dc.w	$e,$12
	dc.w	$a,$16
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$b,$17
	dc.w	$b,$16
	dc.w	$b,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$e,$13
	dc.w	$1d,$17
	dc.w	$1f,$12
	dc.w	$f,$8
	dc.w	$2,$3
	dc.w	$0,$3
	dc.w	$0,$1
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_25_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_25_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$b,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$0,$0
	dc.w	$1,$0
	dc.w	$1,$0
	dc.w	$1,$0
	dc.w	$0,$0
	dc.w	$1,$0
	dc.w	$0,$0
	dc.w	$1,$0
	dc.w	$1,$0
	dc.w	$1,$0
	dc.w	$1,$0
	dc.w	$1,$0
	dc.w	$1,$0
	dc.w	$5,$0
	dc.w	$4,$0
	dc.w	$b,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$3,$0
	dc.w	$1,$0
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_26_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_26_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$100,$ff00
	dc.w	$ffc0,$40
	dc.w	$3e0,$fd20
	dc.w	$2e0,$3a0
	dc.w	$1c0,$7f28
	dc.w	$140,$ffac
	dc.w	$e0,$ffbe
	dc.w	$0,$f03f
	dc.w	$0,$e01f
	dc.w	$100,$ff1f
	dc.w	$ffc0,$4e
	dc.w	$3e0,$fd20
	dc.w	$2e0,$e3a0
	dc.w	$1c0,$ff28
	dc.w	$140,$ffac
	dc.w	$140,$ffae
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$1c0,$e12f
	dc.w	$2e0,$e3af
	dc.w	$3e0,$fd3f
	dc.w	$ffc0,$5f
	dc.w	$100,$ff1f
	dc.w	$0,$f03f
	dc.w	$0,$fffe
	dc.w	$0,$fffc
	dc.w	$0,$7ff8

SPRITE_27_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_27_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$80,$0
	dc.w	$80,$0
	dc.w	$340,$0
	dc.w	$90,$0
	dc.w	$7e98,$0
	dc.w	$fe1c,$0
	dc.w	$ff5e,$0
	dc.w	$f03f,$0
	dc.w	$e01f,$0
	dc.w	$9f,$0
	dc.w	$8e,$0
	dc.w	$340,$0
	dc.w	$e090,$0
	dc.w	$fe98,$0
	dc.w	$fe1c,$0
	dc.w	$fe1e,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e09f,$0
	dc.w	$e09f,$0
	dc.w	$35f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$f03f,$0
	dc.w	$fffe,$0
	dc.w	$fffc,$0
	dc.w	$7ff8,$0

SPRITE_28_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_28_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$4,$7
	dc.w	$7,$0
	dc.w	$4,$7
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_29_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_29_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$4,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_30_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_30_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$100,$ff00
	dc.w	$ffc0,$40
	dc.w	$3e0,$fd20
	dc.w	$2e0,$3a0
	dc.w	$1c0,$7f28
	dc.w	$140,$ffac
	dc.w	$140,$7fae
	dc.w	$140,$1af
	dc.w	$1c0,$12f
	dc.w	$2f0,$3bf
	dc.w	$5a0,$77f
	dc.w	$b40,$edf
	dc.w	$1680,$1d9f
	dc.w	$2d00,$3b3e
	dc.w	$3a00,$367e
	dc.w	$1c00,$24fc
	dc.w	$1400,$2df8
	dc.w	$1400,$2ff0
	dc.w	$1400,$2fe0
	dc.w	$1400,$2fc0
	dc.w	$1400,$2fc0
	dc.w	$1000,$2bc0
	dc.w	$3c00,$2fc0
	dc.w	$0,$7c0
	dc.w	$0,$7c0
	dc.w	$0,$7c0
	dc.w	$0,$380

SPRITE_31_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_31_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$80,$0
	dc.w	$80,$0
	dc.w	$340,$0
	dc.w	$90,$0
	dc.w	$7e98,$0
	dc.w	$fe1c,$0
	dc.w	$7e1e,$0
	dc.w	$1f,$0
	dc.w	$9f,$0
	dc.w	$8f,$0
	dc.w	$11f,$0
	dc.w	$21f,$0
	dc.w	$41f,$0
	dc.w	$83e,$0
	dc.w	$507e,$0
	dc.w	$48fc,$0
	dc.w	$41f8,$0
	dc.w	$43f0,$0
	dc.w	$43e0,$0
	dc.w	$43c0,$0
	dc.w	$43c0,$0
	dc.w	$47c0,$0
	dc.w	$13c0,$0
	dc.w	$7c0,$0
	dc.w	$7c0,$0
	dc.w	$7c0,$0
	dc.w	$380,$0

SPRITE_32_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_32_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$2,$3
	dc.w	$f,$8
	dc.w	$1f,$12
	dc.w	$d,$17
	dc.w	$e,$12
	dc.w	$a,$16
	dc.w	$a,$17
	dc.w	$f,$13
	dc.w	$d,$17
	dc.w	$1a,$13
	dc.w	$f,$8
	dc.w	$1f,$12
	dc.w	$d,$17
	dc.w	$f,$13
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$f,$13
	dc.w	$d,$17
	dc.w	$1f,$12
	dc.w	$f,$8
	dc.w	$2,$3
	dc.w	$0,$3
	dc.w	$0,$1
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_33_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_33_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$b,$0
	dc.w	$24,$0
	dc.w	$24,$0
	dc.w	$20,$0
	dc.w	$21,$0
	dc.w	$24,$0
	dc.w	$24,$0
	dc.w	$c,$0
	dc.w	$4,$0
	dc.w	$b,$0
	dc.w	$24,$0
	dc.w	$24,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$21,$0
	dc.w	$24,$0
	dc.w	$24,$0
	dc.w	$b,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$3,$0
	dc.w	$1,$0
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_34_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_34_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$100,$ff00
	dc.w	$ffc0,$40
	dc.w	$3e0,$fd20
	dc.w	$2c0,$3a0
	dc.w	$1c0,$7f28
	dc.w	$140,$ffac
	dc.w	$140,$ffae
	dc.w	$1c0,$f12f
	dc.w	$82c0,$e3af
	dc.w	$3f0,$fd3f
	dc.w	$ffe0,$7f
	dc.w	$170,$ff3f
	dc.w	$8240,$f3af
	dc.w	$140,$ffae
	dc.w	$140,$ffac
	dc.w	$140,$ffae
	dc.w	$140,$f1af
	dc.w	$140,$e1af
	dc.w	$1c0,$e12f
	dc.w	$82d0,$e3bf
	dc.w	$3e0,$fd3f
	dc.w	$ffc0,$5f
	dc.w	$100,$ff1f
	dc.w	$0,$f03f
	dc.w	$0,$fffe
	dc.w	$0,$fffc
	dc.w	$0,$7ff8

SPRITE_35_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_35_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$80,$0
	dc.w	$80,$0
	dc.w	$340,$0
	dc.w	$90,$0
	dc.w	$7e98,$0
	dc.w	$fe1c,$0
	dc.w	$fe1e,$0
	dc.w	$f09f,$0
	dc.w	$609f,$0
	dc.w	$34f,$0
	dc.w	$9f,$0
	dc.w	$cf,$0
	dc.w	$701f,$0
	dc.w	$fe1e,$0
	dc.w	$fe1c,$0
	dc.w	$fe1e,$0
	dc.w	$f01f,$0
	dc.w	$e01f,$0
	dc.w	$e09f,$0
	dc.w	$608f,$0
	dc.w	$35f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$f03f,$0
	dc.w	$fffe,$0
	dc.w	$fffc,$0
	dc.w	$7ff8,$0

SPRITE_36_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_36_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$2,$3
	dc.w	$f,$8
	dc.w	$1f,$12
	dc.w	$d,$17
	dc.w	$e,$12
	dc.w	$a,$16
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$a,$17
	dc.w	$f,$13
	dc.w	$d,$17
	dc.w	$1f,$12
	dc.w	$f,$8
	dc.w	$2,$3
	dc.w	$0,$3
	dc.w	$0,$1
	dc.w	$1c,$14
	dc.w	$a,$16
	dc.w	$e,$12
	dc.w	$d,$17
	dc.w	$1f,$12
	dc.w	$f,$8
	dc.w	$2,$3
	dc.w	$0,$3
	dc.w	$0,$1
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_37_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_37_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$b,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$0,$0
	dc.w	$1,$0
	dc.w	$1,$0
	dc.w	$1,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$b,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$3,$0
	dc.w	$1,$0
	dc.w	$8,$0
	dc.w	$0,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$b,$0
	dc.w	$4,$0
	dc.w	$4,$0
	dc.w	$3,$0
	dc.w	$1,$0
	dc.w	$0,$0
	dc.w	$0,$0

SPRITE_38_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_38_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$100,$ff00
	dc.w	$ffc0,$40
	dc.w	$3e0,$fd20
	dc.w	$2d0,$3b0
	dc.w	$1c0,$7f28
	dc.w	$140,$ffac
	dc.w	$140,$ffae
	dc.w	$140,$f1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$8140,$e1af
	dc.w	$340,$ffaf
	dc.w	$ff40,$1af
	dc.w	$340,$ffaf
	dc.w	$140,$f1af
	dc.w	$140,$ffaf
	dc.w	$140,$ffaf
	dc.w	$140,$7faf
	dc.w	$1c0,$12f
	dc.w	$2d0,$3bf
	dc.w	$3e0,$fd3f
	dc.w	$ffc0,$5f
	dc.w	$100,$ff1f
	dc.w	$0,$f03f
	dc.w	$0,$fffe
	dc.w	$0,$fffc
	dc.w	$0,$7ff8

SPRITE_39_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_39_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$80,$0
	dc.w	$80,$0
	dc.w	$340,$0
	dc.w	$80,$0
	dc.w	$7e98,$0
	dc.w	$fe1c,$0
	dc.w	$fe1e,$0
	dc.w	$f01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$601f,$0
	dc.w	$1f,$0
	dc.w	$21f,$0
	dc.w	$1f,$0
	dc.w	$f01f,$0
	dc.w	$fe1f,$0
	dc.w	$fe1f,$0
	dc.w	$7e1f,$0
	dc.w	$9f,$0
	dc.w	$8f,$0
	dc.w	$35f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$f03f,$0
	dc.w	$fffe,$0
	dc.w	$fffc,$0
	dc.w	$7ff8,$0

SPRITE_40_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_40_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$e02,$a03
	dc.w	$140f,$1a08
	dc.w	$2e1f,$3212
	dc.w	$181d,$2417
	dc.w	$2e0e,$32d2
	dc.w	$140a,$1ad6
	dc.w	$140a,$1ad7
	dc.w	$140a,$1ad7
	dc.w	$140a,$1ad7
	dc.w	$140a,$1ad7
	dc.w	$140a,$1ad7
	dc.w	$140a,$1ad7
	dc.w	$140a,$1ad7
	dc.w	$140a,$1ad7
	dc.w	$140a,$1ad7
	dc.w	$140a,$1ad7
	dc.w	$140a,$1ad7
	dc.w	$140a,$1ad7
	dc.w	$140e,$1ad3
	dc.w	$141d,$1ad7
	dc.w	$141f,$1af2
	dc.w	$140f,$1af8
	dc.w	$e02,$bf3
	dc.w	$0,$1f3
	dc.w	$0,$1f1
	dc.w	$0,$1f0
	dc.w	$0,$e0

SPRITE_41_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_41_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$404,$0
	dc.w	$104,$0
	dc.w	$50b,$0
	dc.w	$a24,$0
	dc.w	$4e4,$0
	dc.w	$1e0,$0
	dc.w	$1e1,$0
	dc.w	$1e1,$0
	dc.w	$1e1,$0
	dc.w	$1e1,$0
	dc.w	$1e1,$0
	dc.w	$1e1,$0
	dc.w	$1e1,$0
	dc.w	$1e1,$0
	dc.w	$1e1,$0
	dc.w	$1e1,$0
	dc.w	$1e1,$0
	dc.w	$1e1,$0
	dc.w	$1e5,$0
	dc.w	$1e4,$0
	dc.w	$1eb,$0
	dc.w	$1f4,$0
	dc.w	$5f4,$0
	dc.w	$1f3,$0
	dc.w	$1f1,$0
	dc.w	$1f0,$0
	dc.w	$e0,$0

SPRITE_42_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_42_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$100,$ff00
	dc.w	$ffc0,$40
	dc.w	$3e0,$fd20
	dc.w	$2e0,$3a0
	dc.w	$1c0,$7f28
	dc.w	$140,$ffac
	dc.w	$140,$ffae
	dc.w	$140,$f1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$1c0,$e12f
	dc.w	$2e0,$e3af
	dc.w	$3e0,$fd3f
	dc.w	$ffc0,$5f
	dc.w	$100,$ff1f
	dc.w	$0,$f03f
	dc.w	$0,$fffe
	dc.w	$0,$fffc
	dc.w	$0,$7ff8

SPRITE_43_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_43_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$80,$0
	dc.w	$80,$0
	dc.w	$340,$0
	dc.w	$90,$0
	dc.w	$7e98,$0
	dc.w	$fe1c,$0
	dc.w	$fe1e,$0
	dc.w	$f01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e09f,$0
	dc.w	$e09f,$0
	dc.w	$35f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$f03f,$0
	dc.w	$fffe,$0
	dc.w	$fffc,$0
	dc.w	$7ff8,$0

SPRITE_44_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_44_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$e00,$a00
	dc.w	$1400,$1a00
	dc.w	$2e00,$3200
	dc.w	$1800,$2400
	dc.w	$2e00,$32e0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1ae0
	dc.w	$1400,$1ae0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$e00,$bf0
	dc.w	$0,$1f0
	dc.w	$0,$1f0
	dc.w	$0,$1f0
	dc.w	$0,$e0

SPRITE_45_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_45_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$400,$0
	dc.w	$100,$0
	dc.w	$500,$0
	dc.w	$a00,$0
	dc.w	$4e0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1e0,$0
	dc.w	$1e0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$5f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$e0,$0

SPRITE_46_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_46_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$e00,$a00
	dc.w	$1400,$1a00
	dc.w	$2e00,$3200
	dc.w	$1800,$2400
	dc.w	$2e00,$32e0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1ae0
	dc.w	$1400,$1ae0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$e00,$bf0
	dc.w	$0,$1f0
	dc.w	$0,$1f0
	dc.w	$0,$1f0
	dc.w	$0,$e0

SPRITE_47_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_47_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$400,$0
	dc.w	$100,$0
	dc.w	$500,$0
	dc.w	$a00,$0
	dc.w	$4e0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1e0,$0
	dc.w	$1e0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$5f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$e0,$0

SPRITE_48_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_48_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$e02,$a03
	dc.w	$140f,$1a08
	dc.w	$2e1f,$3212
	dc.w	$181d,$2417
	dc.w	$2e0e,$32d2
	dc.w	$140a,$1ad6
	dc.w	$141c,$1af5
	dc.w	$1400,$1ae3
	dc.w	$1400,$1ae3
	dc.w	$1400,$1af3
	dc.w	$1402,$1af3
	dc.w	$140f,$1af8
	dc.w	$141f,$1af2
	dc.w	$141d,$1ad7
	dc.w	$140e,$1ad2
	dc.w	$140a,$1ad6
	dc.w	$140a,$1ad7
	dc.w	$140a,$1ad7
	dc.w	$140e,$1ad3
	dc.w	$141d,$1ad7
	dc.w	$141f,$1af2
	dc.w	$140f,$1af8
	dc.w	$e02,$bf3
	dc.w	$0,$1f3
	dc.w	$0,$1f1
	dc.w	$0,$1f0
	dc.w	$0,$e0

SPRITE_49_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_49_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$404,$0
	dc.w	$104,$0
	dc.w	$50b,$0
	dc.w	$a24,$0
	dc.w	$4e4,$0
	dc.w	$1e0,$0
	dc.w	$1e9,$0
	dc.w	$1e3,$0
	dc.w	$1e3,$0
	dc.w	$1f3,$0
	dc.w	$1f4,$0
	dc.w	$1f4,$0
	dc.w	$1eb,$0
	dc.w	$1e4,$0
	dc.w	$1e4,$0
	dc.w	$1e0,$0
	dc.w	$1e1,$0
	dc.w	$1e1,$0
	dc.w	$1e5,$0
	dc.w	$1e4,$0
	dc.w	$1eb,$0
	dc.w	$1f4,$0
	dc.w	$5f4,$0
	dc.w	$1f3,$0
	dc.w	$1f1,$0
	dc.w	$1f0,$0
	dc.w	$e0,$0

SPRITE_50_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_50_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$100,$ff00
	dc.w	$ffc0,$40
	dc.w	$3e0,$fd20
	dc.w	$2e0,$3a0
	dc.w	$1c0,$7f28
	dc.w	$140,$ffac
	dc.w	$140,$ffae
	dc.w	$140,$f1af
	dc.w	$1c0,$e12f
	dc.w	$2e0,$e3af
	dc.w	$3e0,$fd3f
	dc.w	$ffc0,$5f
	dc.w	$100,$ff1f
	dc.w	$0,$3f
	dc.w	$0,$7ffe
	dc.w	$0,$fffc
	dc.w	$0,$fff8
	dc.w	$0,$f000
	dc.w	$0,$e000
	dc.w	$0,$e000
	dc.w	$20,$ffe0
	dc.w	$fff0,$10
	dc.w	$20,$ffe0
	dc.w	$0,$f000
	dc.w	$0,$fffe
	dc.w	$0,$ffff
	dc.w	$0,$7ffe

SPRITE_51_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_51_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$80,$0
	dc.w	$80,$0
	dc.w	$340,$0
	dc.w	$90,$0
	dc.w	$7e98,$0
	dc.w	$fe1c,$0
	dc.w	$fe1e,$0
	dc.w	$f01f,$0
	dc.w	$e09f,$0
	dc.w	$e09f,$0
	dc.w	$35f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$3f,$0
	dc.w	$7ffe,$0
	dc.w	$fffc,$0
	dc.w	$fff8,$0
	dc.w	$f000,$0
	dc.w	$e000,$0
	dc.w	$e000,$0
	dc.w	$0,$0
	dc.w	$20,$0
	dc.w	$0,$0
	dc.w	$f000,$0
	dc.w	$fffe,$0
	dc.w	$ffff,$0
	dc.w	$7ffe,$0

SPRITE_52_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_52_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$e02,$a03
	dc.w	$140f,$1a08
	dc.w	$2e1f,$3212
	dc.w	$181d,$2417
	dc.w	$2e0e,$32f2
	dc.w	$140a,$1af6
	dc.w	$141c,$1af5
	dc.w	$1400,$1ae3
	dc.w	$1400,$1ae3
	dc.w	$1400,$1af3
	dc.w	$1400,$1af1
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$141c,$1af4
	dc.w	$140a,$1af6
	dc.w	$140e,$1af2
	dc.w	$141d,$1af7
	dc.w	$141f,$1af2
	dc.w	$140f,$1af8
	dc.w	$e02,$bf3
	dc.w	$0,$1f3
	dc.w	$0,$1f1
	dc.w	$0,$1f0
	dc.w	$0,$e0

SPRITE_53_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_53_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$404,$0
	dc.w	$104,$0
	dc.w	$50b,$0
	dc.w	$a04,$0
	dc.w	$4e4,$0
	dc.w	$1e0,$0
	dc.w	$1e9,$0
	dc.w	$1e3,$0
	dc.w	$1e3,$0
	dc.w	$1f3,$0
	dc.w	$1f1,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1e8,$0
	dc.w	$1e0,$0
	dc.w	$1e4,$0
	dc.w	$1e4,$0
	dc.w	$1eb,$0
	dc.w	$1f4,$0
	dc.w	$5f4,$0
	dc.w	$1f3,$0
	dc.w	$1f1,$0
	dc.w	$1f0,$0
	dc.w	$e0,$0

SPRITE_54_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_54_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$100,$ff00
	dc.w	$ffc0,$40
	dc.w	$3e0,$fd20
	dc.w	$2e0,$3a0
	dc.w	$1c0,$7f28
	dc.w	$140,$ffac
	dc.w	$140,$ffae
	dc.w	$140,$f1af
	dc.w	$1c0,$e12f
	dc.w	$2e0,$e3af
	dc.w	$be0,$cd3f
	dc.w	$1fc0,$105f
	dc.w	$be0,$d3f
	dc.w	$2e0,$3af
	dc.w	$1c0,$12e
	dc.w	$140,$1ac
	dc.w	$140,$1ae
	dc.w	$140,$1af
	dc.w	$1c0,$12f
	dc.w	$2e0,$3af
	dc.w	$3e0,$fd3f
	dc.w	$ffc0,$5f
	dc.w	$100,$ff1f
	dc.w	$0,$f03f
	dc.w	$0,$fffe
	dc.w	$0,$fffc
	dc.w	$0,$7ff8

SPRITE_55_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_55_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$80,$0
	dc.w	$80,$0
	dc.w	$340,$0
	dc.w	$80,$0
	dc.w	$7e98,$0
	dc.w	$fe1c,$0
	dc.w	$fe1e,$0
	dc.w	$f01f,$0
	dc.w	$e09f,$0
	dc.w	$e09f,$0
	dc.w	$c35f,$0
	dc.w	$89f,$0
	dc.w	$35f,$0
	dc.w	$9f,$0
	dc.w	$9e,$0
	dc.w	$1c,$0
	dc.w	$1e,$0
	dc.w	$1f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$35f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$f03f,$0
	dc.w	$fffe,$0
	dc.w	$fffc,$0
	dc.w	$7ff8,$0

SPRITE_56_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_56_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$e1c,$a14
	dc.w	$140a,$1a16
	dc.w	$2e0a,$3216
	dc.w	$180a,$2416
	dc.w	$2e0a,$32f7
	dc.w	$140a,$1af7
	dc.w	$140a,$1af7
	dc.w	$140a,$1af7
	dc.w	$140e,$1af3
	dc.w	$141d,$1af7
	dc.w	$141f,$1af2
	dc.w	$140f,$1af8
	dc.w	$1402,$1af3
	dc.w	$1400,$1af3
	dc.w	$1400,$1af1
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$e00,$bf0
	dc.w	$0,$1f0
	dc.w	$0,$1f0
	dc.w	$0,$1f0
	dc.w	$0,$e0

SPRITE_57_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_57_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$408,$0
	dc.w	$100,$0
	dc.w	$500,$0
	dc.w	$a00,$0
	dc.w	$4e1,$0
	dc.w	$1e1,$0
	dc.w	$1e1,$0
	dc.w	$1e1,$0
	dc.w	$1e5,$0
	dc.w	$1e4,$0
	dc.w	$1eb,$0
	dc.w	$1f4,$0
	dc.w	$1f4,$0
	dc.w	$1f3,$0
	dc.w	$1f1,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$5f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$e0,$0

SPRITE_58_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_58_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$e0,$a0
	dc.w	$140,$1a0
	dc.w	$140,$1a0
	dc.w	$140,$1a0
	dc.w	$140,$c1ae
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$140,$e1af
	dc.w	$340,$ffaf
	dc.w	$ff40,$1af
	dc.w	$340,$ffaf
	dc.w	$140,$f1af
	dc.w	$140,$ffaf
	dc.w	$140,$ffaf
	dc.w	$140,$7faf
	dc.w	$140,$1af
	dc.w	$140,$1af
	dc.w	$140,$1af
	dc.w	$140,$1af
	dc.w	$140,$1af
	dc.w	$e0,$bf
	dc.w	$0,$1f
	dc.w	$0,$1f
	dc.w	$0,$1f
	dc.w	$0,$e

SPRITE_59_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_59_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$40,$0
	dc.w	$10,$0
	dc.w	$10,$0
	dc.w	$10,$0
	dc.w	$c01e,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$e01f,$0
	dc.w	$1f,$0
	dc.w	$21f,$0
	dc.w	$1f,$0
	dc.w	$f01f,$0
	dc.w	$fe1f,$0
	dc.w	$fe1f,$0
	dc.w	$7e1f,$0
	dc.w	$1f,$0
	dc.w	$1f,$0
	dc.w	$1f,$0
	dc.w	$1f,$0
	dc.w	$1f,$0
	dc.w	$5f,$0
	dc.w	$1f,$0
	dc.w	$1f,$0
	dc.w	$1f,$0
	dc.w	$e,$0

SPRITE_60_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_60_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$e02,$a03
	dc.w	$140f,$1a08
	dc.w	$2e1f,$3212
	dc.w	$181d,$2417
	dc.w	$2e0e,$32d2
	dc.w	$140a,$1ad6
	dc.w	$140a,$1ad7
	dc.w	$140e,$1ad3
	dc.w	$141d,$1ad7
	dc.w	$141f,$1af2
	dc.w	$140f,$1af8
	dc.w	$1402,$1af3
	dc.w	$1400,$1af3
	dc.w	$1400,$1af1
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$1400,$1af0
	dc.w	$141c,$1af4
	dc.w	$140a,$1ad6
	dc.w	$140c,$1ad2
	dc.w	$141f,$1af2
	dc.w	$140f,$1af8
	dc.w	$e02,$bf3
	dc.w	$0,$1f3
	dc.w	$0,$1f1
	dc.w	$0,$1f0
	dc.w	$0,$e0

SPRITE_61_CTL:

	dc.b	$a0,$7d	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_61_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$404,$0
	dc.w	$104,$0
	dc.w	$50b,$0
	dc.w	$a24,$0
	dc.w	$4e4,$0
	dc.w	$1e0,$0
	dc.w	$1e1,$0
	dc.w	$1e5,$0
	dc.w	$1e4,$0
	dc.w	$1eb,$0
	dc.w	$1f4,$0
	dc.w	$1f4,$0
	dc.w	$1f3,$0
	dc.w	$1f1,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1f0,$0
	dc.w	$1e8,$0
	dc.w	$1e0,$0
	dc.w	$1e4,$0
	dc.w	$1eb,$0
	dc.w	$1f4,$0
	dc.w	$5f4,$0
	dc.w	$1f3,$0
	dc.w	$1f1,$0
	dc.w	$1f0,$0
	dc.w	$e0,$0

SPRITE_62_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$0	; VSTOP Lo,Hi

SPRITE_62_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$20,$ffe0
	dc.w	$fff0,$10
	dc.w	$20,$ffe0
	dc.w	$0,$0
	dc.w	$0,$7ffe
	dc.w	$0,$ffff
	dc.w	$0,$fffe
	dc.w	$0,$f000
	dc.w	$0,$e000
	dc.w	$100,$ff00
	dc.w	$ffc0,$40
	dc.w	$3e0,$fd20
	dc.w	$2e0,$f3a0
	dc.w	$1c0,$ff28
	dc.w	$140,$ffac
	dc.w	$140,$7fae
	dc.w	$140,$1af
	dc.w	$140,$1af
	dc.w	$1c0,$12f
	dc.w	$2e0,$3af
	dc.w	$3e0,$fd3f
	dc.w	$ffc0,$5f
	dc.w	$100,$ff1f
	dc.w	$0,$e03f
	dc.w	$0,$fffe
	dc.w	$0,$fffc
	dc.w	$0,$7ff8

SPRITE_63_CTL:

	dc.b	$a0,$85	; Y,X
	dc.b	$c0,$80	; VSTOP Lo,Hi

SPRITE_63_DATA:

; Sprite Data

	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$20,$0
	dc.w	$0,$0
	dc.w	$0,$0
	dc.w	$7ffe,$0
	dc.w	$ffff,$0
	dc.w	$fffe,$0
	dc.w	$f000,$0
	dc.w	$e000,$0
	dc.w	$80,$0
	dc.w	$80,$0
	dc.w	$340,$0
	dc.w	$f090,$0
	dc.w	$fe98,$0
	dc.w	$fe1c,$0
	dc.w	$7e1e,$0
	dc.w	$1f,$0
	dc.w	$1f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$35f,$0
	dc.w	$9f,$0
	dc.w	$9f,$0
	dc.w	$e03f,$0
	dc.w	$fffe,$0
	dc.w	$fffc,$0
	dc.w	$7ff8,$0
