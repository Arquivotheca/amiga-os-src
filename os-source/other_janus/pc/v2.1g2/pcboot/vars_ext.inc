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
if	(scroll ge 1)	
extrn	 ScrollFlag:byte	   ; Scroll status
endif
extrn	 WaitFlag:byte		   ; Wait for service come available
extrn	 DummyFlag:byte		   ; Free
;
extrn	 ustack:word		   ; User stack
extrn	 sstack:word		   ; System stack
;
extrn	 HandlerPtr:dword	   ; temp pointer to service handler
if	(kludge16 ge 1)	
extrn	 ticks:word		   ; timer tick counter	
endif
extrn	 IntCount:word		   ; interrupt counter for HW diagnostic	
extrn	 ServStatTab:byte  	   ; Table for pending services 	
extrn	 space:byte 		   ; Free	

if	(counters ge 1)
extrn	irq3_count:byte
extrn	irq3_reenter:byte
extrn	irq3_hw_count:byte
extrn	irq3_hw_reenter:byte
extrn	irq3_sw_count:byte
extrn	irq3_sw_reenter:byte
extrn	irq3_hardexit:byte
endif
;
;
;-----------------------------------------------------------------------------
.list

