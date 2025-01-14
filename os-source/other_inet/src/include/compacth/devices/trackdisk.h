��DEVICES_TRACKDISK_H�DEVICES_TRACKDISK_H���"exec/io.h"��EXEC_DEVICES_H�"exec/devices.h"��NUMSECS 11�NUMUNITS 4�TD_SECTOR 512�TD_SECSHIFT 9�TD_NAME "trackdisk.device"�TDF_EXTCOM (1L<<15)�TD_MOTOR (CMD_NONSTD+0)�TD_SEEK (CMD_NONSTD+1)�TD_FORMAT (CMD_NONSTD+2)�TD_REMOVE (CMD_NONSTD+3)�TD_CHANGENUM (CMD_NONSTD+4)�TD_CHANGESTATE (CMD_NONSTD+5)�TD_PROTSTATUS (CMD_NONSTD+6)�TD_RAWREAD (CMD_NONSTD+7)�TD_RAWWRITE (CMD_NONSTD+8)�TD_GETDRIVETYPE (CMD_NONSTD+9)�TD_GETNUMTRACKS (CMD_NONSTD+10)�TD_ADDCHANGEINT (CMD_NONSTD+11)�TD_REMCHANGEINT (CMD_NONSTD+12)�TD_LASTCOMM (CMD_NONSTD+13)�ETD_WRITE (CMD_WRITE|TDF_EXTCOM)�ETD_READ (CMD_READ|TDF_EXTCOM)�ETD_MOTOR (TD_MOTOR|TDF_EXTCOM)�ETD_SEEK (TD_SEEK|TDF_EXTCOM)�ETD_FORMAT (TD_FORMAT|TDF_EXTCOM)�ETD_UPDATE (CMD_UPDATE|TDF_EXTCOM)�ETD_CLEAR (CMD_CLEAR|TDF_EXTCOM)�ETD_RAWREAD (TD_RAWREAD|TDF_EXTCOM)�ETD_RAWWRITE (TD_RAWWRITE|TDF_EXTCOM)
�IOExtTD{
�IOStdReq iotd_Req;
�iotd_Count;
�iotd_SecLabel;
};�IOTDB_INDEXSYNC 4�IOTDF_INDEXSYNC (1<<4)�TD_LABELSIZE 16�TDB_ALLOW_NON_3_5 0�TDF_ALLOW_NON_3_5 (1<<0)�DRIVE3_5 1�DRIVE5_25 2�TDERR_NotSpecified 20�TDERR_NoSecHdr 21�TDERR_BadSecPreamble 22�TDERR_BadSecID 23�TDERR_BadHdrSum 24�TDERR_BadSecSum 25�TDERR_TooFewSecs 26�TDERR_BadSecHdr 27�TDERR_WriteProt 28�TDERR_DiskChanged 29�TDERR_SeekError 30�TDERR_NoMem 31�TDERR_BadUnitNum 32�TDERR_BadDriveType 33�TDERR_DriveInUse 34�TDERR_PostReset 35
�TDU_PublicUnit{
�Unit tdu_Unit;
�tdu_Comp01Track;
�tdu_Comp10Track;
�tdu_Comp11Track;
�tdu_StepDelay;
�tdu_SettleDelay;
�tdu_RetryCnt;
};