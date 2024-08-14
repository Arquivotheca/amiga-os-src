
/*
** Scarfed from GNU project.  Rewrite in assembly language at some
** point.
*/


int
ffs (i)
     	register unsigned long i;
{
  	register int value = 1;

  	if (!i){
		return -1;
	}

  	if (!(i & 0xFFFF)){
    		value += 16, i >>= 16;
	}

  	if (!(i & 0xFF)){
    		value += 8, i >>= 8;
	}

  	if (!(i & 0xF)){
    		value += 4, i >>= 4;
	}

  	if (!(i & 0x3)){
    		value += 2, i >>= 2;
	}

  	if (!(i & 1)){
    		value++;
	}

  	return value;
}

