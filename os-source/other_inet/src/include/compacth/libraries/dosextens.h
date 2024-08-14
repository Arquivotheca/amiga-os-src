ÄàLIBRARIES_DOSEXTENS_HÄLIBRARIES_DOSEXTENS_HàEXEC_TYPES_Hå"exec/types.h"áàEXEC_TASKS_Hå"exec/tasks.h"áà∏å"exec/ports.h"áàEXEC_LIBRARIES_Hå"exec/libraries.h"áàLIBRARIES_DOS_Hå"libraries/dos.h"á
ÉProcess{
ÉTask pr_Task;
É©pr_MsgPort;
òpr_Pad;
°pr_SegList;
ípr_StackSize;
îpr_GlobVec;
ípr_TaskNum;
°pr_StackBase;
ípr_Result2;
°pr_CurrentDir;
°pr_CIS;
°pr_COS;
îpr_ConsoleTask;
îpr_FileSystemTask;
°pr_CLI;
îpr_ReturnAddr;
îpr_PktWait;
îpr_WindowPtr;
};
ÉFileHandle{
ÉØ*fh_Link;
É©*fh_Port;
É©*fh_Type;
ífh_Buf;
ífh_Pos;
ífh_End;
ífh_Funcs;Äfh_Func1 fh_Funcs
ífh_Func2;
ífh_Func3;
ífh_Args;Äfh_Arg1 fh_Args
ífh_Arg2;
};
ÉDosPacket{
ÉØ*dp_Link;
É©*dp_Port;
ídp_Type;
ídp_Res1;
ídp_Res2;Ädp_Action dp_TypeÄdp_Status dp_Res1Ädp_Status2 dp_Res2Ädp_BufAddr dp_Arg1
ídp_Arg1;
ídp_Arg2;
ídp_Arg3;
ídp_Arg4;
ídp_Arg5;
ídp_Arg6;
ídp_Arg7;
};
ÉStandardPacket{
ÉØsp_Msg;
ÉDosPacket sp_Pkt;
};ÄACTION_NIL 0ÄACTION_GET_BLOCK 2ÄACTION_SET_MAP 4ÄACTION_DIE 5ÄACTION_EVENT 6ÄACTION_CURRENT_VOLUME 7ÄACTION_LOCATE_OBJECT 8ÄACTION_RENAME_DISK 9ÄACTION_WRITE 'W'ÄACTION_READ 'R'ÄACTION_FREE_LOCK 15ÄACTION_DELETE_OBJECT 16ÄACTION_RENAME_OBJECT 17ÄACTION_MORE_CACHE 18ÄACTION_COPY_DIR 19ÄACTION_WAIT_CHAR 20ÄACTION_SET_PROTECT 21ÄACTION_CREATE_DIR 22ÄACTION_EXAMINE_OBJECT 23ÄACTION_EXAMINE_NEXT 24ÄACTION_DISK_INFO 25ÄACTION_INFO 26ÄACTION_FLUSH 27ÄACTION_SET_COMMENT 28ÄACTION_PARENT 29ÄACTION_TIMER 30ÄACTION_INHIBIT 31ÄACTION_DISK_TYPE 32ÄACTION_DISK_CHANGE 33ÄACTION_SET_DATE 34ÄACTION_SCREEN_MODE 994ÄACTION_READ_RETURN 1001ÄACTION_WRITE_RETURN∂ÄACTION_SEEK 1008ÄACTION_FINDUPDATE 1004ÄACTION_FINDINPUT 1005ÄACTION_FINDOUTPUT 1006ÄACTION_END 1007ÄACTION_TRUNCATE 1022ÄACTION_WRITE_PROTECT 1023
ÉDosLibrary{
ÉLibrary dl_lib;
îdl_Root;
îdl_GV;
ídl_A2;
ídl_A5;
ídl_A6;
};
ÉRootNode{
°rn_TaskArray;
°rn_ConsoleSegment;
ÉDateStamp rn_Time;
írn_RestartSeg;
°rn_Info;
°rn_FileHandlerSegment;
};
ÉDosInfo{
°di_McName;
°di_DevInfo;
°di_Devices;
°di_Handlers;
îdi_NetHand;
};
ÉCommandLineInterface{
ícli_Result2;
BSTR cli_SetName;
°cli_CommandDir;
ícli_ReturnCode;
BSTR cli_CommandName;
ícli_FailLevel;
BSTR cli_Prompt;
°cli_StandardInput;
°cli_CurrentInput;
BSTR cli_CommandFile;
ícli_Interactive;
ícli_Background;
°cli_CurrentOutput;
ícli_DefaultStack;
°cli_StandardOutput;
°cli_Module;
};
ÉDeviceList{
°dl_Next;
ídl_Type;
É©*dl_Task;
°dl_Lock;
ÉDateStamp dl_VolumeDate;
°dl_LockList;
ídl_DiskType;
ídl_unused;
BSTR*dl_Name;
};
ÉDevInfo{
°dvi_Next;
ídvi_Type;
îdvi_Task;
°dvi_Lock;
BSTR dvi_Handler;
ídvi_StackSize;
ídvi_Priority;
ídvi_Startup;
°dvi_SegList;
°dvi_GlobVec;
BSTR dvi_Name;
};
ÉDosList{
°dol_Next;
ídol_Type;
É©*dol_Task;
°dol_Lock;
´{
É{
BSTR dol_Handler;
ídol_StackSize;
ídol_Priority;
ódol_Startup;
°dol_SegList;
°dol_GlobVec;
}dol_handler;
É{
ÉDateStamp dol_VolumeDate;
°dol_LockList;
ídol_DiskType;
}dol_volume;
}dol_misc;
BSTR dol_Name;
};ÄDLT_DEVICE 0ÄDLT_DIRECTORY 1ÄDLT_VOLUME 2
ÉFileLock{
°fl_Link;
ífl_Key;
ífl_Access;
É©*fl_Task;
°fl_Volume;
};á