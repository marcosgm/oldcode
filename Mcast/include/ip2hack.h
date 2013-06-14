/*START OF A BIG HACK, es un truco para que IProute2 funcione!!!*/
#include "ip/rt_names.h"
#include "ip/utils.h"

static struct
{
	int tb;
	int flushed;
	char *flushb;
	int flushp;
	int flushe;
	struct rtnl_handle *rth;
	int protocol, protocolmask;
	int scope, scopemask;
	int type, typemask;
	int tos, tosmask;
	int iif, iifmask;
	int oif, oifmask;
	int realm, realmmask;
	inet_prefix rprefsrc;
	inet_prefix rvia;
	inet_prefix rdst;
	inet_prefix mdst;
	inet_prefix rsrc;
	inet_prefix msrc;
} filter;
static int flush_update(void)
{
	if (rtnl_send(filter.rth, filter.flushb, filter.flushp) < 0) {
		perror("Failed to send flush request\n");
		return -1;
	}
	filter.flushp = 0;
	return 0;
}
void iproute_reset_filter()
{
        memset(&filter, 0, sizeof(filter));
        filter.mdst.bitlen = -1;
        filter.msrc.bitlen = -1;
}
int print_route(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg);
int rtnl_rtntype_a2n(int *id, char *arg);
char *rtnl_rtntype_n2a(int id, char *buf, int len);
int resolve_hosts;
int show_stats=3;
int iproute_get(int argc, char **argv);
/*END OF A BIG HACK*/
