/*
 * Format of inetd startup message.  This is sent to server by inetd
 * when the server is forked.  The idea is that the server should
 * perform some short initialization and then reply to the message.
 * inetd will time out after a few seconds of waiting if this message
 * is not returned.
 */

#include <exec/types.h>
#include <exec/ports.h>

struct INETDmsg {
	struct Message in_msg;
	void 	*in_sock;	/* socket that will be stdin/stdout	*/
	short	in_errno;	/* error number				*/
	char	future[32];	/* room for some expansion		*/
};

#define INETD_TIMEOUT	(10*50)
#define INETD_RETRY	(5)
