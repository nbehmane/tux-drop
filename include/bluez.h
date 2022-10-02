//
// Created by nima on 7/31/22.
//

#ifndef TUXDROP_BLUEZ_H
#define TUXDROP_BLUEZ_H
#include <glib-2.0/glib.h>
#include <glib-2.0/gio/gio.h>

#define BLUEZ_ORG "org.bluez" /** Bluez Org **/
#define BLUEZ_ADAPTER_IFACE "org.bluez.Adapter1"
#define BLUEZ_ADAPTER_OBJECT "/org/bluez/hci0"
#define FREE_PROPERTIES "org.freedesktop.DBus.Properties"
#define FREE_OBJECT_MANAGER "org.freedesktop.DBus.ObjectManager"
#define AGENT_PATH "/org/bluez/AutoPinAgent"
#define MAX_IFACES 10
#define MAX_PROPS  20

void bluez_iface_appeared(GDBusConnection *sig,
                          const gchar *sender_name,
                          const gchar *object_path,
                          const gchar *interface,
                          const gchar *signal_name,
                          GVariant *parameters,
                          gpointer user_data);

void bluez_signal_adapter_changed(GDBusConnection *conn,
                                  const gchar *sender,
                                  const gchar *path,
                                  const gchar *interface,
                                  const gchar *signal,
                                  GVariant *params,
                                  void *userdata);

/**
 * @brief Verifies that there is a HCI controller.
 *
 * @return
 */
int bluez_verify_hci_controller();
GVariant* bluez_adapter_properties_call(GDBusConnection *conn,
                                  const char *obj_path,
                                  const char *method);
int bluez_adapter_set_property(GDBusConnection *conn, const char *prop, GVariant *value);
void bluez_property_value(const gchar *key, GVariant *value);
int bluez_adapter_call_method(GDBusConnection *conn, const char *method);
void bluez_scan_print_devices(GHashTable *device_table);
void bluez_scan_remove_devices(GHashTable *device_table);

gboolean bluez_scan_timeout_signal(gpointer loop);
int bluez_register_autopair_agent(GDBusConnection *conn);
int bluez_agent_call_method(const gchar *method, GVariant *param, GDBusConnection *conn);
void bluez_signal_pairing_props_changed(GDBusConnection *sig,
                                        const gchar *sender_name,
                                        const gchar *object_path,
                                        const gchar *interface,
                                        const gchar *signal_name,
                                        GVariant *parameters,
                                        gpointer user_data);

void bluez_signal_connection_props_changed(GDBusConnection *sig,
                                           const gchar *sender_name,
                                           const gchar *object_path,
                                           const gchar *interface,
                                           const gchar *signal_name,
                                           GVariant *parameters,
                                           gpointer user_data);

/**
 * @brief Prints out devices that were scanned using -s.
 * @param device_table
 */
char* bluez_choose_device(GHashTable *device_table);
#endif //TUXDROP_BLUEZ_H