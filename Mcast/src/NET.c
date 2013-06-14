#include "defs.h"

int NET_libnet_inject(struct ether_header *eth, struct ip6_hdr *ip6, u_char *data, int data_size , u_char *dev)
{
	libnet_t *l=NULL;
	char errbuf[LIBNET_ERRBUF_SIZE];
	int i=0;
	//dev es un char[10] asi que no me preocupa controlar su longitud
	l=libnet_init (LIBNET_LINK, dev,errbuf);
	if (l==NULL)
	{
		printf("\nlibnet_init breaks, %s", errbuf); exit (EXIT_FAILURE);
	}
	libnet_ptag_t ip6hdr;
	struct libnet_in6_addr *src = (struct libnet_in6_addr *) &(ip6->ip6_src);
	struct libnet_in6_addr *dst = (struct libnet_in6_addr *) &(ip6->ip6_dst);
//A estas funciones se les pasa parametros en HOST byte order, pero yo cogí los datos en NET order
//TODO hacer #define ip6_flow ip6->ip6_ctlun.ip6_un1.ip6_un1_flow como en archivo Ipv6.h de sendip-2.5

//en ip6_flow el ntohs() es necesario pq en PCAP_handle_ipv6 no puedo corregirlo	
	ip6hdr = libnet_build_ipv6(	\
		ip6->ip6_vfc,		ntohs(ip6->ip6_flow),\
		ip6->ip6_plen,		ip6->ip6_nxt,\
		ip6->ip6_hlim,			*src,	*dst,	data, data_size, l,0);
	
	libnet_autobuild_ethernet(eth->ether_dhost,ntohs(eth->ether_type),l);
	i=libnet_write(l);

	#ifdef IMPRIMELOTODO
	printf ("Device es %s, ethertype es %x\n", dev, ntohs(eth->ether_type));
	printf("write da %d\n",i);
#endif
	if (i<0) printf("%s",libnet_geterror(l));
	libnet_destroy(l);
	
	return (i);
}

int NET_get_MAC_address (u_char *MAC, u_char *dev)
{	//obtenido de internet
	int skfd=0;
	struct ifreq ifr;
	strcpy(ifr.ifr_name,dev);
	skfd = socket(AF_INET,SOCK_DGRAM,0);
	if(skfd<0)
	{	printf("Can't open socket!\n"); return -1;	
	}
	if(ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0)
	{	printf("Can't read local mac address!\n"); return -1;
	}
	int a;
	memcpy (MAC, &ifr.ifr_hwaddr.sa_data, 6);	
	close(skfd);
	return 0;
}



static int sock;
static struct icmp6_filter filter;
static struct addrinfo *dst = NULL, *mc = NULL;
static int quit = 0;
static char *nick = NULL;
static int use_multicast = 0;
static int only_send = 0;
static char *msg = NULL;
int NET_icmp6_init_sock(void)
{
    int fd;
    int on = 1;

    if ((fd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)) < 0)
    {
        perror("socket");
        exit(-1);
    }

    ICMP6_FILTER_SETBLOCKALL(&filter);
	ICMP6_FILTER_SETPASS(ICMP6_ECHO_REPLY, &filter);

	if (setsockopt(fd, IPPROTO_ICMPV6, ICMP6_FILTER, &filter, sizeof(filter)) < 0)
    {
        perror("setsockopt ICMP6_FILTER");
        exit(-1);
    }

    if (setsockopt(fd, IPPROTO_IPV6, IPV6_DSTOPTS, &on, sizeof(on)) < 0)
    {
        perror("setsockopt IPV6_DSTOPTS");
        exit(-1);
    }

    return fd;
}

