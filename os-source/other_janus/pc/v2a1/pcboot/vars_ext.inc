;*****************************************************************************
;
; vars_ext.inc declares variable datas of PC janus handler as external;
;	       these datas are defined in vars.inc	   
;
;--- external data -----------------------------------------------------------
.xlist
;
extrn	 chain_vec:dword	   ; pointer to old BIOS INT3 service
extrn	 bios_int10:dword	   ; pointer to BIOS INT10 service
extrn	 bios_int13:dword	   ; pointer to BIOS INT13 service
extrn	 bios_int16:dword	   ; pointer to BIOS INT16 service
extrn	 bios_int1c:dword	   ; pointer to BIOS INT1C service
extrn	 DOS_int28:dword	   ; pointer to DOS idle INT28
;
extrn	 janus_base_seg:word	   ; segment of janus main structure
extrn	 janus_param_seg:word	   ; segment of janus parameter ram
extrn	 janus_buffer_seg:word	   ; segment of janus buffer ram
extrn	 janus_part_base:word	   ; pointer to janus partition tables
;
extrn	 int_req:dword		   ; Set bits for interrupt rquests
extrn	 int_enable:dword	   ; Set bits for enabled ints
;
extrn	 ActiveFlag:byte	   ; Handler mode for multiple entries
extrn	 FakeDosFlag:byte	   ; Status of redirected interrupt vectors
extrn	 ScrollFlag:byte	   ; Scroll status
extrn	 DummyFlag:byte		   ; Free
;
extrn	 ustack:word		   ; User stack
extrn	 sstack:word		   ; System stack
;
extrn	 HandlerPtr:dword	   ; temp pointer to service handler
extrn	 ticks:word		   ; timer tick counter	
extrn	 ServStatTab:byte  	   ; Table for pending services 	
extrn	 space:byte 		   ; Free	
;
;
;-----------------------------------------------------------------------------
.list

