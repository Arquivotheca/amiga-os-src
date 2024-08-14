all: 
   echo " "

release:
   copy \#?         Other:janus/Amiga/cur/fonts all
   spat "chmod 755" Other:janus/Amiga/cur/fonts/\#?
   delete           Other:janus/Amiga/cur/fonts/\#?.(o|obj|bak)

clean:
   delete \#?.(o|obj|bak)

local:
   lock sys: off
   copy pcfont.font   fonts:
   copy pcfont		  fonts:pcfont
   lock sys: on

disk:
   copy pcfont.font   PCInstall:fonts
   copy pcfont		  PCInstall:fonts/pcfont
