/*
 *	Do the actual making for make
 */

#include <stdio.h>
#ifdef unix
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#endif
#ifdef eon
#include <sys/stat.h>
#include <sys/err.h>
#endif
#ifdef os9
#include <time.h>
#include <os9.h>
#include <modes.h>
#include <direct.h>
#include <errno.h>
#endif
#ifdef amiga
#include <ctype.h>
#include <errno.h>
#include <libraries/dosextens.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <exec/ports.h>
#include <proto/all.h>
#ifdef LATTICE
#include <string.h>
#endif
#endif
#include "h.h"



/*
 *	Exec a shell that returns exit status correctly (/bin/esh).
 *	The standard EON shell returns the process number of the last
 *	async command, used by the debugger (ugg).
 *	[exec on eon is like a fork+exec on unix]
 */
int dosh(string, shell)
    char           *string;
    char           *shell;
{
    int             number;

#ifdef amiga
    char           *av[3], *s, *p;
/*    long	outfile; */

    p = s = malloc((unsigned) strlen(string) + 1);
    strcpy(p, string);			/* make a copy of the string */
    av[0] = gettok(&p);			/* get first argument */

#if LATTICE
/*
    outfile= ((struct CommandLineInterface *)BADDR( ((struct Process *)
	FindTask(0))->pr_CLI ))->cli_CurrentOutput;
   outfile=Output();

    if ( Execute(string,0,outfile) != -1)
	fatal("couldn't execute command '%s', error return %02x\n",
	      av[0], errno);
    number = IoErr();
*/
    number=system(string);
    free(s);
    if(number == -1)
	fatal("couldn't execute command '%s',error return %02x\n",av[0],errno);
    if(number > 200) return(10);
    return number;
#else
    av[1] = p;				/* get rest of command line */
    av[2] = NULL;

    if (fexecv(av[0], av) == -1)
	fatal("couldn't execute command '%s', error return %02x\n",
	      av[0], errno);
    number = wait();
    free(s);
    return number;
#endif
#endif

#ifdef unix
    return system(string);
#endif
#ifdef eon
    return ((number = execl(shell, shell, "-c", string, 0)) == -1) ?
	-1 :			/* couldn't start the shell */
	wait(number);		/* return its exit status */
#endif
#ifdef os9
    int             status, pid;

    strcat(string, "\n");
    if ((number = os9fork(shell, strlen(string), string, 0, 0, 0)) == -1)
	return -1;		/* Couldn't start a shell */
    do {
	if ((pid = wait(&status)) == -1)
	    return -1;		/* child already died!?!? */
    } while (pid != number);

    return status;
#endif
}


/*
 *	Do commands to make a target
 */
void
docmds1(np, lp)
    struct name    *np;
    struct line    *lp;
{
    bool            ssilent;
    bool            signore;
    int             estat;
    register char  *q;
    register char  *p;
    char           *shell;
    register struct cmd *cp;
#ifdef amiga
    long SetSignal();
#endif


    if (*(shell = getmacro("SHELL")) == '\0')
#ifdef eon
	shell = ":bin/esh";
#endif
#ifdef unix
    shell = "/bin/sh";
#endif
#ifdef os9
    shell = "shell";
#endif
#ifdef amiga
    shell = NULL;
#endif

    for (cp = lp->l_cmd; cp; cp = cp->c_next) {
	strcpy(str1, cp->c_cmd);
	expand(str1);
	q = str1;
	ssilent = silent;
	signore = ignore;
	while ((*q == '@') || (*q == '-')) {
	    if (*q == '@')	/* Specific silent  */
		ssilent = TRUE;
	    else		/* Specific ignore  */
		signore = TRUE;
	    q++;		/* Not part of the command  */
	}

	if (!domake)
	    ssilent = 0;

	if (!ssilent)
	    fputs("    ", stdout); /* always was stdout */

	for (p = q; *p; p++) {
	    if (*p == '\n' && p[1] != '\0') {
		*p = ' ';
		if (!ssilent)
		    fputs("\\\n", stdout); /* always was stdout */
	    } else if (!ssilent)
		putchar(*p);
	}
	if (!ssilent)
	    putchar('\n');

	if (domake) {		/* Get the shell to execute it  */
	    if ((estat = dosh(q, shell)) != 0) {
		if (estat == -1)
		    fatal("Couldn't execute %s", shell);
		else {
		    printf("%s: Error code %d", myname, estat);
		    if (signore)
			fputs(" (Ignored)\n", stdout); /* always was stdout */
		    else {
			putchar('\n');
			if (!(np->n_flag & N_PREC))
			    if (unlink(np->n_name) == 0)
				printf("%s: '%s' removed.\n", myname, np->n_name);
			exit(estat);
		    }
		}
	    }
#ifdef amiga
	    if ((SetSignal(0L, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C) != 0) {
		fputs("make: stopped\n", stdout); /* always was */
		exit(1);
	    }
#endif	    
	}
    }
}


void docmds(np)
    struct name    *np;
{
    register struct line *lp;


