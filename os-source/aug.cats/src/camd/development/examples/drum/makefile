#=============================================================================
# Makefile for Drum Kit
#=============================================================================
#CFLAGS	=	gst=sc:gsts/std_headers.gst debug=fullflush ansi \
#		unsignedchars nostackcheck ignore=62 ignore=51 ignore=147

#CFLAGS	=	debug=fullflush ansi \
#		unsignedchars nostackcheck multiplecharconstants

CFLAGS	=	unsignedchars nostackcheck multiplecharconstants


.c.o:
	SC $*.c $(CFLAGS) nolink

.a.o:
	asm -iinclude: $*

OBJS	= drum.o asymbevel.o timer.o arrows.o
OUT	= drum

all: $(OUT)

$(OUT): $(OBJS)
	SLINK lib:c.o $(OBJS) library lib:sc.lib lib:amiga.lib to $(OUT)
