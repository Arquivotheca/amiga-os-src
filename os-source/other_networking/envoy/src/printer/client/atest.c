


#include        <stdio.h>
#include        <dos/dos.h>


main()
{

 APTR f;

 f= Open("PRT:", MODE_NEWFILE);
 if (f)
 {
    printf("'tis open!\n");
    Write(f,"Testing\n",8);
    Close(f);
 }
 else
    printf("Error is %lx\n",IoErr());

}
