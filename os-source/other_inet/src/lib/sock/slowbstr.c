/*
** Slow bstrings module for testing.
*/
/*
long
bcmp(b1, b2, len)
     	register char *b1, *b2;
     	register long len;
{
	for(; (*b2++ == *b1++) && --len; ){
	}

	if(len == 0){
		return 0;
	}

	return 1 + len;
}
*/
/*
bzero(s, l)
	register char *s;
	register unsigned int l;
{
	while(l--){
		*s++ = 0;
	}
}
*/
#define SHIFT 4
#define SIZE (1<<SHIFT)

bcopy(f, t, l)
	register char *f, *t;
	register int l;
{
	register long chunk;
	register int size;

	size = SIZE;
	if( (((long)t)&1)==0 && (((long)f)&1)==0 ){
		chunk = l >> SHIFT;

		while(chunk--){
			((short *)t)[0] = ((short *)f)[0];
			((short *)t)[1] = ((short *)f)[1];
			((short *)t)[2] = ((short *)f)[2];
			((short *)t)[3] = ((short *)f)[3];
			((short *)t)[4] = ((short *)f)[4];
			((short *)t)[5] = ((short *)f)[5];
			((short *)t)[6] = ((short *)f)[6];
			((short *)t)[7] = ((short *)f)[7];
			t += size;
			f += size;
		}

		l &= SIZE-1;
	}

 	while(l--){
		*t++ = *f++;
	}
}


