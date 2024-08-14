

struct hdrrdr 
{
    int		level;
    boolean	(*readhdr)(BPTR fp, LzHeader * hdr);
    void	(*writehdr)(BPTR fp, LzHeader * hdr);
};
