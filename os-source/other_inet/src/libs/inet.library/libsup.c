/* MANX 5.0 library support */

#include <exec/lists.h>
#include <exec/resident.h>
#include <exec/libraries.h>
#include <netinet/inet.h>
#include <functions.h>


int _socket(char *AP);
int _inherit(char *AP);
int _networkclose(char *AP);
int _bind(char *AP);
int _listen(char *AP);
int _accept(char *AP);
int _connect(char *AP);
int _sendto(char *AP);
int _send(char *AP);
int _sendmsg(char *AP);
int _recvfrom(char *AP);
int _recv(char *AP);
int _recvmsg(char *AP);
int _shutdown(char *AP);
int _setsockopt(char *AP);
int _getsockopt(char *AP);
int _getsockname(char *AP);
int _getpeername(char *AP);
int _select(char *AP);
int _ioctl(char *AP);

void myInit(void);
long myOpen(void);
long myClose(void);
long myExpunge(void);
void doinit(struct InetBase *lib);  /* our init function */

#pragma amicall(InetBase, 0x06, myOpen())
#pragma amicall(InetBase, 0x0c, myClose())
#pragma amicall(InetBase, 0x12, myExpunge())

/* library initialization table, used for AUTOINIT libraries			*/
struct InitTable {
	unsigned long	it_DataSize;		  /* library data space size		*/
	void			**it_FuncTable;		  /* table of entry points			*/
	void 			*it_DataInit;		  /* table of data initializers		*/
	void			(*it_InitFunc)(void); /* initialization function to run	*/
};

void *libfunctab[] = {	/* my function table */
	myOpen,					/* standard open	*/
	myClose,				/* standard close	*/
	myExpunge,				/* standard expunge	*/
	0,

	/* ------	user UTILITIES */
        _socket,
        _bind,
        _ioctl,
        _listen,
        _accept,
        _connect,
        _sendto,
        _send,
        _sendmsg,
        _recvfrom,
        _recv,
        _recvmsg,
        _shutdown,
        _setsockopt,
        _getsockopt,
        _getsockname,
        _getpeername,
        _select,
        _networkclose,
        _inherit,
	(void *)-1				/* end of function vector table */
};

struct InitTable myInitTab =  {
	sizeof (struct InetBase),
	libfunctab,
	0,						/* will initialize my data in funkymain()	*/
	myInit
};

#define MYREVISION	4		/* would be nice to auto-increment this		*/

char myname[] = "inet.library";
char myid[] = "INET 0.1 (13 Jul 1990)\r\n";

extern struct Resident	myRomTag;

long
_main(struct InetBase *InetBase, unsigned long seglist)
{
	InetBase->seglist = seglist;

    kprintf("Now in __main()\n");

	/* ----- init. library structure  -----		*/
	InetBase->lib.lib_Node.ln_Type = NT_LIBRARY;
	InetBase->lib.lib_Node.ln_Name = (char *) myname;	
	InetBase->lib.lib_Flags = LIBF_SUMUSED | LIBF_CHANGED;
	InetBase->lib.lib_Version = myRomTag.rt_Version;
	InetBase->lib.lib_Revision = MYREVISION;
	InetBase->lib.lib_IdString = (APTR) myid;
}

static unsigned char inited = 0;

long
myOpen(void)
{
	struct InetBase *InetBase;

    kprintf("Now in open\n");

	/* mark us as having another customer					*/
	InetBase->lib.lib_OpenCnt++;

	/* prevent delayed expunges (standard procedure)		*/
	InetBase->lib.lib_Flags &= ~LIBF_DELEXP;


/*	don't init for now 
    if (!inited) {
        doinit(InetBase);
        inited++;
    }
*/
	return ((long) InetBase);
}

long
myClose(void)
{
	struct InetBase *InetBase;
	long retval = 0;

    kprintf("Now in Close\n");

	if (--InetBase->lib.lib_OpenCnt == 0) {

		if  (InetBase->lib.lib_Flags & LIBF_DELEXP) {
			/* no more people have me open,
			 * and I have a delayed expunge pending
			 */
			retval = myExpunge(); /* return segment list	*/
		}
	}

	return (retval);
}

long
myExpunge(void)
{
	struct InetBase *InetBase;
	unsigned long seglist = 0;
	long libsize;
	extern struct Library *DOSBase;

    kprintf("Now in expunge\n");

	if (InetBase->lib.lib_OpenCnt == 0) {
		/* really expunge: remove libbase and freemem	*/

		seglist	= InetBase->seglist;

		Remove(&InetBase->lib.lib_Node);
								/* i'm no longer an installed library	*/

		libsize = InetBase->lib.lib_NegSize+InetBase->lib.lib_PosSize;
		FreeMem((char *)InetBase-InetBase->lib.lib_NegSize, libsize);
		CloseLibrary(DOSBase);		/* keep the counts even */
	}
	else
		InetBase->lib.lib_Flags |= LIBF_DELEXP;

	/* return NULL or real seglist				*/
	return ((long) seglist);
}

