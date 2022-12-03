#include "scan.h"

void scan_print_devices(GHashTable *device_table)
{
   GHashTableIter iter;
   gpointer key, value;
   /** TODO: Check that device_table is of GHashTable type. **/

   g_hash_table_iter_init(&iter, device_table);
   while (g_hash_table_iter_next(&iter, &key, &value))
      g_print("Scanned Device: %s\n", (gchar *)key);
}

void scan_remove_devices(GHashTable *device_table)
{
   /** TODO: Remove each entry key manually. This is the suggested way of doing it. **/
   g_hash_table_unref(device_table);
}

// GVariant *scan_select_device(GHashTable *device_table)
