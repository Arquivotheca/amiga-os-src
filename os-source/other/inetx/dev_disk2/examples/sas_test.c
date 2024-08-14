/* -----------------------------------------------------------------------
 * sas_test.c   July 21, 1992
 *
 * demos how to use the new s_dev_list() function with the SAS compiler's
 * UFB struct.
 *
 *------------------------------------------------------------------------
 */

#include <stdio.h>
#include <fcntl.h>
#include <ios1.h>
#include <ss/socket.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <proto/exec.h>

extern int _nufbs;
extern struct UFB _ufbs[];

extern struct ExecBase *SysBase;
struct Library *SockBase;

main()
{
	int i, s;
	int sock[10];
	int fd1, fd2, fd3;

	SockBase = OpenLibrary("inet:libs/socket.library",4);
	if (SockBase)
	{
		printf("Library Opened Sucessfully.\n");
		setup_sockets( _nufbs, &errno );
		s_dev_list((u_long)_ufbs,sizeof(struct UFB));
		printf("_ufbs=%lx\n",_ufbs);
		printf("size=%d\n",sizeof(struct UFB));

		for(i = 0; i < _nufbs; i++)
		   if(_ufbs[i].ufbflg == 0)
			printf("%d empty\n",i);
		    else
			printf("%d used\n",i);


		for(i=0;i<5;i++) {
			sock[i] = socket( AF_INET, SOCK_STREAM, 0 ) ;
			printf("opened socket = %ld\n", sock[i] ) ;
		}

		for(i=0;i<5;i++)
                	s_close(sock[i]);

		for(i=0;i<5;i++) {
			sock[i] = socket( AF_INET, SOCK_STREAM, 0 ) ;
			printf("opened socket = %ld\n", sock[i] ) ;
		}
		for(i=0;i<5;i++)
                	s_close(sock[i]);

		fd1 = open("nil:",2);
		printf("opened fd %ld\n",fd1);
		fd2 = open("nil:",2);
		printf("opened fd %ld\n",fd2);
		fd3 = open("nil:",2);
		printf("opened fd %ld\n",fd3);

		printf("closing %ld\n",fd2);
		close(fd2);

		for(i = 0; i < _nufbs; i++)
		   if(_ufbs[i].ufbflg == 0)
			printf("%d empty\n",i);
		    else
			printf("%d used\n",i);

		s = socket( AF_INET, SOCK_STREAM, 0 ) ;
		printf("opened socket = %ld\n", s ) ;


		cleanup_sockets();

		CloseLibrary(SockBase);
		printf("Library Closed.\n");

	} else
		printf("Library Failed to Open.\n");
}
