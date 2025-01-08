#ifndef KEYS
#define KEYS
#include <stdint.h>

typedef void (*key_change_cb)(const uint8_t *, int);

void kb_keys_init(key_change_cb new_key_cb, void (*new_pairing_cb)(void)); 

#endif