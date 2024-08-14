



PCDosServ struc

	f_function	dw 0
	f_error		dw 0
	f_filehandle	dw 0
	f_access	dw 0
	f_offhi		dw 0
	f_offset	dw 0 
	Xfer		db 43  dup (0)		
	f_name		db 0
	f_name1		db 511 dup (0); quick fix to dump prepended ":" 
PCDosServ ends

