* block_types.i

ST_ROOT		EQU	1
ST_USERDIR	EQU	2
ST_SOFTLINK	EQU	3	; looks like dir, but is stored like file
ST_LINKDIR	EQU	4
ST_FILE		EQU	-3	; must be negative for FIB!
ST_LINKFILE	EQU	-4
