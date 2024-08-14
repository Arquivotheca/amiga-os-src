;/* test_random.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfist -v -j73 test_random.c
Blink FROM LIB:c.o,test_random.o,random.o TO test_random LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit

* Shows the use of both the Amiga.lib RangeRand() function and the
* random.asm Random() function it is linked with to demonstrate
* two similar ways for generating random numbers.
*
* Uses Intuition CurrentTime() to get a seed for the random number generator
* Alternately, you could use the 2.0 timer.device function ReadEClock
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(void)  { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */
#endif


#define MINARGS 1

UBYTE *vers = "\0$VER: test_random 39.1";

/* Option 1: Random.asm  RandomSeed() and Random() and functions
 *
 * RandomSeed() - Seed with a variable number such as the current time
 *
 * Random() - Pass limit of 0-65535, returns a number between 0 and (limit-1)
 */
extern void RandomSeed(ULONG seedvalue);
extern int  Random(LONG limit);

/* Option 2:
 * Amiga.lib ULONG RangeSeed and RangeRand() function
 * 
 * RangeRand() - Pass limit of 0-65535, returns a number between 0 and (limit-1)
 */
#include <clib/alib_protos.h>
extern ULONG far RangeSeed;


void testrand(LONG limit);

struct Library *IntuitionBase;

void main(int argc, char **argv)
    {
    ULONG secs,mics,seed;

    if(IntuitionBase = OpenLibrary("intuition.library",0))
	{
	/* We'll use CurrentTime to get a starter semi-random "seed" value.
	 * You could also use the 2.0 timer.device function ReadEClock.
	 * To make this seed more random, you should do this whenever
	 * a user starts a new game.  That adds the randomness
	 * of how long until the user does something.  If you don't
	 * seed a random number generator, you'll get the same
	 * sequence of "random" values each time you run the program.
	 */
	CurrentTime(&secs,&mics);  /* Use as our seed */
	seed = secs + mics;
	printf("secs=%ld mics=%ld, seed = $08lx\n",secs, mics, seed);

	/* We'll be testing both the random.asm Random() function
 	 * and the amiga.lib RangeRand() function so we need
	 * to seed each with the starter random value
	 * by using the seed initialization method each requires
	 */
	RandomSeed(seed);	   /* seed random.asm Random()   */
	RangeSeed = seed;	   /* seed Amiga.lib RangeRand() */

	/* Call both functions to get some random numbers, passing limit
	 * Both functions return random numbers from 0 through limit-1
	 * Note however that 0 cannot be passed as limit to RangeRand()
	 */
	testrand(3);
	testrand(11);
	testrand(512);
	testrand(65535);

    	CloseLibrary(IntuitionBase);
	}
    }


void testrand(LONG limit)
    {
    int k, ktries;

    ktries = 10;

    /* Note: Can pass limit 0-65535, returns number from 0 to limit-1 */  
    printf("Random(): %ld random values between 0 and %ld:\n",ktries,limit-1);
    for(k=0; k<ktries; k++)   printf("%ld ",Random(limit));
    printf("\n");

    /* Note: Can pass limit 1-65535, returns number from 0 to limit-1  */  
    printf("RangeRand(): %ld random values between 0 and %ld:\n",ktries,limit-1);
    for(k=0; k<ktries; k++)   printf("%ld ",RangeRand(limit));
    printf("\n\n");
    }
