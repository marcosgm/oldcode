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
	int i;
    u_int16_t type = PCAP_handle_ethernet(args,pkthdr,packet);
	IFS_interfaz_t *parent = (IFS_interfaz_t *) args;
	int ret;
	u_char	*post_ipv6_header;
	const u_char *hbh_opt_pointer, *dest_opt_pointer;
	static int pkt_counter=0;
	struct ip6_hdr cabecera_ip6;
	
	pkt_counter ++;	
	printf ("\n\n------%d------\tTime: %d'%d lengh %d, of %d. Recv at %s\n", pkt_counter, pkthdr->ts.tv_sec, pkthdr->ts.tv_usec, pkthdr->caplen, pkthdr->len, parent->dev_name);
	for (i=0; i<pkthdr->len; i++)
		printf("%x",packet[i]);
	memset (&cabecera_ip6,0,sizeof (struct ip6_hdr));
	//packet tiene ETH + IPV6 + EXTHDR + DATA
	struct ether_header *eth_hdr = (struct ether_header *) packet;
	
	if(type == ETH_P_IPV6) 	//if(type == ETHERTYPE_IPV6)
    {
		fflush(stdout);
		printf("\nPaquetes recibidos %d\n", pkt_counter);
		packet += sizeof(struct ether_header); //14 bytes de eth_header
//packet tiene IPV6 + EXTHDR + DATA
		ret = PCAP_handle_ipv6 (args, pkthdr, packet, &cabecera_ip6);
		packet += sizeof (struct ip6_hdr); //40 bytes
//packet ahora NO tiene IPV6. Guardo este puntero
		post_ipv6_header = (u_char *) packet;
#ifdef IMPRIMELOTODO
		printf("Retorno %d. hop es %d dst es %d udp es %d e icmp es %d\n",ret, IPPROTO_HOPOPTS, IPPROTO_DSTOPTS, IPPROTO_UDP, IPPROTO_ICMPV6); //IPV6_HOPOPTS es 3 en vez de 0!!!!
#endif
//si ret != IPPROTO_HOPOPTS es que packet tiene SOLO DATA, pq las otras opts las ignoramos
		u_char next_h=0;
		u_char len=0;
		if (ret == IPPROTO_HOPOPTS)
		{	next_h = *(packet); //campo ip6h_nxt
			len = *(packet+1); //campo ip6h_len
			printf("La hopbyhop opt.header es [%x,%x] ,.;:%s:;.,\n", next_h, len, packet +2);
			hbh_opt_pointer = packet+2;
			packet += (len+ 1)*8 ;
		}
		else if (ret == IPPROTO_DSTOPTS)
		{	next_h = *(packet); //campo ip6d_nxt
			len = *(packet+1); //campo ip6d_len
			printf("La destination opt.header es %x,%x, %s\n",  next_h, len, packet +2);
			dest_opt_pointer = packet+2;
			packet += (len+ 1)*8 ;
		}
		else if (ret == IPPROTO_UDP || ret == IPPROTO_ICMPV6)  //si es UDP ni ICMP6
		{	printf ("No hay extension headers\n");
			goto die;
		}
		else 
		{	printf ("Protocolo no manejable\n");
			goto die;
		}
		int aux = pkthdr->len - (len+ 1)*8 - sizeof(struct ether_header) - sizeof (struct ip6_hdr);
		if (aux > 0)	printf("%d bytes de payload: ,.;:%s:;.,\n",aux,packet);
/*		//TODO: arreglar esto, mirar a qué apunta next_h después de procesar 1 ext_h (solo proceso 1, procesar más, vaya)
		
//AHORA NO MUEVO EL PUNTERO DE PACKET, tiene mis datos; SINO QUE PASO A HACER LAS COMPROBACIONES DE ENRUTADO

//PASO 1 Broadcast HopLimit: si ya ha llegado a su límite de vida, expiro el paquete
//PASO 2 RPF: mirar si el paquete ha venido por la ifaz mas corta hacia la IP origen
//PASO 3 GeoPaging: mirar si el campo Paging_Distance de la hopbyhop_opt, comparado contra
//			la tabla de enrutado de cada ifaz DOWNLINK, hace necesario la duplicación del paquete
//			por esas interfaces.
//PASO 4 INYECTO el paquete por las interfaces que han superados los 3 pasos.
//mi paso 3 es como hacer TRPF, pero en vez de ver si IGMP dice si hay o no hay usuarios mcast,
//será mi algoritmo quien lo decida. No hay nada parecido a RPM, con sus purge/join msgs, aqui
//las tablas de enrutado unicast son lo que se usan, mirando el parametro Paging_Distance, que es totalmente dinamico
*/
		//PASO 1
			if (cabecera_ip6.ip6_hlim > 1) //si es 1, descarto			
			{
				-- cabecera_ip6.ip6_hlim; //decremento HopLimit
				printf("PASO 1: TTL check Superado\n");
				//PASO 2
				int retorn;
				extern RT_t todas_rutas; //del main.c
				char ifaz_uplink_str[20];
				//Parent lo hemos recibido por parámetro, es un IFS_interfaz_t, no char*

//HECHO este TODO: crear otro thread en el MAIN que actualize cada 3 seg. las rutas

				retorn = RT_busca_ifaz_uplink(ifaz_uplink_str, &todas_rutas, &cabecera_ip6.ip6_src);
				if (retorn == 0) //PASO 2A
				{	printf("PASO 2a: Ruta encontrada via ifaz %s, falta ver RPF\n", ifaz_uplink_str);
					//PASO 2 y medio
					extern IFS_t sitelocal; //en main.c
					//SOLO utilizo las interfaces con IPs que son del tipo SITELOCAL
					IFS_interfaz_t ifaz_uplink;
					if (IFS_consigue_struct_ifaz (&sitelocal, &ifaz_uplink, ifaz_uplink_str) != 0) 
					{	printf ("!!!!PASO 2b!!!!:Error buscando una struct. ifaz por su nombre\n");	
						goto die;}
					
//					printf("Tenemos %s vs. %s\n", parent->dev_name, ifaz_uplink.dev_name);
					//ahora tengo la structura de la ifaz por donde me llegó el pkt
					else if (IFS_compara(parent,&ifaz_uplink)==0) //PASO 2B: comparo esa ifaz con la parent
						{
							printf("PASO 2b: Uplink check Superado\n");
							//PASO 3
							int distance=0, aux, distanciaCalculada=0;
							RT_t rutas_propias;
							RT_filtra_por_ifaz (&rutas_propias, &todas_rutas, "eth0");
							sscanf(hbh_opt_pointer,"[Paging_Distance]=%d", &distance);
							printf("Distancia recibida: %d\n", distance);
							
							for (aux=0; aux < rutas_propias->cantidad; aux++)
							{
								distanciaCalculada = GEOPAG_calcula_distancia (&cabecera_ip6.ip6_dst, &rutas_propias.ruta[aux].dst_network);
								printf("Dist. calculada: %d\n", distanciaCalculada);
								if (distanciaCalculada <= distance ) //si está dentro del rango que marca el paquete
								{	printf("PASO 3: Paging distance check Superado\n");
									/*PASO 4: obligado usar LIBNET o GNET:
									Stevens dice: (pag 658)
									No hay nada similar al socket option IP_HDRINCL de IPv4 para ipv6, en ipv6
									no podemos leer ni escribir paquetes completos IPv6 incluidos Ext_headers
									mediante ipv6_raw. Casi todos los campos de la cabecera, incluso las ext_h
									se deben leer/escribir mediante anciliary o bien con sockopts.*/
									IFS_t child_links;
									int x;	
									IFS_crear_lista_ifaces_downlink(&child_links, &sitelocal, parent);
									for (x=0; x < child_links.cantidad; x++)
									{
										//UTIL_change_srcIp (&cabecera_ip6, &child_links.interfaz[x].direccion);
										int payload_len = pkthdr->len - sizeof (struct ether_header) - sizeof (struct ip6_hdr);				
										NET_libnet_inject (eth_hdr, &cabecera_ip6, post_ipv6_header, payload_len, child_links.interfaz[x].dev_name); 
										printf ("PASO 4: Forwarding por ifaz %s realizado\n", child_links.interfaz[x].dev_name);
									}
								} //if PASO 3
								else printf("PASO 3: ");
									
						}
					//else if PASO 2b
					else printf ("Descarte en Paso 2b: RPF fallido pq paquete ha llegado por ifaz != parent\n");
				} 
				//if PASO 2a
				else printf("Descarte en Paso2a: RPF fallido porque Ruta NO encontrada!!n");
			}
			//if PASO1
			else printf ("Descartado en Paso 1: TTL de comprobación de broadcast falló\n");
	} 
	//if is IPV6?
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
#ifdef IMPRIMELOTODO
	struct ip6_hdr *ipv6addr = (struct ip6_hdr *) packet;
	char dir_buf[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6,&ipv6addr->ip6_src,dir_buf,INET6_ADDRSTRLEN);
	printf("IPv6 src addr %s\n", dir_buf);
	inet_ntop(AF_INET6,&ipv6addr->ip6_dst,dir_buf,INET6_ADDRSTRLEN);
	printf("IPv6 dst addr %s\n", dir_buf);
#endif
	memset(cabecera,0,sizeof(struct ip6_hdr));
	memcpy (cabecera, packet, sizeof(struct ip6_hdr));

	return cabecera->ip6_nxt; //devolvemos el valor de la siguiente cabecera
}
