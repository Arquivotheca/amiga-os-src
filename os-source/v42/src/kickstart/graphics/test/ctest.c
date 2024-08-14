unsigned long *hw=(unsigned long *) 0xb80038;

main()
{
	int i;
	for(i=0;i<8;i++)
		*hw=0x88888888;
	for(i=0;i<8;i++)
		printf("bpl %d=%08lx\n",i,*hw);
}
