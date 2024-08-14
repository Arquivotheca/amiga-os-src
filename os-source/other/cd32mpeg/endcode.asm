*
* endcode.asm
*
	section text,data

	XDEF EndCode
	XDEF _CL450MicroCode
	XDEF _DramDumpStart
	XDEF _DramDumpEnd
        XDEF _DumpCL450Flag
	XDEF _Y_START
	XDEF _C_START
	XDEF _FB_ADDR

_DramDumpStart:
	dc.l	$100

_DramDumpEnd:
	dc.l	$200

_DumpCL450Flag:
	dc.l	1

_Y_START:
	dc.l	$8580

_C_START:
	dc.l	$2D780

_FB_ADDR:
	dc.l	$170

_CL450MicroCode:
	INCBIN	"dram450.bin"

EndCode:
	END
