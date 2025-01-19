#include "kb_hid_common.h"
#include <stdbool.h>
#include <stddef.h>

void print_kb_state(keyboard_state_t *kb_state) {
    printk("Keyboard state: ");
    printk("0x%x", kb_state->ctrl_keys_state);
    printk("0x%x", kb_state->pad);
    for(size_t i = 0; i < sizeof(kb_state->keys_state); i++) {
        printk("0x%x", kb_state->keys_state[i]);
    }
    printk("\n");
}