struct j { 
	int max,min;
};

static int __regargs f(x)
struct j *x;
{
	return(x->max);
}

main()
{
	struct j tmp;
	tmp.max=5000;
	printf("max=%ld\n",f(&tmp));
}

	