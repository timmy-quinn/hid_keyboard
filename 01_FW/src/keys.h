#ifndef KEYS
#define KEYS
/* 
* NRF dongle pins
* side 1
* 0.13, 0.15, 0.17, 0.20, 0.22, 1.24, 1.00, 0.09, 0.10 
* 
* side 2 
* 1.10, 1.13, 1.15, 0.02, 0.29, 0.31
* If all pins are used, we have 56 keys available
*/

typedef enum {
    KEY_PRESSED, 
    KEY_RELEASED
} keyState_e; 

typedef struct _keyBrdKey_ {
    keyState_e state; 
    keyState_e statePrev; 
    uint8_t scanCode; 
} KB_key_t;

typedef void (*key_change_cb)(const uint8_t *, int);

void key_mtrx_init(key_change_cb callback); 
#endif