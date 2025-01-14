/* readrdsk.c */

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/trackdisk.h>

#include <dos/dos.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

/* for lattice */
#include <proto/exec.h>
#include <proto/dos.h>

void
main (argc,argv)
	int argc;
	char **argv;
{
	int ret = 20;
	struct IOStdReq *ior = NULL;
	struct MsgPort *port = NULL;
	int opened = FALSE,i;
	char *mem;
	BPTR fh = NULL;
	char *name = "rdsk";
	char *device = "scsi.device";
	int unit = 0, write = FALSE,start = 0,length=17*4*512;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i],"device") == 0)
			device = argv[++i];
		else if (strcmp(argv[i],"unit") == 0)
			unit = atoi(argv[++i]);
		else if (strcmp(argv[i],"write") == 0)
			write = TRUE;
		else if (strcmp(argv[i],"read") == 0)
			write = FALSE;
		else if (strcmp(argv[i],"start") == 0)
			start = atoi(argv[++i]);
		else if (strcmp(argv[i],"length") == 0)
			length = atoi(argv[++i]);
		else if (strcmp(argv[i],"?") == 0)
		{
			printf(
"usage: readrdsk [device x] [unit y] [Read|write] [start z] [length q] file\n");
			exit(0);
		} else
			name = argv[i];
	}

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	if (i = OpenDevice(device,unit,
			   (struct IORequest *) ior,0L))
	{
		printf("Error %d on device open!\n",i);
		goto cleanup;
	}
	opened = TRUE;

	if (write)
	{
		fh = Open(name,MODE_OLDFILE);
		if (!fh)
		{
			printf("Can't open %s!\n",name);
			goto cleanup;
		}

		if (length == 17*4*512)	/* bad but quick test */
		{
			Seek(fh,0,OFFSET_END);
			length = Seek(fh,0,OFFSET_BEGINNING);
		}
	} else {
		if (!(fh = Open(name,MODE_NEWFILE)))
		{
			printf("Can't open %s!\n",name);
			goto cleanup;
		}
	}

	mem = AllocMem(length,MEMF_CLEAR);
	if (!mem)
	{
		printf("can't allocate mem!\n");
		goto cleanup;
	}

	if (write)
	{
		Read(fh,mem,length); /* no error checking */
	}

	ior->io_Data 	= (APTR) mem;
	ior->io_Length	= length;
	ior->io_Offset	= start;
	ior->io_Command	= write ? CMD_WRITE : CMD_READ;

	DoIO((struct IORequest *) ior);
	if (ior->io_Error)
	{
		printf("Error %ld on read/write!\n",ior->io_Error);
		goto cleanup;
	}

	if (!write)
	{
		if (Write(fh,mem,length) != length)
			goto cleanup;
	}

	ret = 0;	/* success */

cleanup:
	if (fh)
		Close(fh);
	if (opened)
		CloseDevice((struct IORequest *) ior);
	if (ior)
		DeleteStdIO(ior);
	if (port)
		DeletePort(port);
	if (mem)
		FreeMem(mem,length);

	exit(ret);
}
