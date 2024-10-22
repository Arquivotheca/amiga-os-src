/* Copyright (C) 1986,1987 by Manx Software Systems, Inc. */

/*
 * ioctl - control operations on sockets
 */

#include <errno.h>
#include <libraries/dosextens.h>

#include "fd_to_fh.h"
#include <socket.h>

#ifndef LATTICE
#include <sgtty.h>
#endif

int
ioctl(s, cmd, data)
	int	s, cmd;
	struct sgttyb *data;
{
	struct ioctlargs ioa;

#ifndef LATTICE
	switch(cmd){
	case TIOCGETP:
	case TIOCSETP:
		return manxioctl(s, cmd, data);
	}
#endif
	GETSOCK(s, ioa.fp);
	ioa.cmd = cmd;
	ioa.data = data;
	ioa.errno = 0L;
	IoCtlAsm(&ioa);
	errno = ioa.errno;

	return errno? -1:0;
}

#ifndef LATTICE

int
manxioctl(fd, cmd, arg)
int fd, cmd;
struct sgttyb *arg;
{
	register struct _dev *refp;
	register struct FileHandle *fhp;
	register struct Process *mp, *FindTask();
	refp = _devtab + fd;
	if (fd < 0 || fd >= _numdev || refp->fd == 0) {
		errno = EBADF;
		return(-1);
	}
	switch (cmd) {
	case TIOCGETP:
		arg->sg_flags = (refp->mode&O_CONRAW)?RAW:0;
		break;
	case TIOCSETP:
		mp = FindTask(0L);
		fhp = (struct FileHandle *)(refp->fd << 2);
		if (fhp->fh_Type == (struct MsgPort *)mp->pr_ConsoleTask) {
			for (refp=_devtab;refp<_devtab+_numdev;refp++) {
				fhp = (struct FileHandle *)(refp->fd << 2);
				if (fhp==NULL || fhp->fh_Type != (struct MsgPort *)mp->pr_ConsoleTask) 
					continue;
				if (arg->sg_flags & RAW)
					refp->mode |= O_CONRAW;
				else
					refp->mode &= ~O_CONRAW;
			}
			dos_packet(mp->pr_ConsoleTask, 994L, arg->sg_flags&RAW?-1L:0L);
		}
		break;
	}
	return 0;
}
#endif
