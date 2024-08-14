extern	int	ffs(register unsigned long);

int	ffs(w)
register unsigned long w;
{
  	register int bit, m;

	for(m = bit = 1; bit <= 8*sizeof(w); bit++){
		if(w & m){
			break;
		}
		m <<= 1;
	}

  	if(bit > 8*sizeof(w)){
		bit = -1;
	}

	return bit;
}

