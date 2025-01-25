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

static uint8_t button_ctrl_code(uint8_t key)
{
	if (KEY_CTRL_CODE_MIN <= key && key <= KEY_CTRL_CODE_MAX) {
		return (uint8_t)(1U << (key - KEY_CTRL_CODE_MIN));
	}
	return 0;
}


 int kb_periph_key_set(uint8_t key, kb_state_t *state)
{
	uint8_t ctrl_mask = button_ctrl_code(key);

	if (ctrl_mask) {
		state->ctrl_keys_state |= ctrl_mask;
		return 0;
	}
	for (size_t i = 0; i < KEY_PRESS_MAX; ++i) {
		if (state->keys_state[i] == 0) {
			state->keys_state[i] = key;
			return 0;
		}
	}
	/* All slots busy */
	return -EBUSY;
}


int kb_periph_key_clear(uint8_t key, kb_state_t *state)
{
	uint8_t ctrl_mask = button_ctrl_code(key);

	if(state == NULL) {
		return -EINVAL;
	}

	if (ctrl_mask) {
		state->ctrl_keys_state &= ~ctrl_mask;
		return 0;
	}
	for (size_t i = 0; i < KEY_PRESS_MAX; ++i) {
		if (state->keys_state[i] == key) {
			state->keys_state[i] = 0;
			return 0;
		}
	}
	/* Key not found */
	return -EINVAL;
}


void kb_periph_key_event(uint8_t scan_code, kb_state_t *state, bool is_pressed) {
	printk("kb_periph_key_event\n");
	if (is_pressed) {
 		kb_periph_key_set(scan_code, state);
	} else {
		kb_periph_key_clear(scan_code, state);
	}
}