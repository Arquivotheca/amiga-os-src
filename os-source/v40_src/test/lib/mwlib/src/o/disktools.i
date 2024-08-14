/* Prototypes for functions defined in c/DiskTools.c */
BOOL _GetPathname(ULONG lock,
                  UBYTE **pbuf,
                  ULONG *psiz);
BOOL GetPathname(ULONG lock,
                 UBYTE *buf,
                 ULONG size);
