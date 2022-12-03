#ifndef DBUS_H
#define DBUS_H
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <glib.h>
#include <gio/gio.h>

void dbus_print_message_itr(GVariant *resp);



#endif // DBUS_H
