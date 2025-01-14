
#include <exec/types.h>
#include <exec/devices.h>
#include <exec/io.h>

main()
{

    struct IOStdReq *io;
    struct MsgPort *rp;

    rp = (struct MsgPort *) CreateMsgPort();
    printf("msgport %lx\n",rp);
    io = (struct IOStdReq *) CreateIORequest(rp,sizeof(struct IOStdReq));
    printf("ioreq %lx\n",io);
    if (!OpenDevice("printer.device",0,io,0))
        printf("Sheesh!\n");
    if (!io->io_Error)
    {
        int x;
        io->io_Command = CMD_WRITE;
        io->io_Data = "This is the data I wish to test\n";
        io->io_Length = strlen(io->io_Data)+1;
        printf("Writing ...\n");
        DoIO(io);
        printf("Closing ... \n");
        Delay(2*50);
        CloseDevice(io);
    }
    else
        printf("Open error %lx\n",io->io_Error);

    DeleteIORequest(io);
    DeleteMsgPort(rp);
}


