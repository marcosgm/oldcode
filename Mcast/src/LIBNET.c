#include <stdio.h>
#include <libnet.h>
#include <net/ethernet.h>

int NET_libnet_inject(struct ether_header *eth, u_char *buf_ip6, int data_size , u_char *dev, int dev_name_len)
{
	libnet_t *l=NULL;
	char errbuf[LIBNET_ERRBUF_SIZE];
	int i=0;
	char device [dev_name_len];

	strncpy (device,dev,dev_name_len);
	
	l=libnet_init (LIBNET_LINK, device,errbuf);
	if (l==NULL)
	{
		printf("\nlibnet_init breaks, %s", errbuf); exit (EXIT_FAILURE);
	}
	libnet_autobuild_ethernet(eth->ether_dhost,eth->ether_type,l);
	
	i=libnet_write(l);
	printf("write da %d\n",i);
	if (i<0) printf("%s",libnet_geterror(l));
	libnet_destroy(l);
	
	return (0);
}
