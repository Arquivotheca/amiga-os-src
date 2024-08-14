/*
 * $Id: egcldpro.h,v 3.4.1.1 91/07/08 14:14:32 katogi GM Locker: katogi $
 */

extern  short egcinit(
#ifdef P_TYPE
#ifdef  DOS_EGBRIDGE
unsigned char*, unsigned short, unsigned char*, unsigned short
#else
void 
#endif
#endif /* P_TYPE */
);
extern  short egcexpdicinfo(
#ifdef P_TYPE
char **hdics
#endif /* P_TYPE */
);
extern  short egcusoff(
#ifdef P_TYPE
void 
#endif /* P_TYPE */
);
extern  short egcfinish(
#ifdef P_TYPE
void 
#endif /* P_TYPE */
);
#ifdef UNIX
extern  struct _iobuf *egcfopen(
#ifdef P_TYPE
unsigned char *open_path,unsigned char *type
#endif /* P_TYPE */
);
extern  int egcfclose(
#ifdef P_TYPE
struct _iobuf *close_fp
#endif /* P_TYPE */
);
extern  int egcfseek(
#ifdef P_TYPE
struct _iobuf *seek_fp,long seek_len,int seek_pos
#endif /* P_TYPE */
);
extern  int egcfread(
#ifdef P_TYPE
unsigned char *read_buf,int read_size,int read_len,struct _iobuf *read_fp
#endif /* P_TYPE */
);
extern  int egcfwrite(
#ifdef P_TYPE
unsigned char *write_buf,int write_size,int write_len,struct _iobuf *write_fp
#endif /* P_TYPE */
);
#endif
#ifdef DOS_EGC
extern   short egcfopen(
#ifdef P_TYPE
unsigned char *open_path,short type
#endif /* P_TYPE */
);
extern  int egcfclose(
#ifdef P_TYPE
short close_fp
#endif /* P_TYPE */
);
extern  int egcfseek(
#ifdef P_TYPE
short seek_fp,long seek_len,int seek_pos
#endif /* P_TYPE */
);
extern  int egcfread(
#ifdef P_TYPE
unsigned char *read_buf,int read_size,int read_len,short read_fp
#endif /* P_TYPE */
);
extern  int egcfwrite(
#ifdef P_TYPE
unsigned char *write_buf,int write_size,int write_len,short write_fp
#endif /* P_TYPE */
);
#endif

