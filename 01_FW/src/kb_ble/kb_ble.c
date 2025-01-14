#include "ble.h"
#include "central.h"
#include "peripheral.h"


#include <zephyr/bluetooth/conn.h>
#include <zephyr/settings/settings.h>
#include <stdbool.h>

static bool periph_is_enabled;
static bool cent_is_enabled;

/* Static function declarations */
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey);
static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey);
static void auth_cancel(struct bt_conn *conn);

static void pairing_complete(struct bt_conn *conn, bool bonded); 
static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason); 

static void connected(struct bt_conn *conn, uint8_t err); 
static void disconnected(struct bt_conn *conn, uint8_t reason); 
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err);

void enable_periph(bool enable) {
    periph_is_enabled = enable;
}

void enable_central(bool enable) {
    cent_is_enabled = enable;
}

/* Connection authorization callbacks */
static struct bt_conn_auth_cb conn_auth_callbacks = {
	.passkey_display = auth_passkey_display,
	.passkey_confirm = auth_passkey_confirm,
	.cancel = auth_cancel,
};

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey) {
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	printk("Passkey for %s: %06u\n", addr, passkey);
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey) {
    struct bt_conn_info info; 	
    bt_conn_get_info(conn, &info);  	
	if (info.role == BT_CONN_ROLE_CENTRAL) {
        cent_auth_passkey_confirm(conn, passkey);
    }
    else if (info.role == BT_CONN_ROLE_PERIPHERAL) {
        periph_auth_passkey_confirm(conn, passkey);
    } 
}

static void auth_cancel(struct bt_conn *conn) {
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	printk("Pairing cancelled: %s\n", addr);
}

/* Connection authorization info callbacks */
static struct bt_conn_auth_info_cb conn_auth_info_callbacks = {
	.pairing_complete = pairing_complete,
	.pairing_failed = pairing_failed
};

static void pairing_complete(struct bt_conn *conn, bool bonded) {
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	printk("Pairing completed: %s, bonded: %d\n", addr, bonded);
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason) {
    struct bt_conn_info info; 	
    bt_conn_get_info(conn, &info);  	
	if (info.role == BT_CONN_ROLE_CENTRAL) {
        cent_pairing_failed(conn, reason);
    }
    else if (info.role == BT_CONN_ROLE_PERIPHERAL) {
        periph_pairing_failed(conn, reason);
    } 
}

/* Connection callbacks */
BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected        = connected,
	.disconnected     = disconnected,
	.security_changed = security_changed
};

static void connected(struct bt_conn *conn, uint8_t err) {
    struct bt_conn_info info; 	
    bt_conn_get_info(conn, &info);  	
	if (info.role == BT_CONN_ROLE_CENTRAL) {
        cent_connected(conn, err);
    }
    else if (info.role == BT_CONN_ROLE_PERIPHERAL) {
        periph_connected(conn, err); 
	} 
}

static void disconnected(struct bt_conn *conn, uint8_t reason) {
    struct bt_conn_info info; 	
    bt_conn_get_info(conn, &info);  	
	if (info.role == BT_CONN_ROLE_CENTRAL) {
        cent_disconnected(conn, reason);
    }
    else if (info.role == BT_CONN_ROLE_PERIPHERAL) {
        periph_disconnected(conn, reason);
    } 
}

static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err) { 
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (!err) {
		printk("Security changed: %s level %u\n", addr, level);
	} else {
		printk("Security failed: %s level %u err %d\n", addr, level, err);
	}

    struct bt_conn_info info; 	
    bt_conn_get_info(conn, &info);  	

	if (info.role == BT_CONN_ROLE_CENTRAL) {
		cent_gatt_discover(conn);
	}
}


int ble_init() {
	int err;

	printk("Initializing bluetooth for Keyboard\n");

	err = bt_conn_auth_cb_register(&conn_auth_callbacks);
	if (err) {
		printk("failed to register authorization callbacks.\n");
		return err;
	}
	printk("Registered authorization callbacks.\n");

	err = bt_conn_auth_info_cb_register(&conn_auth_info_callbacks);
	if (err) {
		printk("Failed to register authorization info callbacks.\n");
		return err;
	}
	printk("Registered authorization info callbacks.\n");

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return err;
	}
	printk("Bluetooth initialized\n");
}