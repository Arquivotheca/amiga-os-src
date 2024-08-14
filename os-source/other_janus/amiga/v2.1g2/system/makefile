all: 
   echo " "

release:
   copy \#?         Other:janus/Amiga/cur/system
   spat "chmod 755" Other:janus/Amiga/cur/system/\#?
   delete           Other:janus/Amiga/cur/system/\#?.(o|obj|bak)

clean:
   delete \#?.(o|obj|bak)

local:
   lock sys: off
   copy SidecarKeys.table   sys:pc/system/SidecarKeys.Table
   copy Scancode.table      sys:pc/system/Scancode.Table
   lock sys: on

disk:
   copy SidecarKeys.table   PCInstall:pc/system/SidecarKeys.Table
   copy SidecarKeys.table   PCInstall:+pc/system/SidecarKeys.Table
   copy Scancode.table      PCInstall:pc/system/Scancode.Table
   copy Scancode.table      PCInstall:+pc/system/Scancode.Table
