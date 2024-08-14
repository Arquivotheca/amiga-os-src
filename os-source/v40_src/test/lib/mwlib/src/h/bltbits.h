/*****************************************************************************
    BltBits.h
    
    Mitchell/Ware Systems           Version 1.00            29-Mar-89
    
    Additional defines for Blits
*****************************************************************************/

#define BLTCON0(ash, use, lf)   (((ash) << 12) | ((use) << 8) | (lf))
#define BLTCON1(bsh, cbits)     (((bsh) << 12) | (cbits))

/* bits to use with BLTCON0 - use
*/
#define USED    1
#define USEC    2
#define USEB    4
#define USEA    8

/* bits to use with BLTCON1
*/
#define EFE (1<<4)      /* Exclusive Fill Enable */
#define IFE (1<<3)      /* Inclusive Fill Enable */
#define FCI (1<<2)      /* Fill Carry Input */
#define DESC (1<<1)     /* Descending control bit */
#define LINE  1         /* Line drawing mode */
