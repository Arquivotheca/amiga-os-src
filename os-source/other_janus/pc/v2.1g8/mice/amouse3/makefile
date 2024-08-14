#   Microsoft Mouse Driver
#
#   Requirements:
#
#	MASM 4.01 or greater with the environment variable INCLUDE set to
#	the directories containing CMACROS.INC, and WINDEFS.INC.
#
#	MASM 4.00 or greater with the ASM inference definition changed to
#	include the directories containing CMACROS.INC, and WINDEFS.INC.


#   Options:
#
#	The command line may define options to MASM by defining the OPT
#	macro.	By defining the OPT parameter in the make file, any
#	possible interaction with an environment definition is avoided.

    OPT = -l				    #NOP the options feature


#   Define the default assemble command.  This command could actually
#   be overridden from the command line, but shouldn't be.

# MASM 4.01 & >
    ASM = masm -v -ML $(OPT)


# MASM 4.00
#   ASM = masm -v -ML  $(OPT) -I\finc


#   Define the default inference rules

    .asm.obj:
	$(ASM) $*,$@;

amouse3.obj:	amouse3.asm amouse3.inc

amouse3.drv:	amouse3.def amouse3.obj
      link4 @amouse3.lnk
      rc amouse3.drv
      mapsym amouse3
