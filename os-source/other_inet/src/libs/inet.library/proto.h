
#ifdef DEBUG(x)
        #undef DEBUG(x)
#endif
#ifdef DEBUG_IT
        #define DEBUG(x) kprintf(x)
#else
        #define DEBUG(x)
#endif

#ifdef LATTICE
        #include <proto/exec.h>
#endif

extern struct ExecBase *SysBase;

typedef char * caddr_t;

/* af.c */
void null_init(void);
void null_hash(struct sockaddr *addr, struct afhash *hp);
int  null_netmatch(struct sockaddr *a1, struct sockaddr *a2);

/* bstring.c */
int ffs(unsigned long i);

/* compat.c */
/* void microtime(void); */
void selwakeup(struct sockbuf *sb);
// int MAX(int a, int b);
//int imin(int a, int b);
#define imin(a,b)       ((a)<(b)?(a):(b))
// void log(int type, char *message, ...);
void panic(char *msg);
#ifdef AZTEC_C
int uiomove(char *bufp, int len, enum uio_op, struct uio *uio);
#else
int uiomove(char *bufp, int len, int uio_op, struct uio *uio);
#endif
void asignal(struct socket *so, int which);


/* if.c */
void ifinit(void);
void if_attach(struct ifnet *ifp);
struct ifaddr *ifa_ifwithaddr(struct sockaddr *addr);
struct ifaddr *ifa_ifwithdstaddr(struct sockaddr *addr);
struct ifaddr *ifa_ifwithnet(struct sockaddr *addr);
void if_down(struct ifnet *ifp);
void if_qflush(struct ifqueue *ifq);
void if_slowtimo(void);
struct ifnet *ifunit(char *name);
int ifioctl(struct socket *so, int cmd, char *data);
int ifconf(int cmd, char *data);

/* if_aa.c */
#ifdef LATTICE
void __saveds __stdargs aaintrC(void);
#endif

void aapoll(void);
int aaattach(void);

/* if_ae.c */
#ifdef LATTICE
void __saveds __stdargs aeintrC(void);
#else
void aeintrC(void);
#endif

void aepoll(void);
int aeattach(void);

/* if_sl.c */
void slattach(void);
void slpoll(void);


/* if_ether.c */
void arp_nlist_init(void);
void arptimer(void);
void arpwhohas(struct arpcom *ac, struct in_addr *addr);
int arpresolve(struct arpcom *ac, struct mbuf *m, struct in_addr *destip, unsigned char *desten, int *usetrailers);
void arpinput(struct arpcom *ac, struct mbuf *m);
void in_arpinput(struct arpcom *ac, struct mbuf *m);
void arptfree(struct arptab *at);
struct arptab *arptnew(struct in_addr *addr);
int arpioctl(int cmd, char *data);
char *ether_sprintf(unsigned char *ap);

/* if_loop.c */
void loattach(void);
int looutput(struct ifnet *ifp, struct mbuf *m0, struct sockaddr *dst);
int loioctl(struct ifnet *ifp, int cmd, char *data);

/* in.c */
/* don't include these unless proper #includes have already been read */

void inet_hash(struct sockaddr_in *sin, struct afhash *hp);
int  inet_netmatch(struct sockaddr_in *sin1, struct sockaddr_in *sin2);
struct in_addr in_makeaddr(unsigned long net, unsigned long host);
#ifdef IPPROTO_IP
unsigned long in_netof(struct in_addr in);
unsigned long in_lnaof(struct in_addr in);
int in_localaddr(struct in_addr in);
int in_canforward(struct in_addr in);
int in_control(struct socket *so, int cmd, char *data, struct ifnet *ifp);
int in_ifinit(struct ifnet *ifp, struct in_ifaddr *ia, struct sockaddr_in *sin);
struct in_ifaddr *in_iaonnetof(unsigned long net);
int in_broadcast(struct in_addr in);
#endif

/* inet.c */
int startINET(void);
void killINET(void);
void setsoftnet(void);
void setsoftif(void);
void setsoftifaa(void);
int timeout(void (*func)(), char *arg, int ticks);
void free(void);
void printf(char *fmt, ...);

