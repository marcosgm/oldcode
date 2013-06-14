#include <asm/types.h>	
#include <netinet/ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>		
#include <sys/ioctl.h>	
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
//http://www.faqs.org/rfcs/rfc3493.html API de IPv6
//http://www.faqs.org/rfcs/rfc3542.html
#include <stdio.h>
#include <netinet/ip6.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/icmp6.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netdb.h>
#include <getopt.h>
#include <signal.h>
#include <pcap.h>
#include <libnet.h>

#define ETHERTYPE_IPV6               0x86DD
#define IF_INET6_PROCFILE "/proc/net/if_inet6"
#define IPV6_ROUTE_PROCFILE "/proc/net/ipv6_route"
#define MAX_IFACES 20
#define MAX_RUTAS 200
typedef union { u_char i[16]; } ip_buf; //128 bits para guardar una IP
struct ip_from_proc
{	struct in6_addr direccion;
	int nl_dev_number;
	int mask;
	int scope;
	int flags;
	char dev_name[10];};
typedef struct ip_from_proc IFS_interfaz_t;
struct interficies_ip
{	int cantidad;
	IFS_interfaz_t interfaz[MAX_IFACES];};
struct ruta_from_proc
{
	struct in6_addr dst_network;
	unsigned int dst_len;
	struct in6_addr src_network;
	unsigned int src_len;
	struct in6_addr nh_network; /*nexthop*/
	unsigned int metric;
	unsigned int ref;
	unsigned int use;
	unsigned int flags;
	char dev_name[10];};
typedef struct ruta_from_proc RT_ruta_t;
struct rutas_ip
{	int cantidad;
	RT_ruta_t ruta[MAX_RUTAS];};

u_char cell_label[21]; //64 bits, solo cojo 63, consigo 21 ID de celda	
/********************/
/*START ZONA PUBLICA*/
/********************/
///CRITERIO: retorno de funcion se pasa como PRIMER argumento

/*start		IFS_*		*/
typedef struct interficies_ip IFS_t ;
//typedef struct ip_from_proc IFS_interfaz_t;
void IFS_parsea_from_proc(IFS_t *ifs);	
void IFS_sacar_solo_sitelocal(IFS_t *solositelocal,IFS_t *todas_ifaces);
void IFS_sacar_solo_linklocal(IFS_t *sololinklocal,IFS_t *todas_ifaces);
int IFS_consigue_struct_ifaz (IFS_t *ifs, IFS_interfaz_t *resultado, char *nombre);
int IFS_compara(IFS_interfaz_t *if1, IFS_interfaz_t *if2);
void IFS_crear_lista_ifaces_child (IFS_t *child, IFS_t *todas_ifs, IFS_interfaz_t *parent);
/*endof-------IFS_*------*/

/*start		RT_*		*/
typedef struct rutas_ip RT_t ;
//typedef struct ruta_from_proc RT_ruta_t;
void RT_parsea_from_proc (RT_t *rts);
void RT_filtra_por_ifaz(RT_t *rts_filtrado, RT_t *rts_todas, char * devname);
void RT_imprime_rutas (RT_t *rts);
int RT_busca_ifaz_uplink(char *ifaz, RT_t *rts, struct in6_addr *direccion);
void * RT_bucle_parsea_from_proc (void *rts);
void RT_filtra_sitelocals_por_rango(RT_t *rtsitelocal_filtrado,RT_t *rutas, struct in6_addr *rango, int mask_len);

/*endof--------RT_*------*/

/*start		UTIL_*		*/
int UTIL_get_hexchar_val(char h);
void UTIL_hexstring_to_ipv6(struct in6_addr *ipv6, char *str);
int UTIL_get_number_of_lines (FILE *fd, int buf_len);
int UTIL_compare_network(struct in6_addr *red, struct in6_addr *direccion, int mask_len);
void UTIL_change_srcIp (struct ip6_hdr *cabecera, struct in6_addr *direccion);
void UTIL_mac_to_string(u_char *MAC, char *str);
/*endof-------UTIL_*-----*/

/*start		NET_*		*/
int 				NET_icmp6_init_sock(void);
void				NET_icmp6_send_msg(const char *msg, struct sockaddr_in6 *addr);
struct cmsghdr *	NET_ipv6_make_exthdr(const char *msg, int *mclen);
void				NET_ipv6_dump(void);
void				NET_ipv6_join_mcast_grp(struct sockaddr_in6 *addr);
void				NET_ipv6_at_exit(void);
int NET_get_MAC_address (u_char *MAC, u_char *dev);
int NET_libnet_inject(struct ether_header *eth, struct ip6_hdr *ip6, u_char *data, int data_size , u_char *dev);
/*endof-------NET_*-----*/

/*start		PCAP_*		*/
void PCAP_receptor_msg(u_char *args,const struct pcap_pkthdr* pkthdr,const u_char*  packet);
int PCAP_handle_udp        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet);
int PCAP_handle_icmp6         (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet);
int PCAP_handle_ethernet         (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet);
int PCAP_handle_ipv6        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet, struct ip6_hdr *cabecera);
/*endof-------PCAP_*-----*/

//extern int *R;
#include "GEOPAG.h"
