
#include <stdio.h>
#include <exec/types.h>

extern ULONG __asm Div64by32(register __d0 ULONG msb, register __d1 ULONG lsb, register __d2 ULONG divisor);

main()
{

 ULONG answer;

 answer = Div64by32(0L,120,20);
 printf("answer is %ld\n",answer);

}
