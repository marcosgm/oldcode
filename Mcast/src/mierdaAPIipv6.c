#define MAXBUFSIZE 65536 // Max UDP Packet size is 64 Kbyte
	int sock, status, sockopt;
   char buffer[MAXBUFSIZE];
   struct sockaddr_in6 destino;
   struct ip_mreq imreq;

   
   memset(&destino, 0, sizeof(struct sockaddr_in6));
   memset(&imreq, 0, sizeof(struct ip_mreq));

	sock = socket(AF_INET6,SOCK_DGRAM,IPPROTO_IP);
	if (sock < 0)
  {
	  printf("Error en socket\n");
	  return -1;
  }
  /*struct sockaddr_in6 {
    u_int16_t       sin6_family; 
    u_int16_t       sin6_port; 
    u_int32_t       sin6_flowinfo;
    struct in6_addr sin6_addr; 
    u_int32_t   sin6_scope_id; 
};

struct in6_addr {
    unsigned char   s6_addr[16];
};*/
  
	//Tengo que hacer un BIND a la IP-puerto q quiero
	  inet_pton(AF_INET6,"ff02::9",&destino.sin6_addr);  
	destino.sin6_family = AF_INET6;
	 destino.sin6_port = 6969;
	if (bind(sock,(struct sockaddr *)&destino,sizeof(destino)))
		return (-1);
  
	strncpy(buffer,"VAMOS PALLA",12);
  sockopt=1;
	status = setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, &sockopt, sizeof(sockopt));
  printf("\nstatus %d, si es 0 es exitoso\n",status);
  struct cmsghdr ancilliary;
  ancilliary.cmsg_len=32;
  ancilliary.cmsg_level=IPPROTO_IPV6;
  //printf("Enviados %d\n",sendmsg(sock,(struct msghdr *) buffer,0));
  printf("Enviados %d\n",send(sock,buffer,strlen(buffer),0));
  
   // JOIN multicast group on default interface
   status = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
              (const void *)&imreq, sizeof(struct ip_mreq));

   // shutdown socket
   shutdown(sock, 2);
   // close socket
   close(sock);

			  
			  
/****************************************************/
			  void *test (void *arg)
{
	IFS_interfaz_t *ifaz = (IFS_interfaz_t *) arg;
	struct sockaddr_in6 mcastdir;
	int sockfd = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP);
	int on=1;
	setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	mcastdir.sin6_family=AF_INET6;
	mcastdir.sin6_port=5001;
	mcastdir.sin6_flowinfo=0;
	//inet_pton(AF_INET6, "FF00::31", &mcastdir.sin6_addr);
	//LA API ME DA ESTO:
	mcastdir.sin6_addr=in6addr_any; /*o esto*/
	//struct in6_addr anyaddr = IN6ADDR_ANY_INIT;
	//char buffero[60];
	//inet_ntop(AF_INET6, &mcastdir.sin6_addr, buffero, 60);
	
	printf ("socket es %d y Bind m da %d\n",sockfd, bind(sockfd, (struct sockaddr *) &mcastdir, sizeof(struct sockaddr_in6)));
	perror("Me ves?");
//linux/in6.h
//struct sockaddr_in6 {
//        unsigned short int      sin6_family;    /* AF_INET6 */
//        __u16                   sin6_port;      /* Transport layer port # */
//        __u32                   sin6_flowinfo;  /* IPv6 flow information */
//        struct in6_addr         sin6_addr;      /* IPv6 address */
//        __u32                   sin6_scope_id;  /* scope id (new in RFC2553) */};
// struct ipv6_mreq {
//        /* IPv6 multicast address of group */
//        struct in6_addr ipv6mr_multiaddr;
//        /* local IPv6 address of interface */
//        int             ipv6mr_ifindex;};

/////////////////////////////////
//struct sockaddr_in6
//  {    __SOCKADDR_COMMON (sin6_);
//    in_port_t sin6_port;        /* Transport layer port # */
//    uint32_t sin6_flowinfo;     /* IPv6 flow information */
//    struct in6_addr sin6_addr;  /* IPv6 address */
//    uint32_t sin6_scope_id;     /* IPv6 scope-id */  };
//struct ipv6_mreq  {
//    /* IPv6 multicast address of group */
//    struct in6_addr ipv6mr_multiaddr;
//     /* local interface */
//    unsigned int ipv6mr_interface;  };
	
	// multicast request
	inet_pton(AF_INET6, "FF00::11", &mcastdir.sin6_addr);
	struct ipv6_mreq mreq;
	mreq.ipv6mr_multiaddr = mcastdir.sin6_addr;
	mreq.ipv6mr_interface = 4;
	//segun linux es "mreq.ipv6mr_ifindex = 4;"
   int ret;
	ret=setsockopt(sockfd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
	printf("setsockopt es %d\n",ret);
	if (ret<0)
		perror("Setsock!\n");
	
	char cache[] = "PEPEeeeeeee!!";
	
	/*ret=connect(sockfd,(struct sockaddr *) &mcastdir, sizeof (mcastdir));
	printf ("connect es %d\n",ret);
	if (ret<0)
		perror("Connect!\n");
	*/
	ret=sendto(sockfd,cache,strlen(cache),0,(struct sockaddr *) &mcastdir, sizeof (mcastdir));
	printf ("enviados %d\n",ret);
	if (ret<0)
		perror("Sendto!\n");
	pthread_detach(pthread_self());
	return NULL;
}
