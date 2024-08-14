Äàfilenoà_VA_LISTÄ_VA_LIST 1
•Ñ*va_list;áÄ_BUFSIZ 512ÄBUFSIZ 512Ä_NFILE 20
É_iobuf
{
É_iobuf*_next;
éÑ*_ptr;
Ç_rcnt;
Ç_wcnt;
éÑ*_base;
Ç_size;
Ç_flag;
Ç_file;
éÑ_cbuff;
};
ÅÉ_iobuf _iob[];Ä_IOFBF 0Ä_IOREAD 1Ä_IOWRT 2Ä_IONBF 4Ä_IOMYBUF 8Ä_IOEOF 16Ä_IOERR 32Ä_IOLBF 64Ä_IORW 128Ä_IOAPP 0x4000Ä_IOXLAT 0x8000àúÄú0LáÄ¢É_iobufÄEOF (-1)Ästdin (&_iob[0])Ästdout (&_iob[1])Ästderr (&_iob[2])Ägetc(p) (--(p)->_rcnt>=0?*(p)->_ptr++:_filbf(p))Ägetchar()getc(stdin)Äputc(c,p) (--(p)->_wcnt>=0?((Ç)(*(p)->_ptr++=(c))):_flsbf((éÑ)(c),p))Äputchar(c) putc(c,stdout)Äfeof(p) (((p)->_flag&_IOEOF)!=0)Äferror(p) (((p)->_flag&_IOERR)!=0)Äfileno(p) (p)->_fileÄrewind(fp) fseek(fp,0L,0)Äfflush(fp) _flsbf(-1,fp)Äclearerr(fp) clrerr(fp)à__ARGS∞™Ä__ARGS(a) ()ùÄ__ARGS(a) aááÄprintf __builtin_printf
Åãclrerr __ARGS((¢*));
ÅÇfclose __ARGS((¢*));
ÅÇfcloseall __ARGS((ã));
Å¢*fdopen __ARGS((Ç,Ñ*));
ÅÇfgetc __ARGS((¢*));
ÅÇfgetchar __ARGS((ã));
ÅÑ*fgets __ARGS((Ñ*,Ç,¢*));
ÅÇflushall __ARGS((ã));
ÅÇfmode __ARGS((¢*,Ç));
Å¢*fopen __ARGS((Ñ*,Ñ*));
ÅÇfprintf __ARGS((¢*,Ñ*,...));
ÅÇ_writes __ARGS((Ñ*,...));
ÅÇ_tinyprintf __ARGS((Ñ*,...));
ÅÇfputc __ARGS((Ç,¢*));
ÅÇfputchar __ARGS((Ç));
ÅÇfputs __ARGS((Ñ*,¢*));
ÅÇfread __ARGS((Ñ*,Ç,Ç,¢*));
Å¢*freopen __ARGS((Ñ*,Ñ*,¢*));
ÅÇfscanf __ARGS((¢*,Ñ*,...));
ÅÇfseek __ARGS((¢*,ç,Ç));
Åçftell __ARGS((¢*));
ÅÇfwrite __ARGS((Ñ*,Ç,Ç,¢*));
ÅÑ*gets __ARGS((Ñ*));
ÅÇprintf __ARGS((Ñ*,...));
ÅÇputs __ARGS((Ñ*));
ÅÇscanf __ARGS((Ñ*,...));
ÅÇsetbuf __ARGS((¢*,Ñ*));
ÅÇsetnbf __ARGS((¢*));
ÅÇsetvbuf __ARGS((¢*,Ñ*,Ç,Ç));
ÅÇsprintf __ARGS((Ñ*,Ñ*,...));
ÅÇsscanf __ARGS((Ñ*,Ñ*,...));
ÅÇungetc __ARGS((Ç,¢*));
ÅÇ_filbf __ARGS((¢*));
ÅÇ_flsbf __ARGS((Ç,¢*));
ÅÇaccess __ARGS((Ñ*,Ç));
ÅÇchdir __ARGS((Ñ*));
ÅÇchmod __ARGS((Ñ*,Ç));
ÅÑ*getcwd __ARGS((Ñ*,Ç));
ÅÇmkdir __ARGS((Ñ*));
ÅÇperror __ARGS((Ñ*));
ÅÇrename __ARGS((Ñ*,Ñ*));
ÅÇunlink __ARGS((Ñ*));
ÅÇremove __ARGS((Ñ*));
ÅÇrmdir __ARGS((Ñ*));
ÅÇvprintf __ARGS((Ñ*,va_list));
ÅÇvfprintf __ARGS((¢*,Ñ*,va_list));
ÅÇvsprintf __ARGS((Ñ*,Ñ*,va_list));àabsÄabs(x) ((x)<0?-(x):(x))áàmaxÄmax(a,b) ((a)>(b)?(a):(b))Ämin(a,b) ((a)<=(b)?(a):(b))áá