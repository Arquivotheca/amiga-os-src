#ifndef LASTLOG_H
#define LASTLOG_H

struct lastlog {
	char	ll_name[32];
	char	ll_host[32];
	char	ll_line[16];
	u_long	ll_time;
};

#define LASTFILE	"inet:log/lastlog"

#endif