/* in_cksum.c */
int in_cksum(struct mbuf *m, int len);

/* in_pcb.c */
#ifdef IPPROTO_IP
int in_pcballoc(struct socket *so, struct inpcb *head);
int in_pcbbind(struct inpcb *inp, struct mbuf *nam);
int in_pcbconnect(struct inpcb *inp, struct mbuf *nam);
void in_pcbdisconnect(struct inpcb *inp);
void in_pcbdetach(struct inpcb *inp);
void in_setsockaddr(struct inpcb *inp, struct mbuf *nam);
void in_setpeeraddr(struct inpcb *inp, struct mbuf *nam);
void in_pcbnotify(struct inpcb *head, struct in_addr *dst, int errno, void (*notify)(struct inpcb *));
void in_losing(struct inpcb *inp);
void in_rtchange(struct inpcb *inp);
struct inpcb *in_pcblookup(struct inpcb *, struct in_addr, u_short, struct in_addr, u_short, int);
#endif

/* ip_icmp.c */
void icmp_nlist_init(void);
#ifdef IPPROTO_IP
void icmp_error(struct ip *oip, int type, int code, struct ifnet *ifp, struct in_addr dest);
void icmp_input(struct mbuf *m, struct ifnet *ifp);
void icmp_reflect(struct ip *ip, struct ifnet *ifp);
struct in_ifaddr *ifptoia(struct ifnet *ifp);
void icmp_send(struct ip *ip, struct mbuf *opts);
unsigned long iptime(void);

/* ip_input.c */
void ip_init(void);
struct ip *ip_reass(struct ipasfrag *ip, struct ipq *fp);
void ip_freef(struct ipq *fp);
void ip_enq(struct ipasfrag *p, struct ipasfrag *prev);
void ip_deq(struct ipasfrag *p);
void ip_slowtimo(void);
void ip_drain(void);
int ip_dooptions(struct mbuf *m, struct ifnet *ifp);
struct in_ifaddr *ip_rtaddr(struct in_addr dst);
void save_rte(unsigned char *option, struct in_addr dst);
struct mbuf *ip_srcroute(void);
void ip_stripoptions(struct ip *ip, struct mbuf *mopt);
void ip_forward(struct mbuf *m, struct ifnet *ifp);
#endif
void ipintr(void);

/* ip_output.c */
int ip_output(struct mbuf *m0, struct mbuf *opt, struct route *ro, int flags);
struct mbuf *ip_insertoptions(struct mbuf *m, struct mbuf *opt, int *phlen);
int ip_optcopy(struct ip *ip, struct ip *jp);
int ip_ctloutput(int op, struct socket *so, int level, int optname, struct mbuf **m);
int ip_pcbopts(struct mbuf **pcbopt, struct mbuf *m);

/* lib_custom.c */
void enter_nlist(char *name, void *value);

/* proc.c */
int sleep(unsigned long chan, int pri);
void wakeup(unsigned long chan);
void InitList(struct List *l);
int splnet(void);
/* int splimp(void); */
int splx(int s);
#define splimp  splnet

/* q.c */
void insque(struct qelem *elem, struct qelem *pred);
void remque(struct qelem *elem);

/* raw_cb.c */
int raw_attach(struct socket *so, int proto);
void raw_detach(struct rawcb *rp);
void raw_disconnect(struct rawcb *rp);
int raw_bind(struct socket *so, struct mbuf *nam);
void raw_connaddr(struct rawcb *rp, struct mbuf *nam);

/* raw_ip.c */
void rip_input(struct mbuf *m);
int rip_output(struct mbuf *m0, struct socket *so);
int rip_ctloutput(int op, struct socket *so, int level, int optname, struct mbuf **m);

/* raw_usrreq.c */
void raw_init(void);
void raw_input(struct mbuf *m0, struct sockproto *proto, struct sockaddr *src, struct sockaddr *dst);
void rawintr(void);
void raw_ctlinput(int cmd, struct sockaddr *arg);
int raw_usrreq(struct socket *so, int req, struct mbuf *m, struct mbuf *nam, struct mbuf *rights);

