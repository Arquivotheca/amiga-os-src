��fileno�_VA_LIST�_VA_LIST 1
��*va_list;��_BUFSIZ 512�BUFSIZ 512�_NFILE 20
�_iobuf
{
�_iobuf*_next;
��*_ptr;
�_rcnt;
�_wcnt;
��*_base;
�_size;
�_flag;
�_file;
��_cbuff;
};
��_iobuf _iob[];�_IOFBF 0�_IOREAD 1�_IOWRT 2�_IONBF 4�_IOMYBUF 8�_IOEOF 16�_IOERR 32�_IOLBF 64�_IORW 128�_IOAPP 0x4000�_IOXLAT 0x8000����0L����_iobuf�EOF (-1)�stdin (&_iob[0])�stdout (&_iob[1])�stderr (&_iob[2])�getc(p) (--(p)->_rcnt>=0?*(p)->_ptr++:_filbf(p))�getchar()getc(stdin)�putc(c,p) (--(p)->_wcnt>=0?((�)(*(p)->_ptr++=(c))):_flsbf((��)(c),p))�putchar(c) putc(c,stdout)�feof(p) (((p)->_flag&_IOEOF)!=0)�ferror(p) (((p)->_flag&_IOERR)!=0)�fileno(p) (p)->_file�rewind(fp) fseek(fp,0L,0)�fflush(fp) _flsbf(-1,fp)�clearerr(fp) clrerr(fp)�__ARGS���__ARGS(a) ()��__ARGS(a) a���printf __builtin_printf
��clrerr __ARGS((�*));
��fclose __ARGS((�*));
��fcloseall __ARGS((�));
��*fdopen __ARGS((�,�*));
��fgetc __ARGS((�*));
��fgetchar __ARGS((�));
��*fgets __ARGS((�*,�,�*));
��flushall __ARGS((�));
��fmode __ARGS((�*,�));
��*fopen __ARGS((�*,�*));
��fprintf __ARGS((�*,�*,...));
��_writes __ARGS((�*,...));
��_tinyprintf __ARGS((�*,...));
��fputc __ARGS((�,�*));
��fputchar __ARGS((�));
��fputs __ARGS((�*,�*));
��fread __ARGS((�*,�,�,�*));
��*freopen __ARGS((�*,�*,�*));
��fscanf __ARGS((�*,�*,...));
��fseek __ARGS((�*,�,�));
��ftell __ARGS((�*));
��fwrite __ARGS((�*,�,�,�*));
��*gets __ARGS((�*));
��printf __ARGS((�*,...));
��puts __ARGS((�*));
��scanf __ARGS((�*,...));
��setbuf __ARGS((�*,�*));
��setnbf __ARGS((�*));
��setvbuf __ARGS((�*,�*,�,�));
��sprintf __ARGS((�*,�*,...));
��sscanf __ARGS((�*,�*,...));
��ungetc __ARGS((�,�*));
��_filbf __ARGS((�*));
��_flsbf __ARGS((�,�*));
��access __ARGS((�*,�));
��chdir __ARGS((�*));
��chmod __ARGS((�*,�));
��*getcwd __ARGS((�*,�));
��mkdir __ARGS((�*));
��perror __ARGS((�*));
��rename __ARGS((�*,�*));
��unlink __ARGS((�*));
��remove __ARGS((�*));
��rmdir __ARGS((�*));
��vprintf __ARGS((�*,va_list));
��vfprintf __ARGS((�*,�*,va_list));
��vsprintf __ARGS((�*,�*,va_list));�abs�abs(x) ((x)<0?-(x):(x))��max�max(a,b) ((a)>(b)?(a):(b))�min(a,b) ((a)<=(b)?(a):(b))��