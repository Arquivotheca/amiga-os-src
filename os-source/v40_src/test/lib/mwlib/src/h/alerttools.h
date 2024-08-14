/****** mwhead/AlertTools *****************************************************

    Mitchell/Ware Systems, Inc          Version 1.00            19 Oct 1990

        Structs for AlertTools - ALMESS

        ALMESS contains the alert message structure that DisplayAlert
        expects.

****************************************************************************/

typedef struct  almess
{
    USHORT x;
    UBYTE  y;
    char mess[1];
} ALMESS;

