
/*      Connect4.h - Structs for Connect4
*
*       Mitchell/Ware           Version 1.00            12/13/87
*
*/

typedef struct  c4board
{
        char    border1[3][6+3]; /* fill with -2's */
        char    b[7][6+3];        /* board x-y, 0,0 is at lower left */
        char    border2[3][6+3]; /* fill with -2's */
        int     pos[7];         /* next availiable slot   */
        int     nextmove[7];    /* eval of the next seven possibilities  */
} C4BOARD;

#define sgn(_x)         ( (_x) ? ( ((_x) < 0) ? -1 : 1 ) : 0 )