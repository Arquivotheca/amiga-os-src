% Local DEBUGGING (FLAG SET -d5)
% Mitchell/Ware Systems, Inc.
-d5
-r0 -cwsi -v -j30
-ih -iMWHead:
#
# -r0	default all subroutine calls to FAR
# -v    disable stack checking
# -cwsi	no return warnings
#       create only one copy of identical strings
#       supress multiple include
# -O	perform global optimization
# -i...	use special headers
# -j30	turn off warning about mix-matched pointer types
# -m3   Compile for a 68030
# -f8   Compile for a 68881