/* route.c */
void route_nlist_init(void);
void rtalloc(struct route *ro);
void rtfree(struct rtentry *rt);
void rtredirect(struct sockaddr *dst, struct sockaddr *gateway, int flags, struct sockaddr *src);
int rtioctl(int cmd, char *data);
int rtrequest(int req, struct rtentry *entry);
void rtinit(struct sockaddr *dst, struct sockaddr *gateway, int cmd, int flags);

/* sys_socket.c */
int soo_ioctl(struct socket *so, int cmd, char *data);

/* tcp_debug.c */
void tcp_trace(short act, short ostate, struct tcpcb *tp, struct tcpiphdr *ti, int req);

/* tcp_input.c */
int tcp_reass(struct tcpcb *tp, struct tcpiphdr *ti);
void tcp_input(struct mbuf *m0);
void tcp_dooptions(struct tcpcb *tp, struct mbuf *om, struct tcpiphdr *ti);
void tcp_pulloutofband(struct socket *so, struct tcpiphdr *ti);
int tcp_mss(struct tcpcb *tp);

/* tcp_output.c */
int tcp_output(struct tcpcb *tp);
void tcp_setpersist(struct tcpcb *tp);

/* tcp_subr.c */
void tcp_init(void);
struct tcpiphdr *tcp_template(struct tcpcb *tp);
void tcp_respond(struct tcpcb *tp, struct tcpiphdr *ti, unsigned long ack, unsigned long seq, int flags);
struct tcpcb *tcp_newtcpcb(struct inpcb *inp);
struct tcpcb *tcp_drop(struct tcpcb *tp, int errno);
struct tcpcb *tcp_close(struct tcpcb *tp);
/* int tcp_drain(void); */
void tcp_notify(struct inpcb *inp);
void tcp_ctlinput(int cmd, struct sockaddr *sa);
void tcp_quench(struct inpcb *inp);

/* tcp_timer.c */
void tcp_fasttimo(void);
void tcp_slowtimo(void);
void tcp_canceltimers(struct tcpcb *tp);
struct tcpcb *tcp_timers(struct tcpcb *tp, int timer);

/* tcp_usrreq.c */
int tcp_usrreq(struct socket *so, int req, struct mbuf *m, struct mbuf *nam, struct mbuf *rights);
int tcp_ctloutput(int op, struct socket *so, int level, int optname, struct mbuf **mp);
int tcp_attach(struct socket *so);
struct tcpcb *tcp_disconnect(struct tcpcb *tp);
struct tcpcb *tcp_usrclosed(struct tcpcb *tp);

/* udp_usrreq.c */
void udp_init(void);
void udp_input(struct mbuf *m0, struct ifnet *ifp);
void udp_notify(struct inpcb *inp);
void udp_ctlinput(int cmd, struct sockaddr *sa);
int udp_output(struct inpcb *inp, struct mbuf *m0);
int udp_usrreq(struct socket *so, int req, struct mbuf *m, struct mbuf *nam, struct mbuf *rights);

/* uipc_domain.c */
void domaininit(void);
struct protosw *pffindtype(int family, int type);
struct protosw *pffindproto(int family, int protocol, int type);
void pfctlinput(int cmd, struct sockaddr *sa);
void pfslowtimo(void);
void pffasttimo(void);



/* uipc_mbuf.c */
void mbinit(void);
char *m_clalloc(int ncl, int how, int canwait);
int m_expand(int canwait);
struct mbuf *m_get(int canwait, int type);
struct mbuf *m_getclr(int canwait, int type);
struct mbuf *m_free(struct mbuf *m);
struct mbuf *m_more(int canwait, int type);
void m_freem(struct mbuf *m);
struct mbuf *m_copy(struct mbuf *m, int off, int len);
void m_copydata(struct mbuf *m, int off, int len, char *cp);
void m_cat(struct mbuf *m, struct mbuf *n);
void m_adj(struct mbuf *mp, int len);
struct mbuf *m_pullup(struct mbuf *n, int len);

