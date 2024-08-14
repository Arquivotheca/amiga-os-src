
#include <exec/types.h>

void main()
{
struct Library *lb;

 lb = (struct Library *) OpenLibrary("dos.library",0L);

 if (lb) printf("whoa!");
 if (!lb) printf("wow!");


 CloseLibrary(lb);


}
