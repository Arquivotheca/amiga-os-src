/************************************************************************
**********                                                     **********
**********                     CDTV Tools                      **********
**********                     ----------                      **********
**********                                                     **********
**********           Copyright (C) Pantaray Inc. 1992          **********
**********               Ukiah, CA  707-462-4878               **********
**********                All Rights Reserved.                 **********
**********                                                     **********
*************************************************************************
***
***  MODULE:
***
***	OptCD.h
***
***  PURPOSE:
***
***	Header for OptCD modules
***
***  QUOTE:
***	"I'll buy your lemonade if you buy my delicious Girl Scout cookies..."
***		- Girl Scout
***	"Are they made from real Girl Scouts?"
***		- Wednesday, The Addams Family
***
***  HISTORY:
***
***	0.01 2716 Ken Yeast	Created.
***
************************************************************************/


/***********************************************************************
***
***  Functions
***
************************************************************************/

// Record.c
void	InitRecord( void );
void	Record( CSTR, ULONG, ULONG );
void	QuitRecord( void );
