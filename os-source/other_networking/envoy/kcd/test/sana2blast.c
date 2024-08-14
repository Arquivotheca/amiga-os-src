#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <devices/sana2.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#define SysBase (*(struct Library **)4L);

void __asm myCopyFromBuff(register __a0 APTR to, register __a1 APTR from, register __d0 LONG length);
void __asm myCopyToBuff(register __a0 APTR to, register __a1 APTR from, register __d0 LONG length);

void __saveds sana2blast(void)
{
    struct IOSana2Req *ios2,*ios2cmd;
    struct MsgPort *replyport;
    ULONG signals, mask;
    UBYTE *mybuff;

    ULONG buftags[5] = { S2_CopyFromBuff, 0,
    		   S2_CopyToBuff, 0,
    		   TAG_DONE };

    UWORD queue = 0;

    if(mybuff = AllocMem(2048,MEMF_PUBLIC))
    {
	if(replyport = CreateMsgPort())
	{
           if(ios2cmd = CreateIORequest(replyport,sizeof(struct IOSana2Req)))
           {
           	buftags[1] = (ULONG)&myCopyFromBuff;
           	buftags[3] = (ULONG)&myCopyToBuff;

           	ios2cmd->ios2_BufferManagement = buftags;

           	if(!OpenDevice("a2065.device",0,(struct IORequest *)ios2cmd,0))
           	{
           	    mask = (1L << replyport->mp_SigBit) | SIGBREAKF_CTRL_C;

           	    while(TRUE)
           	    {
           	    	while(queue < 30)
           	    	{
           	    	    if(ios2 = CreateIORequest(replyport,sizeof(struct IOSana2Req)))
           	    	    {
           	    	    	ios2->ios2_Req.io_Device = ios2cmd->ios2_Req.io_Device;
           	    	    	ios2->ios2_Req.io_Unit = ios2cmd->ios2_Req.io_Unit;
	                        ios2->ios2_PacketType = 2048;
           	    	    	ios2->ios2_BufferManagement = ios2cmd->ios2_BufferManagement;
           	    	    	ios2->ios2_Data = mybuff;
           	    	    	ios2->ios2_DataLength = 1500;
           	    	    	ios2->ios2_Req.io_Command = S2_BROADCAST;

           	    	    	SendIO((struct IORequest *)ios2);
           	    	    	queue++;
           	    	    }
           	    	}
           	    	if(ios2 = (struct IOSana2Req *)GetMsg(replyport))
           	    	{
                            ios2->ios2_Req.io_Device = ios2cmd->ios2_Req.io_Device;
                            ios2->ios2_Req.io_Unit = ios2cmd->ios2_Req.io_Unit;
                            ios2->ios2_PacketType = 2048;
                            ios2->ios2_BufferManagement = ios2cmd->ios2_BufferManagement;
                            ios2->ios2_Data = mybuff;
                            ios2->ios2_DataLength = 1500;
                            ios2->ios2_Req.io_Command = S2_BROADCAST;

                            SendIO((struct IORequest *)ios2);
                        }

                        signals = Wait(mask);

                        if(signals & SIGBREAKF_CTRL_C)
                            break;
                    }
                    while(queue > 0)
                    {
                    	WaitPort(replyport);
                    	ios2 = (struct IOSana2Req *)GetMsg(replyport);
                    	DeleteIORequest((struct IORequest *)ios2);
                    	queue--;
                    }
                    CloseDevice((struct IORequest *)ios2cmd);
                }
                DeleteIORequest((struct IORequest *)ios2cmd);
            }
            DeleteMsgPort(replyport);
        }
        FreeMem(mybuff,2048);
    }
}

void __asm myCopyFromBuff(register __a0 APTR to, register __a1 APTR from, register __d0 LONG length)
{
	CopyMemQuick(from,to,length);
}

void __asm myCopyToBuff(register __a0 APTR to, register __a1 APTR from, register __d0 LONG length)
{
	CopyMem(from,to,length);
}

