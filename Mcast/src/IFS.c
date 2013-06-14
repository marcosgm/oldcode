#include "defs.h"

void IFS_parsea_from_proc(IFS_t *ifs)
{
	int contador=0;
	char *i;	
	int j;
	FILE *proc_fd;
	
#ifdef FROMFILE
	if ((proc_fd=fopen("if_inet6","r"))==NULL)
#else 
	if ((proc_fd=fopen(IF_INET6_PROCFILE,"r"))==NULL)
#endif
	{	perror("Abriendo /proc/net/if_inet6");
		exit(1);
	}
	
	memset(ifs,0,sizeof(IFS_t));
	ifs->cantidad=0;	
	
	contador=UTIL_get_number_of_lines(proc_fd,70);
#ifdef IMPRIMELOTODO
	printf("Numero de IPs %d\n",contador);
#endif
	rewind(proc_fd);/*vuelvo al principio del fichero*/
	
	for (j=0;j<contador;j++)
	{
		unsigned char dev_name[10]; /*p.ej eth2 o wlan0*/
		unsigned int nl_dev_number=0,mask=0,scope=0,flags=0;
		unsigned char dir[INET6_ADDRSTRLEN];
		struct in6_addr ipv6dir;	
		
		memset(&ipv6dir,0,sizeof(struct in6_addr));
		*dir=*dev_name='\0';
		/* SEE .../doc/proc_ip6info.txt */
		fscanf(proc_fd,"%32s %x %x %x %x\t%s\n",dir,&nl_dev_number,&mask,&scope,&flags,dev_name);
			
		UTIL_hexstring_to_ipv6(&ipv6dir, dir);
		
		/*cantidad me sirve para entrar en el SIGUIENTE elemento del vector*/
		IFS_interfaz_t *puntero = &(ifs->interfaz[ifs->cantidad]);
		memcpy(&puntero->direccion, &ipv6dir, sizeof(struct in6_addr));
		puntero->nl_dev_number=nl_dev_number;
		puntero->mask=mask;
		puntero->scope=scope;
		puntero->flags=flags;
		strcpy(puntero->dev_name,dev_name);
#ifdef IMPRIMELOTODO
		/*Ahora meteré en DIR la direccion IPv6 bien construida*/
		inet_ntop(AF_INET6,	&(puntero->direccion),dir,INET6_ADDRSTRLEN);
		printf("%s ",dir);
		printf ("\tID %2d, mask %3d, scope %3d, en %s\n", puntero->nl_dev_number,puntero->mask,puntero->scope,puntero->dev_name);
#endif
		ifs->cantidad++;
		/*SCOPE 0 es Global, 16 es host, 32 es linklocal, 64 es sitelocal*/
	}
	fclose(proc_fd);
}
void IFS_sacar_solo_sitelocal(IFS_t *solositelocal,IFS_t *todas_ifaces)
{
	int i=todas_ifaces->cantidad;
	memset(solositelocal,0,sizeof(IFS_t));
	
	while (i>=0)
	{
		if (IN6_IS_ADDR_SITELOCAL(todas_ifaces->interfaz[i].direccion.s6_addr))
		{
			memcpy(&(solositelocal->interfaz[solositelocal->cantidad]), &(todas_ifaces->interfaz[i]), sizeof(IFS_interfaz_t));
			solositelocal->cantidad++;
		}	
		i--;
	}
	
}
void IFS_sacar_solo_linklocal(IFS_t *sololinklocal,IFS_t *todas_ifaces)
{	
	int i=todas_ifaces->cantidad;
	memset(sololinklocal,0,sizeof(IFS_t));
	
	while (i>=0) 
	{
		if (IN6_IS_ADDR_LINKLOCAL(todas_ifaces->interfaz[i].direccion.s6_addr))
		{
			memcpy(&(sololinklocal->interfaz[sololinklocal->cantidad]), &(todas_ifaces->interfaz[i]), sizeof(IFS_interfaz_t));
			sololinklocal->cantidad++;
		}	
		i--;
	}
	
}

int IFS_consigue_struct_ifaz (IFS_t *ifs, IFS_interfaz_t *resultado, char *nombre)
{	//retorna 0 si todo OK, otra cosa si no va bien
	int i;
	for (i=0; i< ifs->cantidad; i++)
		if (!strcmp(ifs->interfaz[i].dev_name, nombre))
		{	memcpy (resultado,&ifs->interfaz[i],sizeof(IFS_interfaz_t));
			return 0;
		}
	return -1;
}

int IFS_compara(IFS_interfaz_t *if1, IFS_interfaz_t *if2)
{	//devuelve 0 si son iguales, -1 sino
	if (if1->nl_dev_number==if2->nl_dev_number)
		return 0;
	else	
		return -1;
}

void IFS_crear_lista_ifaces_child(IFS_t *child, IFS_t *todas_ifs, IFS_interfaz_t *parent)
{	//Algoritmo: buscar en todas_ifs, las que NO tengan el mismo nl_dev_number que parent, las metemos en child
	int i, j;
	memset (child, 0 ,sizeof (todas_ifs)); //downlink sera CASI igual de grande que todas_ifs
	for (i=0, j=0; i < todas_ifs->cantidad; i++)
	{
		if ( todas_ifs->interfaz[i].nl_dev_number != parent->nl_dev_number)
		{
			memcpy (&child->interfaz[j], &todas_ifs->interfaz[i], sizeof (IFS_interfaz_t));
			child->cantidad++;
			j++;
		}
	}
}
