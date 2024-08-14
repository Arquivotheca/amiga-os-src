extern long stdout;

/* print the string s to standard output */
puts( s )
    char *s;
{
    fprintf (stdout, "%s\n", s);
}
