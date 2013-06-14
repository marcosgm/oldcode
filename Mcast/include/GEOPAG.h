
#define MAXNIVELES 21
#ifndef NIVELES
#define NIVELES 7
#endif


struct coordenada
{
	int x;
	int y;
};
typedef struct coordenada GEOPAG_coordenada_t;
struct matriz
{
	GEOPAG_coordenada_t c[NIVELES]; 
};
typedef struct matriz GEOPAG_matriz_t;

int GEOPAG_obten_coord (GEOPAG_coordenada_t *c, int x);
int GEOPAG_parse_string_IDcelda ( int * id, char * str); 
void GEOPAG_parse_IDcelda_desde_IPV6(int *id, struct in6_addr *dir);
int GEOPAG_calcula_distancia (struct in6_addr *direccion1, struct in6_addr *direccion2);
int GEOPAG_calcula_distancia_str(char *ip1, char *ip2);
void GEOPAG_inicializa_R();
int GEOPAG_router (struct ip6_hdr *cabeceraIP6, struct ether_header *cabeceraETH, int distancia, IFS_interfaz_t *parent);
