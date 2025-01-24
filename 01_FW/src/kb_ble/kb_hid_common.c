#include "kb_hid_common.h"
#include <stdbool.h>
#include <stddef.h>
#include <zephyr/kernel.h>


K_MSGQ_DEFINE(kb_key_press_queue, sizeof(kb_state_t), 10, 4);

int kb_hid_get_kb_state(kb_state_t *kb_state) {
	return k_msgq_get(&kb_key_press_queue, kb_state, K_NO_WAIT);
}

int kb_hid_put_kb_state(kb_state_t *kb_state) {
    print_kb_state(kb_state);
	return k_msgq_put(&kb_key_press_queue, kb_state, K_NO_WAIT);
}

void print_kb_state(kb_state_t *kb_state) {
    printk("Keyboard state: ");
    printk("0x%x ", kb_state->ctrl_keys_state);
    printk("0x%x ", kb_state->pad);
    for(size_t i = 0; i < sizeof(kb_state->keys_state); i++) {
        printk("0x%x ", kb_state->keys_state[i]);
    }
    printk("\n");
}