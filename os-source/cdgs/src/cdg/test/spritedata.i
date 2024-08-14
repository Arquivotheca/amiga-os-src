**
**	$Id: spritedata.i,v 1.1 92/1/8 08:00:00 darren Exp $
**
**	CDTV CD+G -- spritedata.i (data for on-screen sprites -
**				   such as channel selection)
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

		SECTION cdg

	; sprite table follows

leftsprites:
		dc.l		0
		dc.l		left1SpriteData+4
		dc.l		left2SpriteData+4
		dc.l		left3SpriteData+4
		dc.l		left4SpriteData+4
		dc.l		left5SpriteData+4
		dc.l		left6SpriteData+4
		dc.l		left7SpriteData+4
		dc.l		left8SpriteData+4
		dc.l		left9SpriteData+4
		dc.l		left10SpriteData+4
		dc.l		left11SpriteData+4
		dc.l		left12SpriteData+4
		dc.l		left13SpriteData+4
		dc.l		left14SpriteData+4
		dc.l		left15SpriteData+4


	; sprite data follows

left1SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000



right1SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$3fff
		dc.w		$fffc,$3fff
		dc.w		$fffc,$3fff
		dc.w		$fffc,$3fff
		dc.w		$fffc,$3fff
		dc.w		$fffc,$3fff
		dc.w		$fffc,$3fff
		dc.w		$fffc,$3fff
		dc.w		$fffc,$0fff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000


left2SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000



right2SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$9fff
		dc.w		$fffc,$3fff
		dc.w		$fffc,$7fff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$0fff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000



left3SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000


right3SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000


left4SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ff9
		dc.w		$ffff,$3ff8
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000



right4SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$9fff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$9fff
		dc.w		$fffc,$9fff
		dc.w		$fffc,$0fff
		dc.w		$fffc,$9fff
		dc.w		$fffc,$9fff
		dc.w		$fffc,$0fff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000
		dc.w		$ffff,$ffff


left5SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000



right5SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$0fff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000



left6SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000



right6SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$7fff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000



left7SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000



right7SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$0fff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$9fff
		dc.w		$fffc,$3fff
		dc.w		$fffc,$3fff
		dc.w		$fffc,$3fff
		dc.w		$fffc,$3fff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000




left8SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000




right8SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000



left9SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffc
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ffe
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000



right9SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$1fff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$0fff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$9fff
		dc.w		$fffc,$3fff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000


left10SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3fe3
		dc.w		$ffff,$3fe3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3fc0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000


right10SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$e1ff
		dc.w		$fffc,$ccff
		dc.w		$fffc,$ccff
		dc.w		$fffc,$c4ff
		dc.w		$fffc,$c0ff
		dc.w		$fffc,$c8ff
		dc.w		$fffc,$ccff
		dc.w		$fffc,$ccff
		dc.w		$fffc,$e1ff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000



left11SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3fe3
		dc.w		$ffff,$3fe3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3fc0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000



right11SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$f3ff
		dc.w		$fffc,$e3ff
		dc.w		$fffc,$e3ff
		dc.w		$fffc,$f3ff
		dc.w		$fffc,$f3ff
		dc.w		$fffc,$f3ff
		dc.w		$fffc,$f3ff
		dc.w		$fffc,$f3ff
		dc.w		$fffc,$c0ff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000



left12SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3fe3
		dc.w		$ffff,$3fe3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3fc0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000



right12SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$e1ff
		dc.w		$fffc,$ccff
		dc.w		$fffc,$ccff
		dc.w		$fffc,$fcff
		dc.w		$fffc,$f9ff
		dc.w		$fffc,$f3ff
		dc.w		$fffc,$e7ff
		dc.w		$fffc,$ccff
		dc.w		$fffc,$c0ff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000


left13SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3fe3
		dc.w		$ffff,$3fe3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3fc0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000


right13SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$e1ff
		dc.w		$fffc,$ccff
		dc.w		$fffc,$fcff
		dc.w		$fffc,$fcff
		dc.w		$fffc,$f1ff
		dc.w		$fffc,$fcff
		dc.w		$fffc,$fcff
		dc.w		$fffc,$ccff
		dc.w		$fffc,$e1ff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000


left14SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3fe3
		dc.w		$ffff,$3fe3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3fc0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000


right14SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$f9ff
		dc.w		$fffc,$f1ff
		dc.w		$fffc,$e1ff
		dc.w		$fffc,$c9ff
		dc.w		$fffc,$99ff
		dc.w		$fffc,$80ff
		dc.w		$fffc,$f9ff
		dc.w		$fffc,$f9ff
		dc.w		$fffc,$f0ff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000


left15SpriteData:
		dc.w		$0000,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$0000
		dc.w		$ffff,$3fff
		dc.w		$ffef,$3ff0
		dc.w		$ffdf,$3fe7
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffbf,$3fcf
		dc.w		$ffdf,$3fe7
		dc.w		$ffef,$3ff0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3fe3
		dc.w		$ffff,$3fe3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3ff3
		dc.w		$ffff,$3fc0
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$ffff,$3fff
		dc.w		$c000,$3fff
		dc.w		$8000,$7fff
		dc.w		$0000,$0000


right15SpriteData:
		dc.w		$0000,$0000
		dc.w		$fffe,$0001
		dc.w		$fffc,$0003
		dc.w		$fffc,$ffff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bffc,$c0ff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$bbfc,$ccff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$c0ff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$cfff
		dc.w		$fffc,$c1ff
		dc.w		$fffc,$fcff
		dc.w		$fffc,$fcff
		dc.w		$fffc,$fcff
		dc.w		$fffc,$ccff
		dc.w		$fffc,$e1ff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$fffc,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$ffff
		dc.w		$0000,$0000


	; and make external

		XDEF	MAXSPRITESIZE

MAXSPRITESIZE	EQU	left2SpriteData-left1SpriteData
ONESPRITESIZE	EQU	right1SpriteData-left1SpriteData
