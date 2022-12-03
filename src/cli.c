#include "cli.h"

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
GDBusConnection *session_conn;
GMainLoop *loop;
// Loops

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
    int return_code_cli = 0;
    GError *error = NULL;
    char *device_path = '\0'; // Need to get the object path somehow
    /*** Check DBUS connection ***/
    conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
    session_conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
    if (conn == NULL)
    {
        g_error("Not able to connect to system bus.\n");
        return -1;
    }
    if (session_conn == NULL)
    {
        g_error("Not able to connect to session bus.\n");
        return -1;
    }
    /*** Check DBUS connection end ***/

    /*** Hashtable setup start ***/
    GHashTable *devices = g_hash_table_new(g_str_hash, g_str_equal);
    gpointer *tbl_pointer = (gpointer *)devices;
    /*** Hashtable setup end ***/
    // Make the adapter pairable
    bluez_adapter_set_property(conn, "Pairable", g_variant_new_boolean(1));
    return_code_cli = bluez_register_autopair_agent(conn);
    if (return_code_cli)
    {
        g_print("Could not register autopair agent.\n");
        exit(EXIT_FAILURE);
    }


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
            case 's': // Scan Time - DONE
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

                g_timeout_add_seconds(time_seconds, bluez_scan_timeout_signal, loop);
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
                device_path = bluez_choose_device(devices);
                break;
            case 'l': // List devices - DONE
                g_print("List Devices\n");
                bluez_scan_print_devices(devices);
                break;
            case 'p': // Pairing
                g_print("Pairing\n");
                guint pairing_device_props_changed = 0;
                int pairing_timeout = 10;
                // TODO: ADD CODE TO SELECT DEVICE AND SET IT TO PATH
                device_path = bluez_choose_device(devices);

                pairing_device_props_changed = g_dbus_connection_signal_subscribe(
                        conn,
                       "org.bluez",
                       "org.freedesktop.DBus.Properties",
                       "PropertiesChanged",
                       device_path,
                       "org.bluez.Device1",
                       G_DBUS_SIGNAL_FLAGS_NONE,
                       bluez_signal_pairing_props_changed,
                       NULL,
                       NULL);

                g_dbus_connection_call_sync(
                        conn,
                       BLUEZ_ORG,
                       device_path,
                       "org.bluez.Device1",
                       "Pair",
                       NULL,
                       NULL,
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       &error);

                if (error != NULL)
                    g_error(error->message);

                loop = g_main_loop_new(NULL, FALSE);
                g_timeout_add_seconds(pairing_timeout, bluez_scan_timeout_signal, loop);
                g_main_loop_run(loop); // Blocking call (:

                g_dbus_connection_signal_unsubscribe(conn, pairing_device_props_changed);
                g_main_loop_unref(loop);
                g_print("Paired to %s\n", device_path);
                break;
            case 'c': // Connecting
                g_print("Connecting\n");
                // Print the devices that we can connect too.
                int connection_device_props_changed = 0;
                int connection_timeout = 10;

                device_path = bluez_choose_device(devices);

                connection_device_props_changed = g_dbus_connection_signal_subscribe(
                        conn,
                       "org.bluez",
                       "org.freedesktop.DBus.Properties",
                       "PropertiesChanged",
                       device_path,
                       "org.bluez.Device1",
                       G_DBUS_SIGNAL_FLAGS_NONE,
                       bluez_signal_connection_props_changed,
                       conn,
                       NULL);


                g_dbus_connection_call_sync(
                        conn,
                        BLUEZ_ORG,
                        device_path,
                        "org.bluez.Device1",
                        "Connect",
                        NULL,
                        NULL,
                        G_DBUS_CALL_FLAGS_NONE,
                        -1,
                        NULL,
                        &error);

                if (error != NULL)
                    g_error(error->message);

                /** This is the one way of doing it.
                g_dbus_connection_call_sync(
                         conn,
                        "org.bluez.obex",
                        "/org/bluez/obex",
                        "org.bluez.obex.Client1",
                        "CreateSession",
                        g_variant_new("(s{sv})", "/org/bluez/dev_5C_87_30_66_F4_35", "Target", g_variant_new("s", "ftp")),
                        G_VARIANT_TYPE("(o)"),
                        G_DBUS_CALL_FLAGS_NONE,
                        -1,
                        NULL,
                        &error);

                if (error != NULL)
                    g_error(error->message);
                */

                /** This is another way of doing it.
                GDBusMessage *call = g_dbus_message_new_method_call(
                        "org.bluez.obex",
                        "/org/bluez/obex",
                        "org.bluez.obex.Client1",
                        "CreateSession");
                g_dbus_message_set_body(call, g_variant_new("(s{sv})", "/org/bluez/dev_5C_87_30_66_F4_35", "Target", g_variant_new("s", "ftp")));
                g_print("%s\n", g_dbus_message_print(call, 1));

                GDBusMessage *reply = g_dbus_connection_send_message_with_reply_sync(
                        conn,
                        call,
                        G_DBUS_SEND_MESSAGE_FLAGS_PRESERVE_SERIAL,
                        -1,
                        NULL,
                        NULL,
                        &error
                        );
                */

                /** Here's the third way.
                GDBusProxy *proxy = g_dbus_proxy_new_sync(
                        session_conn,
                        G_DBUS_PROXY_FLAGS_NONE,
                        NULL,
                        "org.bluez.obex",
                        "/org/bluez/obex",
                        "org.bluez.obex.AgentManager1",
                        NULL,
                        &error
                        );

                if (error != NULL)
                    g_error(error->message);

                g_dbus_proxy_call_sync(
                        proxy,
                        "CreateSession",
                        g_variant_new("(s{sv})", "/org/bluez/dev_5C_87_30_66_F4_35", "Target", g_variant_new("s", "ftp")),
                        G_DBUS_CALL_FLAGS_NONE,
                        -1,
                        NULL,
                        &error
                        );

                while (1); //Debug statement.

                if (error != NULL)
                    g_error(error->message);
                    */

                loop = g_main_loop_new(NULL, FALSE);
                g_timeout_add_seconds(connection_timeout, bluez_scan_timeout_signal, loop);
                g_main_loop_run(loop); // Blocking call (:


                g_dbus_connection_signal_unsubscribe(conn, connection_device_props_changed);
                g_main_loop_unref(loop);
                g_print("Connected to %s\n", device_path);
                break;
            case 'd': // Disconnect
                g_print("Disconnect\n");
                int disconnect_timeout = 10;
                guint disconnect_device_props_changed = 0;
                device_path = bluez_choose_device(devices);

                disconnect_device_props_changed = g_dbus_connection_signal_subscribe(
                        conn,
                        "org.bluez",
                        "org.freedesktop.DBus.Properties",
                        "PropertiesChanged",
                        device_path,
                        "org.bluez.Device1",
                        G_DBUS_SIGNAL_FLAGS_NONE,
                        bluez_signal_pairing_props_changed,
                        NULL,
                        NULL);

                g_dbus_connection_call_sync(
                        conn,
                        BLUEZ_ORG,
                        device_path,
                        "org.bluez.Device1",
                        "Disconnect",
                        NULL,
                        NULL,
                        G_DBUS_CALL_FLAGS_NONE,
                        -1,
                        NULL,
                        &error);

                if (error != NULL)
                    g_error(error->message);

                loop = g_main_loop_new(NULL, FALSE);
                g_timeout_add_seconds(disconnect_timeout, bluez_scan_timeout_signal, loop);
                g_main_loop_run(loop); // Blocking call (:

                g_dbus_connection_signal_unsubscribe(conn, disconnect_device_props_changed);
                g_main_loop_unref(loop);
                g_print("Disconnected.");
                break;
            case 'r': // Remove device
                g_print("Remove Device\n");
                break;
            case 'q': // Quit
                exit(EXIT_SUCCESS);
            case 'S': //Set
                g_print("Setting device properties\n");
                break;
            case 'G': //Get - DONE
                g_print("Device Properties\n");

                GVariant *property_container = bluez_adapter_properties_call(conn, NULL, "GetAll");
                if (g_variant_is_container(property_container))
                {
                    GVariant * properties = g_variant_get_child_value(property_container, 0);
                    g_print("%s\n", g_variant_print_string(properties, NULL, 0)->str);
                    g_variant_unref(properties);
                }
                break;
            default:
                break;
        }
    }
    return 0;
}