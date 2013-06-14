#include "2008ex_common.h"
#include <string.h>
#include <linux/if_arp.h>
#include <stdlib.h>

// buffer to hold the RTNETLINK  RTM_GETLINK request
/* struct ifinfomsg {
    unsigned char  ifi_family; /* AF_UNSPEC 
    unsigned short ifi_type;   /* Device type (see include <linux/if_arp.h>)
    int            ifi_index;  /* Interface index 
    unsigned int   ifi_flags;  /* Device flags  
    unsigned int   ifi_change; /* change mask 
};*/
typedef struct {
	struct nlmsghdr   nl;
	struct ifinfomsg  iimsg;
	char           	  buf[BUFSIZE];
} req_getlink_t;

// buffer to hold the RTNETLINK  RTM_GETADDR request
/*struct ifaddrmsg {
	unsigned char ifa_family;    /* Address type 
	unsigned char ifa_prefixlen; /* Prefixlength of address 
	unsigned char ifa_flags;     /* Address flags 
	unsigned char ifa_scope;     /* Address scope 
	int           ifa_index;     /* Interface index 
};
*/
typedef struct {
	struct nlmsghdr   nl;
	struct ifaddrmsg  iamsg;
	char           	  buf[BUFSIZE];
} req_getaddr_t;


char *get_interface_ip (int interface_index);

void form_request_getlink(req_getlink_t *req)
{
	assert (req!=NULL);
  // initialize the request buffer
	bzero(req, sizeof(req_getlink_t));

  // set the NETLINK header
	req->nl.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req->nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req->nl.nlmsg_type = RTM_GETLINK;
}


void read_reply_getlink(char *buf, int nl_length)
{
	char rtadata[128];
	struct nlmsghdr *nlp = NULL;
	struct ifinfomsg *iip=NULL;
	int ii_length;
	struct rtattr *rtap=NULL;
	int i=0;
	unsigned char *chr_buf=NULL;
	char *ip = NULL; //will contain IP address of each interface
	
	// outer loop: loops thru all the NETLINK
	// headers that also include the route entry
	// header 
	nlp = (struct nlmsghdr *) buf;
	for(; NLMSG_OK(nlp, nl_length); nlp=NLMSG_NEXT(nlp, nl_length))
	{
		iip = (struct ifinfomsg *) NLMSG_DATA(nlp);
		
		//unsigned short ifi_type;   /* Device type 
		//int            ifi_index;  /* Interface index 
		printf ("index %d ", iip->ifi_index);
		switch (iip->ifi_type)
		{ //see include <linux/if_arp.h>
			case ARPHRD_ETHER:
				printf ("type Ethernet =>");
				break;
			case ARPHRD_LOOPBACK:
				printf ("type Loopback =>");
				break;
			default:
				printf ("type %d =>", iip->ifi_type);
				break;
		}
		//ARPHRD_ETHER 
		
    	// init all the strings
		bzero(rtadata, sizeof(rtadata));

	// inner loop: loop thru all the attributes of
    	// one route entry 
		rtap = (struct rtattr *) IFLA_RTA(iip);
		ii_length = IFLA_PAYLOAD(nlp);
		for(; RTA_OK(rtap, ii_length); rtap=RTA_NEXT(rtap,ii_length))
		{
			switch(rtap->rta_type) 
			{
		        // hardware address  interface L2 address
				case IFLA_ADDRESS:
					chr_buf = (char*)RTA_DATA(rtap);
					printf("MAC address = %02x",chr_buf[0]);
					for (i=1; i<6; ++i) 
					{
						printf(":%02x",chr_buf[i]);
					}
					break;
        		// ascii string  Device name.
				case IFLA_IFNAME:
					printf (" name %s, ", RTA_DATA(rtap));
					break;
			// int  Link type.
				case IFLA_LINK:
					printf ("link type %d ", *((int *)RTA_DATA(rtap)));
					break;

				default:
					break;
			}
		}
		ip = get_interface_ip (iip->ifi_index);
		if (ip!=NULL)	
		{
			printf (" IP %s\n", ip);
			free (ip);
		}
		else	
			printf ("\n", ip);
	}
/*
	rta_type  value type  description
	IFLA_UNSPEC  -  unspecified.
	IFLA_ADDRESS  hardware address  interface L2 address
	IFLA_BROADCAST  hardware address  L2 broadcast address.
	IFLA_IFNAME  asciiz string  Device name.
	IFLA_MTU  unsigned int  MTU of the device.
	IFLA_LINK  int  Link type.
	IFLA_QDISC  asciiz string  Queueing discipline.
	IFLA_STATS   see below   Interface Statistics.
*/
}

