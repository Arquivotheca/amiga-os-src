#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int lastpts;
    int thispts;

    int currentpts;

	lastpts = 0;

    while(fscanf(stdin,"%lx\n",&currentpts))
    {
         printf("%08lx\n",currentpts-lastpts);
         lastpts = currentpts;
    }
}