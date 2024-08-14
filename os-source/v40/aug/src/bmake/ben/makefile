#	Makefile for ben.lib
#	(c) Copyright 1991 by Ben Eng, All Rights Reserved
#

CC = cc
AS = cc
CFLAGS = -2.0 -ms -r -proto

OBJ = scdir.o basename.o system.o

ben.lib : $(OBJ)
	join $(OBJ) as $@

install:
	copy ben.lib dlib:

