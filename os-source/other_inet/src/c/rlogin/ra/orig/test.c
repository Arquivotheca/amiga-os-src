
char con_scroll[]      = { '\x9b', '3','A','3','A','3','A','3','A','\x53','\0' } ;
#define CS_ONES cons_scroll[8] 
#define CS_TENS cons_scroll[6] 
#define CS_HUNDREDS cons_scroll[4] 
#define CS_THOUSANDS cons_scroll[2] 


main()
{
	func( 1234 ) ;
	func( 999 ) ;
	func(  3 ) ;
}

int func( int num )
{
	int x = num ;
	int y = 1000 ;
	int val ;
	int offset = 2 ;
	int len = strlen(con_scroll) ;
	char combuf[128]  ;
	
	CopyMem( con_scroll, combuf, 32L) ;
	printf("num = %d\n", num) ;
	
	while ( y )
	{
		val = x / y ;
		printf( "x = %d  y = %d  val = %d\n", x, y, val ) ;
		printf( "offset %d = %c\n\n", offset, val + '0' ) ;
		con_scroll[offset] = (char)(val + '0') ;
		offset += 2 ;
		x = x - (val * y ) ;
		y /= 10 ;
	}
	
	for( x = 1 ; x <len ;x++ )
	{
		printf( "%c/03d\n",con_scroll[x], con_scroll[x]) ;
	}
	printf("\n") ;
}