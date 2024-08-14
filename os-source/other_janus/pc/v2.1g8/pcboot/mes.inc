;****************************************************************************
;
; mes.inc contains external messages for PC janus handler
;
;****************************************************************************
.xlist
;
extrn	 BaseMsg:byte
extrn	 NoBaseMsg:byte
extrn	 NoServiceBaseMsg:byte
extrn	 HandlerVersionMsg:byte
extrn	 LibVersionMsg:byte
extrn	 WrongVersionMsg:byte
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
extrn	 AutoLoadService:byte
extrn	 AutoLoadWait:byte
extrn	 ServiceAvailable:byte
extrn	 LeaveGetService:byte
extrn	 ServiceAdded:byte
extrn	 ADDS_EXIT:byte
extrn	 GotServiceInt:byte 
extrn	 CallAmigaService:byte 
extrn	 ChannelMsg:byte
extrn	 HandlerMsg:byte    
extrn	 NextSlotMsg:byte
extrn	 ExitSDMsg:byte
extrn	 DeleteServiceEnter:byte
extrn	 DeleteServiceBase:byte
extrn	 DeleteServiceLock:byte
extrn	 UCountMsg:byte
extrn 	 RemoveMsg:byte	
extrn	 ServiceDeleted:byte
extrn	 AmigaCallMsg:byte
extrn	 AmigaAddMsg:byte
extrn	 AmigaDeleteMsg:byte
extrn	 AJR_StrucMem:byte	
extrn	 AJR_CustomMem:byte	
extrn	 AJR_InitStruc:byte	
extrn	 AJR_End:byte	
extrn	 AtJR_FindEnd:byte	
extrn	 AtJR_To:byte	
extrn	 AtJR_From:byte	
extrn	 AtJR_End:byte	
extrn	 FJR_Next:byte	
extrn	 FJR_End:byte	
extrn	 ASM_Alloc:byte	
extrn	 ASM_End:byte		
extrn	 FSM_Free:byte	
extrn	 FSM_FreeOne:byte
extrn	 FSM_Join:byte	
extrn	 FSM_End:byte		
extrn	 AM_Alloc:byte	
extrn	 FM_Free:byte	
extrn	 FM_Fre1:byte
extrn	 FM_Fre2:byte
extrn	 RS_Entry:byte	
extrn	 SEND_JANUS_INT_MSG:byte
;
.list


