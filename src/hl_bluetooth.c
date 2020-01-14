#include "hl_bluetooth.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>
#include <bluetooth/services/hrs.h>

struct bt_conn *_hl_bluetooth_default_conn;

static u8_t ct[10];
static u8_t ct_update;

uint8_t* fetch_ct() {
    return ct;
}


static void ct_ccc_cfg_changed(const struct bt_gatt_attr *attr, u16_t value)
{
	/* TODO: Handle value */
}

static ssize_t read_ct(struct bt_conn *conn, const struct bt_gatt_attr *attr,
		       void *buf, u16_t len, u16_t offset)
{
	const char *value = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 sizeof(ct));
}

static ssize_t write_ct(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			const void *buf, u16_t len, u16_t offset,
			u8_t flags)
{
	u8_t *value = attr->user_data;

	if (offset + len > sizeof(ct)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	ct_update = 1U;

	return len;
}

BT_GATT_SERVICE_DEFINE(cts_cvs,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_CTS),
	BT_GATT_CHARACTERISTIC(BT_UUID_CTS_CURRENT_TIME, BT_GATT_CHRC_READ |
			       BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			       read_ct, write_ct, ct),
	BT_GATT_CCC(ct_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

// The data used to advertise this device over Bluetooth.
static const struct bt_data _hl_ad_data[] = {
    // Flags: This device is a general discoverable device which does not
    // support EDR (enhanced data rate).
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),

    // Available GATT services:
    //   - 0x180D: Heart rate service
    //   - 0x180F: Battery service
    //   - 0x180A: Device information
    //   - 0x1805: Current time service
    // https://www.bluetooth.com/specifications/gatt/services/
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x0d, 0x18, 0x0f, 0x18, 0x0a, 0x18, 0x05, 0x18),
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
