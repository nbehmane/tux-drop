#include "bluez.h"
#include "dbus.h"

void bluez_property_value(const gchar *key, GVariant *value)
{
	const gchar *type = g_variant_get_type_string(value);

	g_print("\t%s : ", key);
	switch(*type) {
		case 'o':
		case 's':
			g_print("%s\n", g_variant_get_string(value, NULL));
			break;
		case 'b':
			g_print("%d\n", g_variant_get_boolean(value));
			break;
		case 'u':
			g_print("%d\n", g_variant_get_uint32(value));
			break;
		case 'a':
		/* TODO Handling only 'as', but not array of dicts */
			if(g_strcmp0(type, "as"))
				break;
			g_print("\n");
			const gchar *uuid;
			GVariantIter i;
			g_variant_iter_init(&i, value);
			while(g_variant_iter_next(&i, "s", &uuid))
				g_print("\t\t%s\n", uuid);
			break;
		default:
			g_print("Other\n");
			break;
	}
}

int bluez_adapter_set_property(GDBusConnection *conn, const char *prop, GVariant *value)
{
	GVariant *result;
	GError *error = NULL;

	result = g_dbus_connection_call_sync(conn,
					     "org.bluez",
					     "/org/bluez/hci0",
					     "org.freedesktop.DBus.Properties",
					     "Set",
					     g_variant_new("(ssv)", "org.bluez.Adapter1", prop, value),
					     NULL,
					     G_DBUS_CALL_FLAGS_NONE,
					     -1,
					     NULL,
					     &error);
	if(error != NULL)
		return 1;

	g_variant_unref(result);
	return 0;
}


void bluez_device_appeared(GDBusConnection *sig,
				const gchar *sender_name,
				const gchar *object_path,
				const gchar *interface,
				const gchar *signal_name,
				GVariant *parameters,
				gpointer user_data)
{
	(void)sig;
	(void)sender_name;
	(void)object_path;
	(void)interface;
	(void)signal_name;

   GHashTable *tbl = (GHashTable *)user_data;
	GVariantIter *interfaces;
	const char *object;
	const gchar *interface_name;
	GVariant *properties;

	g_variant_get(parameters, "(&oa{sa{sv}})", &object, &interfaces);
	while(g_variant_iter_next(interfaces, "{&s@a{sv}}", &interface_name, &properties)) {
		if(g_strstr_len(g_ascii_strdown(interface_name, -1), -1, "device")) {
			g_print("[ %s ]\n", object);
         if (!g_hash_table_contains(tbl, (gconstpointer) object)) 
         {
			   g_print("[ adding %s ]\n", object);
            g_hash_table_add(tbl, (gpointer)object);
         }
			const gchar *property_name;
			GVariantIter i;
			GVariant *prop_val;
			g_variant_iter_init(&i, properties);
			while(g_variant_iter_next(&i, "{&sv}", &property_name, &prop_val))
				bluez_property_value(property_name, prop_val);
			g_variant_unref(prop_val);
		}
		g_variant_unref(properties);
	}
	return;
}

void bluez_signal_adapter_changed(GDBusConnection *conn,
					const gchar *sender,
					const gchar *path,
					const gchar *interface,
					const gchar *signal,
					GVariant *params,
					void *userdata)
{
	(void)conn;
	(void)sender;
	(void)path;
	(void)interface;
	(void)userdata;

	GVariantIter *properties = NULL;
	GVariantIter *unknown = NULL;
	const char *iface;
	const char *key;
	GVariant *value = NULL;
	const gchar *signature = g_variant_get_type_string(params);

	if(g_strcmp0(signature, "(sa{sv}as)") != 0) {
		g_print("Invalid signature for %s: %s != %s", signal, signature, "(sa{sv}as)");
		goto done;
	}

	g_variant_get(params, "(&sa{sv}as)", &iface, &properties, &unknown);
	while(g_variant_iter_next(properties, "{&sv}", &key, &value)) {
		if(!g_strcmp0(key, "Powered")) {
			if(!g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN)) {
				g_print("Invalid argument type for %s: %s != %s", key,
						g_variant_get_type_string(value), "b");
				goto done;
			}
			g_print("Adapter is Powered \"%s\"\n", g_variant_get_boolean(value) ? "on" : "off");
		}
		if(!g_strcmp0(key, "Discovering")) {
			if(!g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN)) {
				g_print("Invalid argument type for %s: %s != %s", key,
						g_variant_get_type_string(value), "b");
				goto done;
			}
			g_print("Adapter scan \"%s\"\n", g_variant_get_boolean(value) ? "on" : "off");
		}
	}
done:
	if(properties != NULL)
		g_variant_iter_free(properties);
	if(value != NULL)
		g_variant_unref(value);
}

int bluez_adapter_call_method(GDBusConnection *conn, const char *method)
{
	GVariant *result;
	GError *error = NULL;

	result = g_dbus_connection_call_sync(conn,
					     "org.bluez",
					/* TODO Find the adapter path runtime */
					     "/org/bluez/hci0",
					     "org.bluez.Adapter1",
					     method,
					     NULL,
					     NULL,
					     G_DBUS_CALL_FLAGS_NONE,
					     -1,
					     NULL,
					     &error);

	if(error != NULL)
		return 1;

	g_variant_unref(result);
	return 0;
}

int bluez_adapter_call_method_filters(GDBusConnection *conn)
{
	GVariant *result = NULL;
	GError *error = NULL;

	result = g_dbus_connection_call_sync(
                     conn,
					      "org.bluez",
					      /* TODO Find the adapter path runtime */
					      "/org/bluez/hci0",
					      "org.bluez.Adapter1",
					      "GetDiscoveryFilters",
					      NULL,
					      NULL,
					      G_DBUS_CALL_FLAGS_NONE,
					      -1,
					      NULL,
					      &error);
     

	if(error != NULL) {
      g_error(error->message);
		return 1;
   }
   
   dbus_print_message_itr(result);
   
	g_variant_unref(result);
	return 0;
}

/** int bluez_pair **/










