;/* gamma.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -y -j73 gamma.c
Blink FROM LIB:c.o,gamma.o TO gamma LIBRARY LIB:LCM.lib,LIB:LC.lib,LIB:Amiga.lib
quit
*/

/*
 * Copyright 1991, Hewlett-Packard Company
 * Right to distribute this program, in source or object form, to
 * commercial or individual users, is granted, provided the program
 * is used strictly to improve print quality of Hewlett-Packard printers.
 *
 * This program is distributed as is; no warranty for any particular
 * purpose is expressed or implied.  Hewlett-Packard Company will not
 * provide support for this program.
 *
 * 
 * FILE
 *    gamma.c -- write a gamma compensation curve on stdout
 *
 *    Usage:    gamma gammaVal numSteps
 *    
 *    Example:  gamma 1.2 256
 *
 * DESCRIPTION
 *    Writes a gamma compensation curve based on the floating point number
 *    gammaVal on stdout.  The curve contains numSteps values.  The first
 *    and last values are fixed at 0 and (numSteps - 1).
 *
 *    Exits with 0 if successful, 1 if not.
 *
 * LIMITATIONS
 *    0 < gammaVal <= 10.0
 *    0 < numSteps
 *    
 * SAMPLE COMPILE COMMAND
 *   cc -o gamma gamma.c -lm
 *
 * AUTHOR
 *    willa -- Will Allen, HP Vancouver Division
 *
 * SEE ALSO
 *    Using Computer Color Effectively, An Illustrated Reference
 *    L. G. Thorell and W. J. Smith
 *    Copyright 1990, Prentice Hall
 *    ISBN 0-13-939878-3
 *    pp. 29, 30
 *
 * HISTORY
 *     2 AUG 91 willa    Original version
 *    15 AUG 91 willa    Added numSteps command line parameter
 */


/* INCLUDES */

#include <stdio.h>
#include <math.h>


/* CODE */

main(argc, argv)
int argc;
char *argv[];
{
    int
	i,				/* loop counter */
	g,				/* computed gamma function value */
	numSteps;			/* number of values to compute */

    double
	numSteps1,			/* numSteps - 1 */
	gammaVal;			/* gamma value for curve */

    
    /* Check for correct number of parameters and read 'em in */
    if (argc != 3) {
	fprintf(stderr, "Usage:  %s gammaVal numSteps\n", argv[0]);
	fprintf(stderr, 
		"   gammaVal is the floating point gamma value\n", argv[0]);
	fprintf(stderr, 
		"   numSteps is the integer number of steps for gamma curve\n",
		argv[0]);
	fprintf(stderr, "Example:  gamma 1.2 256\n\n");
	exit(1);
    }
    sscanf(argv[1], "%lf", &gammaVal);
    sscanf(argv[2], "%d", &numSteps);
    
    /* Make sure parameters are reasonable */
    if (gammaVal <= (double)0.0 || gammaVal > (double)10.0) {
	fprintf(stderr, "Usage:  %s gammaVal numSteps\n", argv[0]);
	fprintf(stderr, "   gammaVal must be > 0.0 and <= 10.0\n\n");
	exit(1);
    }
    if (numSteps <= 0) {
	fprintf(stderr, "Usage:  %s gammaVal numSteps\n", argv[0]);
	fprintf(stderr, "   numSteps must be > 0\n\n");
	exit(1);
    }

    numSteps1 = (double)(numSteps - 1);

    /* Crank out the gamma curve */
    for (i = 0; i < numSteps; i++) {
	g = (int)(numSteps1 * 
		  pow((double)i/numSteps1, gammaVal) + (double)0.5);
	printf("%d", g);
	if(!((i+1) % 16))	printf("\n"); 
	else printf(",");
    }

    printf("\n");

    /* Later dude! */
    exit(0);

} /* main() */
