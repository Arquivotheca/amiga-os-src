/* Initialize an array of integers */

void init(ip)
int ip[];
{
 	int i;
	int *p;
	
	p = ip;
	for (i = 0; i < 10; i++)
		*p++ = i;
}

/* Pairwise sort elements of an array */

sort(ip)
int *ip;
{
	int i, s;
	int *p;
	
	p = ip;
	s = 0;	/* no swaps performed yet */
	for (i=0; i < 10 - 1; i++)
	    if (p[i] < p[i+1] ) {
		swap(&p[i], &p[i+1] );
		s = 1;	/* indicate a swap took place */
	    }
	return(s);
}