// author : Asanga Udugama (adu@comnets.uni-bremen.de)
// date : 2005-09-10
// compile & link : gcc -o get_routing_table get_routing_table.c 


//Netlink API: http://people.suug.ch/~tgr/libnl/doc/group__nl.html
#include <stdio.h>
#include <strings.h>

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <assert.h>

#define BUFSIZE 8192




int send_request(int fd, struct nlmsghdr *nl);
int recv_reply(int fd, char *buf);
void open_netlink_socket (int *fd);

/**
fd must be a valid (bound) netlink socket 
nl must be a valid (created, fulfilled) pointer to the message header of a valid request, including total message length, wich will always be greater than nlmsghdr size.
*/
int send_request(int fd, struct nlmsghdr *nl)
{
	struct msghdr msg;
	struct iovec iov;
	struct sockaddr_nl pa;
	int rtn = 0; //return value
	
	assert (nl!=NULL);

  // create the remote address
  // to communicate
	bzero(&pa, sizeof(struct sockaddr_nl));
	pa.nl_family = AF_NETLINK;

  // initialize & create the struct msghdr supplied
  // to the sendmsg() function
	bzero(&msg, sizeof(struct msghdr));
	msg.msg_name = (void *) &pa;
	msg.msg_namelen = sizeof(struct sockaddr_nl);

  // place the pointer & size of the RTNETLINK
  // message in the struct msghdr 
	iov.iov_base = (void *) nl; //(void *) &req.nl;
	iov.iov_len = nl->nlmsg_len;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

  // send the RTNETLINK message to kernel
	rtn = sendmsg(fd, &msg, 0);
	return rtn;
}

int recv_reply(int fd, char *buf)
{
	char *p=NULL;
	struct nlmsghdr *nl_msg_p=NULL;
	int bytes_rcvd = 0;
	int total_bytes_rcvd = 0;

  	// initialize the socket read buffer
	bzero(buf, BUFSIZE);

	//p points to buf[0];
	p = buf;

	// read from the socket until the NLMSG_DONE is
	// returned in the type of the RTNETLINK message
	// or if it was a monitoring socket
	while(p < (buf+BUFSIZE)) //while(1)
	{
		bytes_rcvd = recv(fd, p, BUFSIZE - total_bytes_rcvd, 0); //p points to somewhere inside buf[];

		nl_msg_p = (struct nlmsghdr *) p;

		if(nl_msg_p->nlmsg_type == NLMSG_DONE) 
			break;

		// increment the buffer pointer to place
		// next message
		p += bytes_rcvd;

		// increment the total size by the size of 
		// the last received message
		total_bytes_rcvd += bytes_rcvd;

		/*if((la.nl_groups & RTMGRP_IPV4_ROUTE) == RTMGRP_IPV4_ROUTE)
		break;*/
	}
	return total_bytes_rcvd;
}

void open_netlink_socket (int *fd)
{
	struct sockaddr_nl la;
	// open socket
	*fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

	// setup local address & bind using
	// this address
	bzero(&la, sizeof(struct sockaddr_nl));
	la.nl_family = AF_NETLINK;
	la.nl_pid = getpid();
	bind(*fd, (struct sockaddr*) &la, sizeof(struct sockaddr_nl));
}


