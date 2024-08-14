/******************
    GameSelect.h

    W.D.L 931019
*******************/



typedef struct GSButton
{
    struct GSButton	* next;
    WORD		  LeftEdge,TopEdge;
    WORD		  Width, Height;
    UWORD		  Flags;
    UWORD		  ID;
    struct IntuiText	* itext;
    struct Border	* border;
    APTR		  UserData;

} GSBUTTON;

