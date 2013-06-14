
all: GUIclient GUImaster
	strip GUIclient
	strip GUImaster

GUIclient: GUIclient.o ../phonemgr/libphonemgr.a
	gcc -g -O2 -o GUIclient GUIclient.o\
../../src/phonemgr/libphonemgr.a \
/usr/lib/fltk-1.1/libfltk.a \
-lXft -lm -lXext -lX11 -lsupc++\
/usr/local/lib/libjvoip.a \
/usr/local/lib/libjthread.a \
/usr/local/lib/libjrtp.a  \
/usr/local/sipx/lib/libsipXtack.a\
/usr/local/sipx/lib/libsipXport.a\
/usr/lib/libssl.a\
/usr/lib/libcrypto.a\
/usr/lib/libgthread-2.0.a \
/usr/lib/libglib-2.0.a\
/usr/lib/libpcre.a\
-lpthread \
-lrt \
-lresolv \
-ldl\
-lz\
/usr/lib/gcc-lib/i686-pc-linux-gnu/3.3.5-20050130/libstdc++.a 
GUImaster: GUImaster.o ../phonemgr/libphonemgr.a
	gcc -g -O2 -o GUImaster GUImaster.o\
../../src/phonemgr/libphonemgr.a \
/usr/lib/fltk-1.1/libfltk.a \
-lXft -lm -lXext -lX11 -lsupc++\
/usr/local/lib/libjvoip.a \
/usr/local/lib/libjthread.a \
/usr/local/lib/libjrtp.a  \
/usr/local/sipx/lib/libsipXtack.a\
/usr/local/sipx/lib/libsipXport.a\
/usr/lib/libssl.a\
/usr/lib/libcrypto.a\
/usr/lib/libgthread-2.0.a \
/usr/lib/libglib-2.0.a\
/usr/lib/libpcre.a\
-lpthread \
-lrt \
-lresolv \
-ldl\
-lz\
/usr/lib/gcc-lib/i686-pc-linux-gnu/3.3.5-20050130/libstdc++.a 

GUIclient.o:	GUIclient.cpp
	g++ -DSIPXTAPI_EXCLUDE -DHAVE_CONFIG_H -I. -I/home/marcos/Proyecto/source/common/iptrunking/src/GUIclient -I../.. -I/home/marcos/Proyecto/source/common/iptrunking/src/phonemgr -I/usr/include/freetype2 -I/usr/include/fltk-1.1 -I/usr/lib/glib-2.0/include -I/usr/local/sipx/include -I/usr/include/glib-2.0 -I/usr/local/include/jthread -I/usr/local/include/jvoiplib     -g -O2  -c -o GUIclient.o GUIclient.cpp; 

GUImaster.o:	GUImaster.cpp
	g++ -DSIPXTAPI_EXCLUDE -DHAVE_CONFIG_H -I. -I/home/marcos/Proyecto/source/common/iptrunking/src/GUIclient -I../.. -I/home/marcos/Proyecto/source/common/iptrunking/src/phonemgr -I/usr/include/freetype2 -I/usr/include/fltk-1.1 -I/usr/lib/glib-2.0/include -I/usr/local/sipx/include -I/usr/include/glib-2.0 -I/usr/local/include/jthread -I/usr/local/include/jvoiplib     -g -O2  -c -o GUImaster.o GUImaster.cpp; 

clean:
	rm -f *.o GUImaster GUIclient





#/bin/sh ../../libtool --tag=CXX --mode=link g++  -g -O2   -o GUIclient -L/usr/local/sipx/lib GUIclient.o ../../src/phonemgr/libphonemgr.a -lXft -lpthread -lm -lXext -lX11 -lsupc++ /usr/lib/fltk-1.1/libfltk.a -ljthread -ljvoip -lsipXport -lsipXtack
#g++ -g -O2 -o GUIclient GUIclient.o  -L/usr/local/sipx/lib ../../src/phonemgr/libphonemgr.a -lXft -lm -lXext -lX11 -lsupc++ /usr/lib/fltk-1.1/libfltk.a -ljthread -ljvoip -L/usr/lib -L/usr/i686-pc-linux-gnu/bin -L/usr/i686-pc-linux-gnu/lib -L/usr/lib/gcc-lib/i686-pc-linux-gnu/../../../i686-pc-linux-gnu/lib /usr/local/sipx/lib/libsipXtack.so -L/usr/local/lib -L/test/sipxunit /usr/local/sipx/lib/libsipXport.so -lssl -lcrypto /usr/lib/libgthread-2.0.so /usr/lib/libglib-2.0.so -lrt -lpthread -lresolv -ldl /usr/lib/gcc-lib/i686-pc-linux-gnu/3.3.5-20050130/libstdc++.so /usr/lib/libpcre.so -Wl,--rpath -Wl,/usr/local/sipx/lib -Wl,--rpath -Wl,/usr/lib/gcc-lib/i686-pc-linux-gnu/3.3.5-20050130 -Wl,--rpath -Wl,/usr/local/sipx/lib -Wl,--rpath -Wl,/usr/lib/gcc-lib/i686-pc-linux-gnu/3.3.5-20050130


