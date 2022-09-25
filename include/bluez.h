//
// Created by nima on 7/31/22.
//

#ifndef TUXDROP_BLUEZ_H
#define TUXDROP_BLUEZ_H
#include "bluez-internal.h"

GVariant* bluez_adapter_properties_call(GDBusConnection *conn,
                                  const char *obj_path,
                                  const char *method);
/**
 * @brief Calls methods on the Adapter1 object.
 *
 * @return
 */
int bluez_adapter_method_call(GDBusConnection *conn,
                              const char *obj_path,
                              const char *method);

#endif //TUXDROP_BLUEZ_H