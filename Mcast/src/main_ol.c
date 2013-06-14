/* Created by Anjuta version 1.2.2 */
/*	This file will not be overwritten */
//http://www.faqs.org/rfcs/rfc3493.html
//http://www.faqs.org/rfcs/rfc3542.html
#include "defs.h"
#include <linux/in_route.h>
//#include <linux/mroute.h>
//LO DEL k_req_incoming NO FUNCIONA, OLVIDATE DE IOCTL para obtener rutas
#include "ip2hack.h"
#include <stdlib.h>
/*#ifndef RTAX_RTTVAR
#define RTAX_RTTVAR RTAX_HOPS
#endif
*/
#include <pcap.h>
void *start_daemon (void *arg);

RT_t todas_rutas;	
IFS_t todas_ifaces;
static int pkt_counter=0;

int main()
{
	IFS_t sitelocal;
	IFS_t linklocal;
//	RT_t rutas_de_ifaz;	
//ESTO DE AQUI ABAJO ES LO QUE DICE LA API QUE HAY QUE HACER, PERO NO VA!!
//	struct if_nameindex *interfaces;
//	int ret;	
//	interfaces = if_nameindex();/*esto es como hacer un malloc pero con valores correctos*/
//	for (ret=0;interfaces[ret].if_index!=0 && interfaces[ret].if_name!=NULL ;ret++)
//		printf ("ID %d y nombre %s\n", interfaces[ret].if_index, interfaces[ret].if_name);
//	if_freenameindex(interfaces);
	IFS_parsea_from_proc(&todas_ifaces);
	printf("Fin del parseo de /proc/net/if_inet6\n");
	IFS_sacar_solo_sitelocal(&sitelocal,&todas_ifaces);
	IFS_sacar_solo_linklocal(&linklocal,&todas_ifaces);
	printf("Hay %d sitelocals\n",sitelocal.cantidad);
	printf("Hay %d linklocals\n",linklocal.cantidad);
	
	RT_parsea_from_proc(&todas_rutas); //ojo pq se resetea a 0 el &todas_rutas

	char dir_route[] = "fec0:7::4:3:1/64";
	char *vector[1];
	vector[0]=dir_route;
	printf("IProute devuelve %d\n",iproute_get(1, vector));
	
	char dir_route2[] = "fec0:7::4:3:1";
	char ifaz[20];
	char ipdir[90];
	struct in6_addr ip;
	int a;
	memset (&ip,0,sizeof (struct in6_addr));
	//LA FUNCION inet_pton NO ACEPTA EL /64, solo vale la IPv6
	if (!inet_pton(AF_INET6,dir_route2,&ip))
	{
		printf("Error convirtiendo la direccion IPv6 a string");
		return 0;
	}
/* para probar estas funciones
	RT_t rutas_de_ifaz;	
	RT_filtra_por_ifaz(&rutas_de_ifaz, &todas_rutas, "eth1");
	printf("\nRutas filtradas:\n");
	RT_imprime_rutas(&rutas_de_ifaz);
*/
	a=RT_busca_ifaz_uplink(ifaz,&todas_rutas, &ip);
	if (a!=0)
	{
		printf("Error al buscar ruta en lista de rutas!");
		return 0;
	}
	inet_ntop(AF_INET6, &ip, ipdir, sizeof (ip));
	printf("Busco ruta hasta %s, resultado %d ifaz %s\n", ipdir, a, ifaz);
	
	/*Comienzo pruebas de threads*/
	/*int pthread_create(pthread_t *tid, const pthread_attr_t *tattr,
void*(*start_routine)(void *), void *arg);*/

//	pthread_attr_t tattr;
	
	int i,j;
	pthread_t thread_number[MAX_IFACES];
	for (i=0; i<sitelocal.cantidad; i++)
	{
		#ifdef IMPRIMELOTODO
			u_char buf[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &(sitelocal.interfaz[i].direccion), buf, INET6_ADDRSTRLEN);
			printf("Esta direccion es %s\n",buf);
		#endif
		if (pthread_create (&thread_number[i], NULL, start_daemon, &sitelocal.interfaz[i])<0)
			perror("Threadin!");
		printf("%d:Creado thread %ld\n", i,thread_number[i]);
	}
	
	for (j=0; j<i; j++)
	{	
		pthread_join (thread_number[j],NULL); //me da igual su estado de finalizacion
		printf("#%d:Destruyo thread %ld\n", j, thread_number[j]);
	}
	return 0;
}

u_int16_t handle_udp
        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet);
