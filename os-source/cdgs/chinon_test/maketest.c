
#include <stdio.h>
#include <stdlib.h>

#include <dos/dos.h>
#include <clib/dos_protos.h>

UBYTE buffer[1024];

void
main (int argc, char **argv)
{
	BPTR fh,out;
	long len,num,i;

	if (argc <= 1)
	{
		printf("error: must specify file\n");
		exit(10);
	}
	fh = Open(argv[1],MODE_OLDFILE);
	if (fh)
	{
		out = Open("test_code.asm",MODE_NEWFILE);
		if (!out)
		{
			printf("can't open test_code.asm!\n");
			Close(fh);
			exit(10);
		}

		num = 0;
		len = Read(fh,buffer,sizeof(buffer));
		while (len > 0)
		{
			for (i = 0; i < len; i++)
			{
				if (num == 0)
					FPuts(out,"\n dc.b ");
				else
					FPuts(out,",");

				FPrintf(out,"$%02lx",buffer[i]);
				num++;
				if (num >= 8)
					num = 0;
			}
			len = Read(fh,buffer,sizeof(buffer));
		}
		if (len < 0)
		{
			printf("error %ld reading %s\n",IoErr(),argv[1]);
			Close(out);
			Close(fh);
			exit(20);
		}
		FPuts(out,"\n");
		Close(out);
		Close(fh);
	} else {
		printf("Can't open %s!\n",argv[1]);
		exit(10);
	}

	printf("done.  Please remake load.ld\n");
	exit(0);
}
