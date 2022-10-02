//
// Created by nima on 7/31/22.
//

#ifndef TUXDROP_BLUEZ_H
#define TUXDROP_BLUEZ_H
#include "bluez-internal.h"

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

#endif //TUXDROP_BLUEZ_H