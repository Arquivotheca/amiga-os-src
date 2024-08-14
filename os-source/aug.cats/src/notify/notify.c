;/* Notify - 2.04 file notification for scripts
LC -b1 -cfistq -v -y -j73 -L notify
quit
*/

# include	<exec/types.h>
# include	<exec/memory.h>
# include	<exec/ports.h>
# include	<dos/dos.h>
# include	<dos/notify.h>
# include	<utility/tagitem.h>

# include	<clib/alib_stdio_protos.h>
# include	<clib/dos_protos.h>
# include	<clib/exec_protos.h>

#ifdef	LATTICE
int	CXBRK(void)	{ return 0; }
int	chkabort(void)	{ return 0; }
#endif

# define	MYNAME	"Notify"

# define	FILENAME	0
# define	AGAIN		1
# define	CALL		2
# define	NUM_ARGS	3

# define	TEMPLATE	"File/A/M,Again/S,Call/K"

LONG	Args[NUM_ARGS];

extern struct Library	*DOSBase;

main(int arcg, char * argv [] )
{
    struct NotifyRequest	*nReq;	/* pointer to array of notifiy requests */
    struct MsgPort		*mp;	/* message port for notify */
    struct NotifyMessage	*msg;	/* pointer to a retrieved notify message */
    struct RdArgs		*args;	/* command line arguments */

    UBYTE		**files;	/* pointer to filelist from command line */
    int			i;		/* iterator */
    int			nfiles;		/* number of files & length of nReq[] */
    ULONG		sigs;		/* signal(s) that broke the Wait() */
    ULONG		usig;		/* Control-C signal */
    ULONG		mpsig;		/* MessagePort signal */
    char		cmd[256];	/* buffer for user-supplied command */
    LONG		rvalue;		/* the value to return */

    rvalue = RETURN_OK;
    
    args = ReadArgs(TEMPLATE, Args, NULL);
    mp = CreateMsgPort();
		
    if (args && Args[FILENAME] && mp)
    {
	files = (char **)Args[FILENAME];
		
	for (i = 0; files[i]; i++);
		
	nfiles = i;
		
	nReq = AllocMem(sizeof(struct NotifyRequest) * nfiles, MEMF_CLEAR);

	if (nReq)
	{
	    for (i = 0; i < nfiles; i++)
	    {
		nReq[i].nr_Name = files[i];

		nReq[i].nr_Flags |= NRF_SEND_MESSAGE | NRF_WAIT_REPLY;
		nReq[i].nr_stuff.nr_Msg.nr_Port = mp;

	    }
	    for (i = 0; i < nfiles; i++)
		if (StartNotify(nReq + i) != DOSTRUE)
		    break;
		    
	    if (i == nfiles)
	    {
		usig = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D;
		
		mpsig = 1L << mp->mp_SigBit;
		
		for (;;)
		{
		    sigs = Wait(mpsig | usig);

		    if (sigs & mpsig)
		    {
			while ((msg = (struct NotifyMessage *) GetMsg(mp)) != NULL)
			{
			    if (Args[CALL])
			    {
				sprintf(cmd, (char *)Args[CALL], msg->nm_NReq->nr_Name);
				SystemTags(cmd, TAG_DONE);
			    }
			    else
				if (Args[AGAIN])
				{
				    FPuts(Output(), msg->nm_NReq->nr_Name);
				    FPutC(Output(), '\n');
				}
			    
			    ReplyMsg((struct Message *)msg);
			}

			if (Args[AGAIN] == 0)
			    break;
		    }

		    if (sigs & SIGBREAKF_CTRL_C)
		    {
			rvalue = RETURN_WARN;
			break;
		    }

		    if (sigs & SIGBREAKF_CTRL_D)
		    {
			rvalue = RETURN_FAIL;
			break;
		    }
		}
	    }
	    else
	    {
		PrintFault(IoErr(), MYNAME);
		rvalue = RETURN_FAIL;
	    }
	    

	    for (i = 0; i < nfiles; i++)
		EndNotify(nReq);

	    FreeMem(nReq, sizeof(struct NotifyRequest) * nfiles);
	}
	else
	{
	    PrintFault(IoErr(), MYNAME);
	    rvalue = RETURN_ERROR;
	}
    }
    else
    {
	PrintFault(IoErr(), MYNAME);
	if (Args && mp)
	    rvalue = RETURN_WARN;
	else
	    rvalue = RETURN_FAIL;
    }

    if (mp)
	DeleteMsgPort(mp);

    if (Args)
	FreeArgs(args);
    
    exit(rvalue);
}
