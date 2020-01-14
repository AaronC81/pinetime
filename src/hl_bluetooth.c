#include "hl_bluetooth.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>
#include <bluetooth/services/hrs.h>

void bluetooth_debug_notify(char *str);
struct bt_conn *_hl_bluetooth_default_conn;

static struct bt_uuid_128 vnd_uuid = BT_UUID_INIT_128(
	0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
	0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);

#define MAX_DATA 74
static u8_t vnd_long_value[] = {
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '1',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '2',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '3',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '4',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '5',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '6',
		  '.', ' ' };

static ssize_t read_long_vnd(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr, void *buf,
			     u16_t len, u16_t offset)
{
	const char *value = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 sizeof(vnd_long_value));
}

static ssize_t write_long_vnd(struct bt_conn *conn,
			      const struct bt_gatt_attr *attr, const void *buf,
			      u16_t len, u16_t offset, u8_t flags)
{
	u8_t *value = attr->user_data;

	if (flags & BT_GATT_WRITE_FLAG_PREPARE) {
		return 0;
	}

	if (offset + len > sizeof(vnd_long_value)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);

	return len;
}

static const struct bt_uuid_128 vnd_long_uuid = BT_UUID_INIT_128(
	0xf3, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
	0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);

static struct bt_gatt_cep vnd_long_cep = {
	.properties = BT_GATT_CEP_RELIABLE_WRITE,
};

/* Vendor Primary Service Declaration */
BT_GATT_SERVICE_DEFINE(vnd_svc,
	BT_GATT_PRIMARY_SERVICE(&vnd_uuid),
	//BT_GATT_CCC(vnd_ccc_cfg_changed,
	//	    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT),
	BT_GATT_CHARACTERISTIC(&vnd_long_uuid.uuid, BT_GATT_CHRC_READ |
			       BT_GATT_CHRC_WRITE | BT_GATT_CHRC_EXT_PROP,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE |
			       BT_GATT_PERM_PREPARE_WRITE,
			       read_long_vnd, write_long_vnd, &vnd_long_value),
	BT_GATT_CEP(&vnd_long_cep),
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
    // https://www.bluetooth.com/specifications/gatt/services/
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x0d, 0x18, 0x0f, 0x18, 0x0a, 0x18),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL,
		      0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
		      0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12),
};

// Callback invoked when Bluetooth connects.
void _hl_bluetooth_connected(struct bt_conn *conn, u8_t err)
{
	if (!err) {
		_hl_bluetooth_default_conn = bt_conn_ref(conn);
	}

	bluetooth_debug_notify("bruh moment");
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

void bluetooth_debug_notify(char *str)
{
	strncpy(vnd_long_value, str, MAX_DATA);
	bt_gatt_notify(_hl_bluetooth_default_conn, &vnd_svc.attrs[0], vnd_long_value, MAX_DATA);
}

// Opens this Bluetooth device for connections.
void hl_bluetooth_init(void) {
    bt_enable(NULL);
    bt_le_adv_start(BT_LE_ADV_CONN_NAME, _hl_ad_data, ARRAY_SIZE(_hl_ad_data), NULL, 0);

	bt_conn_cb_register(&_hl_bluetooth_conn_cb);
}
