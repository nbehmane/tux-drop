/**
* @file bluez.c
* @author Nima Behmanesh
* @brief This is a helper library that wraps up some bluez stuff.
*/
/** Includes **/
#include "bluez-internal.h"

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
 * @brief Calls .
 *
 * @return
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
     * 3. Set
     */

    response =  g_dbus_connection_call_sync(conn,
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
    g_variant_unref(response);
}

/**
 * @brief Calls methods on the Adapter1 object.
 *
 * @return
 */
int bluez_adapter_method_call(GDBusConnection *conn,
                              const char *obj_path,
                              const char *method)
{
    /**
     * TODO:
     *  1. GetDiscoveryFilters() -> {sv}
     *  2. RemoveDevice(obj_path) -> ()
     *  3. SetDiscoveryFilter({sv}) -> ()
     *  4. StartDiscovery()
     *  5. StopDiscovery()
     */
     ;
}