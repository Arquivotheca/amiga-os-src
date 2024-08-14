.SUFFIXES: .c .asm .o
PROGRAM = postscript
OBJ = library.o lib_public.o newscan.o newsupport.o newbin_enc.o char_array.o debug.o \
	stream.o memory.o names.o dict.o math.o scan_add.o pathpaint.o\
	op_array.o op_attr.o op_bool.o op_control.o op_debug.o op_device.o \
	op_dict.o op_file.o op_font.o op_gstate.o op_math.o op_matrix.o \
	op_mec.o op_misc.o op_opstack.o op_paint.o op_path.o op_string.o \
	op_struct.o op_userob.o op_vm.o systemnames.o \
	lib_end.o

CFLAGS = -dMAKELIBRARY -v -cfus -m2 -f8 -iinclude: -O

MISC = errors.h pstypes.h stream.h memory.h objects.h misc.h stack.h gstate.h context.h
AMISC = errors.i objects.i stack.i context.i macros.i
.asm.o:
	asm -dMAKELIBRARY -o$@ -m2 -iinclude: $*.asm
.c.o:
	lc $(CFLAGS) $*.c 

$(PROGRAM):  $(OBJ) 
	blink WITH ps.wth

library.o:	ps_rev.i
lib_public.o:		$(MISC)

stream.o:	 	stream.h pstypes.h
char_array.o:	pstypes.h stream.h memory.h objects.h scanner.h
memory.o:		pstypes.h memory.h
main.o:			$(MISC) scanner.h
dict.o:			$(MISC) dict.h
pathpaint.o:	$(MISC)
names.o:		$(MISC)
debug.o:		$(MISC) scanner.h

newscan.o:		$(MISC) scanner.h
newsupport.o:	$(MISC) scanner.h
newbin_enc.o:	$(MISC) scanner.h

op_array.o:		$(MISC) dict.h
op_attr.o:		$(MISC) 
op_bool.o:		$(MISC)
op_control.o:	$(MISC) dict.h
op_debug.o:		$(MISC)
op_device.o:	$(MISC)
op_dict.o:		$(MISC) dict.h
op_file.o:		$(MISC) scanner.h
op_font.o:		$(MISC) fonts.h
op_gstate.o:	$(MISC)
op_math.o:		$(MISC)
op_matrix.o:	$(MISC)
op_misc.o:		$(MISC)
op_opstack.o:	$(MISC) dict.h
op_paint.o:		$(MISC)
op_path.o:		$(MISC)
op_string.o:	$(MISC)
op_vm.o:		$(MISC)

math.o:			$(AMISC)
systemnames.o:

ps:				$(MISC) ps.o
	lc -L -v -cfs -m2  ps.c

#op_userob.o:	$(MISC)
#op_struct.o:	$(MISC)
#op_mec.o:		$(MISC)

clean:
	delete $(OBJ)

version:
	bumprev 36 ps_rev
