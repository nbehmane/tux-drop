#ifndef SCAN_H
#define SCAN_H
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <glib.h>
#include <gio/gio.h>

void scan_print_devices(GHashTable *device_table);
void scan_remove_devices(GHashTable *device_table);


#endif // SCAN_H
