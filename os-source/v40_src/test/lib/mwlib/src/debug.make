#   DEBUG libraries: Update system from std directory
#
#   Mitchell/Ware Systems       Version 1.10        6/14/88
#
#   NOTE: Copyright (c) 1989 Mitchell/Ware Systems.
#         May be used by Commodore for in-house use only during the
#         duration of employment of Fred Mitchell.

R = ram:DebugDevel.lib

DebugDevel.lib:  deb1.lib deb2.lib deb3.lib deb4.lib
        join deb1.lib deb2.lib deb3.lib deb4.lib as $@

deb1.lib:  \
RequesterTools.odebug \
GadgetTools.odebug \
GelTools.odebug    \
WindowTools.odebug \
MenuTools.odebug   \
Fuckup.odebug
        oml $@ <r @<(ram:olist)
        $?
        <

deb2.lib: \
Librarian.odebug   \
TextTools.odebug   \
Ifunctions.odebug  \
sfix_dec.odebug    \
AlertTools.odebug  \
AddVectorBuffs.odebug \
ListTools.odebug \
SortTools.odebug
        oml $@ <r @<(ram:olist)
        $?
        <

deb3.lib:     \
BSTRtoSTR.odebug   \
amtopm.odebug      \
Crumple.odebug     \
packer.odebug      \
unpacker.odebug    \
GetA0.odebug       \
CollaspeBitMap.odebug
        oml $@ <r @<(ram:olist)
        $?
        <

deb4.lib: \
ProportionalTools.odebug \
EdTools.odebug    \
niff_write.odebug  \
niff_read.odebug    \
ILBMReadToBm.odebug  \
ILBMReadToRp.odebug   \
ImageTools.odebug   \
RastPortTools.odebug \
TokenTools.odebug
        oml $@ <r @<(ram:olist)
        $?
        <

sfix_dec.odebug:          sfix_dec.c
        dbc -o$*.odebug $*

RequesterTools.odebug:    RequesterTools.c
        dbc -o$*.odebug $*

GelTools.odebug:          GelTools.c
        dbc -o$*.odebug $*

WindowTools.odebug:       WindowTools.c
        dbc -o$*.odebug $*

GadgetTools.odebug:       GadgetTools.c
        dbc -o$*.odebug $*

WindowTools.odebug:       WindowTools.c
        dbc -o$*.odebug $*

MenuTools.odebug:         MenuTools.c
        dbc -o$*.odebug $*

ProportionalTools.odebug: ProportionalTools.c
        dbc -o$*.odebug $*

Librarian.odebug:         Librarian.c
        dbc -o$*.odebug $*

MenuTools.odebug:         MenuTools.c
        dbc -o$*.odebug $*

TokenTools.odebug:        TokenTools.c
        dbc -o$*.odebug $*

Fuckup.odebug:            Fuckup.c
        dbc -o$*.odebug $*

ListTools.odebug:         ListTools.c
        dbc -o$*.odebug $*

parse.odebug:             parse.c
        dbc -o$*.odebug $*

IAE.odebug:               IAE.c
        dbc -o$*.odebug $*

Librarian.odebug:         Librarian.c
        dbc -o$*.odebug $*

Ifunctions.odebug:        Ifunctions.c
        dbc -o$*.odebug $*

Xmodem.odebug:            Xmodem.c
        dbc -o$*.odebug $*

GadgetTools.odebug:       GadgetTools.c
        dbc -o$*.odebug $*

TextTools.odebug:         TextTools.c
        dbc -o$*.odebug $*

AlertTools.odebug:        AlertTools.c
        dbc -o$*.odebug $*

AddVectorBuffs.odebug:    AddVectorBuffs.c
        dbc -o$*.odebug $*

DosFix.h:            DosFix.h
        dbc -o$*.odebug $*

BSTRtoSTR.odebug:         BSTRtoSTR.c
        dbc -o$*.odebug $*

amtopm.odebug:            amtopm.c
        dbc -o$*.odebug $*

Crumple.odebug:           Crumple.c
        dbc -o$*.odebug $*

unpacker.odebug:          unpacker.c
        dbc -o$*.odebug $*

packer.odebug:            packer.c
        dbc -o$*.odebug $*

GetA0.odebug:             GetA0.a
        asm $*
        copy $*.o to $*.odebug

CollaspeBitMap.odebug:    CollaspeBitMap.c
        dbc -o$*.odebug $*

EdTools.odebug:           EdTools.c
        dbc -o$*.odebug $*

niff_write.odebug:  		niff_write.c     
        dbc -o$*.odebug $*

niff_read.odebug:     	niff_read.c      
        dbc -o$*.odebug $*

ILBMReadToBm.odebug:   	ILBMReadToBm.c   
        dbc -o$*.odebug $*

ILBMReadToRp.odebug:   	ILBMReadToRp.c   
        dbc -o$*.odebug $*

ImageTools.odebug:       ImageTools.c
        dbc -o$*.odebug $*

RastPortTools.odebug:    RastPortTools.c
        dbc -o$*.odebug $*

SortTools.odebug:       SortTools.c
        dbc -o$*.odebug $*
