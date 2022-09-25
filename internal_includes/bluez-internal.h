//
// Created by nima on 7/31/22.
//

#ifndef TUXDROP_BLUEZ_INTERNAL_H
#define TUXDROP_BLUEZ_INTERNAL_H
#include <glib-2.0/glib.h>
#include <glib-2.0/gio/gio.h>

extern GVariant* bluez_adapter_properties_call(GDBusConnection *conn,
                                  const char *obj_path,
                                  const char *method);
/**
 * @brief Calls methods on the Adapter1 object.
 *
 * @return
 */
extern int bluez_adapter_method_call(GDBusConnection *conn,
                              const char *obj_path,
                              const char *method);

#endif //TUXDROP_BLUEZ_INTERNAL_H