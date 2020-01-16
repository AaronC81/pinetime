#include "hl_bluetooth.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>
#include <bluetooth/services/hrs.h>

struct bt_conn *_hl_bluetooth_default_conn;

// The data used to advertise this device over Bluetooth.
static const struct bt_data _hl_ad_data[] = {
    // Flags: This device is a general discoverable device which does not
    // support EDR (enhanced data rate).
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),

    // Available GATT services:
    //   - 0x180D: Heart rate service
    //   - 0x180F: Battery service
    //   - 0x180A: Device information
    // https://www.bluetooth.com/specifications/gatt/services/
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x0d, 0x18, 0x0f, 0x18, 0x0a, 0x18),
};

// Callback invoked when Bluetooth connects.
void _hl_bluetooth_connected(struct bt_conn *conn, u8_t err)
{
    if (!err) {
        _hl_bluetooth_default_conn = bt_conn_ref(conn);
    }
}

// Callback invoked when Bluetooth disconnects.
void _hl_bluetooth_disconnected(struct bt_conn *conn, u8_t reason)
{
    if (_hl_bluetooth_default_conn) {
        bt_conn_unref(_hl_bluetooth_default_conn);
        _hl_bluetooth_default_conn = NULL;
    }
}

static struct bt_conn_cb _hl_bluetooth_conn_cb = {
    .connected = _hl_bluetooth_connected,
    .disconnected = _hl_bluetooth_disconnected,
};

// Sends a new GATT Battery Service notification to the connected device,
// with the given battery level percentage.
void hl_bluetooth_bas_notify(uint8_t battery_level)
{
    bt_gatt_bas_set_battery_level(battery_level);
}

// Sends a new GATT Heart Rate Service notification to the connected device, 
// with the given heart rate value.
void hl_bluetooth_hrs_notify(uint16_t heart_rate)
{
    bt_gatt_hrs_notify(heart_rate);
}

// Opens this Bluetooth device for connections.
void hl_bluetooth_init(void) {
    bt_enable(NULL);
    bt_le_adv_start(BT_LE_ADV_CONN_NAME, _hl_ad_data, ARRAY_SIZE(_hl_ad_data), NULL, 0);

    bt_conn_cb_register(&_hl_bluetooth_conn_cb);
}
