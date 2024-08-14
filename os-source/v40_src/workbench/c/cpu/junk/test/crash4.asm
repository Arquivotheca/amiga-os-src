
	move.l	#$dddd0000,d0
	move.l	#$dddd1111,d1
	move.l	#$dddd2222,d2
	move.l	#$dddd3333,d3
	move.l	#$dddd4444,d4
	move.l	#$dddd5555,d5
	move.l	#$dddd6666,d6
	move.l	#$dddd7777,d7
	move.l	#$aaaa0000,a0
	move.l	#$aaaa1111,a1
	move.l	#$aaaa2222,a2
	move.l	#$aaaa3333,a3
	move.l	#$aaaa4444,a4
	move.l	#$aaaa5555,a5
	move.l	#$aaaa6666,a6
;	move.l	#$aaaa7777,a7

	move.l	#'zack',$00fc0000

	moveq	#0,d0
	rts

	END
