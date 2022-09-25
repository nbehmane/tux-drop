#ifndef BLUEZ_H
#define BLUEZ_H
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <glib.h>
#include <gio/gio.h>
#define BT_ADDRESS_STRING_SIZE 18

void bluez_property_value(const gchar *key, GVariant *value);
int bluez_adapter_set_property(GDBusConnection *conn, const char *prop, GVariant *value);
void bluez_device_appeared(GDBusConnection *sig,
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

int bluez_adapter_call_method(GDBusConnection *conn, const char *method);
int bluez_adapter_call_method_filters(GDBusConnection *conn);

#endif // BLUEZ_H
