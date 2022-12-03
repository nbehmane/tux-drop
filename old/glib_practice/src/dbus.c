#include "dbus.h"

void dbus_print_message_itr(GVariant *resp)
{
   gsize num_items = -1;
   gboolean is_container = 0;
   const gchar *type_sig = NULL;
   GVariant *value = NULL;
	const gchar *prop = NULL;
	GVariantIter iter; 

   // Grab the type sig.
   type_sig = g_variant_get_type_string(resp);
   g_print("Type Signature: %s\n", type_sig);

   // Check if its a container.
   is_container = g_variant_is_container(resp);
   g_print("Container Status: %d\n", is_container);

   // Grab the number of children in the container.
   num_items = g_variant_n_children(resp);
   g_print("Number of Children: %ld\n", num_items);

   // Get the first child value (theres only one.)
   value = g_variant_get_child_value(resp, 0);

   // Verify that its an array of strings.
   type_sig = g_variant_get_type_string(value);
   g_print("Type Signature of Child: %s\n", type_sig);

	g_variant_iter_init(&iter, value);
	while(g_variant_iter_next(&iter, "s", &prop))
      g_print("\t%s\n", prop);
}


