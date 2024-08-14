/*
 * index() for lattice, and BSD compatible one for Manx
 */

char *
index(s, c)
	register char *s, c;
{
	while(*s){
		if(*s == c){
			break;
		}
		s++;
	}

	if(*s == c){
		return s;
	}
	return 0;
}
