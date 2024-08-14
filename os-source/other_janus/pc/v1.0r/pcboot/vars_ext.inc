;*****************************************************************************
;
; vars_ext.inc declares variable datas of PC janus handler as external;
;	       these datas are defined in vars.inc	   
;
;--- external data -----------------------------------------------------------
.xlist
;
extrn	        chain_vec:dword
extrn	        bios_int13:dword
extrn	        DOS_int28:dword
extrn	        int_req:dword
extrn	        int_enable:dword
extrn	        janus_part_base:word
extrn		janus_base_seg:word
extrn		janus_param_seg:word
extrn		janus_buffer_seg:word
extrn	        ActiveFlag:byte
extrn	        FakeDosFlag:byte
extrn	        ustack:word
extrn	        sstack:word
extrn		ServStatTab:byte
extrn	        space:byte
;
;-----------------------------------------------------------------------------
.list

