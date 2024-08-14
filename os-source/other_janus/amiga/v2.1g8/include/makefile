all: 
   echo " "

release:
   copy \#?         Other:janus/Amiga/cur/include
   spat "chmod 755" Other:janus/Amiga/cur/include/\#?
   delete           Other:janus/Amiga/cur/include/\#?.(o|obj|bak)

include:
   copy \#?.h       V:include/janus
   copy \#?.i       V:include/janus
   spat "chmod 755" V:include/janus/\#?

clean:
   delete \#?.(o|obj|bak)

local:
   lock include: off
   copy \#?.h            include:janus
   copy \#?.i            include:janus
   lock include: on

disk:
   echo "nothing to do"
