		SECTION	gadgets,DATA
		INCLUDE	"exec/types.i"
		INCLUDE	"intuition/intuition.i"
		INCLUDE	"graphics/text.i"

		XDEF	CancelGadget,UseGadget,SaveGadget
		XDEF	A000Gadget,D000Gadget,E000Gadget
		XDEF	MonoGadget,OffText,OnText
		XDEF	ColorGadget,SerialGadget

; These gadgets can stay here in any kind of memory since they are always
; present and contain no actual imagery that needs to be in chip memory.
; They will always be last on the list of gadgets to allow a variable
; number of gadgets before them.

CancelGadget	DC.L	UseGadget		next gadget
		DC.W	8,14,90,13		left,top,width,height
		DC.W	GADGIMAGE!GADGHBOX	flags
		DC.W	RELVERIFY		activation flags
		DC.W	BOOLGADGET		gadget type
		DC.L	CommonImage		imagery for this gadget
		DC.L	0			no alternate image
		DC.L	CancelText		text for this gadget
		DC.L	0,0			mutualexclude specialinfo
		DC.W	0			Gadget ID
		DC.L	0			user data field
CancelText	DC.B	2,1			frontpen,backpen
		DC.B	RP_JAM2,0		drawmode,klugebyte
		DC.W	13,3			leftedge,topedge
		DC.L	CommonFont		font for all intuitext
		DC.L	CancelLetters		text string
		DC.L	0			next text
CancelLetters	DC.B	'CANCEL',0
		CNOP	0,2

UseGadget	DC.L	SaveGadget		next gadget
		DC.W	105,14,90,13		left,top,width,height
		DC.W	GADGIMAGE!GADGHBOX	flags
		DC.W	RELVERIFY		activation flags
		DC.W	BOOLGADGET		gadget type
		DC.L	CommonImage		imagery for this gadget
		DC.L	0			no alternate image
		DC.L	UseText			text for this gadget
		DC.L	0,0			mutualexclude specialinfo
		DC.W	1			gadget ID
		DC.L	0			user data field
UseText		DC.B	2,1			frontpen,backpen
		DC.B	RP_JAM2,0		draw mode,klugebyte
		DC.W	28,3			leftedge,topedge
		DC.L	CommonFont		font for all intuitext
		DC.L	UseLetters		text string
		DC.L	0			next text
UseLetters	DC.B	'USE',0
		CNOP	0,2

SaveGadget	DC.L	A000Gadget		next gadget
		DC.W	202,14,90,13		left,top,width,height
		DC.W	GADGIMAGE!GADGHBOX	flags
		DC.W	RELVERIFY		activation flags
		DC.W	BOOLGADGET		gadget type
		DC.L	CommonImage		imagery for this gadget
		DC.L	0			no alternate image
		DC.L	SaveText		text for this gadget
		DC.L	0,0
		DC.W	2			gadget ID
		DC.L	0			user data field
SaveText	DC.B	2,1			frontpen,backpen
		DC.B	RP_JAM2,0		drawmode,klugebyte
		DC.W	23,3			leftedge,topedge
		DC.L	CommonFont		font for all intuitext
		DC.L	SaveLetters		text string
		DC.L	0			next text
SaveLetters	DC.B	'SAVE',0
		CNOP	0,2

; A large software fix is required to get BOOLGADGET gadgets working
; with a complemented box around them.  Intuition has major problems when
; trying to refresh these gadgets and often leaves bits of boxes lying
; around the screen.  Mutualexclude and RefreshGadgets also appear to be
; completely broken so I have done all the highlighting myself.

A000Gadget	DC.L	D000Gadget		next gadget
		DC.W	8,34,90,13		left top width height
		DC.W	GADGIMAGE!GADGHNONE	flags
		DC.W	GADGIMMEDIATE!TOGGLESELECT	activation flags
		DC.W	BOOLGADGET		gadget type
		DC.L	CommonImage		imagery for this gadget
		DC.L	0			no alternate image
		DC.L	A000Text		text for this gadget
		DC.L	0,0			mutual exclusion
		DC.W	1			gadget ID (RAM bank)
		DC.L	0			user data field
A000Text	DC.B	2,1			frontpen,backpen
		DC.B	RP_JAM2,0		drawmode,klugebyte
		DC.W	4,3			leftedge,topedge
		DC.L	CommonFont		font for all intuitext
		DC.L	A000Letters		text string
		DC.L	0			next text
A000Letters	DC.B	'RAM=A000',0
		CNOP	0,2

D000Gadget	DC.L	E000Gadget		next gadget
		DC.W	8,50,90,13		left,top,width,height
		DC.W	GADGIMAGE!GADGHNONE	flags
		DC.W    GADGIMMEDIATE!TOGGLESELECT      activation flags
		DC.W    BOOLGADGET              gadget type
		DC.L    CommonImage             imagery for this gadget
		DC.L    0                       no alternate image
		DC.L	D000Text		text for this gadget
		DC.L    0,0			mutual exclusion
		DC.W	2			gadget ID (RAM bank)
		DC.L	0			user data field
