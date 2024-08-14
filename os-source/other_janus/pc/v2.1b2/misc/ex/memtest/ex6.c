#define LINT_ARGS                   /* Enable Janus type checking          */

#include <janus/janus.h>

main(argc,argv)
int argc;
char *argv[];
{
	APTR	ptr1,ptr2,ptr3;
	UBYTE error;

   /************************************************************************/
   /* Make sure INT JFUNC_JINT is redirected (Wake up services) before use */
   /************************************************************************/
   { UWORD i;
      error=GetBase(JSERV_PCDISK,&i,&i,&i);
      printf("Error from  GetBase = "); PError(error);
   }

	ptr1 = AllocJanusMem((ULONG)100,(ULONG)MEMF_BUFFER);
	printf("ptr1 = %lx\n",ptr1);
	ptr2 = AllocJanusMem((ULONG)100,(ULONG)MEMF_BUFFER);
	printf("ptr2 = %lx\n",ptr2);
	ptr3 = AllocJanusMem((ULONG)100,(ULONG)MEMF_BUFFER);
	printf("ptr3 = %lx\n",ptr3);

	printf("Return to free ptr2"); { char buf[255]; gets(buf); }

	error=FreeJanusMem(ptr2);
	printf("Error from Free2 = %lx\n",(ULONG)error);

	printf("Return to free ptr1"); { char buf[255]; gets(buf); }

	error=FreeJanusMem(ptr1);

	printf("Error from Free1 = %lx\n",(ULONG)error);

	printf("Return to free ptr3"); { char buf[255]; gets(buf); }

	error=FreeJanusMem(ptr3);
	printf("Error from Free3 = %lx\n",(ULONG)error);
}
