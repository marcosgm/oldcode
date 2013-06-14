INCLUDES = -I$(top_srcdir)/src/phonemgr -I/usr/include/freetype2 \
	-I/usr/include/fltk-1.1 -I/usr/lib/glib-2.0/include -I/usr/local/sipx/include \
	-I/usr/include/glib-2.0 -I/usr/local/include/jthread -I/usr/local/include/jvoiplib \
	$(all_includes)
METASOURCES = AUTO
bin_PROGRAMS = GUIclient
GUIclient_SOURCES = GUIclient.cpp GUImaster.cpp
GUIclient_LDADD = $(top_builddir)/src/phonemgr/libphonemgr.a -lXft -lpthread \
	-lm -lXext -lX11 -lsupc++ /usr/lib/fltk-1.1/libfltk.a -ljthread -ljvoip \
	-lsipXport -lsipXtack
GUIclient_LDFLAGS = -L/usr/local/sipx/lib
noinst_HEADERS = GUIclient.h
