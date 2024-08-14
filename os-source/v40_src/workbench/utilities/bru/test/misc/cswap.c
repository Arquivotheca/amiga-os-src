#include <stdio.h>

main ()
{
    register int ch1;
    register int ch2;

    ch1 = ch2 = 0;
    while (ch1 != EOF && ch2 != EOF) {
	ch1 = getchar ();
	ch2 = getchar ();
	putchar (ch2);
	putchar (ch1);
    }
}
