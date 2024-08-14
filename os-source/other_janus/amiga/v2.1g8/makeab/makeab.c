#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <stdio.h>

#undef DEBUG

#ifdef DEBUG
#define D(x) x;
#else
#define D(x) ;
#endif

struct AB_BLOCK {
	char name[8];
	short Heads;
	short SecTrk;
	short Cyl;
};

void main();

VOID BuildAbsolutePath(UBYTE *name, UBYTE *buf, LONG len)
{
struct FileLock *lock, *lock2;
struct FileInfoBlock *fib;
UBYTE *tmpbuf;
UBYTE *pname = name;
UBYTE *pbuf = buf;

	D( printf("BAP: pname='%s'\n", pname); )
	*pbuf = '\0';
	if ((fib = malloc(sizeof(struct FileInfoBlock))) != NULL) {
		if ((tmpbuf = malloc(len)) != NULL) {
			if ((lock2 = (struct FileLock *)Lock(pname, ACCESS_READ)) != NULL) {
				do {
					lock = lock2; /* offspring becomes parent */
					lock2 = (struct FileLock *)ParentDir((BPTR)lock); /* get lock on parent */
					if (Examine((BPTR)lock, fib)) {
						strcpy(tmpbuf, fib->fib_FileName);
						D( printf("DirEntryType=%ld\n", fib->fib_DirEntryType); )
						if (fib->fib_DirEntryType > 0) {
							if (lock2 != NULL) {
								strcat(tmpbuf, "/");
							} else {
								strcat(tmpbuf, ":");
							}
						}
						strcat(tmpbuf, pbuf);
						strcpy(pbuf, tmpbuf);
					} else {
						D( printf("Couldn't examine lock\n"); )
						UnLock((BPTR)lock); /* unlock offspring */
						UnLock((BPTR)lock2); /* unlock parent */
						break;
					}
					UnLock((BPTR)lock); /* unlock offspring */
				} while (lock2 != NULL);
			}
#ifdef DEBUG
			else {
				printf("BAP: Couldn't get lock on '%s'\n", pname);
			}
#endif
			free(tmpbuf);
		}
#ifdef DEBUG
		else {
			printf("BAP: Couldn't get mem for tmpbuf\n");
		}
#endif
		free(fib);
	}
#ifdef DEBUG
	else {
		printf("BAP: Couldn't get mem for fib\n");
	}
#endif
	if (*pbuf != '\0' && pbuf[strlen(pbuf) - 1] == '/') {
		pbuf[strlen(pbuf) - 1] = '\0'; /* overwrite last '/' */
	}

	D( printf("BAP: pbuf='%s'\n\n", pbuf); )
}

LONG avail_blocks(char *volume)
{
BPTR lock;
struct InfoData *info;
LONG retval;
char *p, c;

	if (p = strchr(volume, ':')) {
		c = *(p+1);
		*(p+1) = 0;
	}

	retval = -1;

	if  (info = malloc(sizeof(*info))) {
		if (lock = Lock(volume, SHARED_LOCK)) {
			if (Info(lock, info)) {
				retval = info->id_NumBlocks - info->id_NumBlocksUsed;
			}

			UnLock(lock);
		}

		free(info);
	}

	if (p)
		*(p+1) = c;

	return retval;
}

