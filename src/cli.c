#include "cli-internal.h"

void usage()
{
    fprintf(stderr, "\t-s x Scan for x seconds.\n");
    fprintf(stderr, "\t-l List current devices.\n");
    fprintf(stderr, "\t-e List debug info.\n");
    fprintf(stderr, "\t-c Connect to a device.\n");
    fprintf(stderr, "\t-d Disconnect from a device.\n");
    fprintf(stderr, "\t-r Remove a device.\n");
    fprintf(stderr, "\t-p Pair a device.\n");
    fprintf(stderr, "\t-S Set device adapter settings.\n");
    fprintf(stderr, "\t-G Get device adapter settings.\n");
}

static struct option long_options[] =
        {
                {"help", no_argument, 0, 'h'},
                {"scan", required_argument, 0, 's'},
                {"debug", no_argument, 0, 'e'},
                {"list", no_argument, 0, 'l'},
                {"pair", no_argument, 0, 'p'},
                {"connect", no_argument, 0, 'c'},
                {"remove", no_argument, 0, 'r'},
                {"quit", no_argument, 0, 'q'},
                {"Set", no_argument, 0, 'S'},
                {"Get", no_argument, 0, 'G'},
                {0, 0, 0, 0}
        };


/*** Main Loop Start ***/
GDBusConnection *conn;
GMainLoop *loop;
// Loops

static gboolean scan_timeout_signal(gpointer loop)
{
    g_main_loop_quit(loop);
    return FALSE;
}

/*** Main Loop End ***/

/**
 * @brief Runs the CLI. Meant to be in the main loop.
 *
 * @param argc Count from main.
 * @param argv Argv from main.
 * @return
 */
int cli_run(int argc, char **argv)
{
    int c = 0;
    int time_seconds = 0;
    /*** Check DBUS connection ***/
    conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
    if (conn == NULL)
    {
        g_error("Not able to connect to system bus.\n");
        return -1;
    }
    /*** Check DBUS connection end ***/

    /*** Hashtable setup start ***/
    GHashTable *devices = g_hash_table_new(g_str_hash, g_str_equal);
    gpointer *tbl_pointer = (gpointer *)devices;
    /*** Hashtable setup end ***/

    while (1)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "hs:elpcdrqSG", long_options, &option_index);
        if (c == -1)
            break;
        switch (c)
        {
            case 'h':
                usage();
                break;
            case 's': // Scan Time
                time_seconds = atoi(optarg); // Time to scan
                guint adapter_props_change = 0;
                guint adapter_iface_added = 0;
                guint return_code = 0;
                loop = g_main_loop_new(NULL, FALSE);

                adapter_props_change = g_dbus_connection_signal_subscribe(
                        conn,
                        "org.bluez",
                        "org.freedesktop.DBus.Properties",
                        "PropertiesChange",
                        NULL,
                        "org.bluez.Adapter1",
                        G_DBUS_SIGNAL_FLAGS_NONE,
                        bluez_signal_adapter_changed,
                        NULL,
                        NULL
                        );

                adapter_iface_added = g_dbus_connection_signal_subscribe(
                        conn,
                        "org.bluez",
                        "org.freedesktop.DBus.ObjectManager",
                        "InterfacesAdded",
                        NULL,
                        NULL,
                        G_DBUS_SIGNAL_FLAGS_NONE,
                        bluez_iface_appeared,
                        tbl_pointer,
                        NULL
                        );

                return_code = bluez_adapter_set_property(
                        conn,
                        "Powered",
                        g_variant_new("b", TRUE)
                        );
                if (return_code)
                {
                    g_print("Not able to enable adapter\n");
                    goto scanfail;
                }

                return_code = bluez_adapter_call_method(conn, "StartDiscovery");
                if (return_code)
                {
                    g_print("Not able to scan for new devices\n");
                    goto scanfail;
                }

                g_timeout_add_seconds(time_seconds, scan_timeout_signal, loop);
                g_main_loop_run(loop); // Blocking call (:

                g_print("Stopping discovery...\n");
                return_code = bluez_adapter_call_method(conn, "StopDiscovery");
                if (return_code)
                {
                    g_print("Not able to stop scanning\n");
                    goto scanfail;
                }
                g_usleep(100);

            scanfail: // Why a goto? Well why not...
                g_dbus_connection_signal_unsubscribe(conn, adapter_props_change);
                g_dbus_connection_signal_unsubscribe(conn, adapter_iface_added);
                g_main_loop_unref(loop);
                break;
            case 'e': // Debug List devices
                g_print("Debug List Devices\n");
                break;
            case 'l': // List devices
                g_print("List Devices\n");
                bluez_scan_print_devices(devices);
                break;
            case 'p': // Pairing
                g_print("Pairing\n");
                break;
            case 'c': // Connecting
                g_print("Connecting\n");
                break;
            case 'd': // Disconnect
                g_print("Disconnect\n");
                break;
            case 'r': // Remove device
                g_print("Remove Device\n");
                break;
            case 'q': // Quit
                return -1;
            case 'S': //Set
                g_print("Setting device properties\n");
                break;
            case 'G': //Get
                g_print("Device Properties\n");

                GVariant *property_container = bluez_adapter_properties_call(conn, NULL, "GetAll");
                if (g_variant_is_container(property_container))
                {
                    GVariant * properties = g_variant_get_child_value(property_container, 0);
                    g_print("%s\n", g_variant_print_string(properties, NULL, 0)->str);
                }
                break;
            default:
                break;
        }
    }
}