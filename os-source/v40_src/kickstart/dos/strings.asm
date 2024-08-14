
	SECTION strings,DATA

	XDEF	_dos_errstrs

*
* strings for dos (macro means we HAVE to use assem until cape gets STRLEN)
*

STR	MACRO
	dc.b	n2\@-n1\@
n1\@
	dc.b	\1,0
n2\@
	ENDM

_dos_errstrs:
	STR	<'Software Failure'>			; -162
	STR	<'%s failed returncode %ld',$0a>	; -161
	STR	<'%TH USE COUNT',$0a,$0a>		; -160
	STR	<'NAME'>				; -159
	STR	<'%TH DISABLED',$0a>			; -158
	STR	<'%TH INTERNAL',$0a>			; -157
	STR	<'%TH SYSTEM',$0a>			; -156
	STR	<'Fault %3ld'>				; -155
	STR	<'Fail limit: %ld',$0a>			; -154
	STR	<'Bad return code specified'>		; -153
	STR	<'Current_directory'>			; -152
	STR	<'The last command did not set a return code'> ; -151
	STR	<'Last command failed because '>	; -150
	STR	<'Process %N ending'>			; -149
	STR	<'Requested size too small'>		; -148
	STR	<'Requested size too large'>		; -147
	STR	<'Current stack size is %ld bytes',$0a>	; -146
	STR	<'NewShell failed'>			; -145
	STR	<'Missing ELSE or ENDIF'>		; -144
	STR	<'Must be in a command file'>		; -143
	STR	<'More than one directory matches'>	; -142
	STR	<'Can',$27,'t set %s',$0a>		; -141
	STR	<'Block %ld corrupt directory'>		; -140
	STR	<'Block %ld corrupt file'>		; -139
	STR	<'Block %ld bad header type'>		; -138
	STR	<'Block %ld out of range'>		; -137
	STR	<'Block %ld used twice'>		; -136
	STR	<'Error validating %b'>			; -135
	STR	<'on disk block %ld'>			; -134
	STR	<'has a checksum error'>		; -133
	STR	<'has a write error'>			; -132
	STR	<'has a read error'>			; -131
	STR	<'Unable to create process',$0a>	; -130
	STR	<'New Shell process %ld',$0a>		; -129
	STR	<'Cannot open FROM file %s',$0a>	; -128
	STR	<'Suspend|Reboot'>			; -127
	STR	<'Retry|Cancel'>			; -126
	STR	<'No room for bitmap'>			; -125
	STR	<'Command too long'>			; -124
	STR	<'Shell error:'>			; -123
	STR	<'Error in command name'>		; -122
	STR	<'Unknown command'>			; -121
	STR	<'Unable to load'>			; -120
	STR	<'syntax error'>			; -119
	STR	<'unable to open redirection file'>	; -118
	STR	<'Error '>				; -117
	STR	<''>					; -116
	STR	<'Disk corrupt - task stopped'>		; -115
	STR	<'Program failed (error #'>		; -114
	STR	<'Wait for disk activity to finish.'>	; -113
	STR	<'in device %s%s'>			; -112
	STR	<'in unit %ld%s'>			; -111
	STR	<'You MUST replace volume'>		; -110
	STR	<'has a read/write error'>		; -109
	STR	<'No disk present'>			; -108
	STR	<'Not a DOS disk'>			; -107
	STR	<'in any drive'>			; -106
	STR	<'Please replace volume'>		; -105
	STR	<'Please insert volume'>		; -104
	STR	<'is full'>				; -103
	STR	<'is write protected'>			; -102
	STR	<'is not validated'>			; -101
	STR	<'Volume'>				; -100

	STR	<'not enough memory available'>		;  103

	STR	<'process table full'>			;  105

	STR	<'bad template'>			;  114
	STR	<'bad number'>				;  115
	STR	<'required argument missing'>		;  116
	STR	<'value after keyword missing'>		;  117
	STR	<'wrong number of arguments'>		;  118
	STR	<'unmatched quotes'>			;  119
	STR	<'argument line invalid or too long'>	;  120
	STR	<'file is not executable'>		;  121
	STR	<'invalid resident library'>		;  122

	STR	<'object is in use'>			;  202
	STR	<'object already exists'>		;  203
	STR	<'directory not found'>			;  204
	STR	<'object not found'>			;  205
	STR	<'invalid window description'>		;  206
	STR	<'object too large'>			;  207

	STR	<'packet request type unknown'>		;  209
	STR	<'object name invalid'>			;  210
	STR	<'invalid object lock'>			;  211
	STR	<'object is not of required type'>	;  212
	STR	<'disk not validated'>			;  213
	STR	<'disk is write-protected'>		;  214
	STR	<'rename across devices attempted'>	;  215
	STR	<'directory not empty'>			;  216
	STR	<'too many levels'>			;  217
	STR	<'device (or volume) is not mounted'>	;  218
	STR	<'seek failure'>			;  219
	STR	<'comment is too long'>			;  220
	STR	<'disk is full'>			;  221
	STR	<'object is protected from deletion'>	;  222
	STR	<'file is write protected'>		;  223
	STR	<'file is read protected'>		;  224
	STR	<'not a valid DOS disk'>		;  225
	STR	<'no disk in drive'>			;  226

	STR	<'no more entries in directory'>	;  232
	STR	<'object is soft link'>			;  233
	STR	<'object is linked'>			;  234
	STR	<'bad loadfile hunk'>			;  235
	STR	<'function not implemented'>		;  236

	STR	<'record not locked'>			;  240
	STR	<'record lock collision'>		;  241
	STR	<'record lock timeout'>			;  242
	STR	<'record unlock error'>			;  243

	STR	<'buffer overflow'>			;  303
	STR	<'***Break'>				;  304
	STR	<'file not executable'>			;  305

	dc.b    0

	END
