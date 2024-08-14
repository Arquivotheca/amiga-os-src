
**
** TEST CODE - make CDGPrefs - create sprites from data table for test
** purposes

	INCLUDE	"exec/types.i"
	INCLUDE	"/cdgprefs.i"

	SECTION	test

	INCLUDE	"dospr.i"


		XDEF	_MakeCDGPrefs
		XDEF	_FreeCDGPrefs

_MakeCDGPrefs:
		move.l	4(sp),a0

		movem.l	a5-a6,-(sp)

		move.l	a0,a5

	; copy sprite colors
		
		lea	SPRITE_COLORS(pc),a0
		moveq	#7,d0

		lea	cdgp_SpriteColors(a5),a1
copycolors:
		move.w	(a0)+,(a1)+
		dbf	d0,copycolors

	; set size

		move.w	#30,cdgp_SpriteHeight(a5)

	; copy pointers too - notice this only works in CHIP ram!!

		lea	pointers(pc),a0
		move.l	a5,a1
		moveq	#63,d0

copyptrs:
		move.l	(a0)+,(a1)+
		dbf	d0,copyptrs

	; for testing purposes, set-up pointers to these channel numbers
	; for PLAY/PAUSE, etc. sprites

		lea	cdgp_PAUSESprite(a5),a1
		lea	channel1(pc),a0

		moveq	#((7*4)-1),d1
copymodes:
		move.l	(a0)+,(a1)+
		dbf	d1,copymodes
		
	; d0 = TRUE
		movem.l	(sp)+,a5-a6
_FreeCDGPrefs:
		rts

pointers:
		dc.l	SPRITE_0_DATA
		dc.l	SPRITE_1_DATA
		dc.l	SPRITE_2_DATA
		dc.l	SPRITE_3_DATA
channel1:
		dc.l	SPRITE_4_DATA
		dc.l	SPRITE_5_DATA
		dc.l	SPRITE_6_DATA
		dc.l	SPRITE_7_DATA
		dc.l	SPRITE_8_DATA
		dc.l	SPRITE_9_DATA

		dc.l	SPRITE_10_DATA
		dc.l	SPRITE_11_DATA
		dc.l	SPRITE_12_DATA
		dc.l	SPRITE_13_DATA
		dc.l	SPRITE_14_DATA
		dc.l	SPRITE_15_DATA
		dc.l	SPRITE_16_DATA
		dc.l	SPRITE_17_DATA
		dc.l	SPRITE_18_DATA
		dc.l	SPRITE_19_DATA

		dc.l	SPRITE_20_DATA
		dc.l	SPRITE_21_DATA
		dc.l	SPRITE_22_DATA
		dc.l	SPRITE_23_DATA
		dc.l	SPRITE_24_DATA
		dc.l	SPRITE_25_DATA
		dc.l	SPRITE_26_DATA
		dc.l	SPRITE_27_DATA
		dc.l	SPRITE_28_DATA
		dc.l	SPRITE_29_DATA

		dc.l	SPRITE_30_DATA
		dc.l	SPRITE_31_DATA
		dc.l	SPRITE_32_DATA
		dc.l	SPRITE_33_DATA
		dc.l	SPRITE_34_DATA
		dc.l	SPRITE_35_DATA
		dc.l	SPRITE_36_DATA
		dc.l	SPRITE_37_DATA
		dc.l	SPRITE_38_DATA
		dc.l	SPRITE_39_DATA

		dc.l	SPRITE_40_DATA
		dc.l	SPRITE_41_DATA
		dc.l	SPRITE_42_DATA
		dc.l	SPRITE_43_DATA
		dc.l	SPRITE_44_DATA
		dc.l	SPRITE_45_DATA
		dc.l	SPRITE_46_DATA
		dc.l	SPRITE_47_DATA
		dc.l	SPRITE_48_DATA
		dc.l	SPRITE_49_DATA

		dc.l	SPRITE_50_DATA
		dc.l	SPRITE_51_DATA
		dc.l	SPRITE_52_DATA
		dc.l	SPRITE_53_DATA
		dc.l	SPRITE_54_DATA
		dc.l	SPRITE_55_DATA
		dc.l	SPRITE_56_DATA
		dc.l	SPRITE_57_DATA
		dc.l	SPRITE_58_DATA
		dc.l	SPRITE_59_DATA

		dc.l	SPRITE_60_DATA
		dc.l	SPRITE_61_DATA
		dc.l	SPRITE_62_DATA
		dc.l	SPRITE_63_DATA
