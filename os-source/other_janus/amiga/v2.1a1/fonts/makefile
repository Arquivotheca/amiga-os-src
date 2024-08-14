all: 
   echo " "

release:
   copy \#?         Other:janus/Amiga/cur/fonts all
   spat "chmod 755" Other:janus/Amiga/cur/fonts/\#?
   delete           Other:janus/Amiga/cur/fonts/\#?.o
   delete           Other:janus/Amiga/cur/fonts/\#?.obj
   delete           Other:janus/Amiga/cur/fonts/\#?.bak

clean:
   delete \#?.o
   delete \#?.obj
   delete \#?.bak

local:
   lock sys: off
   copy pcfont.font   fonts:
   copy pcfont		  fonts:pcfont
   lock sys: on

disk:
   copy pcfont.font   PCInstall:fonts
   copy pcfont		  PCInstall:fonts/pcfont
