
extern long stdout;

printf( fmt, args )
char *fmt, *args;
{
    return( _doprnt( stdout, fmt, &args ) );
}
