#ifndef KEYS
#define KEYS

#include "common.h"
#include <stdint.h>

#define GEN_0 0xF0
#define GEN_1 0xF1
#define GEN_2 0xF2
#define GEN_3 0xF3

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