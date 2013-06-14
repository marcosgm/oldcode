/*
 * Copyright (C) Marcos Garcia 2008 <marcos.gm@gmail.com>
*/

/**
Considere un archivo de texto ”input.txt” con un n ́ mero variable de l ́neas, cada l ́nea conteniendo 3
enteros, separados por un espacio.
123 545 645
234 323 454
...
El primer entero identifica el n ́ mero de registro, y los dos siguientes son atributos del mismo, atributo ”a”
y atributo ”b”. Tanto el n ́ mero de registro como los atributos se codifican con un entero sin signo de 32
bits. El programa final debe:
   1. Al ser ejecutado, abrir el archivo en modo lectura, recorrer cada una de las l ́neas y construir en
       memoria una estructura de datos (e.g. mapa, o tabla hash, en su defecto una lista doblemente en-
       lazada) que contenga todos los registros le ́dos y que permita la b ́ squeda r ́ pida por identificador de
       registro.
   2. Una vez le ́do el archivo, iterar cada uno de los registros desde la estructura de datos propuesta y
       para cada registro enviar un datagrama UDP a la direcci ́ n IP 10.10.1.1, puerto 2020, conteniendo el
       identificador de registro y los dos atributos.

*/

#include <stdio.h>
#include <glib.h>
#include "attribute.h"
#include <sys/socket.h>
#include <netinet/in.h>
    
/**
validates argc and argv to assure file is executed with the desired parameters
*/
gboolean  validate_args (int argc, char** argv)
{
	if (argc == 2) return TRUE;
	else
	{
		g_printf ("Usage: %s input.txt\n", argv[0]);
		return FALSE;
	}
}

/**
this just frees the key and the value: http://www-128.ibm.com/developerworks/linux/library/l-glib2.html 
*/
static void free_a_hash_table_entry(gpointer key, gpointer value, gpointer user_data)
{
	g_free(key);
	g_free(value);
}

static void print_hash_table_entry(gpointer key, gpointer value, gpointer user_data)
{
	register_attributes_t *val = (register_attributes_t *) value;
	g_printf("%d: %d,%d\n", *((int *)key), val->a, val->b);
}

/**
PART I
filename is a string with the name of the input file, must have valid data (3 integers, see header)
data is the contanier where we will insert the data (MUST BE CREATED)
returns: TRUE if file's information is correctly inserted into "data" (0 or more values)
*/
gboolean read_file (const gchar *filename, GHashTable *data)
{
   GIOChannel *file = NULL;
   GError *err = NULL;
   GString *line_str = NULL;
   int line_size=0;
   int i=0;
   gboolean quit_loop=FALSE;

   
   /* OPEN THE FILE */
   file = g_io_channel_new_file (filename, "r", &err);
   if (file==NULL)
   { 
     g_printf ("File not found or error ocurred when opening\n"); 
     return FALSE;
   }
    /* PARSE FILE CONTENT: loop iterating over stream of strings delimited by "\n", until EOF or incorrect line is found*/
   line_str = g_string_sized_new (15); /*Creates the buffer with initial size 15 chars*/
   g_assert(line_str!=NULL);
   
   for (i=0, quit_loop=FALSE; quit_loop==FALSE; i++)
   {
     //g_printf ("loop %d; ",i);
     if (g_io_channel_read_line_string (file,line_str, &line_size, &err) == G_IO_STATUS_NORMAL)
     {
	//Parse the line into it's 3 components in the HashTable
	gint key=0;
	register_attributes_t value;	
	gchar **line_data=NULL;
	
	g_return_if_fail (line_str->len >=0); //if an empty line is read, quit the IF block
	
	/* line_data will hold an array of strings, splitted from origininal line_str */
	//g_printf ("Line read: %d bytes: %s", line_size, *line_str);
	line_data = g_strsplit (line_str->str, " ", 3); //3 tokens per line
	g_return_if_fail(line_data!=NULL);
	
	//g_printf ("in this line %d: %s, %s, %s\n", i, line_data[0],line_data[1],line_data[2]);
	
	/*Convert string into values (see header for format definition), storing into Hashtable DATA:
	El primer entero identifica el n ́ mero de registro, y los dos siguientes son atributos del mismo, atributo ”a”
	y atributo ”b”. Tanto el n ́ mero de registro como los atributos se codifican con un entero sin signo de 32
	bits.*/
	key = atoi(line_data[0]);
	value.a = atoi(line_data[1]); //atoi= int32
	value.b = atoi(line_data[2]);
	/*HashTables only stores pointers, so memory allocation is needed. Later, when deleting HashTable, free() must be called foreach item*/
	g_hash_table_insert(data, g_memdup(&key,sizeof(key)), g_memdup(&value,sizeof(value)));
	
	g_strfreev(line_data);
     }
     else if (g_io_channel_read_line_string (file,line_str, &line_size, &err) == G_IO_STATUS_EOF)
     {
       //End of file
       quit_loop=TRUE;
     }
     else  
     {
       //Error reading a line
       g_printf ("Error reading a line\n");
       quit_loop=TRUE;
     }
   }
   g_io_channel_close(file);
   g_io_channel_unref (file); /* need to unref as Valgrind detect's memory leakage*/
   g_string_free (line_str,FALSE);
   return TRUE;
}

