int i,trycolor,gotcolor,err;
main()
{
	for(i=0;i<20;i++)
	{
		trycolor=0x7fff+err;
		gotcolor=trycolor & 0xff00;
		gotcolor=gotcolor+(gotcolor>>8);
		printf("%04x ",gotcolor);
		err=trycolor-gotcolor;
	}
}
