

fprintf( out, fmt, args )
char *fmt, *args;
{
    return( _doprnt( out, fmt, &args ) );
}