/**
return FALSE if any operation fails
Reference for UDP Sockets: http://www.fortunecity.com/skyscraper/arpanet/6/cc.htm
*/
gboolean send_UDP (const gchar *ip, gint port, const gint *key, const register_attributes_t *val)
{
	gint socket_fd =0;
	struct sockaddr_in sin;
	gint ret=0;
	GString *msg = NULL;
	
	/*Arguments check*/
	g_assert(ip!=NULL);
	g_assert(key!=NULL);
	g_assert(val!=NULL);
	
	/*create a socket and fullfill SockAddr structure, checking for errors*/
	socket_fd = socket (AF_INET, SOCK_DGRAM, 0);
	g_return_val_if_fail (socket_fd != -1, FALSE); //error-checking
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = inet_addr(ip);
	g_return_val_if_fail (sin.sin_addr.s_addr != INADDR_NONE, FALSE); //error-checking
 
	/*bind the socket and check for errors*/
	ret = bind(socket_fd, (struct sockaddr *)&sin, sizeof(sin));
	//g_return_val_if_fail (ret!=-1, FALSE); //ret=-1 is error
	
	/*Build the message. If key=3 and attrib={13,51}, message will be "3:13,51"*/
	msg = g_string_new ("");
	g_assert(msg!=NULL); 
	g_string_printf (msg, "%d:%d,%d", *key, val->a, val->b);
	g_assert(msg!=NULL); 
	
	/*Send the message via UDP */	
	ret = sendto(socket_fd, msg->str, (msg->len)+1, 0, (struct sockaddr *)&sin, sizeof(sin)); 
	//len is string size without \0, so +1 must be added
	g_return_val_if_fail (ret!=-1, FALSE);
	
	close (socket_fd);
	
	g_string_free(msg, FALSE);
}
/** 
PART II
key,value are pointers to a hashtable entry. This function is executed into hashtable_foreach()
SCALABILITY AND PERFORMANCE ISSUE: socket is created and deleted foreach entry. Quick solution: set socket file descriptor in global space, so opening, binding and closing is done outside hashtable_foreach loop.
*/
static void send_hash_table_entry_to_UDP(gpointer key, gpointer value, gpointer user_data)
{
	g_assert(key!=NULL);
	g_assert(value!=NULL);
	register_attributes_t *val = (register_attributes_t *) value;
	send_UDP ("10.10.1.1", 2020, (int *) key, (register_attributes_t *)val);
}

gint main(int argc, char** argv)
{
	GHashTable *data=NULL;
	register_attributes_t *value;
	int key=0;
	
	if (!validate_args(argc,argv)) return -1;
	
	data = g_hash_table_new (g_int_hash, g_int_equal);
	/*data = g_hash_table_new (register_attributes_t_hash, register_attributes_t_equal);*/
	g_assert(data!=NULL);		
	
	if (!read_file(argv[1], data)) return -1;

	key=714;
	value = g_hash_table_lookup(data,&key);
	if (value!=NULL)
		g_printf ("valor %d: %d, %d\n", key, value->a, value->b);
	else
		g_printf ("valor %d no encontrado \n", key);	
	
	g_hash_table_foreach (data, print_hash_table_entry, NULL);
	g_hash_table_foreach (data, send_hash_table_entry_to_UDP, NULL);
	
	/* DE-ALLOCATE HASHTABLE AND ALL ITS ITEMS: foreach item in hashtable: free(item)*/
	g_hash_table_foreach (data, free_a_hash_table_entry, NULL);
	g_hash_table_destroy (data);
	
	test_register_attributes_hashtable();
	
	return (0);
}
