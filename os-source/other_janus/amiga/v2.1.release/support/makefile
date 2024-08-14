OBJ  = ErrorReq.o
       
CC   = lc 
CCF  = -v -caft -b0 -HV:include/all.ph -iV:include -iV:inc.lattice

support.lib: $(OBJ)
   oml support.lib R $(OBJ)

ErrorReq.o: ErrorReq.c ErrorReq.h
   $(CC) $(CCF) ErrorReq.c

release:
   copy \#?         Other:janus/Amiga/cur/support
   spat "chmod 755" Other:janus/Amiga/cur/support/\#?
   delete           Other:janus/Amiga/cur/support/\#?.(o|obj|bak)

clean:
   delete \#?.(o|obj|bak|lnk|info|ld|doc)

listing:
   lp2d (makefile|\#?.asm|\#?.h|\#?.c)
