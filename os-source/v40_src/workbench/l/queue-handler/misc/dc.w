%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Local switches for Que Compililation     %%
%%                 DC.W                     %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
-cwsi -v -O
#-j30
-i -ih
-cr -ms
#-rb
# It was -rb that was causing the flakiness. Will defer investigation
# for later.

#
# -r0	default all subroutine calls to FAR
# -rb   compile with both registerized and non-registerized calls
# -b0   Absolute data references
# -v    disable stack checking
# -cwsi	no return warnings
#       create only one copy of identical strings
#       supress multiple include
# -O	perform global optimization
# -i...	use special headers
# -j30	turn off warning about mix-matched pointer types
# -m3   Compile for a 68030
# -f8   Compile for a 68881
