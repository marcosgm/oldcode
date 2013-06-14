#include <stdio.h>
#include <stdlib.h>
#include <math.h>//necesario para rint() y sqrt()
#include <netinet/in.h>//necesario para in6_addr
#include <string.h>///necesario para strchr() y memset()
#include <sys/types.h>////necesario para inet_pton()
#include <sys/socket.h>//// idem
#include <arpa/inet.h>//// idem
#include <errno.h> //para errno


//#include "GEOPAG.h"
#include "defs.h"

int main(int argc, char **argv)
{
	GEOPAG_inicializa_R();
	if (argc == 3)
	{	
		printf ("\nUsamos %d niveles\n", NIVELES);
		printf ("Distancia %d", GEOPAG_calcula_distancia_str(argv[1], argv[2]));
		return 0;
	}
	else
		printf ("Error! \nUSO: %s fec0::32 fec0::12\n", argv[0]);
	return -1;
}