    for (lp = np->n_line; lp; lp = lp->l_next)
	docmds1(np, lp);
}


#ifdef os9
/*
 *	Some stuffing around to get the modified time of a file
 *	in an os9 file system
 */
getmdate(fd, tbp)
    struct sgtbuf  *tbp;
{
    struct registers regs;
    static struct fildes fdbuf;


    regs.rg_a = fd;
    regs.rg_b = SS_FD;
    regs.rg_x = &fdbuf;
    regs.rg_y = sizeof(fdbuf);

    if (_os9(I_GETSTT, &regs) == -1) {
	errno = regs.rg_b & 0xff;
	return -1;
    }
    if (tbp) {
	_strass(tbp, fdbuf.fd_date, sizeof(fdbuf.fd_date));
	tbp->t_second = 0;	/* Files are only acurate to mins */
    }
    return 0;
}


/*
 *	Kludge routine to return an aproximation of how many
 *	seconds since 1980.  Dates will be in order, but will not
 *	be lineer
 */
time_t
cnvtime(tbp)
    struct sgtbuf  *tbp;
{
    long            acc;


    acc = tbp->t_year - 80;	/* Baseyear is 1980 */
    acc = acc * 12 + tbp->t_month;
    acc = acc * 31 + tbp->t_day;
    acc = acc * 24 + tbp->t_hour;
    acc = acc * 60 + tbp->t_minute;
    acc = acc * 60 + tbp->t_second;

    return acc;
}


/*
 *	Get the current time in the internal format
 */
time(tp)
    time_t         *tp;
{
    struct sgtbuf   tbuf;


    if (getime(&tbuf) < 0)
	return -1;

    if (tp)
	*tp = cnvtime(&tbuf);

    return 0;
}
#endif


/*
 *	Get the modification time of a file.  If the first
 *	doesn't exist, it's modtime is set to 0.
 */
void
modtime(np)
    struct name    *np;
{
#ifdef unix
    struct stat     info;
    int             fd;


    if (stat(np->n_name, &info) < 0) {
	if (errno != ENOENT)
	    fatal("Can't open %s; error %d", np->n_name, errno);

	np->n_time = 0L;
    } else
	np->n_time = info.st_mtime;
#endif
#ifdef eon
    struct stat     info;
    int             fd;


    if ((fd = open(np->n_name, 0)) < 0) {
	if (errno != ER_NOTF)
	    fatal("Can't open %s; error %02x", np->n_name, errno);

	np->n_time = 0L;
    } else if (getstat(fd, &info) < 0)
	fatal("Can't getstat %s; error %02x", np->n_name, errno);
    else
	np->n_time = info.st_mod;

    close(fd);
#endif
#ifdef os9
    struct sgtbuf   info;
    int             fd;


    if ((fd = open(np->n_name, 0)) < 0) {
	if (errno != E_PNNF)
	    fatal("Can't open %s; error %02x", np->n_name, errno);

	np->n_time = 0L;
    } else if (getmdate(fd, &info) < 0)
	fatal("Can't getstat %s; error %02x", np->n_name, errno);
    else
	np->n_time = cnvtime(&info);

    close(fd);
#endif
#ifdef amiga
    struct FileInfoBlock *fib;
    BPTR myLock;
    long ioErr;

