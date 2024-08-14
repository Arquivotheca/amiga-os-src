#include <exec/types.h>
#include <stdio.h>

extern FILE *console;

void dprintf(char *format,ULONG arg1,ULONG arg2,ULONG arg3,ULONG arg4);
void kprintf(char *format,...);

void dprintf(char *format,ULONG arg1,ULONG arg2,ULONG arg3,ULONG arg4)
{
   if(console)
      fprintf(console,format,arg1,arg2,arg3,arg4);
   else
      kprintf(format,arg1,arg2,arg3,arg4);
}
