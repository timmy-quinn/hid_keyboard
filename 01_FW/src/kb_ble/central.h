#ifndef KB_BLE_CENTRAL 
#define KB_BLE_CENTRAL 

#include <zephyr/bluetooth/conn.h>

void cent_auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey);
void cent_pairing_failed(struct bt_conn *conn, enum bt_security_err reason);

void cent_gatt_discover(struct bt_conn *conn);
void cent_connected(struct bt_conn *conn, uint8_t conn_err);

void cent_connected(struct bt_conn *conn, uint8_t conn_err);
void cent_disconnected(struct bt_conn *conn, uint8_t reason);

void ble_cent_scan_start(); 
void cent_pairing_accept(); 

int cent_ble_init(void);

#endif
