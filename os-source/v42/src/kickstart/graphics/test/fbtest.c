char *f1;
int i;
main()
{
	f1=(char *) 0x40200000;
	for(i=0;i<1152*900;i++)
		*(f1++)=i & 0xff;
}
