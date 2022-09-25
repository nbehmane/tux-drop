/*
 * @file main.c
 * @author Nima Behmanesh
 */

/**** INCLUDES ****/
#include <glib-2.0/glib.h>
#include <glib-2.0/gio/gio.h>

#include "main-loop.h"
#include "cli.h"


int main(int argc, char **argv)
{
    /** Loop stuff... May not be necessary yet. **/
    Loop *loop = loop_init();
    loop_destroy(loop);

    /** GDBusConnection Stuff **/
    GError *error;
    GDBusConnection *conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    g_assert_no_error(error);

    /** Start the CLI stuff **/
    cli_run(argc, argv, conn);

    /** CLEAN UP **/
    g_object_unref(conn);

    // Annoying errors by IDE
    (void)argc;
    (void)argv;
   return 0;
}