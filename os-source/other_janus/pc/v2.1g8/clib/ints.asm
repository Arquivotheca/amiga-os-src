.model large

_TEXT	segment word public 'CODE'

	public	_enable_int

_enable_int proc far
	sti
	ret
_enable_int endp

	end