    fib = (struct FileInfoBlock *) malloc((unsigned) sizeof(struct FileInfoBlock));
    if ((myLock = Lock(np->n_name, ACCESS_READ)) == NULL) {
	if ((ioErr = IoErr()) != (long)ERROR_OBJECT_NOT_FOUND)
	    fatal("Can't Lock '%s'; error %3ld", np->n_name, ioErr);
	np->n_time = 0L;
    } else if (!Examine(myLock, fib)) {
	UnLock(myLock);
	fatal("Can't Examine '%s'; error %3ld", np->n_name, IoErr());
    } else {
 	np->n_time = fib->fib_Date.ds_Tick/TICKS_PER_SECOND +
		 60*fib->fib_Date.ds_Minute + 86400*fib->fib_Date.ds_Days;
    	UnLock(myLock);
    }
    free((char *) fib);
#endif

}


/*
 *	Update the mod time of a file to now.
 */
void touch(np)
    struct name    *np;
{
#ifndef amiga
    char            c;
    int             fd;
#endif

    if (!domake || !silent)
	printf("    touch(%s)\n", np->n_name);

    if (domake) {
#ifdef unix
	long            a[2];

	a[0] = a[1] = time(0);
	if (utime(np->n_name, &a[0]) < 0)
	    printf("%s: '%s' not touched - non-existant\n",
		   myname, np->n_name);
#endif
#ifdef eon
	if ((fd = open(np->n_name, 0)) < 0)
	    printf("%s: '%s' not touched - non-existant\n",
		   myname, np->n_name);
	else {
	    uread(fd, &c, 1, 0);
	    uwrite(fd, &c, 1);
	}
	close(fd);
#endif
#ifdef os9
	/*
	 * Strange that something almost as totally useless as this is easy
	 * to do in os9! 
	 */
	if ((fd = open(np->n_name, S_IWRITE)) < 0)
	    printf("%s: '%s' not touched - non-existant\n",
		   myname, np->n_name);
	close(fd);
#endif
#ifdef amiga
	struct MsgPort *task;
	struct DateStamp dateStamp;
	BPTR lock, plock;
	UBYTE *pointer;

	if(!(pointer= (UBYTE *)AllocMem(64L,MEMF_PUBLIC)))
		fatal("Can't get 64 bytes for pointer");
	if(!(task=(struct MsgPort *)DeviceProc(np->n_name))) {
		printf("%s: can't get MsgPort for '%s'\n", myname, np->n_name);
		FreeMem((void *) pointer, 64L);
		return;
	}
	if(!(lock = Lock(np->n_name,SHARED_LOCK))) {
		printf("%s: '%s' not touched - non-existant\n",
			myname, np->n_name);
		FreeMem((void *) pointer, 64L);
		return;
	}
	plock = ParentDir(lock);
	UnLock(lock);

	strcpy((pointer + 1),np->n_name);
	*pointer = strlen(np->n_name);
	dos_packet(task, ACTION_SET_DATE, NULL, plock, (ULONG) &pointer[0] >> 2,
		(ULONG) DateStamp(&dateStamp), 0L, 0L, 0L);
	UnLock(plock);
	FreeMem((void *) pointer, 64L);
#endif
    }
}

/*
 * Recursive routine to make a target. 
 */
int make(np, level)
    struct name    *np;
    int             level;
{
    register struct depend *dp;
    register struct line *lp;
    register struct depend *qdp;
    time_t          dtime = 1, time();
    bool            didsomething = 0;


    if (np->n_flag & N_DONE) return 0;

    if (!np->n_time)
	modtime(np);		/* Gets modtime of this file  */

    if (rules) {
	for (lp = np->n_line; lp; lp = lp->l_next)
	    if (lp->l_cmd)
		break;
	if (!lp) {
	    dyndep(np);
	}
    }
    if (!(np->n_flag & N_TARG) && np->n_time == 0L)
	fatal("Don't know how to make %s", np->n_name);

    for (qdp = (struct depend *) 0, lp = np->n_line; lp; lp = lp->l_next) {
	for (dp = lp->l_dep; dp; dp = dp->d_next) {
	    make(dp->d_name, level + 1);
	    if (np->n_time < dp->d_name->n_time)
		qdp = newdep(dp->d_name, qdp);
	    dtime = max(dtime, dp->d_name->n_time);
	}
	if (!quest && (np->n_flag & N_DOUBLE) && (np->n_time < dtime)) {
	    make1(np, lp, qdp);	/* free()'s qdp */
	    dtime = 1;
	    qdp = (struct depend *) 0;
	    didsomething++;
	}
    }

    np->n_flag |= N_DONE;

    if (quest) {
	long            t;

	t = np->n_time;
	time(&np->n_time);
	return t < dtime;
    } 
    else if (np->n_time < dtime && !(np->n_flag & N_DOUBLE)) {
	make1(np, (struct line *) 0, qdp);	/* free()'s qdp */
	time(&np->n_time);
    } else if (level == 0 && !didsomething)
	printf("%s: '%s' is up to date\n", myname, np->n_name);
    return 0;
}


make1(np, lp, qdp)
    register struct depend *qdp;
    struct line    *lp;
    struct name    *np;
{
    register struct depend *dp;
    char *suff;

