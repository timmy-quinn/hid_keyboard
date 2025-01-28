#include "kb_hid_common.h"

#include <stddef.h>
#include <zephyr/kernel.h>

void print_kb_state(kb_state_t *kb_state) {
    printk("Keyboard state: ");
    printk("0x%x ", kb_state->ctrl_keys);
    printk("0x%x ", kb_state->pad);
    for (size_t i = 0; i < sizeof(kb_state->keys_state); i++) {
        printk("0x%x ", kb_state->keys_state[i]);
    }
    printk("\n");
}

uint8_t get_ctrl_scan_code(uint8_t ctrl_mask, uint8_t bit_num) {
    if (ctrl_mask & (1U << bit_num)) {
        return (uint8_t)(bit_num + KEY_CTRL_CODE_MIN);
    }
    return 0;
}

static uint8_t get_ctrl_mask(uint8_t key) {
    if (KEY_CTRL_CODE_MIN <= key && key <= KEY_CTRL_CODE_MAX) {
        return (uint8_t)(1U << (key - KEY_CTRL_CODE_MIN));
    }
    return 0;
}

int kb_periph_key_set(kb_state_t *state, uint8_t key) {
    uint8_t ctrl_mask = get_ctrl_mask(key);

    if (ctrl_mask) {
        state->ctrl_keys |= ctrl_mask;
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

int kb_periph_key_clear(kb_state_t *state, uint8_t key) {
    uint8_t ctrl_mask = get_ctrl_mask(key);

    if (state == NULL) {
        return -EINVAL;
    }

    if (ctrl_mask) {
        state->ctrl_keys &= ~ctrl_mask;
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

void kb_hid_update(kb_state_t *kb, kb_key_t *key) {
    printk("key_ress: \n");
    if (key->is_pressed) {
        kb_periph_key_set(kb, key->scan_code);
    } else {
        kb_periph_key_clear(kb, key->scan_code);
    }
}

K_MSGQ_DEFINE(kb_key_press_queue, sizeof(kb_key_t), 32, 4);

int kb_hid_set_key_evt(kb_key_t *key_evt) {
    if (!key_evt) {
        return -EINVAL;
    }
    printk("key evt %x %u\n", key_evt->scan_code, key_evt->is_pressed);
    return k_msgq_put(&kb_key_press_queue, key_evt, K_NO_WAIT);
}

void kb_hid_key_change(uint8_t prev_scan_code, uint8_t new_scan_code) {
    if (prev_scan_code == new_scan_code) {
        return;
    }
    kb_key_t key;
    if (prev_scan_code != 0) {
        key.is_pressed = false;
        key.scan_code = prev_scan_code;
        kb_hid_set_key_evt(&key);
    }
    if (new_scan_code != 0) {
        key.is_pressed = true;
        key.scan_code = new_scan_code;
        kb_hid_set_key_evt(&key);
    }
}

void kb_hid_process_kb(kb_state_t *prev_kb, kb_state_t *new_kb) {
    // Initial though on how to do this, for each key pressed in prev_kb compare against all other keys in
    // second report to see if there is a match. But we know the way that the scan codes don't move. So we
    // just compare scan codes in the same position to see if they've changed. this is more efficient.
    // if a scan code is pressed and released in between one report and the next, it will be treated as two
    // separate events, as it should be. (rather than no change)
    for (size_t i = 0; i < sizeof(prev_kb->keys_state); i++) {
        kb_hid_key_change(prev_kb->keys_state[i], new_kb->keys_state[i]);
    }

    if (prev_kb->ctrl_keys == new_kb->ctrl_keys) {  // optimization: avoids bit-by-bit checking
        return;
    }

    for (size_t i = 0; i < (KEY_CTRL_NUM); i++) {
        kb_hid_key_change(get_ctrl_scan_code(prev_kb->ctrl_keys, i),
                          get_ctrl_scan_code(new_kb->ctrl_keys, i));
    }
}

/*
 */
int kb_hid_get_state(kb_state_t *kb) {
    kb_key_t key_evt;
    int err = 0;
    while (k_msgq_num_used_get(&kb_key_press_queue)) {
        err = k_msgq_get(&kb_key_press_queue, &key_evt, K_NO_WAIT);
        if (err) {
            printk("message queue error\n");
            return err;
        } else {
            printk("key press %x %d", key_evt.scan_code, key_evt.is_pressed);
            kb_hid_update(kb, &key_evt);
        }
    }
    return err;
}
