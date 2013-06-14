#include <glib.h>

/**
typedef of data structure
*/
typedef struct register_attributes_s
{
  int a;
  int b;
} register_attributes_t;


void register_attributes_print (register_attributes_t x)
{
	g_printf("%d, %d",x.a, x.b);
}


void test_register_attributes_hashtable()
{
  GHashTable *data=NULL;
  register_attributes_t one,two,three;
  int key1 = 23, key2 = 23;

  data = g_hash_table_new (g_int_hash, g_int_equal);
  //data = g_hash_table_new (register_attributes_t_hash, register_attributes_t_equal);
  g_assert (g_hash_table_size(data) == 0);
  one.a=13;
  one.b=31;
      
  two.a=0;
  two.b=-1;
   
  three.a=2145;
  three.b=31241;

   g_hash_table_insert(data,"1",&one);
   g_hash_table_insert(data,"2",&two);
   g_hash_table_insert(data,"3",&three);

   g_assert (g_hash_table_lookup(data,"1")!=NULL);
   g_assert (g_hash_table_lookup(data,"4")==NULL);
   g_assert (g_hash_table_size(data) == 3);

   g_hash_table_remove(data,"2");
   g_assert (g_hash_table_size(data) == 2);
   g_hash_table_insert(data,"22",&two);
   g_hash_table_insert(data,"2",&two);
   g_assert (g_hash_table_size(data) == 4);
   g_hash_table_remove_all(data);
   g_assert (g_hash_table_size(data) == 0);

   g_hash_table_insert(data,&key1,&three);
   g_assert (g_hash_table_lookup(data,&key2)!=NULL);
   
   g_hash_table_destroy (data);
}

