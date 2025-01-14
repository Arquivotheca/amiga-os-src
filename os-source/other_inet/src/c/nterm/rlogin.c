/*
 * rlogin protocol for nterm
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <config.h>
#include <stdio.h>
#include <errno.h>
#include <libraries/dos.h>

#include <graphics/text.h>
#include <intuition/intuition.h>

#include "serial.h"

long sbit;
static int sock;

/*
 * rlogin messages sent via out-of-band data.
 */
#define TIOCPKT_FLUSHREAD 	0x01	/* flush pending kybd data	*/
#define TIOCPKT_FLUSHWRITE 	0x02	/* flush all data to mark 	*/
#define TIOCPKT_STOP		0x04
#define TIOCPKT_START		0x08
#define TIOCPKT_NOSTOP		0x10
#define TIOCPKT_DOSTOP		0x20
#define TIOCPKT_WINDOW		0x80	/* request window size		*/

void
rlogin_loop()
{
	extern long cbit, ibit;
	fd_set ibits, ebits;
	extern char c;
	long event;

	FD_ZERO(&ibits);
	FD_ZERO(&ebits);
	FD_SET(sock, &ibits);
	FD_SET(sock, &ebits);
	event = SIGBREAKF_CTRL_C | ibit | cbit;

	selectwait(sock+1, &ibits, 0L, &ebits, 0L, &event);

	if(event & ibit){
		HandleIntuition();
	}

	if(event & cbit){
		char ch;
		while((ch = CDMayGetChar()) != -1){
			send(sock, &ch, 1, 0);
		}
	}

	if(FD_ISSET(sock, &ebits)){
		handle_oob(sock);
	}
	if(FD_ISSET(sock, &ibits)){
		static char buf[SD_BUFMAX/2];
		char *ptr;
		int cc;

		if((cc = recv(sock, buf, sizeof(buf), 0)) >= 0){
			if(cc == 0){
				extern char done;
			    	done = 1;
			}
/*			TerminalWrite(buf, cc); replaced by next 6 lines */
			ptr = buf;
			while(cc-->0) {
				c = *ptr++;
				vt100parse();
			}
			flush_string();
		}
	}
}

/*
 * Send window size under rlogin protocol
 */
static 
send_window_size(s)
	int	s;
{
	struct {
		char ws_magic[4];
		unsigned short ws_row, ws_col;
		unsigned short ws_width, ws_height;
	} ws;
	extern struct Window *w;

	ws.ws_magic[0] = 0377;
	ws.ws_magic[1] = 0377;
	ws.ws_magic[2] = 's';
	ws.ws_magic[3] = 's';
	ws.ws_col = htons(w->Width/w->IFont->tf_XSize);
	ws.ws_row = htons(w->Height/w->IFont->tf_YSize);
	ws.ws_width = htons(w->Width);
	ws.ws_height = htons(w->Height);
	return (send(s, &ws, sizeof(ws), 0) != sizeof(ws) ? -1 : 0);
}

/*
 * Handle out of band data protocol
 */
static
handle_oob(s)
	int	s;
{
	char c;

	if(recv(s, &c, sizeof(c), MSG_OOB) > 0){
		if(c & TIOCPKT_WINDOW){
			send_window_size(s);
		}
		if(c & TIOCPKT_FLUSHWRITE){
			for (;;) {
				static char waste[512];
				long atmark;
				int n;

				atmark = 0;
				if (ioctl(s, SIOCATMARK, &atmark) < 0) {
					perror("ioctl");
					break;
				}
				if (atmark){
					break;
				}

    				n = read(s, waste, sizeof(waste));
				if(n <= 0){
					break;
				}
			}
		}
	}
}

/*
 * Open & init network
 */
init_rlogin(hostname, remuser)
	char	*hostname, *remuser;
{
	char *localuser;
	struct config *cf;
	int cc;

	GETCONFIG(cf);
	if(!cf)
		localuser="\0";
	else
		localuser=cf->username;

	if(!remuser)
		remuser = localuser;	

	sock = rcmd(&hostname, 513, localuser, remuser, "vt100", 0);
	if(sock < 0){
		return -1;
	}

	cc = -1;
	if(ioctl(sock, FIONBIO, &cc) < 0){
		perror("ioctl");
	}
	sbit = 1L << AllocSignal(-1L);
	if(sbit == 0){
		return -1;
	}

	return 0;
}

