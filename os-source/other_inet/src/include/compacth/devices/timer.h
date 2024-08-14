€ˆDEVICES_TIMER_H€DEVICES_TIMER_HˆºŒ"exec/io.h"‡€UNIT_MICROHZ 0€UNIT_VBLANK 1€TIMERNAME "timer.device"
ƒtimeval{
—tv_secs;
—tv_micro;
};
ƒtimerequest{
ƒIORequest tr_node;
ƒtimeval tr_time;
};€TR_ADDREQUEST CMD_NONSTD€TR_GETSYSTIME (CMD_NONSTD+1)€TR_SETSYSTIME (CMD_NONSTD+2)‡