/* uipc_socket.c */
int socreate(int dom, struct socket **aso, int type, int proto);
int sobind(struct socket *so, struct mbuf *nam);
int solisten(struct socket *so, int backlog);
void sofree(struct socket *so);
int soclose(struct socket *so);
int soabort(struct socket *so);
int soaccept(struct socket *so, struct mbuf *nam);
int soconnect(struct socket *so, struct mbuf *nam);
int soconnect2(struct socket *so1, struct socket *so2);
int sodisconnect(struct socket *so);
int sosend(struct socket *so, struct mbuf *nam, struct uio *uio, int flags, struct mbuf *rights);
int soreceive(struct socket *so, struct mbuf **aname, struct uio *uio, int flags, struct mbuf **rightsp);
int soshutdown(struct socket *so, int how);
void sorflush(struct socket *so);
int sosetopt(struct socket *so, int level, int optname, struct mbuf *m0);
int sogetopt(struct socket *so, int level, int optname, struct mbuf **mp);
void sohasoutofband(struct socket *so);

/* uipc_socket2.c */
void soisconnecting(struct socket *so);
void soisconnected(struct socket *so);
void soisdisconnecting(struct socket *so);
void soisdisconnected(struct socket *so);
struct socket *sonewconn(struct socket *head);
void soqinsque(struct socket *head, struct socket *so, int q);
int soqremque(struct socket *so, int q);
void socantsendmore(struct socket *so);
void socantrcvmore(struct socket *so);
int sbwait(struct sockbuf *sb);
void sbwakeup(struct sockbuf *sb);
void sowakeup(struct socket *so, struct sockbuf *sb);
int soreserve(struct socket *so, unsigned long sndcc, unsigned long rcvcc);
int sbreserve(struct sockbuf *sb, unsigned long cc);
void sbrelease(struct sockbuf *sb);
void sbappend(struct sockbuf *sb, struct mbuf *m);
void sbappendrecord(struct sockbuf *sb, struct mbuf *m0);
int sbappendaddr(struct sockbuf *sb, struct sockaddr *asa, struct mbuf *m0, struct mbuf *rights0);
int sbappendrights(struct sockbuf *sb, struct mbuf *m0, struct mbuf *rights);
void sbcompress(struct sockbuf *sb, struct mbuf *m, struct mbuf *n);
void sbflush(struct sockbuf *sb);
void sbdrop(struct sockbuf *sb, int len);
void sbdroprecord(struct sockbuf *sb);

/* uipc_syscalls.c */
#ifdef LATTICE
void __asm __saveds _socket(register __d1 char *AP );
void __asm __saveds _inherit(register __d1 char *AP );
void __asm __saveds _networkclose(register __d1 char *AP );
void __asm __saveds _bind(register __d1 char *AP );
void __asm __saveds _listen(register __d1 char *AP );
void __asm __saveds _accept(register __d1 char *AP );
void __asm __saveds _connect(register __d1 char *AP );
void __asm __saveds _sendto(register __d1 char *AP );
void __asm __saveds _send(register __d1 char *AP );
void __asm __saveds _sendmsg(register __d1 char *AP );
void __asm __saveds _recvfrom(register __d1 char *AP );
void __asm __saveds _recv(register __d1 char *AP );
void __asm __saveds _recvmsg(register __d1 char *AP );
void __asm __saveds _shutdown(register __d1 char *AP );
void __asm __saveds _setsockopt(register __d1 char *AP );
void __asm __saveds _getsockopt(register __d1 char *AP );
void __asm __saveds _getsockname(register __d1 char *AP );
void __asm __saveds _getpeername(register __d1 char *AP );
void __asm __saveds _select(register __d1 char *AP );
void __asm __saveds _ioctl(register __d1 char *AP );
#else
void _socket(char *AP);
void _inherit(char *AP);
void _networkclose(char *AP);
void _bind(char *AP);
void _listen(char *AP);
void _accept(char *AP);
void _connect(char *AP);
void _sendto(char *AP);
void _send(char *AP);
void _sendmsg(char *AP);
void _recvfrom(char *AP);
void _recv(char *AP);
void _recvmsg(char *AP);
void _shutdown(char *AP);
void _setsockopt(char *AP);
void _getsockopt(char *AP);
void _getsockname(char *AP);
void _getpeername(char *AP);
void _select(char *AP);
void _ioctl(char *AP);
#endif