void main(argc, argv)
int argc;
char *argv[];
{
struct FileHandle *file;

int Heads, SecTrk, Cyl;

struct AB_BLOCK *bp;
char *buf;
unsigned long bsize;

char abuf[512];
char path[512];
int temp;
int x;

LONG blocks;

char *av[4];
char ibuf[80];
char *token;

#define BRKCHR	" \t"


	if (argc == 0) {
		exit(0);
	}

	for (x = 0; x < argc; x++)
		av[x] = argv[x];

again:
	if (argc > 2) {
		printf("Too many arguments.\n");
		exit(0);
	}

	if (argc == 1) {
		printf("Required argument missing.\n");
		return;
	}

	if (argc == 2 && !strcmp(av[1], "?")) {
		printf("FILENAME/A: ");
		fgets(ibuf, sizeof(ibuf) - 1, stdin);
		x = strlen(ibuf);
		if (x) {
			if (ibuf[x-1] == '\n')
				ibuf[x-1] = 0;
		}
		argc = 1;
		token = strtok(ibuf, BRKCHR);
		while (token && token[0]) {
			av[argc++] = token;
			if (argc == 4)
				break;
			token = strtok(0, BRKCHR);
		}

		goto again;
	}

	do {
		printf("Enter number of Heads (1-16): ");
		scanf("%d\n", &Heads);
	} while (Heads > 16 || Heads < 1);

	do {
		printf("Enter number of Sectors/Track (1-64): ");
		scanf("%d\n", &SecTrk);
	} while (SecTrk > 64 || SecTrk < 1);

	do {
		printf("Enter number of Cylinders (1-1024): ");
		scanf("%d\n", &Cyl);
	} while (Cyl > 1024 || Cyl < 1);

	printf("\nParameters selected:\n");
	printf("Heads             = %d\n", Heads);
	printf("Sectors per Track = %d\n", SecTrk);
	printf("Cylinders         = %d\n", Cyl);
	printf("Virtual drive will be %d bytes\n", (Heads*SecTrk*Cyl*512));
	printf("Total file size will be %d bytes\n", (Heads*SecTrk*Cyl*512)+512);

	printf("\nEnter Y to accept, N to quit: ");
	scanf("%s\n", abuf);
	if (abuf[0] != 'Y'&&abuf[0] != 'y')
		exit(10);

	if (strchr(av[1], ':'))
		blocks = avail_blocks(av[1]);
	else
		blocks = avail_blocks("");

	if ((blocks != -1) && (blocks <= (Heads*SecTrk*Cyl))) {
		printf("Not enough space on disk for file.\n");
		exit(100);
	}

	printf("Creating %s\n", av[1]);
	if ((file = (struct FileHandle *)Open(av[1], MODE_NEWFILE)) == 0) {
		printf("Error creating file.\n");
		exit(100);
	}

	Close(file);

	BuildAbsolutePath(av[1], path, sizeof(path) - 1);

	D( printf("path = %s\n", path); )

	if ((file=(struct FileHandle *)Open(path, MODE_NEWFILE))==0) {
		printf("Error creating file.\n");
		exit(100);
	}

	bsize = 512 * 128;
	buf = 0;
	do {
		buf = malloc(bsize);
		if (!buf)
			bsize -= 512;
	} while (buf == 0 && bsize != 0);

	memset(buf, 0xf6, bsize);

	bp=(struct AB_BLOCK *)buf;
	memcpy(bp->name, "ABOOT\0\0\0", 8);
	bp->Heads=Heads;
	bp->SecTrk=SecTrk;
	bp->Cyl=Cyl;

	if (Write(file, buf, 512) != 512) {
		printf("Error writing, file deleted.\n");
		Close(file);
		DeleteFile(path);
		exit(100);
	}

	memset(buf, 0xf6, bsize);

	temp = (Heads * SecTrk * Cyl) * 512;

	while (temp) {
		if (temp < bsize)
			x = temp;
		else
			x = bsize;

		if (Write(file, buf, x) != x) {
			printf("Error writing, file deleted.\n");
			Close(file);
			DeleteFile(path);
			exit(100);
		}

		temp -= x;

		if (SetSignal(0, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C) {
			printf("MakeAB aborted\n");
			Close(file);
			DeleteFile(path);
			exit(100);
		}
	}

	Close(file);

	if (file = Open("sys:pc/system/aboot.ctrl", MODE_NEWFILE)) {
		x = strlen(path);
		if (Write(file, path, x) != x) {
			printf("Couldn't write SYS:PC/System/Aboot.ctrl\n");
			Close(file);
			DeleteFile(path);
			exit(100);
		}
		if (Write(file, "\n", 1) != 1) {
			printf("Couldn't write SYS:PC/System/Aboot.ctrl\n");
			Close(file);
			DeleteFile(path);
			exit(100);
		}
		Close(file);
		exit(100);
	} else {
		printf("Couldn't create SYS:PC/System/Aboot.ctrl\n");
		DeleteFile(path);
		exit(100);
	}

	exit(0);
}
