#ifndef KB_BLE_PERIPHERAL 
#define KB_BLE_PERIPHERAL 

#include <zephyr/bluetooth/conn.h>

void advertising_start(void);

void periph_auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey);
void periph_pairing_failed(struct bt_conn *conn, enum bt_security_err reason);

void periph_connected(struct bt_conn *conn, uint8_t err);
void periph_disconnected(struct bt_conn *conn, uint8_t reason);

void kb_periph_key_event(const uint8_t *scan_code, int state); 
void kb_periph_bas_notify(void);
bool kb_periph_is_adv(void); 
void kb_periph_accept_pairing(); 
void kb_periph_init(void); 
#endif