u_int16_t handle_icmp6
        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet);
u_int16_t handle_ipv6
        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet);
u_int16_t handle_ethernet
        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet);

void my_callback(u_char *args,const struct pcap_pkthdr* pkthdr,const u_char*  packet)
{
    u_int16_t type = handle_ethernet(args,pkthdr,packet);
	int ret;
//packet tiene ETH + IPV6 + EXTHDR + DATA
	pkt_counter ++;	
	if(type == ETHERTYPE_IPV6)
    {
		fflush(stdout);
		fprintf(stdout,"Paquetes recibidos %d\n", pkt_counter);
		packet += sizeof(struct ether_header); //14 bytes de eth_header
//packet tiene IPV6 + EXTHDR + DATA
		ret = handle_ipv6 (args, pkthdr, packet);
		packet += sizeof (struct ip6_hdr); //40 bytes
		printf("Retorno %d. hopbyhop es %d\n",ret, IPPROTO_HOPOPTS); //IPV6_HOPOPTS es 3 en vez de 0!!!!
//si ret != IPPROTO_HOPOPTS es que packet tiene SOLO DATA pq las otras opts las ignoramos
		char next_h;
		char len;
		if (ret == IPPROTO_HOPOPTS)
		{	next_h = *(packet); //campo ip6h_nxt
			len = *(packet+1); //campo ip6h_len
			printf("La hopbyhop opt.header es %x,%x, %s\n", next_h, len, packet +2);
			packet += len;
		}
		else if (ret == IPPROTO_DSTOPTS)
		{	next_h = *(packet); //campo ip6d_nxt
			len = *(packet+1); //campo ip6d_len
			printf("La destination opt.header es %x,%x, %s\n",  next_h, len, packet +2);
			packet += len;
		}
		else if (!(ret == IPPROTO_UDP || ret == IPPROTO_ICMPV6))  //si no es UDP ni ICMP6
		{	printf ("Cabecera o protocolo desconocido");
			exit (-1);
		}
	
//packet tiene DATA
		//Compruebo que packet apunte a UDP o a ICMP6
		if (next_h == IPPROTO_UDP)
			ret = handle_udp (args,pkthdr,packet);
		else if (next_h == IPPROTO_ICMPV6)
			ret = handle_icmp6 (args,pkthdr,packet);
		else
		{	printf ("Cabecera o protocolo desconocido");
			exit (-1);
		}	
	} //if IPV6
	else
	{
		printf ("Protocolo no IPV6");
	}
}
u_int16_t handle_udp
        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet)
		{return 1;}
u_int16_t handle_icmp6
        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet)
		{return 1;}


u_int16_t handle_ethernet
        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet)
{
    struct ether_header *eptr;  /* net/ethernet.h */
    eptr = (struct ether_header *) packet;
/*struct ether_header{  u_int8_t  ether_dhost[ETH_ALEN]; //ALEN es 6
						   u_int8_t  ether_shost[ETH_ALEN];  
						   u_int16_t ether_type;  } __attribute__ ((__packed__)); */
#ifdef IMPRIMELOTODO
    fprintf(stdout,"ethernet header source: %s",ether_ntoa((struct ether_addr *)eptr->ether_shost));
    fprintf(stdout," destination: %s ",ether_ntoa((struct ether_addr *)eptr->ether_dhost));
#endif
    /* check to see if we have an ip packet */
    if (ntohs (eptr->ether_type) == ETHERTYPE_IPV6) //DEFINIDO EN defs.h, fallo de linux
    {		printf("(IPV6)");
	}else {
        printf("(?)");
        exit(1);
    }
    printf ("\n");
    return ntohs (eptr->ether_type);
}
u_int16_t handle_ipv6
        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet)
{	/*struct ip6_hdr  {    //PESA 40 BYTES EN TOTAL
	union  { 	struct ip6_hdrctl          {
						uint32_t ip6_un1_flow;   // 4 bits version, 8 bits TC,20 bits flow-ID 
						uint16_t ip6_un1_plen;   // payload length 
						uint8_t  ip6_un1_nxt;    // next header 
						uint8_t  ip6_un1_hlim;   // hop limit 
				  } ip6_un1;
				uint8_t ip6_un2_vfc;       // 4 bits version, top 4 bits tclass 
			} ip6_ctlun;
			struct in6_addr ip6_src;      // source address 
			struct in6_addr ip6_dst;      // destination address   };*/
#ifdef IMPRIMELOTODO
	struct ip6_hdr *ipv6addr = (struct ip6_hdr *) packet;
	char dir_buf[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6,&ipv6addr->ip6_src,dir_buf,INET6_ADDRSTRLEN);
	printf("IPv6 src addr %s\n", dir_buf);
	inet_ntop(AF_INET6,&ipv6addr->ip6_dst,dir_buf,INET6_ADDRSTRLEN);
	printf("IPv6 dst addr %s\n", dir_buf);
#endif
	return *(packet+6); //corresponde al campo NEXTHEADER
	}
	
