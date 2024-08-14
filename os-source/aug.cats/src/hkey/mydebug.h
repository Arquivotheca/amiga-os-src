#ifndef MYDEBUG_H
#define MYDEBUG_H

/**********************************************************************/
/* Debug control                                                      */
/*                                                                    */
/* The first define converts any printfs that got in by mistake into  */
/* kprintfs. If you are debuging to the console you can change        */
/* kprintfs into printfs.                                             */
/* The D1(x) define controls debugging in the standard modules. Use   */
/* The D(x) macro for debugging in the custom.c module.               */
/**********************************************************************/

/*
#define DEBUG
*/

#ifdef DEBUG
#define kprintf printf
#define D1(x) x
#define D(x)  x
#else
#define D1(x) ;
#define D(x)  ;
#endif /* NO DEBUG */

#endif MYDEBUG_H
