#ifndef KB_BLE 
#define KB_BLE 

void kb_ble_key_event(const uint8_t *scan_code, int state); 
void kb_ble_bas_notify(void);
bool kb_ble_is_adv(void); 
void kb_ble_init(void); 
#endif