D000Text	DC.B	2,1			frontpen,backpen
		DC.B	RP_JAM2,0		drawmode,klugebyte
		DC.W	4,3			leftedge topedge
		DC.L	CommonFont		font for all intuitext
		DC.L	D000Letters		text string
		DC.L	0			next text
D000Letters	DC.B	'RAM=D000',0
		CNOP	0,2

E000Gadget	DC.L	MonoGadget		next gadget
		DC.W	8,66,90,13		left,top,width,height
		DC.W	GADGIMAGE!GADGHNONE	flags
		DC.W	GADGIMMEDIATE!TOGGLESELECT	activation flags
		DC.W	BOOLGADGET		gadget type
		DC.L	CommonImage		imagery for this gadget
		DC.L	0			no alternate image
		DC.L	E000Text		text for this gadget
		DC.L	0,0			mutual exclusion
		DC.W	3			gadget ID (RAM bank)
		DC.L	0			user data field
E000Text	DC.B	2,1			frontpen,backpen
		DC.B	RP_JAM2,0		drawmode,klugebyte
		DC.W	4,3			leftedge,topedge
		DC.L	CommonFont		font for all intuitext
		DC.L	E000Letters		text string
		DC.L	0			next text
E000Letters	DC.B	'RAM=E000',0
		CNOP	0,2

MonoGadget	DC.L	ColorGadget		next gadget
		DC.W	112,34,180,13		left,top,width,height
		DC.W	GADGIMAGE!GADGHCOMP	flags
		DC.W	RELVERIFY!TOGGLESELECT	activation flags
		DC.W	BOOLGADGET		gadget type
		DC.L	WideImage		imagery when unselected
		DC.L	0			no alternate image
		DC.L	MonoText		text for this gadget
		DC.L	0,0			mutual exclude + info
		DC.W	3			gadgetID
		DC.L	0			user data field
MonoText	DC.B	2,1			frontpen,backpen
		DC.B	RP_JAM2,0		drawmode,klugebyte
		DC.W	10,3			leftedge,topedge
		DC.L	CommonFont		font for all intutitext
		DC.L	MonoLetters		text string
		DC.L	0			next text
MonoLetters	DC.B	'Mono video',0
		CNOP	0,2
		
ColorGadget	DC.L	SerialGadget		next gadget
		DC.W	112,50,180,13		left,top,width,height
		DC.W	GADGIMAGE!GADGHCOMP	flags
		DC.W	RELVERIFY!TOGGLESELECT	activation flags
		DC.W	BOOLGADGET		gadgettype
		DC.L	WideImage		gadget imagery
		DC.L	0			no alternate image
		DC.L	ColorText		text for this gadget
		DC.L	0,0
		DC.W	4			gadget ID
		DC.L	0			user data field
ColorText	DC.B	2,1			frontpen,backpen
		DC.B	RP_JAM2,0
		DC.W	10,3			leftedge,topedge
		DC.L	CommonFont		font 
		DC.L	ColorLetters		text string
		DC.L	0			next text
ColorLetters	DC.B	'Color video',0
		CNOP	0,2

SerialGadget	DC.L	0			next gadget
		DC.W	112,66,180,13		left,top,width,height
		DC.W	GADGIMAGE!GADGHCOMP	flags
		DC.W	RELVERIFY!TOGGLESELECT	activation flags
		DC.W	BOOLGADGET		gadget type
		DC.L	WideImage
		DC.L	0			no alternate image
		DC.L	SerialText		text for this gadget
		DC.L	0,0
		DC.W	5			gadgetID
		DC.L	0			user data field
SerialText	DC.B	2,1			frontpen,backpen
		DC.B	RP_JAM2,0		drawmode
		DC.W	10,3			leftedge,topedge
		DC.L	CommonFont		font
		DC.L	SerialLetters		text string
		DC.L	0			next text
SerialLetters	DC.B	'Serial port',0
		CNOP	0,2

CommonImage	DC.W	0,0,90,13		left,top,width,height
		DC.W	2			depth
		DC.L	0			no actual image
		DC.B	0,1			planepick,planeoff
		DC.L	0			no next image

WideImage	DC.W	0,0,180,13		left,top,width,height
		DC.W	2			depth
		DC.L	0			no actual image
		DC.B	0,1			planepick,planeoff
		DC.L	0			next image

CommonFont	DC.L	TopazName		font we want
		DC.W	9			YSize
		DC.B	FS_NORMAL,FPF_ROMFONT	style,flags
TopazName	DC.B	'topaz.font',0
		CNOP	0,2

OffText		DC.B	2,1			frontpen,backpen
		DC.B	RP_JAM2,0		drawmode
		DC.W	145,3			leftedge,topedge
		DC.L	CommonFont		font
		DC.L	OffLetters		text string
		DC.L	0			next text
OffLetters	DC.B	'OFF',0

OnText		DC.B	1,2			fontpen,backpen
		DC.B	RP_JAM2,0		drawmode
		DC.W	145,3			leftedge,topedge
		DC.L	CommonFont
		DC.L	OnLetters
		DC.L	0
OnLetters	DC.B	'ON ',0

		END

