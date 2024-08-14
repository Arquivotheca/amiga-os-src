/***********************************************************************
*                                                                 
* myclassbase.h -- library base for myclasslib.library
*                                                                
* Copyright (C) 1985, 1989, 1990 Commodore Amiga Inc. 
*	All rights reserved.
*
***********************************************************************/

/*** MUST stay in sync with myclassbase.i	***/

struct MyLibBase {
    struct Library	mlb_Library;
    UBYTE		mlb_Flags;

    void		*mlb_SegList;
    struct IClass	*mlb_MyClass;
};
