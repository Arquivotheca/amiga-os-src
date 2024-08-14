

#include    <exec/types.h>
#include    <libraries/dos.h>




main()
{
ULONG	File=0;
ULONG	Temp=0;



	Temp= *(ULONG *)0x00f7ffe4;
	if(Temp)
	    {
	    Temp -= 0xf00000+4;
	    printf("Writing %lx bytes to bonus file\n",Temp);
	    File=Open("bonus14",MODE_NEWFILE);
	    Write(File,0x00f00000,Temp);
	    Close(File);
	    }
	else
	    printf("Nothing in f-memory\n");
}