    if (dotouch) touch(np);
    else {
	strcpy(str1, "");
	for (dp = qdp; dp; dp = qdp) {
	    if (strlen(str1))
		strcat(str1, " ");
	    strcat(str1, dp->d_name->n_name);
	    qdp = dp->d_next;
	    free(dp);
	}
	setmacro("?", str1);
	setmacro("@", np->n_name);

        strcpy(str1,getmacro("<")); /* new */
	if(suff=suffix(str1)) *suff=0;
	setmacro("*",str1); /* new */

	if (lp)			/* lp set if doing a :: rule */
	    docmds1(np, lp);
	else docmds(np);
    }
return(0);
}

#ifdef amiga
/*
 * Replace the Aztec-provided time function with one which returns something
 * easy to find and compare, namely the number of seconds since the Amiga's
 * reference date.  This is the same thing returned by modtime() above.
 */
time_t
time(v)
    time_t *v;
{
    struct DateStamp t;

    DateStamp(&t);
    t.ds_Days = t.ds_Tick/TICKS_PER_SECOND + 60*t.ds_Minute + 86400*t.ds_Days;
    if (v) *v = t.ds_Days;
    return t.ds_Days;
}
#endif

#ifdef amiga

LONG dos_packet(id,type,arg1,arg2,arg3,arg4,arg5,arg6,arg7)
struct MsgPort *id;	/* process indentifier ... (handlers message port ) */
LONG type;           /* packet type ... (what you want handler to do )   */
ULONG arg1,arg2,arg3,arg4,arg5,arg6,arg7;
{

 struct MsgPort        *replyport;
 struct StandardPacket *packet;
 
LONG  res1=NULL;

if(!(replyport = (struct MsgPort *) CreatePort(NULL,NULL)))return(NULL);

packet=(struct StandardPacket *)
	AllocMem((LONG)sizeof(struct StandardPacket),MEMF_PUBLIC|MEMF_CLEAR);

if(packet) {
    packet->sp_Msg.mn_Node.ln_Name = (UBYTE *)&(packet->sp_Pkt); /*link packet */
    packet->sp_Pkt.dp_Link = &(packet->sp_Msg);        /* to message    */
    packet->sp_Pkt.dp_Port = replyport;         /* set-up reply port   */
    packet->sp_Pkt.dp_Type = type;           /* what to do... */

    /* move all the arguments to the packet */

    packet->sp_Pkt.dp_Arg1 = arg1;
    packet->sp_Pkt.dp_Arg2 = arg2;
    packet->sp_Pkt.dp_Arg3 = arg3;
    packet->sp_Pkt.dp_Arg4 = arg4;
    packet->sp_Pkt.dp_Arg5 = arg5;
    packet->sp_Pkt.dp_Arg6 = arg6;
    packet->sp_Pkt.dp_Arg7 = arg7;

    PutMsg(id,(struct Message *)packet); /* send packet */
    WaitPort(replyport); /* wait for packet to come back */
    GetMsg(replyport);   /* pull message */

    res1 = packet->sp_Pkt.dp_Res1; /* get result */

}
FreeMem(packet,(LONG)sizeof(*packet)); 
DeletePort(replyport); 
return(res1);   
  
}
#endif