/* bcmp.asm */

int bcmp (void *p1, void *p2, long count);

/* bmov.asm */
// #define bcopy CopyMem

#ifdef LATTICE
void __asm bcopy (register __a0 void *src, register __a1 void *dest, register __d0 long count);
#else
void bcopy (void *src, void *dest, long count);
#endif

/* bset.asm */
#ifdef LATTICE
void __asm bzero (register __a0 void *start, register __d0 long count);
#else
void bzero (void *start, long count);
#endif

/* if_s2.c */
void s2attach(void);
void s2init(int x);
BOOL queueread(struct s2_inddev *dev, int type);
void s2poll(void);
int s2output(struct ifnet *ifp, struct mbuf *m, struct sockaddr *dst);
int s2ioctl(struct ifnet *ifp, int cmd, caddr_t data);
BOOL CTB(char *add, ULONG n, struct mbuf *to);
BOOL CFB(char *addr, ULONG n, struct mbuf *from);
BOOL AttemptExchange(struct Device *devbase, struct Unit *unit);
void EndExchange(struct Device *devbase, struct Unit *unit);
void ImitationSendIO(struct Message *ioreq);
void InitExchange(void);
void DeinitExchange(void);
void PollExchange(void);
char nibvert(char ichar);

/* configure.c */
void __asm ConfigureInet(register __a0 struct TagItem *taglist);

#ifdef AZTEC_C
#pragma amicall(InetBase, 0x1e, _socket(d1))
#pragma amicall(InetBase, 0x24, _bind(d1))
#pragma amicall(InetBase, 0x2a, _ioctl(d1))
#pragma amicall(InetBase, 0x30, _listen(d1))
#pragma amicall(InetBase, 0x36, _accept(d1))
#pragma amicall(InetBase, 0x3c, _connect(d1))
#pragma amicall(InetBase, 0x42, _sendto(d1))
#pragma amicall(InetBase, 0x48, _send(d1))
#pragma amicall(InetBase, 0x4e, _sendmsg(d1))
#pragma amicall(InetBase, 0x54, _recvfrom(d1))
#pragma amicall(InetBase, 0x5a, _recv(d1))
#pragma amicall(InetBase, 0x60, _recvmsg(d1))
#pragma amicall(InetBase, 0x66, _shutdown(d1))
#pragma amicall(InetBase, 0x6c, _setsockopt(d1))
#pragma amicall(InetBase, 0x72, _getsockopt(d1))
#pragma amicall(InetBase, 0x78, _getsockname(d1))
#pragma amicall(InetBase, 0x7e, _getpeername(d1))
#pragma amicall(InetBase, 0x84, _select(d1))
#pragma amicall(InetBase, 0x8a, _networkclose(d1))
#pragma amicall(InetBase, 0x90, _inherit(d1))
#endif

#ifdef LATTICE
#pragma libcall InetBase _socket         1e              101
#pragma libcall InetBase _bind           24              101
#pragma libcall InetBase _ioctl          2a              101
#pragma libcall InetBase _listen         30              101
#pragma libcall InetBase _accept         36              101
#pragma libcall InetBase _connect        3c              101
#pragma libcall InetBase _sendto         42              101
#pragma libcall InetBase _send           48              101
#pragma libcall InetBase _sendmsg        4e              101
#pragma libcall InetBase _recvfrom       54              101
#pragma libcall InetBase _recv           5a              101
#pragma libcall InetBase _recvmsg        60              101
#pragma libcall InetBase _shutdown       66              101
#pragma libcall InetBase _setsockopt     6c              101
#pragma libcall InetBase _getsockopt     72              101
#pragma libcall InetBase _getsockname    78              101
#pragma libcall InetBase _getpeername    7e              101
#pragma libcall InetBase _select         84              101
#pragma libcall InetBase _networkclose   8a              101
#pragma libcall InetBase _inherit        90              101
#endif
