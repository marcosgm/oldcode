DEFINES	= 
#DEFINES = -DFROMFILE
#DEFINES = -DIMPRIMELOTODO

#GNET_CFLAGS = `pkg-config gnet --cflags`
#GNET_LIBS = `pkg-config gnet --libs`
#GNET = -I/usr/include/gnet-2.0 -I/usr/lib/gnet-2.0/include/ -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include  -lgnet-2.0 -lgthread-2.0 -lglib-2.0
#$(GNET_CFLAGS) $(GNET_LIBS)

INCLUDES = -I ../include -g `libnet-config --cflags`
LIBS = -L. -lip2utils -lm -lpthread -lpcap `libnet-config --libs`
#-L. -lip2 -lip2libs 


Mcast: clean libnetlink IFS RT UTILS NET PCAP GEOPAG
	gcc main.c IFS.o RT.o libnetlink.o NET.o UTILS.o PCAP.o GEOPAG.o $(INCLUDES) -Wall -fstack-protector -o mcast $(LIBS)

clean:
	rm -f libnetlink.o main.o IFS.o RT.o UTILS.o NET.o PCAP.o

libnetlink:
	gcc -c libnetlink.c $(INCLUDES) $(DEFINES) 

IFS:
	gcc -c IFS.c $(INCLUDES) $(DEFINES)

RT:
	gcc -c RT.c $(INCLUDES) $(DEFINES) 

UTILS:
	gcc -c UTILS.c $(INCLUDES) $(DEFINES)

NET:
	gcc -c NET.c $(INCLUDES) $(DEFINES)

PCAP:
	gcc -c PCAP.c $(INCLUDES) $(DEFINES)

GEOPAG:
	gcc -c GEOPAG.c $(INCLUDES) $(DEFINES) -D DEF_GEOPAG_ROUTER
#ver el codigo fuente de GEOPAG.c para entender este DEF_GEOPAG_ROUTER
