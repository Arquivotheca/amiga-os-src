/*
** Net handler interface operations.  Generic as possible.
*/

#define ACTION_NET	50
#define	ACTION_BIND	(ACTION_NET+1)	/* bind address to socket	*/
#define ACTION_LISTEN	(ACTION_NET+2)	/* put socket into listen state	*/
#define ACTION_CONNECT	(ACTION_NET+3)	/* Connect socket to remote skt	*/
#define ACTION_SENDTO	(ACTION_NET+4)	/* send message to address	*/
#define ACTION_ACCEPT	(ACTION_NET+5)	/* Accept a connection on skt	*/
#define ACTION_SHUTDOWN	(ACTION_NET+6)	/* Shutdown, i.e. refuse data	*/
#define ACTION_RECVFROM	(ACTION_NET+7)	/* recv message from address	*/
#define ACTION_SETOPT	(ACTION_NET+8)	/* set options on socket	*/
#define ACTION_GETOPT	(ACTION_NET+9)	/* get options on socket	*/
#define ACTION_SOCKADDR (ACTION_NET+11) /* get socket name		*/
#define ACTION_PEERADDR (ACTION_NET+12) /* get remote address	 	*/
#define ACTION_SENDMSG  (ACTION_NET+13) /* send data in iov array 	*/
#define ACTION_RECVMSG  (ACTION_NET+14) /* recv to buffers in iov array	*/
#define ACTION_SEND     (ACTION_NET+15) /* send to connected socket 	*/
#define ACTION_RECV     (ACTION_NET+16) /* recv from connected socket 	*/
#define ACTION_SELECT	(ACTION_NET+17) /* wait for i/o ready command	*/
#define ACTION_COMMAND	(ACTION_NET+18) /* ascii command as arg1	*/
#define ACTION_READV	(ACTION_NET+19) /* readv			*/
#define ACTION_WRITEV	(ACTION_NET+20) /* writev			*/
#define ACTION_RECVM	(ACTION_NET+21) /* recv - return mbufs		*/
#define ACTION_SENDM	(ACTION_NET+22) /* send - take mbufs		*/

#define SEL_RDFD	1
#define SEL_WRFD	2
#define SEL_EXFD	4

