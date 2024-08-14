
#include <exec/types.h>

 struct Library *NIPCBase;
 struct Library *ServicesBase;

main()
{

    NIPCBase = (struct Library *) OpenLibrary("nipc.library",37L);
    if (NIPCBase)
    {
        ServicesBase = (struct Library *) OpenLibrary("services.library",37L);
        if (ServicesBase)
        {
            CloseLibrary(ServicesBase);
        }
        CloseLibrary(NIPCBase);
    }
}