struct cmsghdr *
NET_ipv6_make_exthdr(const char *msg, int *mclen)
{
    struct cmsghdr *chdr;
    int len, mlen = strlen(msg);

    len = inet6_option_space(mlen);

    if (!(chdr = (struct cmsghdr *) calloc(1, len)))
    {
        fprintf(stderr, "Can't allocate memory for cmsghdr");
        exit(-1);
    }

    //if (inet6_option_init((void *) chdr, &chdr, IPV6_DSTOPTS) < 0)
	if (inet6_option_init((void *) chdr, &chdr, IPV6_HOPOPTS) < 0)
    {
        perror("inet6_option_init");
        exit(-1);
    }

    {
        char buf[mlen + 2];

        buf[0] = 0x00 | 0x17; /* skip this option & type == 0x17 */
        buf[1] = mlen;

        memcpy(&buf[2], msg, strlen(msg));

        if (inet6_option_append(chdr, buf, 1, 0) < 0)
        {
            perror("inet6_option_append");
            exit(-1);
        }
    }

    *mclen = chdr->cmsg_len;

    return chdr;
}

void
NET_icmp6_send_msg(const char *msg, struct sockaddr_in6 *addr)
{
    struct msghdr mhdr;
    struct iovec iov[1];
    struct icmp6_hdr *hdr = calloc(1, sizeof(struct icmp6_hdr) + 42);

    memset((void *) &mhdr, 0, sizeof(mhdr));
    hdr->icmp6_type = 129;
    hdr->icmp6_code = 0;

    mhdr.msg_name = (caddr_t) addr;
    mhdr.msg_namelen = sizeof(struct sockaddr_in6);

    mhdr.msg_iov = iov;
    mhdr.msg_iovlen = 1;

    iov[0].iov_base = hdr;
    iov[0].iov_len = sizeof(struct icmp6_hdr) + 42;

    mhdr.msg_control = (caddr_t) NET_ipv6_make_exthdr(msg, &mhdr.msg_controllen);

    if (sendmsg(sock, &mhdr, 0) < 0)
    {
        perror("sendmsg");
        exit(-1);
    }

    if (!use_multicast)
        printf("%s", msg);
}

void
NET_ipv6_dump(void)
{
    struct msghdr mhdr = {0};
    struct cmsghdr *cmsg;
    char buf[CMSG_SPACE(1024)];
    char *text, *p;

    mhdr.msg_control = buf;
    mhdr.msg_controllen = sizeof(buf);

    if (recvmsg(sock, &mhdr, 0) < 0)
    {   perror("recvmsg");
        exit(-1);
    }

    if (mhdr.msg_controllen > 0)
    {
        cmsg = CMSG_FIRSTHDR(&mhdr);
        text = CMSG_DATA(cmsg) + 4;

        for (p = text; *p != '\n'; p++);
        *p = '\0';

        printf("%s\n", text);
    }
}

void
NET_ipv6_join_mcast_grp(struct sockaddr_in6 *addr)
{
    struct ipv6_mreq mreq6;

    memcpy(&mreq6.ipv6mr_multiaddr, &addr->sin6_addr, sizeof(struct in6_addr));
    mreq6.ipv6mr_interface = 0;

    if (setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &mreq6, sizeof(mreq6)) < 0)
    {
        perror("setsockopt IPV6_ADD_MEMBERSHIP");
        exit(-1);
    }
}

void
NET_ipv6_at_exit(void)
{
    if (mc && sock > 0)
    {
        struct ipv6_mreq mreq6;

        memcpy(&mreq6.ipv6mr_multiaddr,
            &((struct sockaddr_in6 *) mc->ai_addr)->sin6_addr,
            sizeof(struct in6_addr));
        
        mreq6.ipv6mr_interface = 0;

        if (setsockopt(sock, IPPROTO_IPV6, IPV6_LEAVE_GROUP,
            &mreq6, sizeof(mreq6)) < 0)
            perror("setsockopt(IPV6_LEAVE_GROUP)");
    }

    if (sock > 0)
        close(sock);

    if (dst)
        freeaddrinfo(dst);

    if (mc)
        freeaddrinfo(mc);
}
