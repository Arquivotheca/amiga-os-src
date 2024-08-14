all: 
   echo " "

release:
   copy \#?         Other:janus/Amiga/cur/include
   spat "chmod 755" Other:janus/Amiga/cur/include/\#?
   delete           Other:janus/Amiga/cur/include/\#?.o
   delete           Other:janus/Amiga/cur/include/\#?.obj
   delete           Other:janus/Amiga/cur/include/\#?.bak

include:
   copy \#?.h       V:include/janus
   copy \#?.i       V:include/janus
   spat "chmod 755" V:include/janus/\#?

clean:
   delete \#?.o
   delete \#?.obj
   delete \#?.bak

local:
   lock include: off
   copy \#?.h            include:janus
   copy \#?.i            include:janus
   lock include: on
