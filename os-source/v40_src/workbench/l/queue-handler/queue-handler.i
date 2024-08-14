/* Prototypes for functions defined in d/queue-handler.c */
__saveds void queue_handler(void);
void ReturnPacket(struct DosPacket *pkt,
                  struct MsgPort *myport);
BOOL ParseName(char *name,
               char *dev,
               long *pbsize,
               long *plimit);
QueBuf *AddNewQueBuf(Queue *q);
QueBuf *DeleteOldQueBuf(Queue *q);
void FlushQueBufs(Queue *q);
Queue *AddQueue(UBYTE *devname,
                long bufsize,
                long limit);
Queue *FindQueue(UBYTE *devname);
BOOL DeleteQueue(Queue *q);
void WaitForOutputChannel(Queue *q);
void WaitForInputChannel(Queue *q);
