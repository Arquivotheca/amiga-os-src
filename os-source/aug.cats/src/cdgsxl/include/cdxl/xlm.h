/*******************************

             xlm.h

         W.D.L 930708

 cdgsxl message passing header

********************************/

typedef struct XLMessage {
    struct Message	msg;
    ULONG		command;
    ULONG		status;

} XLMESSAGE;

// Commands
#define	XLM_CMD_QUERY		0
#define	XLM_CMD_ABORT		1

// Status
#define	XLM_STATUS_PLAYING	0x00000001
#define	XLM_STATUS_ABORTING	0x00000002
