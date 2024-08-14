/* Prototypes for functions defined in c/packer.c */
BYTE *PutDump(BYTE *dest,
              int nn);
BYTE *PutRun(BYTE *dest,
             int nn,
             int cc);
LONG PackRow(BYTE **pSource,
             BYTE **pDest,
             LONG rowSize);
