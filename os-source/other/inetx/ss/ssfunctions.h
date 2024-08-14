/* ---------------------------------------------------------
 * ssfunctions.h  -  function protos and pragmas for the
 *                   shared socket library functions
 * ---------------------------------------------------------
 */
 
#ifndef	SSFUNCTIONS_H
#define SSFUNCTION_H 1

int Func0( void ) ;
int Func1( void ) ;
int Func2( long arg ) ;
int Func3( void ) ;
int Func4( void ) ;
int Func5( void ) ;
int Func6( void ) ;
int Func7( void ) ;
 
 

#pragma amicall(SSBase, 0x1e, Func0())
#pragma amicall(SSBase, 0x24, Func1())
#pragma amicall(SSBase, 0x2a, Func2(a0))
#pragma amicall(SSBase, 0x30, Func3())
#pragma amicall(SSBase, 0x36, Func4())
#pragma amicall(SSBase, 0x3c, Func5())
#pragma amicall(SSBase, 0x42, Func6())
#pragma amicall(SSBase, 0x48, Func7())
 
#endif /* SSFUNCTIONS_H */
