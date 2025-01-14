#ifndef KEYS
#define KEYS

#include "common.h"
#include <stdint.h>

#define SCAN_START 0xF0
#define ADV_START 0xF1
#define CENT_PAIRING_ACCEPT 0xF2
#define PERIPH_PAIRING_ACCEPT 0xF3

typedef void (*key_change_cb)(const uint8_t *, int);


typedef struct {
    key_change_cb key_press;
    void_void_func btn_0;
    void_void_func btn_1;
    void_void_func btn_2;
    void_void_func btn_3;
} btn_callbacks;

void btns_init(btn_callbacks *new_cb); 

#endif