#include "2008ex_common.h"

// buffer to hold the RTNETLINK  RTM_GETROUTE request
typedef struct {
	struct nlmsghdr nl;
	struct rtmsg    rt;
	char            buf[BUFSIZE];
} req_getroute_t;


void form_request_getroute(req_getroute_t *req)
{
	assert (req!=NULL);
  // initialize the request buffer
	bzero(req, sizeof(req_getroute_t));

  // set the NETLINK header
	req->nl.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	req->nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req->nl.nlmsg_type = RTM_GETROUTE;

  // set the routing message header
	req->rt.rtm_family = AF_INET;
	req->rt.rtm_table = RT_TABLE_MAIN;
}


void read_reply(char *buf, int nl_length)
{
	// string to hold content of the route 
	// table (i.e. one entry) 
	char dsts[24], gws[24], ifs[16], ms[24];
	int metric=-1;
	int priority=-1;
	struct nlmsghdr *nlp = NULL;
	struct rtmsg *rtp=NULL;
	int rt_length;
	struct rtattr *rtap=NULL;
	
	// outer loop: loops thru all the NETLINK
	// headers that also include the route entry
	// header 
	nlp = (struct nlmsghdr *) buf;
	for(; NLMSG_OK(nlp, nl_length); nlp=NLMSG_NEXT(nlp, nl_length))
	{

    		// get route entry header
		rtp = (struct rtmsg *) NLMSG_DATA(nlp);

		// we are only concerned about the
		// main route table
		if(rtp->rtm_table != RT_TABLE_MAIN)
			continue;

    // init all the strings
		bzero(dsts, sizeof(dsts));
		bzero(gws, sizeof(gws));
		bzero(ifs, sizeof(ifs));
		bzero(ms, sizeof(ms));
		metric=-1;
		priority=-1;

    // inner loop: loop thru all the attributes of
    // one route entry 
		rtap = (struct rtattr *) RTM_RTA(rtp);
		rt_length = RTM_PAYLOAD(nlp);
		for(; RTA_OK(rtap, rt_length); rtap=RTA_NEXT(rtap,rt_length))
		{
			switch(rtap->rta_type) 
			{
		        // destination IPv4 address
				case RTA_DST:
					inet_ntop(AF_INET, RTA_DATA(rtap), 
							dsts, 24);
					break;
        		// next hop IPv4 address
				case RTA_GATEWAY:
					inet_ntop(AF_INET, RTA_DATA(rtap), 
							gws, 24);
					break;
			//Route metrics
				case RTA_METRICS:
					metric = *((int *) RTA_DATA(rtap));
					break;
			//Route priority
				case RTA_PRIORITY:
					priority = *((int *) RTA_DATA(rtap));
					break;
        		// unique ID associated with the network interface
				case RTA_OIF:
					sprintf(ifs, "%d", 
							*((int *) RTA_DATA(rtap)));
				default:
					break;
			}
		}
		sprintf(ms, "%d", rtp->rtm_dst_len);

		printf("Network %s/%s gw %s, interface %s, metric %d, priority %d\n", 
		       dsts, ms, gws, ifs, metric, priority);
	}
}



int main(int argc, char *argv[])
{
	int sock_fd = -1;
	req_getroute_t req;
	int rcvd_length = 0;
	
	// buffer to hold the RTNETLINK reply(ies)
	char buf[BUFSIZE];
	
	open_netlink_socket(&sock_fd);

	// sub functions to create RTNETLINK message,
	// send over socket, reveive reply & process
	// message
	form_request_getroute(&req);
	send_request(sock_fd, &(req.nl));
	rcvd_length = recv_reply(sock_fd, buf);
	read_reply(buf, rcvd_length);

  	// close socket
	close(sock_fd);
}

