
		section	strings.asm,DATA

STRINGARRAY		EQU		1	
		include 'cat_text.asm'

AS_LAST

		cnop	0,4

		xdef _AppStrings
_AppStrings:
		dc.l	AppStrings

		xdef _NumAppStrings
_NumAppStrings:
		dc.l	(AS_LAST-AS0)/8

		end
