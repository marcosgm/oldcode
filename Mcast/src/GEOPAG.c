#include "defs.h"
int R[MAXNIVELES];

int GEOPAG_obten_coord (GEOPAG_coordenada_t *c, int x)
{//si x está entre 0 y 6 devuelvo 0.
	switch (x)
	{
	case 0:
		c->x=0; c->y=0; break;
	case 1:
		c->x=0; c->y=1; break;
	case 2:
		c->x=1; c->y=0; break;
	case 3:
		c->x=1; c->y=1; break;
	case 4:
		c->x=1; c->y=2; break;
	case 5:
		c->x=2; c->y=1; break;
	case 6:
		c->x=2; c->y=2; break;
	case 7: //marco el centro de la celda, pero devuelvo 1 por si acaso
		c->x=1; c->y=1; return 1;
	default:
		return -1;
	}
	return 0; //si x esta entre 0 y 6 devuelvo 0.
}

int GEOPAG_parse_string_IDcelda ( int * id, char * str)
{
	char * c;
	int x=0, contador=1;

	c = str;
	do 
	{
		id[x] = atoi (c);
		x++;
		c = strchr (c,'.');
		if (c!=NULL) contador++;
		else return contador;
		c++; 
	}
	while (x<NIVELES);
	return contador;
}

void GEOPAG_parse_IDcelda_desde_IPV6(int *id, struct in6_addr *dir)
{
	u_char buf[16]; //16 bytes son la dir.IP
	int i,j;
	memcpy (buf,dir,16);
//	for (i=0; i< 16; i++)	buf[i] = dir->in6_u.u6_addr8[i];
	i=NIVELES-1;
	j = 15;
	while (i>=0 && j>=0)
	{
		id[i-7] = (buf[j-2] & 0xE0)>>5;
		id[i-6] = (buf[j-2] & 0x1C)>>2;
		id[i-5] = (buf[j-2] & 0x03)<<1 | ((buf[j-1] & 0x80)>>7);
		id[i-4] = (buf[j-1] & 0x70)>>4;
		id[i-3] = (buf[j-1] & 0x0E)>>1;
		id[i-2] = ((buf[j-1] & 0x01)<<2 | ((buf[j] & 0xC0))>>6);
		id[i-1] = (buf[j] & 0x38)>>3;
		id[i] = (buf[j] & 0x07);
		i-=8;
		j-=3;
	}
}

int GEOPAG_calcula_distancia_str(char *ip1, char *ip2)
{
	struct in6_addr direccion1, direccion2;

	if (inet_pton(AF_INET6, ip1, &direccion1)>0 && inet_pton(AF_INET6, ip2, &direccion2)>0)
	{
		return GEOPAG_calcula_distancia (&direccion1, &direccion2);
	}
	else
	{	perror("DIRECCION ERRONEA");
       return -1;
	}
}

int GEOPAG_calcula_distancia (struct in6_addr *direccion1, struct in6_addr *direccion2)
{
	GEOPAG_matriz_t M1, M2, difM;
	GEOPAG_coordenada_t Dv;
	int i=0, j=0;
	int id1 [NIVELES];
	int id2 [NIVELES];
	
	GEOPAG_parse_IDcelda_desde_IPV6 (id1, direccion1);
	GEOPAG_parse_IDcelda_desde_IPV6 (id2, direccion2);
	
	for (j=0; j< NIVELES; j++) //calculo coordenadas de los ID de celdas
	{	
		if (GEOPAG_obten_coord(&M1.c[j], id1[j]) < 0 ||	GEOPAG_obten_coord(&M2.c[j], id2[j]) < 0)
		{	printf ("Valor fuera de rango. Solo permitido del 0 al 7");
				return -1;
		}
		//printf("id1[%d]= %d y id2[%d]= %d\n", j, id1[j], j, id2[j]);
	}
#ifdef IMPRIMELOTODO
	printf ("||Matriz diferencia|| ");
#endif
	for (i=0; i< NIVELES; i++) //calculo matriz Diferencia
	{
		difM.c[i].x = M2.c[i].x - M1.c[i].x;
		difM.c[i].y = M2.c[i].y - M1.c[i].y;
#ifdef IMPRIMELOTODO
		printf ("(%d,%d) ", difM.c[i].x, difM.c[i].y);
#endif
	}
	memset (&Dv, 0, sizeof (GEOPAG_coordenada_t));
//	for (i=NIVELES-1, j=MAXNIVELES-1; i >= 0; i--, j--) //calculo vector Distancia
	for (i=NIVELES-1, j=MAXNIVELES-1; i >= 0; i--, j--) //calculo vector Distancia
	{
		Dv.x += R[j] * difM.c[i].x;
		Dv.y += R[j] * difM.c[i].y;
	}
#ifdef IMPRIMELOTODO
	printf ("Dv es (%d,%d)\n", Dv.x, Dv.y);
#endif
	//calculo valor del vector Distancia
	if (Dv.x * Dv.y > 0) //signe IGUAL
	{	if (Dv.x > Dv.y)
			return abs(Dv.x);
		else 
			return abs(Dv.x);
	}
	else 
		return (abs (Dv.x) + abs (Dv.y));
	
	
	return -1;
}
void GEOPAG_inicializa_R()
{
	int i;
	float res=1;	
	
	R[MAXNIVELES-1] = 1;
	for (i=MAXNIVELES-2; i >= 0; i--)
	{	
		res *= sqrt (7);
		R[i] = res +1; //sumo 1 por la corona que no cuento
#ifdef IMPRIMELOTODO
		printf("%d, ", R[i]);
#endif
	}
}
#ifdef DEF_GEOPAG_ROUTER 
//si no se define, esta funcion no se incluye y permite ejecutar ./build_GEOPAG_tester
//es decir, si no se define solo podemos compilar el programa de prueba (Tester)
//es necesario definirlo para poder compilar el programa completo

