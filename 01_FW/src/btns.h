#ifndef KEYS
#define KEYS
#include <stdint.h>

#define SCAN_START 0xF0
#define ADV_START 0xF1
#define CENT_PAIRING_ACCEPT 0xF2
#define PERIPH_PAIRING_ACCEPT 0xF3


typedef void (*key_change_cb)(const uint8_t *, int);
typedef void(*vv_cb)(void);


void kb_keys_init(key_change_cb new_key_cb, vv_cb pp_cb, vv_cb a_cb, vv_cb s_cb, vv_cb cp_cb); 

#endif