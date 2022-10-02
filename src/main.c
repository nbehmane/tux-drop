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
    /** Start the CLI stuff **/
    cli_run(argc, argv);
    return 0;
}