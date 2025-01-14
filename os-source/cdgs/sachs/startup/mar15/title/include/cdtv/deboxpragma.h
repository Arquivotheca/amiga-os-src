/* :ts=8									**/
/*									**/
/*	debox_lib.fd							**/
/*									**/
/*	William A. Ware			9006.20				**/
/*									**/
/**************************************************************************/
/*   This information is CONFIDENTIAL and PROPRIETARY			**/
/*   Copyright (C) 1990, Silent Software Incorporated.			**/
/*   All Rights Reserved.						*							**/
/**************************************************************************/
/*-*/
/*pragma libcall DeBoxBase Decomp 1E BA9804*/
/*pragma libcall DeBoxBase STDDecomp 24 0A9804*/
/*pragma libcall DeBoxBase Eor 2A 90803*/
/*- header*/
#pragma libcall DeBoxBase CheckHeader 30 801
#pragma libcall DeBoxBase HeaderSize 36 801
#pragma libcall DeBoxBase NextComp 3C 9802
/*- decompression*/
#pragma libcall DeBoxBase DecompData 42 A9803
/*- bitmap decompression.*/
#pragma libcall DeBoxBase BMInfoSize 48 9802
#pragma libcall DeBoxBase DecompBMInfo 4E A9803
#pragma libcall DeBoxBase FreeBMInfo 54 801
#pragma libcall DeBoxBase DecompBitMap 5A BA9804
/*- other*/
#pragma libcall DeBoxBase MemSet 60 10803
#pragma libcall DeBoxBase AllocBitMap 66 21003
#pragma libcall DeBoxBase FreeBitMap 6C 801
/*- view*/
#pragma libcall DeBoxBase CreateView 72 2109805
#pragma libcall DeBoxBase DeleteView 78 801
#pragma libcall DeBoxBase CenterViewPort 7E 9802
/*- cycle*/
#pragma libcall DeBoxBase CycleColors 84 0802
