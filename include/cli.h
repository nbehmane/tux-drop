/*
 * @file cli.h
 * @brief Command line interface.
 * @brief New location will be in /shared/.
 * @author Nima Behmanesh
 */
#ifndef TUXDROP_CLI_H
#define TUXDROP_CLI_H

/**** INCLUDES ****/
#include <stdio.h>
#include <stdlib.h>

#include <getopt.h>
#include <glib-2.0/glib.h>
#include <glib-2.0/gio/gio.h>

/**
 * @brief Runs the CLI. Meant to be in the main loop.
 *
 * @param argc Count from main.
 * @param argv Argv from main.
 * @return
 */
extern int cli_run(int argc, char **argv, GDBusConnection *conn);

#endif //TUXDROP_CLI_H