#ifndef GATEWAY_SETTINGS_H
#define GATEWAY_SETTINGS_H

gchar *gateway_settings_get_preferred(void);
void gateway_settings_get_device_data(const gchar *device, gchar **name, gchar **unk1, gchar **unk2, gchar **unk3, gchar **unk4, gchar **unk5, gchar **unk6, gchar **unk7, gchar **unk8, gchar **unk9);

#endif // GATEWAY_SETTINGS_H
