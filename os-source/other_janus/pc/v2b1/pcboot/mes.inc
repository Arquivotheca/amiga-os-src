;****************************************************************************
;
; mes.inc contains external messages for PC janus handler
;
;****************************************************************************
.xlist
;
extrn	 inmsg:byte
extrn	 BaseMsg:byte
extrn	 NoBaseMsg:byte
extrn	 NoServiceBaseMsg:byte
extrn	 IntsMsg:byte
extrn	 IdleMsg:byte
extrn	 ParasMsg:byte
extrn	 BufferMsg:byte
extrn	 DosMsgA:byte
extrn	 DosMsgN:byte
extrn	 HWMsgP:byte
extrn	 HWMsgPC:byte
extrn	 HWMsgR:byte
extrn	 IntTblMes:byte
extrn	 FoundIntMes:byte
extrn	 UpdateMsg:byte
extrn	 ExeIntMes:byte
extrn	 ESDI_Mes:byte
extrn	 TestIntMes:byte
extrn	 IntCtrlMes:byte
extrn	 RegMsg:byte
extrn	 NoHanMes:byte
extrn	 NoIntMes:byte
extrn	 NoDriveMsg:byte
extrn	 NoPartMsg:byte
extrn	 LoadErrMsg:byte
extrn	 Diskmsg1:byte
extrn	 WaitStatus:byte
extrn	 MemChunk:byte
extrn	 ServiceSearch:byte
extrn	 MakeService:byte
extrn	 AllocParam:byte
extrn	 InitPointers:byte
extrn	 InitWaitPointers:byte
extrn	 ServiceAvailable:byte
extrn	 ServiceAdded:byte
extrn	 GotServiceInt:byte 
extrn	 CallAmigaService:byte 
extrn	 ChannelMsg:byte
extrn	 HandlerMsg:byte    
extrn	 NextSlotMsg:byte
extrn	 ExitSDMsg:byte
extrn	 UCountMsg:byte
extrn 	 RemoveMsg:byte	
extrn	 ServiceDeleted:byte
extrn	 AmigaCallMsg:byte
extrn	 AmigaAddMsg:byte
extrn	 AmigaDeleteMsg:byte
;
.list