int GEOPAG_router (struct ip6_hdr *cabeceraIP6, struct ether_header *cabeceraETH, int distancia, 	IFS_interfaz_t *parent)
{
	extern RT_t todas_rutas;
	extern IFS_t ifaces_activas;

	
//PASO 1 Broadcast HopLimit: si ya ha llegado a su límite de vida, expiro el paquete
//PASO 2 RPF: mirar si el paquete ha venido por la ifaz mas corta hacia la IP origen
//PASO 3 GeoPaging: mirar si el campo Paging_Distance de la hopbyhop_opt, comparado contra
//			la tabla de enrutado de cada ifaz DOWNLINK, hace necesario la duplicación del paquete
//			por esas interfaces.
//PASO 4 INYECTO el paquete por las interfaces que han superados los 3 pasos.
//mi paso 3 es como hacer TRPF, pero en vez de ver si IGMP dice si hay o no hay usuarios mcast,
//será mi algoritmo quien lo decida. No hay nada parecido a RPM, con sus purge/join msgs, aqui
//las tablas de enrutado unicast son lo que se usan, mirando el parametro Paging_Distance, que es totalmente dinamico

	//////PASO 1//////
	if (cabeceraIP6->ip6_hlim > 1) //si es 1, descarto			
	{
		-- cabeceraIP6->ip6_hlim; //decremento HopLimit
		printf("PASO 1: TTL check Superado (%d)\n", cabeceraIP6->ip6_hlim);
	}
	else 
	{	//if !PASO1
		printf ("Descartado en Paso 1: TTL de comprobación de broadcast falló\n");
		return -1;
	}
	//--//PASO 1//--//

	
	//////PASO 2//////
	int retorno_RTbuscaifaz_uplink;
	char ifaz_uplink_str[20];
	IFS_interfaz_t ifaz_uplink;
	
	/////////////////////////////////////////printf ("IPv6 origen: %02X%02X:%02X%02X ::%02X%02X:%02X%02X\n", cabeceraIP6->ip6_src.s6_addr[0], cabeceraIP6->ip6_src.s6_addr[1], cabeceraIP6->ip6_src.s6_addr[2], cabeceraIP6->ip6_src.s6_addr[3], cabeceraIP6->ip6_src.s6_addr[12], cabeceraIP6->ip6_src.s6_addr[13], cabeceraIP6->ip6_src.s6_addr[14], cabeceraIP6->ip6_src.s6_addr[15]);
	//PASO 2A
	retorno_RTbuscaifaz_uplink = RT_busca_ifaz_uplink(ifaz_uplink_str, &todas_rutas, &cabeceraIP6->ip6_src);
	//ahora ifaz_uplink_str es el nombre de la interfaz uplink
	if (retorno_RTbuscaifaz_uplink == 0) //Primero averiguamos cual es la ruta uplink hacia la IP origen
	{	
		printf("PASO 2a: Ruta encontrada via ifaz %s, falta ver RPF\n", ifaz_uplink_str);
	}
	else
	{	//if !PASO 2a
		printf("Descarte en Paso2a: RPF fallido porque Ruta NO encontrada!!\n");
		return -1;
	}
	//PASO 2B
	if (IFS_consigue_struct_ifaz (&ifaces_activas, &ifaz_uplink, ifaz_uplink_str) != 0) //Segundo conseguimos la struc. ifaz correspondiente a la ifaz uplink
	{	//retorna 0 si bien, otra cosa si va mal
		printf ("!!!!PASO 2b!!!!:Error buscando una struct. ifaz por su nombre\n");	
		return -1;
	}
	if (IFS_compara(parent,&ifaz_uplink) != 0) //Tercero comparo esa struct ifaz con mi struct ifaz parent
	{	//if !PASO 2b
		printf ("Descarte en Paso 2b: RPF fallido pq paquete ha llegado por ifaz != parent\n");
		return -1;
	}
	else 
	{	
		printf("PASO 2b: El paquete ha llegado por la interfaz correcta (la parent)\n");
	}
	//--//PASO 2//--//
	
	//////PASO 3/////
	int index_rutas, index_interfaces, distanciaCalculada=0, variable_control=0;
	RT_t rutas_propias; //son las rutas propias a cada interfaz, p.ej. las rutas accesibles via eth0
	IFS_t childs, interfaces_downlink; 
	RT_t rutas_rango_IDceldas;
	extern struct in6_addr rango_unicast;
	
	memset(&childs, 0, sizeof (IFS_t));
	memset(&interfaces_downlink, 0, sizeof (IFS_t));


	//PRIMER FILTRO DE RUTAS: me quedo con rutas destino al rango de direcciones de las ID de celdas
	//todas_rutas tiene solo direcciones sitelocal, pero yo quiero solo direcciones del rango de las ID de celdas
	RT_filtra_sitelocals_por_rango (&rutas_rango_IDceldas, &todas_rutas, &rango_unicast, 64);
	//childs == activas - parent; downlink = childs que cumplen PASO3
	IFS_crear_lista_ifaces_child(&childs, &ifaces_activas, parent);
	for (index_interfaces = 0; index_interfaces < childs.cantidad; index_interfaces ++)
	{
		variable_control=0;
		//SEGUNDO FILTRO DE RUTAS: me quedo con las accesibles solamente via interfaz ethx
		RT_filtra_por_ifaz (&rutas_propias, &rutas_rango_IDceldas, childs.interfaz[index_interfaces].dev_name);

#ifdef IMPRIMELOTODO
		printf ("\nifaz = %s, rutas filtradas: %d\n", childs.interfaz[index_interfaces].dev_name, rutas_propias.cantidad);
		RT_imprime_rutas (&rutas_propias);
#endif
		for (index_rutas = 0; index_rutas < rutas_propias.cantidad && variable_control == 0; index_rutas++)
		{
			distanciaCalculada = GEOPAG_calcula_distancia (&cabeceraIP6->ip6_dst, &rutas_propias.ruta[index_rutas].dst_network);
			if (distanciaCalculada <= distancia ) //si está dentro del rango que marca el paquete
			{
				char direction [INET6_ADDRSTRLEN];
				inet_ntop (AF_INET6, &rutas_propias.ruta[index_rutas].dst_network, direction, INET6_ADDRSTRLEN);
				printf ("Distancia a red %s es %d\n", direction, distanciaCalculada);
				printf("PASO 3: Paging distance check Superado\n" );
				//inserto la interfaz actual en la lista de interfaces downlink
				memcpy (&interfaces_downlink.interfaz[interfaces_downlink.cantidad], &childs.interfaz[index_interfaces], sizeof (IFS_interfaz_t));
				interfaces_downlink.cantidad ++;
				variable_control = 1; //para salir del bucle for, pues la interfaz ya sirve para enviar el paquete
			}//if
		}//for
	}//for
	//--//PASO 3//--//
	if (interfaces_downlink.cantidad == 0)
	{	
		printf("Descarte en PASO 3: Ninguna interfaz cumple con la disstancia requerida\n");
		return -1;
	}
//PASO 4: obligado usar LIBNET o GNET:
//Stevens dice: (pag 658)
//No hay nada similar al socket option IP_HDRINCL de IPv4 para ipv6, en ipv6
//no podemos leer ni escribir paquetes completos IPv6 incluidos Ext_headers
//mediante ipv6_raw. Casi todos los campos de la cabecera, incluso las ext_h
//se deben leer/escribir mediante anciliary o bien con sockopts.
	
	//////PASO 4//////
	int x;	
	for (x=0; x < interfaces_downlink.cantidad; x++)
		{
		//cabeceraETH es el unico puntero que tengo a TODO el paquete. CabeceraIP6 son datos mios
			NET_libnet_inject (cabeceraETH, cabeceraIP6, ((u_char*)cabeceraETH + sizeof (struct ether_header) + sizeof (struct ip6_hdr)), cabeceraIP6->ip6_plen, interfaces_downlink.interfaz[x].dev_name); 

			printf ("PASO 4: Forwarding por ifaz %s realizado\n", interfaces_downlink.interfaz[x].dev_name);
		}
	//--//PASO 4//--//
	return 0;
}
#endif
