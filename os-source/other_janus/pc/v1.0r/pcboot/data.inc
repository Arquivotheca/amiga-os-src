;*****************************************************************
; File: data.inc
;
; Global data definitions of PC Janus handler
; Defined in vars.inc
;****************************************************************
;
.xlist
;
extrn	 bios_int13:dword	   ; dword pointer to BIOS INT13 service
extrn	 janus_param_seg:word	   ; segment of janus parameter ram
extrn	 janus_buffer_seg:word	   ; segment of janus buffer ram
extrn	 janus_base_seg:word	   ; segment of janus amin structure
extrn	 janus_part_base:word	   ; pointer to janus partition tables
extrn	 chain_vec:dword	   ; pointer to old BIOS INT3 service
extrn	 DOS_int28:dword	   ; pointer to DOS idle INT28
extrn	 int_req:dword		   ; Set bits for interrupt rquests
extrn	 int_enable:dword	   ; Set bits for enabled ints
extrn	 ActiveFlag:byte	   ; Handler mode for multiple entries
extrn	 ustack:word		   ; User stack
extrn	 sstack:word		   ; System stack
extrn	 space:byte		   ; Free
;
;-----------------------------------------------------------------------------
.list
;
extrn	bill_int13:dword
extrn	jparmseg:word
extrn	jparmoff:word
extrn	jbuffseg:word
extrn	jbuffoff:word
extrn	dataseg:word
extrn	dataoff:word
extrn	b_file_open:word
extrn	bk_NumHeads:word
extrn	bk_TrackSecs:word
extrn	billmsg0:byte
extrn	billmsg1:byte
extrn	billmsg2:byte
extrn	billmsg3:byte
extrn	billmsg4:byte
extrn	billmsg5:byte
extrn	AB_file:byte
extrn	okflag:word
