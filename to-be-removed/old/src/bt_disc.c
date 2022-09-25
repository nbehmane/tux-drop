#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <glib.h>
#include <gio/gio.h>

#include "scan.h"
#include "bluez.h"

GDBusConnection *con;
GMainLoop *loop;

static int loop_done = 0;

void on_signal_received(int signo)
{
  if (signo == SIGQUIT || signo == SIGINT)
  {
    /* terminate */
    loop_done = 1;
  }
}

static gboolean on_loop_idle(gpointer udata)
{
  /* triggered via SIGQUIT */
  if (loop_done)
  {
    g_main_loop_quit(loop);
    return G_SOURCE_REMOVE;
  }

  return G_SOURCE_CONTINUE;
}


int main(void)
{
	int rc;
	guint prop_changed;
	guint iface_added;
   GHashTable *devices = g_hash_table_new(g_str_hash, g_str_equal);
   gpointer *tbl_pointer = (gpointer *)devices;

   /** Set up the signals to quit... **/
   signal(SIGINT, on_signal_received);
   signal(SIGQUIT, on_signal_received);

   /** Establish connection with GDBUS **/
	con = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
	if(con == NULL) {
		g_error("Not able to get connection to system bus\n");
		return 1;
	}

	loop = g_main_loop_new(NULL, FALSE);
   

	prop_changed = g_dbus_connection_signal_subscribe(con,
						"org.bluez",
						"org.freedesktop.DBus.Properties",
						"PropertiesChanged",
						NULL,
						"org.bluez.Adapter1",
						G_DBUS_SIGNAL_FLAGS_NONE,
						bluez_signal_adapter_changed,
						NULL,
						NULL);

	iface_added = g_dbus_connection_signal_subscribe(con,
							"org.bluez",
							"org.freedesktop.DBus.ObjectManager",
							"InterfacesAdded",
							NULL,
							NULL,
							G_DBUS_SIGNAL_FLAGS_NONE,
							bluez_device_appeared,
							tbl_pointer,
							NULL);


   /** Turn on the adapter. **/
	rc = bluez_adapter_set_property(con, "Powered", g_variant_new("b", TRUE));
	if(rc) {
		g_print("Not able to enable the adapter\n");
		goto fail;
	}

   /** Start discovery. **/
	rc = bluez_adapter_call_method(con, "StartDiscovery");
	if(rc) {
		g_print("Not able to scan for new devices\n");
		goto fail;
	}

   g_idle_add(on_loop_idle, NULL);
	g_main_loop_run(loop);

   g_print("Stopping discovery...");
	rc = bluez_adapter_call_method(con, "StopDiscovery");
	if(rc)
		g_print("Not able to stop scanning\n");
	g_usleep(100);
   g_print("ok\n");

   g_print("Turning adapater off...");
	rc = bluez_adapter_set_property(con, "Powered", g_variant_new("b", FALSE));
	if(rc)
		g_print("Not able to disable the adapter\n");
   g_print("ok\n");

   bluez_adapter_call_method_filters(con);
   scan_print_devices(devices);

fail:
	g_dbus_connection_signal_unsubscribe(con, prop_changed);
	g_dbus_connection_signal_unsubscribe(con, iface_added);
   g_main_loop_unref(loop);
   g_hash_table_unref(devices);

	g_object_unref(con);
	return 0;
}
