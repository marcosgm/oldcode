#ifndef _NL_H_
#define _NL_H_

#include <asm/types.h>	
#include <netinet/ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>		//have fun w/ that uncommented mess of shiat
#include <sys/ioctl.h>	
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#ifndef ERR_LIB
/* from errr.c*/
#define ERR_LIB
int perr_quit(char *quitmsg)
{	perror(quitmsg);
	exit(1);}
int err_quit(char *quitmsg)
{	printf("Terminating: %s\n", quitmsg);
	exit(1);}
int err_warn(char *warnmsg)
{	printf("Warning: %s\n", warnmsg);
	return 0;}
int arg_quit(char *usage)
{	puts(usage);
	exit(0);}
/*end_of errc.*/
#endif

#define SEQ     6969    //need a tracking number for our message to kernel
#define OBS     512
#define CMASK     0xffffffff      //constant for ifinfomsg structure
#define ATBS	512	//max size of attribute buffer in rquest structure
#define SCOPE_LEN	64
#define TYPE_LEN	64
#define	INS_LEN		64
#define NL_BUFSIZE 	8192
#define NL_ARP_MINLEN 	20

//make sure this message has data in it
#define NL_GOTDATA(nlhdr, dlen) ( (nlhdr)->nlmsg_len >= sizeof(struct nlmsghdr) + dlen )

//routing table structure
struct rt_entry{
	u_int	destaddr;	//the destination address
	u_int	srcaddr;	//the optional preferred source address
	u_int	gateway;	//the gateway it's destined for
	char	iifname[IF_NAMESIZE];	//incoming interface
	char	oifname[IF_NAMESIZE];	//outgoing interface
	char	scope[SCOPE_LEN];	//scope(local, external)
	char	type[TYPE_LEN];		//type(unicast/broadcast/)
	char 	installedby[INS_LEN];	//kernel, admin, at boot
	struct rt_entry	*next;
};

//statistics for an interface, part of our structure
struct iface_stats{
	int	rbytes;	//recieved bytes
	int	rpacks;	//packets
	int	rerrs;	//errors
	int	rdrops;	//dropped
	int	rmulti;	//multicast
	
        int     sbytes;	//sent
        int     spacks;
        int     serrs;
        int     sdrops;
        int     smulti;
};	

//our interface info structure. we dynamically build a linked list of these to
//return to the user.  we provide a create function and free function for mem
struct iface_info{
	char	name[IF_NAMESIZE];
	u_int	index;
	u_int	flags;
	u_int	mtu;
	struct ether_addr hw_uni;
	struct ether_addr hw_brd;
	u_int	uni_addr;
	u_int	brd_addr;
	u_int	netmask;
	struct iface_stats	stats;
	struct iface_info	*next;
};

#define precv stats.rpacks
#define psent stats.spacks
#define brecv stats.rbytes
#define bsent stats.sbytes


//the mini interface info structure :D
struct iface_mini{
	char	name[IF_NAMESIZE];
	u_int	laddr;	//local
	u_int	daddr;	//dest in a PPP connection
	u_int	baddr;	//broadcast
	u_int	acast; 	//anycast
	struct ifa_cacheinfo	cache;	//have yet to actually see this returned :-|
};

//strings corresponding to defines from netlink.h rtnetlink.h
char    *rtypes[] = {
        "unspecified", "unicast", "local", "broadcast",
        "anycast", "multicast", "drop route", "unreachable",
        "prohibited by administrator", "not in this table", "NAT rule",
        "use external resolver" };

char    *rproto[] = {
        "unspecified", "ICMP redirects", "kernel",
        "booter", "admin" };
       
char    *rtattr[] = {
        "unspecified", "destination", "source", "incoming interface", "outgoing interface",
        "gateway", "priority", "preferred source", "metrics", "multipath", "protocol info",
        "flow", "cache info" };

//utility functions
//
//fill out a netlink header
inline void fillnl(struct nlmsghdr *nlmsghdr, int len, int type, int flags, int seq, pid_t pid)
{
	nlmsghdr->nlmsg_len = len;
	nlmsghdr->nlmsg_type = type;
	nlmsghdr->nlmsg_flags = flags;
	nlmsghdr->nlmsg_seq = seq;
	nlmsghdr->nlmsg_pid = pid;
}

//read a netlink reply in it's entirety
//responsibility of caller to make buf must at least NL_BUFSIZE 
inline int 
readnl(int sock, char *buf, int seq, pid_t pid)
{
	int	nread = 0, ntread = 0;
	u_char	flag = 0;
	char	*tmp = buf;
	struct nlmsghdr	*nlhdrp = NULL;
	
	//read the reply, check for multi-part reply
	//fill up buffer to NL_BUFSIZE bytes at most
	do{ 
	    	//we can't read more than NL_BUFSIZE bytes	
		if( (nread = read(sock, tmp, NL_BUFSIZE - ntread )) < 0)
			perr_quit("read");
		
		nlhdrp = (struct nlmsghdr *)tmp;
		
        	//FIRST ALWAYS: check to make sure the message structure is ok
        	//then check to make sure type isn't error type
		if( (NLMSG_OK(nlhdrp, nread) == 0) || (nlhdrp->nlmsg_type == NLMSG_ERROR) )
               		return -1;
		
		//if this is the last message then it contains no data
		//so we dont want to return it
		if(nlhdrp->nlmsg_type == NLMSG_DONE)
			flag = 1;
		else{
			tmp += nread;
			ntread += nread;
		}
		
		//in this case we never had a multipart message 
		if( (nlhdrp->nlmsg_flags & NLM_F_MULTI) == 0 )	
			flag = 1;
		
	}while(nlhdrp->nlmsg_seq != seq || nlhdrp->nlmsg_pid != pid || flag == 0);
                                
	return ntread;
}

#endif
