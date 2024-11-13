#ifndef KEYS
#define KEYS

typedef enum {
    KEY_PRESSED, 
    KEY_RELEASED
} keyState_e; 

typedef struct _keyBrdKey_ {
    keyState_e state; 
    keyState_e statePrev; 
    uint8_t scanCode; 
} KB_key_t;

void keyMatrixTask(); 
#endif