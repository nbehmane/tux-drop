/**
* @file bluez.c
* @author Nima Behmanesh
* @brief This is a helper library that wraps up some bluez stuff.
*/
/** Includes **/
#include "bluez.h"
#include <stdio.h>

/** Macros **/
#define BLUEZ_ORG "org.bluez" /** Bluez Org **/
#define BLUEZ_ADAPTER_IFACE "org.bluez.Adapter1"
#define BLUEZ_ADAPTER_OBJECT "/org/bluez/hci0"
#define FREE_PROPERTIES "org.freedesktop.DBus.Properties"
#define FREE_OBJECT_MANAGER "org.freedesktop.DBus.ObjectManager"
#define AGENT_PATH "/org/bluez/AutoPinAgent"
#define MAX_IFACES 10
#define MAX_PROPS  20

/**
 * @brief Callback called when a new device (interface) is added.
 * @param sig A pointer to a GDBusConnection.
 * @param sender_name Path to the object/service that emitted this signal.
 * @param object_path Path of the object that received this signal.
 * @param interface Interface used to emit this signal.
 * @param signal_name The signal emitted.
 * @param parameters Parameters passed to this signal.
 * @param user_data User specific data.
 */
void bluez_iface_appeared(GDBusConnection *sig,
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

/**
 * @brief Prints out property values.
 * @param key Key name
 * @param value A pointer to a GVariant containing the value to be printed.
 */
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

/**
 * @brief A little more precise way of changing properties.
 * @param conn A pointer to a GDBusConnection.
 * @param obj_path Object path of the Bluetooth controller.
 * @param method A method, either "GetAll", or "Get"
 * @return A GVariant containing the properties.
 */
GVariant* bluez_adapter_properties_call(GDBusConnection *conn,
                              const char *obj_path, // This is currently hardcoded. This will need to change.
                              const char *method)
{
    GError *error = NULL;
    GVariant *response = NULL;

    /**
     * The available methods are
     * 1. GetAll
     * 2. Get
     */

    response =  g_dbus_connection_call_sync(
            conn,
            BLUEZ_ORG,
            BLUEZ_ADAPTER_OBJECT, // Will be replaced with obj_path
            "org.freedesktop.DBus.Properties",
            method,
            g_variant_new("(s)", BLUEZ_ADAPTER_IFACE),
            G_VARIANT_TYPE("(a{sv})"),
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            NULL,
            &error);

    // Check if there was an error.
    g_assert_no_error(error);

    /******** CLEANUP ********/
    //g_variant_unref(response);
    return response;

}
/**
 * @brief DEPRECATED DO NOT USE.
 * @param conn
 * @param method
 * @return
 */
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

/**
 * @brief When the adapter changes, this callback function is called.
 * @param conn A pointer to a GDBusConnection.
 * @param sender Path to the object/service that emitted this signal.
 * @param path Path of the object that received this signal.
 * @param interface Interface used to emit this signal.
 * @param signal The signal emitted.
 * @param params Parameters passed to this signal.
 * @param userdata User specific data.
 */
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

/**
 * @brief Sets a property value for the HCI adapter.
 * @param conn A pointer to a GDBusConnection.
 * @param prop The name of the property to set.
 * @param value The value to set the property.
 * @return 0 if sucess, 1 otherwise.
 */
int bluez_adapter_set_property(GDBusConnection *conn, const char *prop, GVariant *value)
{
    //! GVariant pointer to hold result from bus call..
    GVariant *result;
    GError *error = NULL;

    //! "org.bluez" -> The service that contains the object.
    //! "/org/bluez/hci0" -> The object to call the method 'set' on.
    //! "org.freedesktop.DBus.Properties" -> Interface that describes the method 'set'.
    result = g_dbus_connection_call_sync(conn,
                                         "org.bluez",
                                         "/org/bluez/hci0",
                                         "org.freedesktop.DBus.Properties",
                                         "Set",
                                         //! Note: floating pointer, will be consumed.
                                         g_variant_new("(ssv)", "org.bluez.Adapter1", prop, value),
                                         NULL,
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if(error != NULL)
        return 1;

    //! Dereference the result, thus freeing it.
    g_variant_unref(result);
    return 0;
}

/**
 * @brief Prints out devices that were scanned using -s.
 * @param device_table A pointer to a GHashTable.
 */
void bluez_scan_print_devices(GHashTable *device_table)
{
    GHashTableIter iter;
    gpointer key, value;
    int i = 0;

    /** TODO: Check that device_table is of GHashTable type. **/
    g_hash_table_iter_init(&iter, device_table);
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        g_print("%d: %s\n", i, (gchar *) key);
        i++;
    }

}

/**
 * @brief Remove devices that have been scanned.
 * @param device_table A pointer to a GHashTable.
 */
void bluez_scan_remove_devices(GHashTable *device_table)
{
    /** TODO: Remove each entry key manually. This is the suggested way of doing it. **/
    g_hash_table_unref(device_table);
}

/**
 * @brief A timeout signal to exit the program.
 * @param loop A pointer to the g_main_loop.
 * @return returns False.
 */
gboolean bluez_scan_timeout_signal(gpointer loop)
{
    g_main_loop_quit(loop);
    return FALSE;
}

/**
 * @brief Registers the autopair agent.
 * @param conn Connection handle to dbus.
 */
int bluez_register_autopair_agent(GDBusConnection *conn)
{
    int rc;

    rc = bluez_agent_call_method("RegisterAgent", g_variant_new("(os)", AGENT_PATH, "NoInputNoOutput"), conn);
    if(rc)
        return 1;

    rc = bluez_agent_call_method("RequestDefaultAgent", g_variant_new("(o)", AGENT_PATH), conn);
    if(rc) {
        bluez_agent_call_method("UnregisterAgent", g_variant_new("(o)", AGENT_PATH), conn);
        return 1;
    }

    return 0;
}

/**
 * @brief Calls a given method with parameters to the agent manager.
 * @param method The method to be called.
 * @param param The parameters to be passed to the method.
 * @param conn Connection handle to dbus.
 */
int bluez_agent_call_method(const gchar *method, GVariant *param, GDBusConnection *conn)
{
    GVariant *result;
    GError *error = NULL;

    result = g_dbus_connection_call_sync(conn,
                                         "org.bluez",
                                         "/org/bluez",
                                         "org.bluez.AgentManager1",
                                         method,
                                         param,
                                         NULL,
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);
    if(error != NULL)
    {
        g_print("Register %s: %s\n", AGENT_PATH, error->message);
        return 1;
    }

    g_variant_unref(result);
    return 0;
}

void bluez_signal_pairing_props_changed(GDBusConnection *sig,
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

void bluez_signal_connection_props_changed(GDBusConnection *sig,
                                        const gchar *sender_name,
                                        const gchar *object_path,
                                        const gchar *interface,
                                        const gchar *signal_name,
                                        GVariant *parameters,
                                        gpointer user_data)
{
    // Debugging statement
    // 4D_F9_30_21_CD_7A
    //g_print("%s\n%s\n%s\n%s\n", sender_name, object_path, interface, signal_name);

    const gchar *signature = g_variant_get_type_string(parameters);
    GVariantIter *properties = NULL;
    GVariantIter *unknown = NULL;
    const char *iface;
    const char *key;
    GVariant *value = NULL;
    GError *error;

    if(g_strcmp0(signature, "(sa{sv}as)") != 0) {
        g_print("Invalid signature for %s: %s != %s", signal_name, signature, "(sa{sv}as)");
        goto done;
    }

    g_variant_get(parameters, "(&sa{sv}as)", &iface, &properties, &unknown);
    while(g_variant_iter_next(properties, "{&sv}", &key, &value))
    {
        //g_print("KEY: %s\n", key);
        if(!g_strcmp0(key, "Connected"))
        {
            if(!g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN))
            {
                g_print("Invalid argument type for %s: %s != %s", key,
                        g_variant_get_type_string(value), "b");
                goto done;
            }
            g_print("Connection Status: %s\n", g_variant_get_boolean(value) ? "Connected" : "Disconnected");
            if (g_variant_get_boolean(value) == 0)
            {
                g_print("Disconnecting and quitting...\n");
                exit(EXIT_SUCCESS);
            }
            else
            {
                // This is where we will register the OBEX service.
                g_print("Registering OBEX protocol.\n");
                g_print("%d\n", g_dbus_connection_is_closed(user_data));

                GVariant *response = g_dbus_connection_call_sync(
                        user_data,
                        "org.bluez.obex",
                        "/org/bluez/obex",
                        "org.bluez.obex.Client1",
                        "CreateSession",
                        g_variant_new("(s{sv})", "5C:87:30:66:F4:35", "Target", g_variant_new("s", "ftp")),
                        G_VARIANT_TYPE("(o)"),
                        G_DBUS_CALL_FLAGS_NONE,
                        -1,
                        NULL,
                        &error);

                if (response == NULL)
                    g_print("FATAL Error\n");

                if (error != NULL)
                    g_error(error->message);
            }

        }
        if(!g_strcmp0(key, "Alias"))
        {
            if(!g_variant_is_of_type(value, G_VARIANT_TYPE_STRING))
            {
                g_print("Invalid argument type for %s: %s != %s", key,
                        g_variant_get_type_string(value), "s");
                goto done;
            }
            g_print("ALIAS: %s\n", g_variant_get_string(value, NULL));
        }

        if(!g_strcmp0(key, "Modalias"))
        {
            if(!g_variant_is_of_type(value, G_VARIANT_TYPE_STRING))
            {
                g_print("Invalid argument type for %s: %s != %s", key,
                        g_variant_get_type_string(value), "s");
                goto done;
            }
            g_print("Modalias: %s\n", g_variant_get_string(value, NULL));
        }
    }
    done:
    if(properties != NULL)
        g_variant_iter_free(properties);
    if(value != NULL)
        g_variant_unref(value);

    return;
}

/**
 * @brief Prints out devices that were scanned using -s.
 * @param device_table
 */
char* bluez_choose_device(GHashTable *device_table)
{
    GHashTableIter iter;
    gpointer key, value;
    int i = 0;
    /** TODO: Check that device_table is of GHashTable type. **/

    g_print("Choose device by inputting device index: ");
    int device_chosen = 0;
    scanf("%d", &device_chosen);


    g_hash_table_iter_init(&iter, device_table);
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        if (i == device_chosen)
        {
            g_print("%d: %s\n", i, (gchar *) key);
            return key;
        }
        i += 1;
    }
    char *str = malloc(sizeof(char) * 40);
    str = "/org/bluez/hci0/dev_5C_87_30_66_F4_35"; // Debug statement get rid of this.
    return str;
}