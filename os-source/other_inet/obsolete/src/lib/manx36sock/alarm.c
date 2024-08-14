/*
** Emulation of Unix alarm() function. The idea is to have the process
** Wait(1L<<_alarm_timerbit) somewhere in its main loop.
*/

#include <exec/types.h>
#include <devices/timer.h>
#include <bstr.h>

int _alarm_timerbit;

alarm(val)
	int	val;
{
	extern struct MsgPort *CreatePort();
	static struct timerequest tr;
	static int on;

	if(val && !on){
		bzero(&tr, sizeof(tr));
		tr.tr_node.io_Message.mn_Node.ln_Type = NT_MESSAGE;
		tr.tr_node.io_Message.mn_Length = sizeof(tr);
		tr.tr_node.io_Message.mn_ReplyPort = CreatePort(0L, 0L);
		_alarm_timerbit = tr.tr_node.io_Message.mn_ReplyPort->mp_SigBit;
		if (OpenDevice(TIMERNAME, UNIT_VBLANK, &tr, 0L)) {
			return -1;
		}
		on++;
	}

	if(val==0 & on){
		if(!CheckIO(&tr)){
			if(!AbortIO(&tr)){
				WaitIO(&tr);
			}
			SetSignal(0, 1L<<_alarm_timerbit);
		}
	
		CloseDevice(&tr);
		DeletePort(tr.tr_node.io_Message.mn_ReplyPort);
		on = 0;
	}

	if(val && on){
		if(!CheckIO(&tr)){
			if(!AbortIO(&tr)){
				WaitIO(&tr);
			}
			SetSignal(0, 1L<<_alarm_timerbit);
		}

		tr.tr_node.io_Command = TR_ADDREQUEST;
		tr.tr_time.tv_secs = val;	
		tr.tr_time.tv_micro = 0;
		SendIO(&tr);
	}
}