void *start_daemon (void *arg)
{
	IFS_interfaz_t *ifaz = (IFS_interfaz_t *) arg;
	int sock, ret;

//del Send_Msg_Icmp
	struct msghdr mhdr;
    struct iovec iov[1];
    struct icmp6_hdr *hdr = calloc(1, sizeof(struct icmp6_hdr) + 42);
	char msg[] = "[Paging_Distance]=3";
	struct sockaddr_in6 addr, localdir;
	addr.sin6_family=AF_INET6;
    inet_pton(AF_INET6, "FF01::11", &addr.sin6_addr);
	
	localdir.sin6_family=AF_INET6;
    localdir.sin6_port=htons(31001);
    localdir.sin6_flowinfo=0;
    memcpy(&localdir.sin6_addr, &ifaz->direccion, sizeof(ifaz->direccion));

	if ((sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)) < 0)
    {        perror("socket");
        exit(-1);
    }
	
/////////CONSTRUYO MENSAGE ICMP + OPTIONS/////////
    memset((void *) &mhdr, 0, sizeof(mhdr));
    hdr->icmp6_type = 129;
    hdr->icmp6_code = 0;
    mhdr.msg_name = (caddr_t) &addr;
    mhdr.msg_namelen = sizeof(struct sockaddr_in6);
    mhdr.msg_iov = iov;
    mhdr.msg_iovlen = 1;
    iov[0].iov_base = hdr;
    iov[0].iov_len = sizeof(struct icmp6_hdr) + 42;
    mhdr.msg_control = (caddr_t) NET_ipv6_make_exthdr(msg, &mhdr.msg_controllen); //esta funcion mete HopByHop opts
    
/////////CAMBIO LA INTERFAZ DE SALIDA/////////
    ret=setsockopt(sock,IPPROTO_IPV6, IPV6_MULTICAST_IF, &(ifaz->nl_dev_number), sizeof (ifaz->nl_dev_number));
        if (ret<0)   perror("sockopt IPV6_MULTICAST_IF!\n");
/////////Bind el SOCKET a mi direccion/////////
        ret=bind(sock, (struct sockaddr *) &localdir, sizeof(struct sockaddr_in6));
#ifdef IMPRIMELOTODO
        printf ("socket es %d y Bind m da %d\n",sockfd,ret);
#endif
        if (ret<0)   perror("Errno de bind?");


/////////ENVIO EL MSG ICMPv6 + HopByHop Opts/////////
	if ((ret = sendmsg(sock, &mhdr, 0)) < 0)
    {        perror("sendmsg");
        exit(-1);
    }
	printf("Ret es %d\n",ret);
	
/////////Preparo los filtros de recepción pcap/////////	
	char errbuf[PCAP_ERRBUF_SIZE];
	char rule[] = "dst net ff01::/64";
// tcpdump -i eth1 ip6 multicast and ip6 proto 0 or icmp6 or udp and not udp port 521
// tcpdump -i eth1 ip6 multicast and not udp port 521
	//ICMP6 en  TCPDUMP no va!!! PQ TENEMOS HOPBYHOP!
	struct bpf_program fp;      /* hold compiled program     */
	bpf_u_int32 netp;           /* ip                        */
	pcap_t *p;
	p=pcap_open_live(ifaz->dev_name, 65535, 0, 1, errbuf);
    if(p == NULL)
    { printf("Falla pcap_open_live(): %s\n",errbuf); exit(1); }
	
    /* Lets try and compile the program.. non-optimized */
    if(pcap_compile(p,&fp,rule,0,netp) == -1)
    { fprintf(stderr,"Error calling pcap_compile\n"); exit(1); }

    /* set the compiled program as the filter */
    if(pcap_setfilter(p,&fp) == -1)
    { fprintf(stderr,"Error setting filter\n"); exit(1); }

    /* ... and loop */ 
    pcap_loop(p,3,my_callback,NULL);
	printf ("Acaba contexto de ifaz %d\n", ifaz->nl_dev_number);
    	
	return NULL;
}
#include "ip_functions.c" //ESTO ES UN PASTEL
