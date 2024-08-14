main(int argc, char **argv)
{
	unsigned long num;
	unsigned long *hw=(unsigned long *) 0xb80038 ;
	sscanf(argv[1],"%x",&num);
	*hw=num;
}
