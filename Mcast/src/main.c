#include "defs.h"
#include <linux/in_route.h>
#include "ip2hack.h"//ESTO ES UN TRUCO para probar las funciones de obtener rutas del programa "iproute2"
#include <stdlib.h>

void *nuevo_contexto (void *ifaz_up);

RT_t todas_rutas;	
IFS_t todas_ifaces;
IFS_t ifaces_activas;
struct in6_addr rango_unicast;
struct in6_addr rango_mcast;
	
char pcap_filter [300];
int main (int argc, char **argv)
{
	int i,j;
	pthread_t contextos[MAX_IFACES], actualizador_rutas;
	char rango_mcast_str[80], rango_unicast_str[80];
	
///Necesito como argumento el rango IPv6 multicast
	if (argc < 3)
	{
		printf ("ERROR, mal uso: %s ff05:: [prefijo/64 multicast para paging] fec0:: [prefijo/64 unicast sitelocal para ID celdas]\n", argv[0]);
		return -1;
	}
	//esto evalua si las direcciones son correctas o no
	if (!(inet_pton (AF_INET6, argv[1], &rango_mcast) && IN6_IS_ADDR_MULTICAST (&rango_mcast))) 
	{
		printf ("ERROR: argumento multicast erroneo. Necesito una direccion IPv6 multicast (ff00::/8), usaremos rango /64\n");
		return -1;
	}
	if (!(inet_pton (AF_INET6, argv[2], &rango_unicast) && IN6_IS_ADDR_SITELOCAL (&rango_unicast)))
	{
		printf ("ERROR: argumento unicast sitelocal erroneo. Necesito una direccion IPv6 unicast sitelocal (fec0::/10), usaremos rango /64\n");
		return -1;
	}
	inet_ntop(AF_INET6, &rango_mcast, rango_mcast_str, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, &rango_unicast, rango_unicast_str, INET6_ADDRSTRLEN);
	printf ("Iniciamos Geopaging Router para dominio multicast %s/64\nDominio unicast para ID de celdas %s/64\n", rango_mcast_str, rango_unicast_str);
	
		
///////solo uso las interfaces con direcciones IP6 sitelocal	
	IFS_parsea_from_proc(&todas_ifaces);
	IFS_sacar_solo_sitelocal(&ifaces_activas,&todas_ifaces);
#ifdef IMPRIMELOTODO
	printf("Fin del parseo de /proc/net/if_inet6\n");
	printf("Hay %d sitelocals\n",ifaces_activas.cantidad);
#endif	
	
	RT_parsea_from_proc(&todas_rutas); //ojo pq se resetea a 0 el &todas_rutas
	pthread_create(&actualizador_rutas, NULL, RT_bucle_parsea_from_proc, &todas_rutas);//Actualizo tabla routing unicast

//Preparo el string que será mi filtro PCAP
	//char rule [300] = "dst net ff08:6666:0:6666::/64";
	char rule [300] = "dst net ";
	strcat (rule, rango_mcast_str);
	strcat (rule, "/64"); //Usamos un rango /64
	for (i=0; i<ifaces_activas.cantidad; i++)
	{
		u_char MAC[6];
		char MAC_str[20];
		if (NET_get_MAC_address(MAC,ifaces_activas.interfaz[i].dev_name)!=0)
			printf ("Error con las MACs");
		else
		{	
			UTIL_mac_to_string(MAC,MAC_str);	
			strcat (rule, " and not ether src ");
			strcat (rule, MAC_str);
		}
	}
	strncpy (pcap_filter, rule, strlen (rule)); //meto "rule" en la variable global PCAP_FILTER
	printf ("Filter es %s\n", pcap_filter);

	GEOPAG_inicializa_R();
	
	//Solo crearemos contextos donde tengamos direcciones IPv6 Sitelocal
	for (i=0; i<ifaces_activas.cantidad; i++)
	{
		#ifdef IMPRIMELOTODO
			u_char buf[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &(ifaces_activas.interfaz[i].direccion), buf, INET6_ADDRSTRLEN);
			printf("Esta direccion es %s\n",buf);
		#endif
		if (pthread_create (&contextos[i], NULL, nuevo_contexto, &ifaces_activas.interfaz[i])<0)
		{	perror("Threadin!"); exit (1);}
		printf("%d:Creado contexto en ifaz %s\n", i,ifaces_activas.interfaz[i].dev_name);
	}
	//Espero a la muerte de los hilos del contexto
	for (j=0; j<i; j++)
	{	
		pthread_join (contextos[j],NULL); //me da igual su estado de finalizacion
		printf("#%d:Destruyo thread %ld\n", j, contextos[j]);
	}
	pthread_join(actualizador_rutas, NULL);
	return 0;
}

	
void *nuevo_contexto (void *ifaz_up)
{
	IFS_interfaz_t *parent = (IFS_interfaz_t *) ifaz_up;

/////////Preparo los filtros de recepción pcap/////////	
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;      /* hold compiled program     */
	bpf_u_int32 netp;           /* ip                        */
	pcap_t *p;
	p=pcap_open_live(parent->dev_name, 65535, 0, 1, errbuf);
	if(p == NULL)
    { printf("Falla pcap_open_live(): %s\n",errbuf); exit(1); }
#ifdef IMPRIMELOTODO
	else
		printf("Ok, pcap empieza a funcionar, %x es p\n",p);
#endif
//TODO: probar si mejora el rendimiento cambiando la optimización	
    /* Lets try and compile the program.. non-optimized */
    if(pcap_compile(p,&fp,pcap_filter,0,netp) == -1)
    { fprintf(stderr,"Error calling pcap_compile\n"); exit(1); }

    /* set the compiled program as the filter */
    if(pcap_setfilter(p,&fp) == -1)
    { fprintf(stderr,"Error setting filter\n"); exit(1); }
    /* ... and loop */ 
	pcap_loop(p,-1,PCAP_receptor_msg, (u_char *) parent); //loop infinito
	printf ("Acaba contexto de ifaz %d\n", parent->nl_dev_number);

	
	return NULL;
}
#include "ip_functions.c" //ESTO ES UN TRUCO para probar las funciones de obtener rutas del programa "iproute2"
