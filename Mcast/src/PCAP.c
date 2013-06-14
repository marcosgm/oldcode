#include "defs.h"
/*struct ether_header{  u_int8_t  ether_dhost[ETH_ALEN]; //ALEN es 6
						   u_int8_t  ether_shost[ETH_ALEN];  
						   u_int16_t ether_type;  } __attribute__ ((__packed__)); */
/*struct ip6_hdr  {    //PESA 40 BYTES EN TOTAL
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
////////Estos campos de la cabecera IP6 tienen unos defines más faciles en <netinet/ip6.h>
void PCAP_receptor_msg(u_char *args,const struct pcap_pkthdr* pkthdr,const u_char*  packet)
{
	int i, ret;
    u_int16_t type;
	IFS_interfaz_t *parent = (IFS_interfaz_t *) args;
	static int pkt_counter=0;
	struct ip6_hdr cabecera_ip6;
	
	struct ether_header *eth_hdr;
	u_char *ipv6_hdr, *post_ipv6_hdr, *hbh_opt_pointer, *dest_opt_pointer, *L4_pointer;	
	
	type = PCAP_handle_ethernet(args,pkthdr,packet);
	pkt_counter ++;	
	printf ("\n\n------%d------\tTime: %d'%d lengh %d, of %d. Recv at %s\n", pkt_counter, pkthdr->ts.tv_sec, pkthdr->ts.tv_usec, pkthdr->caplen, pkthdr->len, parent->dev_name);
	for (i=0; i<pkthdr->len; i++)
		printf("%02x",packet[i]);
	memset (&cabecera_ip6,0,sizeof (struct ip6_hdr));

	eth_hdr = (struct ether_header *) packet;
//eth_hdr tiene ETH + IPV6 + EXTHDR + DATA	

	if(type == ETH_P_IPV6) 	//if(type == ETHERTYPE_IPV6)
    {
		fflush(stdout);
		printf("\nPaquetes recibidos %d\n", pkt_counter);
		ipv6_hdr = (u_char *) eth_hdr + sizeof(struct ether_header); //14 bytes de eth_header	
//ipv6_hdr tiene IPV6 + EXTHDR + DATA
		ret = PCAP_handle_ipv6 (args, pkthdr, ipv6_hdr, &cabecera_ip6);
#ifdef IMPRIMELOTODO
		printf("Retorno %d. hop es %d dst es %d udp es %d e icmp es %d\n",ret, IPPROTO_HOPOPTS, IPPROTO_DSTOPTS, IPPROTO_UDP, IPPROTO_ICMPV6); //IPV6_HOPOPTS es 3 en vez de 0!!!!
#endif
		post_ipv6_hdr = ipv6_hdr + sizeof (struct ip6_hdr); //40 bytes
//post_ipv6_hdr no tiene IPV6. solo tiene EXTHDR + DATA
		
		//si ret != IPPROTO_HOPOPTS es que *post_ip6_hdr tiene SOLO DATA, pq las otras opts las ignoramos
		u_char next_h=0;
		u_char len=0;
		L4_pointer = post_ipv6_hdr; //asumo por defecto que no hay ext_hdr
		if (ret == IPPROTO_HOPOPTS)
		{	next_h = *(post_ipv6_hdr); //campo ip6h_nxt
			len = *(post_ipv6_hdr+1); //campo ip6h_len
			printf("La hopbyhop opt.header es [%x,%x] ,.;:%s:;.,\n", next_h, len, post_ipv6_hdr +2);
			hbh_opt_pointer = post_ipv6_hdr+2;
			L4_pointer += (len+ 1)*8 ; //ahora solo tengo DATA de nivel 4
		}
		else if (ret == IPPROTO_DSTOPTS)
		{	next_h = *(post_ipv6_hdr); //campo ip6d_nxt
			len = *(post_ipv6_hdr+1); //campo ip6d_len
			printf("La destination opt.header es %x,%x, %s\n",  next_h, len, post_ipv6_hdr +2);
			dest_opt_pointer = post_ipv6_hdr+2;
			L4_pointer += (len+ 1)*8 ;
		}
		else if (ret == IPPROTO_UDP || ret == IPPROTO_ICMPV6)  //si es UDP ni ICMP6
		{	printf ("No hay extension headers\n");
			goto die;
		}
		else 
		{	printf ("Protocolo no manejable\n");
			goto die;
		}
		int L4_payload_len;
		L4_payload_len = pkthdr->len - (len+ 1)*8 - sizeof(struct ether_header) - sizeof (struct ip6_hdr);
		if (L4_payload_len > 0)	printf("%d bytes de payload: ,.;:%s:;.,\n",L4_payload_len, L4_pointer);
/*		//TODO: arreglar esto, mirar a qué apunta next_h después de procesar 1 ext_h (solo proceso 1, hay q procesar más, vaya)
*/		
		int distance;
		sscanf(hbh_opt_pointer,"[Paging_Distance]=%d", &distance);
		char d [INET6_ADDRSTRLEN];
		inet_ntop (AF_INET6, &cabecera_ip6.ip6_dst, d, INET6_ADDRSTRLEN);
		printf("Distancia recibida: %d, Host destino es %s\n", distance, d);
		//GEOPAG_router (&cabecera_ip6, eth_hdr, distance, parent);
