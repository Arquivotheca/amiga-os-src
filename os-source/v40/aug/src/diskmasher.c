/*

    Disk tester - Bryce Nesbitt


    Writes random sized patterned blocks to the disk, then compares.

 */
#include <exec/types.h>
#include <libraries/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <math.h>


#define MAXFILELEN  (100*1024)
#define BUFFERFUDGE 4	    /* we use a random byte offset into buffer */


long  passes;
long  temp;
char  filename[2]; /* One character file name */

ULONG file1=0;
long  length1;
void *offset1;
APTR  memory1;
APTR  memblk1;
struct DateStamp ds;




int CXBRK()
{
    printf("\n*** BREAK\n");

    if(file1)    Close(file1);
    file1=0;
    if(memblk1)  FreeMem(memblk1,length1+BUFFERFUDGE);
    memblk1=0;
    printf("%ld test passes completed\n",passes);

    return(-1); /* -1 means exit, 0 means stay */
}




main()
{
    printf("Testing disk...\n");

    filename[0]=0;
    filename[1]=0;

    DateStamp(&ds);
    srand48(ds.ds_Tick);

    while(1)
	{
	length1=lrand48()%MAXFILELEN;

	temp=lrand48()%(255*1024)+0x00f80000;    /* random offset into ROM */
	offset1=(void *)(temp+lrand48()%4);

	memblk1=AllocMem(length1+BUFFERFUDGE,0);    /* buffer for readback */
	if(!memblk1)
	    {
	    printf("Memory allocation failed\n");
	    break;
	    }
	memory1=(void *)((long)memblk1+lrand48()%4); /* offset into block */




	printf("Write %ld bytes from %lx buffer %lx\n",length1,offset1,memory1);
	filename[0]='0'+(passes%10);
	file1=Open(filename,MODE_NEWFILE);
	if(!file1)
	    {
	    printf("Error processing file %s\n",&filename);
	    printf("DOS error code %ld\n",IoErr());
	    break;
	    }
	temp=Write(file1,offset1,length1);
	if(temp != length1)
	    {
	    printf("Error processing file %s\n",&filename);
	    printf("%ld bytes of %ld written\n",temp,length1);
	    printf("DOS error code %ld\n",IoErr());
	    break;
	    }
	Close(file1);
	file1=0;




	file1=Open(filename,MODE_OLDFILE);
	temp=Read(file1,memory1,length1);
	if(temp != length1)
	    {
	    printf("Error processing file %s\n",&filename);
	    printf("%ld bytes of %ld read\n",temp,length1);
	    printf("DOS error code %ld\n",IoErr());
	    break;
	    }
	temp=memcmp(memory1,offset1,length1);
	if(temp)
	    {
	    printf("Error processing file %s\n",filename);
	    printf("Readback error.  Data does not match!\n");
	    break;
	    }
	Close(file1);
	file1=0;




	FreeMem(memblk1,length1+BUFFERFUDGE);
	memblk1=0;
	passes++;




	chkabort();
	}




    if(file1)    Close(file1);
    file1=0;
    if(memblk1)  FreeMem(memblk1,length1+BUFFERFUDGE);
    memblk1=0;
    printf("%ld test passes completed\n",passes);
}