void form_request_getaddr(req_getaddr_t *req, int interface_index)
{
	assert (req!=NULL);
  // initialize the request buffer
	bzero(req, sizeof(req_getaddr_t));

  // set the NETLINK header
	req->nl.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	req->nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req->nl.nlmsg_type = RTM_GETADDR;
	
	//ask for the Address of only this interface index
	req->iamsg.ifa_family = AF_INET;
	req->iamsg.ifa_index = interface_index;
}

void read_reply_getaddr(char *buf, int nl_length, char *ip_str, int interface_index)
{
	char rtadata[128];
	struct nlmsghdr *nlp = NULL;
	struct ifaddrmsg *iap=NULL;
	int ia_length;
	struct rtattr *rtap=NULL;
	int i=0;
	unsigned char *chr_buf=NULL;
	
	//assert (ip_str != NULL);

	// outer loop: loops thru all the NETLINK
	// headers that also include the route entry
	// header 
	nlp = (struct nlmsghdr *) buf;
	for(; NLMSG_OK(nlp, nl_length); nlp=NLMSG_NEXT(nlp, nl_length))
	{
		iap = (struct ifaddrmsg *) NLMSG_DATA(nlp);
			
		//ONLY PROCESS IFADDRESS OF THE INTERFACE INDEX WE'RE LOOKING FOR
		if (iap->ifa_index != interface_index) continue;
	
    	// init all the strings
		bzero(rtadata, sizeof(rtadata));

	// inner loop: loop thru all the attributes of
    	// one route entry 
		rtap = (struct rtattr *) IFA_RTA(iap);
		ia_length = IFA_PAYLOAD(nlp);
		for(; RTA_OK(rtap, ia_length); rtap=RTA_NEXT(rtap,ia_length))
		{
			switch(rtap->rta_type) 
			{
		        // hardware address  interface L2 address
				case IFA_ADDRESS:
					inet_ntop(AF_INET, RTA_DATA(rtap), ip_str, 24);
					break;
				default:
					break;
			}
		}
	}
}
/**
returns string with interface IP. This string must be freed once used, as it is malloc'ed inside
*/
char *get_interface_ip (int interface_index)
{
	int sock_fd = -1;
	req_getaddr_t req;
	int rcvd_length = 0;
	char *ip_str = NULL;
	char buf[BUFSIZE];	// buffer to hold the RTNETLINK reply(ies)
	
	ip_str = malloc (20);
	assert (ip_str != NULL);
	memset (ip_str, 0, 20);
	
	open_netlink_socket(&sock_fd);
	form_request_getaddr(&req, interface_index);
	send_request(sock_fd, &(req.nl));
	rcvd_length = recv_reply(sock_fd, buf);
	
	read_reply_getaddr(buf, rcvd_length, ip_str, interface_index); //ip_str will contain interface IP

  	// close socket
	close(sock_fd);
	
	return ip_str;
}

int main(int argc, char *argv[])
{
	int sock_fd = -1;
	req_getlink_t req;
	int rcvd_length = 0;
	
	// buffer to hold the RTNETLINK reply(ies)
	char buf[BUFSIZE];
	
	open_netlink_socket(&sock_fd);
	// sub functions to create RTNETLINK message,
	// send over socket, reveive reply & process
	// message
	form_request_getlink(&req);
	send_request(sock_fd, &(req.nl));
	rcvd_length = recv_reply(sock_fd, buf);
	read_reply_getlink(buf, rcvd_length);

  	// close socket
	close(sock_fd);
}

