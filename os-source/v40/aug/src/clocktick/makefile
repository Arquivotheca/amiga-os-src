#
# ASM Make File
#

# CAPE  68k
.asm.o:
	@CAsm -a $*.asm -i Include: -cqsy -o $*.o

ClockTick: ClockTick.ld
	@BLink $*.ld TO $* nd

ClockTick.ld: ClockTick.o
	@BLink $*.o lib lib:small.lib to $*.ld sc sd

version:
	@BumpRev 1 ClockTick_rev

clean:
	@Delete ClockTick.o