//	//	//printf ("%X es plen y %x es next\n", cabecera_ip6.ip6_plen, cabecera_ip6.ip6_nxt);
//	//	//printf ("%X es vfc y %x es hoplim\n", cabecera_ip6.ip6_vfc, cabecera_ip6.ip6_hlim);
		GEOPAG_router (&cabecera_ip6, eth_hdr, distance, parent);
	}//if is IPV6?
	else printf ("Protocolo no IPV6\n");
	return;
die:
	printf("##%d##Ignorado mensaje recibido\n",pkt_counter); 
	return;
	//llego aquí si falla alguna utilidad de parseo o si los protocolos son raros.
	//finalizamos la rutina sin inyectar nada en la red, pq no enrutamos msgs desconocidos ni fragmentados
	//TODO: manejar paquetes fragmentados o con routing headers

}


int PCAP_handle_udp
        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet)
		{return 1;}
int PCAP_handle_icmp6
        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet)
		{return 1;}

int PCAP_handle_ethernet
        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet)
{
    struct ether_header *eptr;  /* net/ethernet.h */
    eptr = (struct ether_header *) packet;
#ifdef IMPRIMELOTODO
    printf("ethernet header source: %s",ether_ntoa((struct ether_addr *)eptr->ether_shost));
    printf(" destination: %s ",ether_ntoa((struct ether_addr *)eptr->ether_dhost));
#endif
    /* check to see if we have an ip packet */
    if (ntohs (eptr->ether_type) == ETHERTYPE_IPV6) //DEFINIDO EN defs.h, fallo de linux
    {		//printf("(IPV6) ");
	}else {
        printf("(?)");
        exit(1);
    }
    return ntohs (eptr->ether_type); //devolvemos el valor del protocolo del paquete
}
int PCAP_handle_ipv6
        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet, struct ip6_hdr *cabecera)
{	
	struct ip6_hdr *ipv6addr = (struct ip6_hdr *) packet;
#ifdef IMPRIMELOTODO
	char dir_buf[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6,&ipv6addr->ip6_src,dir_buf,INET6_ADDRSTRLEN);
	printf("IPv6 src addr %s\n", dir_buf);
	inet_ntop(AF_INET6,&ipv6addr->ip6_dst,dir_buf,INET6_ADDRSTRLEN);
	printf("IPv6 dst addr %s\n", dir_buf);
#endif
	memset(cabecera,0,sizeof(struct ip6_hdr));
//	memcpy (cabecera, packet, sizeof(struct ip6_hdr));
	cabecera->ip6_vfc = ntohl(ipv6addr->ip6_vfc); //4 bytes
	cabecera->ip6_flow = ipv6addr->ip6_flow;
	cabecera->ip6_plen = ntohs(ipv6addr->ip6_plen); //2 bytes
	cabecera->ip6_nxt = ipv6addr->ip6_nxt; //1 byte no es necesario ntohs()
	cabecera->ip6_hlim = ipv6addr->ip6_hlim;
	cabecera->ip6_src = ipv6addr->ip6_src;
	cabecera->ip6_dst = ipv6addr->ip6_dst;

	return cabecera->ip6_nxt; //devolvemos el valor de la siguiente cabecera
}
