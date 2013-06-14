#include "defs.h"

void RT_parsea_from_proc (RT_t *rts)
{
	int contador=0,j=0;
	FILE *proc_fd;

	proc_fd = fopen(IPV6_ROUTE_PROCFILE,"r");
	contador = UTIL_get_number_of_lines(proc_fd,200);/*la linea mas larga es 149 (wc -L)*/
	rewind (proc_fd);
#ifdef IMPRIMELOTODO	
	printf("Numero de rutas %d\n",contador);
#endif
	memset(rts,0,sizeof(RT_t));
	rts->cantidad=0;	
	
	unsigned char dev_name[10]; /*p.ej eth2 o wlan0*/
	unsigned int dst_len=0, src_len=0,metric=0,ref=0,use=0,flags=0;
	unsigned char srcdir[INET6_ADDRSTRLEN];
	unsigned char dstdir[INET6_ADDRSTRLEN];
	unsigned char nhdir[INET6_ADDRSTRLEN];
	struct in6_addr ipv6dir;	
	//struct in6_addr dst_network,src_network,nh_network;		
	for (j=0;j<contador;j++)
	{		
		memset(&ipv6dir,0,sizeof(struct in6_addr));
		*srcdir,*dstdir,*nhdir=*dev_name='\0';
		dst_len=src_len=metric=ref=use=flags=0;
		
		/* INFO en .../doc/proc_ip6info.txt */
		fscanf(proc_fd,"%32s %2x %32s %2x %32s %8x %8x %8x %8x\t%s\n",dstdir,&dst_len,srcdir,&src_len,nhdir,&metric,&ref,&use,&flags,dev_name);
		//printf("\ndstdir es %s, len es %x, srcdir es %s, len es %d,dev es %s\n",dstdir,dst_len,srcdir,src_len,dev_name);
		/*cantidad m sirve para entrar en el SIGUIENTE elemento del vector*/
		RT_ruta_t *puntero = &(rts->ruta[rts->cantidad]);
		UTIL_hexstring_to_ipv6 (&ipv6dir, dstdir);
//ESTE IF() ES PORQUE SOLO QUIERO RUTAS A DIRECCIONES SITELOCAL	
		if (IN6_IS_ADDR_SITELOCAL (ipv6dir.s6_addr)) 
		{
			memcpy(&(puntero->dst_network), &ipv6dir, sizeof(struct in6_addr));
			UTIL_hexstring_to_ipv6 (&ipv6dir, srcdir);
			memcpy(&(puntero->src_network), &ipv6dir, sizeof(struct in6_addr));
			UTIL_hexstring_to_ipv6 (&ipv6dir, nhdir);
			memcpy(&(puntero->nh_network), &ipv6dir, sizeof(struct in6_addr));
		
			puntero->dst_len=dst_len;
			puntero->src_len=src_len;
			puntero->metric=metric;
			puntero->ref=ref;
			puntero->use=use;
			puntero->flags=flags;
			strcpy(puntero->dev_name,dev_name);
	
			//printf ("\tID %2d, mask %3d, scope %3d, en %s\n", puntero->nl_dev_number,puntero->mask,puntero->scope,puntero->dev_name);
			rts->cantidad++;
			/*SCOPE 0 es Global, 16 es host, 32 es linklocal, 64 es sitelocal*/
		}
	}
	fclose(proc_fd);

}
void * RT_bucle_parsea_from_proc (void *rts)
{
	RT_t *rutas = (RT_t *) rts;
	while (1)
	{
		sleep (3);
		RT_parsea_from_proc (rutas);
	}
}

void RT_filtra_por_ifaz(RT_t *rts_filtrado, RT_t *rts_todas, char * devname)
{
	int i,j=0;
	memset (rts_filtrado, 0, sizeof (RT_t));
	//rts_filtrado->cantidad=0;
	for (i=0; i < rts_todas->cantidad; i++)
			if ( !strcmp( rts_todas->ruta[i].dev_name , devname))//si son iguales
			{
				memcpy (&(rts_filtrado->ruta[j]) , &(rts_todas->ruta[i]) , sizeof(RT_ruta_t));
				j++;
				rts_filtrado->cantidad++;
				//TODO: control de errores MEMCPY
			}
}
void RT_imprime_rutas (RT_t *rts)
{	
	int i=0;
	unsigned char dir[INET6_ADDRSTRLEN];
	for (; i < rts->cantidad; i++)
	{
		RT_ruta_t *puntero = &(rts->ruta[i]);	
		inet_ntop(AF_INET6,	&(puntero->dst_network),dir,INET6_ADDRSTRLEN);
		printf("Llego a la red %s",dir);
		inet_ntop(AF_INET6,	&(puntero->nh_network),dir,INET6_ADDRSTRLEN);
		printf("\t\t via %s en ifaz %s, flags %x\n",dir, puntero->dev_name, puntero->flags);
	}
}
int RT_busca_ifaz_uplink(char *ifaz, RT_t *rts, struct in6_addr *direccion)
{	//devuelve 0 si lo encuentra, -1 en caso contrario
	int i;
	/*La norma para comprobar una ruta en la tabla de enrutado es comprobar si 
	el resultado de multiplicar (AND) la IP a buscar por la máscara de la ruta es 
	EL MISMO de multiplicar (AND) la IP de la ruta por la máscara de la ruta.
	La primera coincidencia es la que vale, sino no habría agregación de rutas
	Hago una función que multiplica (AND) la IP y la MASCARA*/
	for (i=0; i<rts->cantidad; i++)
	{
		if (rts->ruta[i].metric==-1)  //Metrica -1 es UNREACHABLE
			return -1; //no encontrado
		if (UTIL_compare_network(&(rts->ruta[i].dst_network), direccion, rts->ruta[i].dst_len))
		{	
#ifdef IMPRIMELOTODO
			printf ("Resultado ruta #%d\t",i);
			char ip_str[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6,direccion,ip_str, INET6_ADDRSTRLEN);
			printf("%s via %s \n", ip_str, rts->ruta[i].dev_name);
#endif
			strcpy (ifaz, rts->ruta[i].dev_name);
			return 0;	
		}
	}
	return -1; //no encontrado
}


void RT_filtra_sitelocals_por_rango(RT_t *rtsitelocal_filtrado,RT_t *rutas, struct in6_addr *rango, int mask_len)
{	
	int i=0;
	memset(rtsitelocal_filtrado,0,sizeof(RT_t));
	
	while (i<rutas->cantidad)
	{	
		if (UTIL_compare_network(&(rutas->ruta[i].dst_network), rango, mask_len))
		{	
			memcpy(&(rtsitelocal_filtrado->ruta[rtsitelocal_filtrado->cantidad]), &(rutas->ruta[i]), sizeof(RT_ruta_t));
			rtsitelocal_filtrado->cantidad++;
		}	
		i++;
	}
	
}
