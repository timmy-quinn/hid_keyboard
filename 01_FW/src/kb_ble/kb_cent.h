#ifndef KB_BLE_CENTRAL
#define KB_BLE_CENTRAL

#include <zephyr/bluetooth/conn.h>

#include "kb_hid_common.h"

int kb_cent_get_kb_state(kb_state_t *kb_state);
void cent_auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey);
void cent_pairing_failed(struct bt_conn *conn, enum bt_security_err reason);

void cent_gatt_discover(struct bt_conn *conn);
void cent_connected(struct bt_conn *conn, uint8_t conn_err);

void cent_connected(struct bt_conn *conn, uint8_t conn_err);
void cent_disconnected(struct bt_conn *conn, uint8_t reason);

void kb_cent_scan_start();
void cent_pairing_accept();

void kb_cent_init(void);

#endif