extern  short egcopenexpdic(
#ifdef P_TYPE
short idno,unsigned char *path
#endif /* P_TYPE */
);
extern  short egccloseexpdic(
#ifdef P_TYPE
short idno
#endif /* P_TYPE */
);
extern  unsigned char *egcgetuserdic(
#ifdef P_TYPE
unsigned char *dstdicname
#endif /* P_TYPE */
);
extern  unsigned char *egcgetmaindic(
#ifdef P_TYPE
unsigned char *dstdicname
#endif /* P_TYPE */
);
extern  unsigned char *egcsetuserdic(
#ifdef P_TYPE
unsigned char *newdicname
#endif /* P_TYPE */
);
extern  unsigned char *egcsetmaindic(
#ifdef P_TYPE
unsigned char *newdicname
#endif /* P_TYPE */
);
extern  short egctxcvt(
#ifdef P_TYPE
unsigned char *src,unsigned short srcl,unsigned char *dst,unsigned short *dstl,unsigned short *sep,unsigned short *sepl,unsigned short *fixl,unsigned char cmode
#endif /* P_TYPE */
);
extern  short egcclcvt(
#ifdef P_TYPE
unsigned char *src,unsigned short srcl,unsigned char *dstr,unsigned char *dsep,struct wi *dlid,unsigned short *dnbr,unsigned char cmode
#endif /* P_TYPE */
);
extern  short egclearn(
#ifdef P_TYPE
struct wi *dlid
#endif /* P_TYPE */
);
extern  short egclearn2(
#ifdef P_TYPE
struct wi *dlid,unsigned short dnbr,unsigned short tnbr
#endif /* P_TYPE */
);
extern  short egcwdadd(
#ifdef P_TYPE
unsigned char *ystr,unsigned short yl,unsigned char *kstr,unsigned short kl,unsigned char *uh
#endif /* P_TYPE */
);
extern  short egcwddel(
#ifdef P_TYPE
unsigned char *ystr,unsigned short yl,unsigned char *kstr,unsigned short kl
#endif /* P_TYPE */
);
extern  short egccllearn(
#ifdef P_TYPE
unsigned char *kStr,unsigned short kl1,unsigned short kl2,unsigned char *yStr,unsigned short yl1,unsigned short yl2
#endif /* P_TYPE */
);
extern  short egcerr(
#ifdef P_TYPE
short noerr
#endif /* P_TYPE */
);
extern  short egcrmcvt(
#ifdef P_TYPE
unsigned char *src,unsigned short srcl,unsigned char *dst,unsigned short *dstl,unsigned short *totl,unsigned char cmode
#endif /* P_TYPE */
);
extern  short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
egcjacvt(
#ifdef P_TYPE
unsigned char *src,unsigned short srcl,unsigned char *dst,unsigned short *dstl
#endif /* P_TYPE */
);
extern  short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
egcajcvt(
#ifdef P_TYPE
unsigned char *src,unsigned short srcl,unsigned char *dst,unsigned short *dstl,unsigned char cmode
#endif /* P_TYPE */
);
extern  short egcsjcvt(
#ifdef P_TYPE
unsigned char *src,unsigned short srcl,unsigned char *dst,unsigned short *dstl
#endif /* P_TYPE */
);
extern  short egcjscvt(
#ifdef P_TYPE
unsigned char *src,unsigned short srcl,unsigned char *dst,unsigned short *dstl
#endif /* P_TYPE */
);
extern  short egcdhchk(
#ifdef P_TYPE
unsigned char *pt,unsigned char mode,unsigned char *check
#endif /* P_TYPE */
);
extern  short egccreate(
#ifdef P_TYPE
unsigned char *newmain,struct tagDICM *dic
#endif /* P_TYPE */
);
extern  short egcwrite(
#ifdef P_TYPE
unsigned char *yomi,unsigned short yomil,unsigned char *kanji,unsigned short kanjil,unsigned char *hinshi,struct tagDICM *dic
#endif /* P_TYPE */
);
extern  short egceof(
#ifdef P_TYPE
struct tagDICM *dic
#endif /* P_TYPE */
);
extern  short egcndic(
#ifdef P_TYPE
unsigned char *mpath,unsigned char *upath,unsigned short plosize,unsigned char sltvalue
#endif /* P_TYPE */
);
extern  void egcfdel(
#ifdef P_TYPE
unsigned char *wfile
#endif /* P_TYPE */
);
extern  void *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
egcnewptr(
#ifdef P_TYPE
int size
#endif /* P_TYPE */
);
extern  short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
egcfreeptr(
#ifdef P_TYPE
unsigned char *ptr
#endif /* P_TYPE */
);
extern  unsigned char *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
egcmemmove(
#ifdef P_TYPE
unsigned char *dest,unsigned char *src,unsigned short cnt
#endif /* P_TYPE */
);
extern  void
#ifdef  DOS_EGBRIDGE
_pascal
#endif
egcmemset(
#ifdef P_TYPE
unsigned char *s,unsigned char c,unsigned short n
#endif /* P_TYPE */
);
extern  short egcfind(
#ifdef P_TYPE
unsigned char *yomi,unsigned short yomil,unsigned char smode,unsigned char dmode,struct tagDICR *dic
#endif /* P_TYPE */
);
extern  short egcnext(
#ifdef P_TYPE
struct tagDICR *dic
#endif /* P_TYPE */
);
extern  short egcread(
#ifdef P_TYPE
unsigned char *yomi,unsigned short *yomil,unsigned char *kanji,unsigned short *kanjil,unsigned char *hinshi,struct tagDICR *dic
#endif /* P_TYPE */
);
extern  short egcnextdb(
#ifdef P_TYPE
struct tagDICR *dic
#endif /* P_TYPE */
);
extern  short egcdicverchk(
#ifdef P_TYPE
unsigned char *a_path,unsigned char *b_path
#endif /* P_TYPE */
);
extern  short egcsysverchk(
#ifdef P_TYPE
unsigned char *mpath
#endif /* P_TYPE */
);
extern  long egcgetsysverno(
#ifdef P_TYPE
void 
#endif /* P_TYPE */
);
extern  long egcgetmainstrver(
#ifdef P_TYPE
unsigned char *path
#endif /* P_TYPE */
);
extern  long egcgetuserstrver(
#ifdef P_TYPE
unsigned char *path
#endif /* P_TYPE */
);
extern  long egcgetmaindicver(
#ifdef P_TYPE
unsigned char *path
#endif /* P_TYPE */
);
extern  long egcgetuserdicver(
#ifdef P_TYPE
unsigned char *path
#endif /* P_TYPE */
);
extern  long egcsetmainstrver(
#ifdef P_TYPE
unsigned char *path,long lver
#endif /* P_TYPE */
);
extern  long egcsetuserstrver(
#ifdef P_TYPE
unsigned char *path,long lver
#endif /* P_TYPE */
);
extern  long egcsetmaindicver(
#ifdef P_TYPE
unsigned char *path,long lver
#endif /* P_TYPE */
);
extern  long egcsetuserdicver(
#ifdef P_TYPE
unsigned char *path,long lver
#endif /* P_TYPE */
);
extern  long egc_ver_write(
#ifdef P_TYPE
unsigned char *path,long new_ver,long sekofs,int dtype
#endif /* P_TYPE */
);
extern  long egc_ver_read(
#ifdef P_TYPE
unsigned char *path,long sekofs,int dtype
#endif /* P_TYPE */
);
extern  short nullcvt(
#ifdef P_TYPE
unsigned char *src,unsigned short srcl,unsigned char *dst,unsigned short *dstl
#endif /* P_TYPE */
);
extern  short egcsecvt(
#ifdef P_TYPE
unsigned char *src,unsigned short srcl,unsigned char *dst,unsigned short *dstl
#endif /* P_TYPE */
);
extern  short egcescvt(
#ifdef P_TYPE
unsigned char *src,unsigned short srcl,unsigned char *dst,unsigned short *dstl
#endif /* P_TYPE */
);
extern  int egcabort(
#ifdef P_TYPE
void
#endif /* P_TYPE */
);
extern   long egcftell(
#ifdef P_TYPE
short write_fp
#endif
);
