
	TTL	'PARSE.I'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************

        SECTION speech


SPCGraphemes:
	DC.B	0,PHCBL

EXPGraphemes:
	DC.B	0,PHCNULL

DQUGraphemes:
	DC.B	0,PHCNULL

NUMGraphemes:
	DC.B	0,PHCNULL

DLRGraphemes:
	DC.B	0,PHCNULL

PCTGraphemes:
	DC.B	0,PHCNULL

AMPGraphemes:
	DC.B	0,PHCNULL

SQUGraphemes:
	DC.B	0,PHCNULL

LPRGraphemes:
	DC.B	0,PHCLP

RPRGraphemes:
	DC.B	0,PHCRP

ASTGraphemes:
	DC.B	0,PHCNULL

PLSGraphemes:
	DC.B	0,PHCNULL

COMGraphemes:
	DC.B	0,PHCCO

DASGraphemes:
	DC.B	0,PHCDA

PERGraphemes:
	DC.B	0,PHCPE

SlashGraphemes:
	DC.B	'H',PHCHH
	DC.B	'C',PHCCH
	DC.B	0,PHCNULL

ZeroGraphemes:
	DC.B	0,PHC0

OneGraphemes:
	DC.B	0,PHC1

TwoGraphemes:
	DC.B	0,PHC2

ThreeGraphemes:
	DC.B	0,PHC3

FourGraphemes:
	DC.B	0,PHC4

FiveGraphemes:
	DC.B	0,PHC5

SixGraphemes:
	DC.B	0,PHC6

SevenGraphemes:
	DC.B	0,PHC7

EightGraphemes:
	DC.B	0,PHC8

NineGraphemes:
	DC.B	0,PHC9

COLGraphemes:
	DC.B	0,PHCNULL

SEMGraphemes:
	DC.B	0,PHCNULL

LSTGraphemes:
	DC.B	0,PHCNULL

EQUGraphemes:
	DC.B	0,PHCNULL

GRTGraphemes:
	DC.B	0,PHCNULL

QUEGraphemes:
	DC.B	0,PHCQM

ATSGraphemes:
	DC.B	0,PHCNULL

AGraphemes:
	DC.B	'A',PHCAA
	DC.B	'E',PHCAE
	DC.B	'O',PHCAO
	DC.B	'W',PHCAW
	DC.B	'Y',PHCAY
	DC.B	'H',PHCAH
	DC.B	'X',PHCAX
	DC.B	0,PHCNULL

BGraphemes:
	DC.B	0,PHCB

CGraphemes:
	DC.B	'H',PHCCH
	DC.B	0,PHCNULL

DGraphemes:
	DC.B	'H',PHCDH
	DC.B	'X',PHCDX
	DC.B	0,PHCD

EGraphemes:
	DC.B	'H',PHCEH
	DC.B	'Y',PHCEY
	DC.B	'R',PHCER
	DC.B	0,PHCNULL

FGraphemes:
	DC.B	0,PHCF

GGraphemes:
	DC.B	0,PHCG

HGraphemes:
	DC.B	0,PHCNULL

IGraphemes:
	DC.B	'X',PHCIX
	DC.B	'H',PHCIH
	DC.B	'M',PHCIM
	DC.B	'N',PHCIN
	DC.B	'L',PHCIL
	DC.B	'Y',PHCIY
	DC.B	0,PHCNULL

JGraphemes:
	DC.B	0,PHCJ

KGraphemes:
	DC.B	0,PHCK

LGraphemes:
	DC.B	'X',PHCLX
	DC.B	0,PHCL

MGraphemes:
	DC.B	0,PHCM

NGraphemes:
	DC.B	'X',PHCNX
	DC.B	0,PHCN

OGraphemes:
	DC.B	'W',PHCOW
	DC.B	'H',PHCOH
	DC.B	'Y',PHCOY
	DC.B	0,PHCNULL

PGraphemes:
	DC.B	0,PHCP

QGraphemes:
	DC.B	'X',PHCQX
	DC.B	0,PHCQ

RGraphemes:
	DC.B	'X',PHCRX
	DC.B	0,PHCRR

SGraphemes:
	DC.B	'H',PHCSH
	DC.B	0,PHCS

TGraphemes:
	DC.B	'H',PHCTH
	DC.B	0,PHCT

UGraphemes:
	DC.B	'H',PHCUH
	DC.B	'W',PHCUW
	DC.B	'N',PHCUN
	DC.B	'M',PHCUM
	DC.B	'L',PHCUL
	DC.B	0,PHCNULL

VGraphemes:
	DC.B	0,PHCV

WGraphemes:
	DC.B	'H',PHCWH
	DC.B	0,PHCW

XGraphemes:
	DC.B	0,PHCNULL

YGraphemes:
	DC.B	0,PHCY

ZGraphemes:
	DC.B	'H',PHCZH
	DC.B	0,PHCZ


GraphemePointers
	DC.L	SPCGraphemes
	DC.L	EXPGraphemes
	DC.L	DQUGraphemes
	DC.L	NUMGraphemes
	DC.L	DLRGraphemes
	DC.L	PCTGraphemes
	DC.L	AMPGraphemes
	DC.L	SQUGraphemes
	DC.L	LPRGraphemes
	DC.L	RPRGraphemes
	DC.L	ASTGraphemes
	DC.L	PLSGraphemes
	DC.L	COMGraphemes
	DC.L	DASGraphemes
	DC.L	PERGraphemes
	DC.L	SlashGraphemes
	DC.L	ZeroGraphemes
	DC.L	OneGraphemes
	DC.L	TwoGraphemes
	DC.L	ThreeGraphemes
	DC.L	FourGraphemes
	DC.L	FiveGraphemes
	DC.L	SixGraphemes
	DC.L	SevenGraphemes
	DC.L	EightGraphemes
	DC.L	NineGraphemes
	DC.L	COLGraphemes
	DC.L	SEMGraphemes
	DC.L	LSTGraphemes
	DC.L	EQUGraphemes
	DC.L	GRTGraphemes
	DC.L	QUEGraphemes
	DC.L	ATSGraphemes
	DC.L	AGraphemes
	DC.L	BGraphemes
	DC.L	CGraphemes
	DC.L	DGraphemes
	DC.L	EGraphemes
	DC.L	FGraphemes
	DC.L	GGraphemes
	DC.L	HGraphemes
	DC.L	IGraphemes
	DC.L	JGraphemes
	DC.L	KGraphemes
	DC.L	LGraphemes
	DC.L	MGraphemes
	DC.L	NGraphemes
	DC.L	OGraphemes
	DC.L	PGraphemes
	DC.L	QGraphemes
	DC.L	RGraphemes
	DC.L	SGraphemes
	DC.L	TGraphemes
	DC.L	UGraphemes
	DC.L	VGraphemes
	DC.L	WGraphemes
	DC.L	XGraphemes
	DC.L	YGraphemes
	DC.L	ZGraphemes



MAXGRAPHEME	EQU	'Z'-' '
