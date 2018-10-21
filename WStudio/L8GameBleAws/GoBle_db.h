// GoBle_db.h

#ifndef __GATT_DATABASE_H__
#define __GATT_DATABASE_H__

#include "wiced.h"

#define __UUID_GOBLE                                 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xb0, 0xdf, 0x00, 0x00
#define __UUID_GOBLE_SERIALPORTID                    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xb1, 0xdf, 0x00, 0x00

// ***** Primary Service 'Generic Attribute'
#define HDLS_GENERIC_ATTRIBUTE                       0x0001

// ***** Primary Service 'Generic Access'
#define HDLS_GENERIC_ACCESS                          0x0014
// ----- Characteristic 'Device Name'
#define HDLC_GENERIC_ACCESS_DEVICE_NAME              0x0015
#define HDLC_GENERIC_ACCESS_DEVICE_NAME_VALUE        0x0016
// ----- Characteristic 'Appearance'
#define HDLC_GENERIC_ACCESS_APPEARANCE               0x0017
#define HDLC_GENERIC_ACCESS_APPEARANCE_VALUE         0x0018

// ***** Primary Service 'GoBle'
#define HDLS_GOBLE                                   0x0028
// ----- Characteristic 'SerialPortId'
#define HDLC_GOBLE_SERIALPORTID                      0x0029
#define HDLC_GOBLE_SERIALPORTID_VALUE                0x002A
// ===== Descriptor 'Client Configuration'
#define HDLD_GOBLE_SERIALPORTID_CLIENT_CONFIGURATION 0x002B


// External definitions
extern const uint8_t  gatt_database[];
extern const uint16_t gatt_database_len;
extern uint8_t BT_LOCAL_NAME[];
extern const uint16_t BT_LOCAL_NAME_CAPACITY;

#endif /* __GATT_DATABASE_H__ */
