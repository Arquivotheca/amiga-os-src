��DEVICES_TIMER_H�DEVICES_TIMER_H���"exec/io.h"��UNIT_MICROHZ 0�UNIT_VBLANK 1�TIMERNAME "timer.device"
�timeval{
�tv_secs;
�tv_micro;
};
�timerequest{
�IORequest tr_node;
�timeval tr_time;
};�TR_ADDREQUEST CMD_NONSTD�TR_GETSYSTIME (CMD_NONSTD+1)�TR_SETSYSTIME (CMD_NONSTD+2)