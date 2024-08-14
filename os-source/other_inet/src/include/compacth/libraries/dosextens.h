��LIBRARIES_DOSEXTENS_H�LIBRARIES_DOSEXTENS_H�EXEC_TYPES_H�"exec/types.h"��EXEC_TASKS_H�"exec/tasks.h"����"exec/ports.h"��EXEC_LIBRARIES_H�"exec/libraries.h"��LIBRARIES_DOS_H�"libraries/dos.h"�
�Process{
�Task pr_Task;
��pr_MsgPort;
�pr_Pad;
�pr_SegList;
�pr_StackSize;
�pr_GlobVec;
�pr_TaskNum;
�pr_StackBase;
�pr_Result2;
�pr_CurrentDir;
�pr_CIS;
�pr_COS;
�pr_ConsoleTask;
�pr_FileSystemTask;
�pr_CLI;
�pr_ReturnAddr;
�pr_PktWait;
�pr_WindowPtr;
};
�FileHandle{
��*fh_Link;
��*fh_Port;
��*fh_Type;
�fh_Buf;
�fh_Pos;
�fh_End;
�fh_Funcs;�fh_Func1 fh_Funcs
�fh_Func2;
�fh_Func3;
�fh_Args;�fh_Arg1 fh_Args
�fh_Arg2;
};
�DosPacket{
��*dp_Link;
��*dp_Port;
�dp_Type;
�dp_Res1;
�dp_Res2;�dp_Action dp_Type�dp_Status dp_Res1�dp_Status2 dp_Res2�dp_BufAddr dp_Arg1
�dp_Arg1;
�dp_Arg2;
�dp_Arg3;
�dp_Arg4;
�dp_Arg5;
�dp_Arg6;
�dp_Arg7;
};
�StandardPacket{
��sp_Msg;
�DosPacket sp_Pkt;
};�ACTION_NIL 0�ACTION_GET_BLOCK 2�ACTION_SET_MAP 4�ACTION_DIE 5�ACTION_EVENT 6�ACTION_CURRENT_VOLUME 7�ACTION_LOCATE_OBJECT 8�ACTION_RENAME_DISK 9�ACTION_WRITE 'W'�ACTION_READ 'R'�ACTION_FREE_LOCK 15�ACTION_DELETE_OBJECT 16�ACTION_RENAME_OBJECT 17�ACTION_MORE_CACHE 18�ACTION_COPY_DIR 19�ACTION_WAIT_CHAR 20�ACTION_SET_PROTECT 21�ACTION_CREATE_DIR 22�ACTION_EXAMINE_OBJECT 23�ACTION_EXAMINE_NEXT 24�ACTION_DISK_INFO 25�ACTION_INFO 26�ACTION_FLUSH 27�ACTION_SET_COMMENT 28�ACTION_PARENT 29�ACTION_TIMER 30�ACTION_INHIBIT 31�ACTION_DISK_TYPE 32�ACTION_DISK_CHANGE 33�ACTION_SET_DATE 34�ACTION_SCREEN_MODE 994�ACTION_READ_RETURN 1001�ACTION_WRITE_RETURN��ACTION_SEEK 1008�ACTION_FINDUPDATE 1004�ACTION_FINDINPUT 1005�ACTION_FINDOUTPUT 1006�ACTION_END 1007�ACTION_TRUNCATE 1022�ACTION_WRITE_PROTECT 1023
�DosLibrary{
�Library dl_lib;
�dl_Root;
�dl_GV;
�dl_A2;
�dl_A5;
�dl_A6;
};
�RootNode{
�rn_TaskArray;
�rn_ConsoleSegment;
�DateStamp rn_Time;
�rn_RestartSeg;
�rn_Info;
�rn_FileHandlerSegment;
};
�DosInfo{
�di_McName;
�di_DevInfo;
�di_Devices;
�di_Handlers;
�di_NetHand;
};
�CommandLineInterface{
�cli_Result2;
BSTR cli_SetName;
�cli_CommandDir;
�cli_ReturnCode;
BSTR cli_CommandName;
�cli_FailLevel;
BSTR cli_Prompt;
�cli_StandardInput;
�cli_CurrentInput;
BSTR cli_CommandFile;
�cli_Interactive;
�cli_Background;
�cli_CurrentOutput;
�cli_DefaultStack;
�cli_StandardOutput;
�cli_Module;
};
�DeviceList{
�dl_Next;
�dl_Type;
��*dl_Task;
�dl_Lock;
�DateStamp dl_VolumeDate;
�dl_LockList;
�dl_DiskType;
�dl_unused;
BSTR*dl_Name;
};
�DevInfo{
�dvi_Next;
�dvi_Type;
�dvi_Task;
�dvi_Lock;
BSTR dvi_Handler;
�dvi_StackSize;
�dvi_Priority;
�dvi_Startup;
�dvi_SegList;
�dvi_GlobVec;
BSTR dvi_Name;
};
�DosList{
�dol_Next;
�dol_Type;
��*dol_Task;
�dol_Lock;
�{
�{
BSTR dol_Handler;
�dol_StackSize;
�dol_Priority;
�dol_Startup;
�dol_SegList;
�dol_GlobVec;
}dol_handler;
�{
�DateStamp dol_VolumeDate;
�dol_LockList;
�dol_DiskType;
}dol_volume;
}dol_misc;
BSTR dol_Name;
};�DLT_DEVICE 0�DLT_DIRECTORY 1�DLT_VOLUME 2
�FileLock{
�fl_Link;
�fl_Key;
�fl_Access;
��*fl_Task;
�fl_Volume;
};