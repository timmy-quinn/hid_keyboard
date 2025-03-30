#ifndef KB_HID_COMMON
#define KB_HID_COMMON

#include "../scan_codes.h"
#include <stdbool.h>
#include <stdint.h>

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define BASE_USB_HID_SPEC_VERSION 0x0101

#define OUTPUT_REPORT_MAX_LEN 1
#define OUTPUT_REPORT_BIT_MASK_CAPS_LOCK 0x02
#define INPUT_REP_KEYS_REF_ID 0
#define OUTPUT_REP_KEYS_REF_ID 0
#define MODIFIER_KEY_POS 0
#define SHIFT_KEY_CODE 0x02
#define SCAN_CODE_POS 2
#define KEYS_MAX_LEN (INPUT_REPORT_KEYS_MAX_LEN - \
                      SCAN_CODE_POS)

/* HIDs queue elements. */
#define HIDS_QUEUE_SIZE 10

/* ********************* */
/* Buttons configuration */

/* Note: The configuration below is the same as BOOT mode configuration
 * This simplifies the code as the BOOT mode is the same as REPORT mode.
 * Changing this configuration would require separate implementation of
 * BOOT mode report generation.
 */
#define KEY_CTRL_CODE_MIN 224 /* Control key codes - required 8 of them */
#define KEY_CTRL_CODE_MAX 231 /* Control key codes - required 8 of them */
#define KEY_CTRL_NUM (KEY_CTRL_CODE_MAX - KEY_CTRL_CODE_MIN)
#define KEY_CODE_MIN 0   /* Normal key codes */
#define KEY_CODE_MAX 101 /* Normal key codes */
#define KEY_PRESS_MAX 6  /* Maximum number of non-control keys pressed simultaneously*/
#define KEY_OFFSET_IN_REPORT 2

/* Number of bytes in key report
 *
 * 1B - control keys
 * 1B - reserved
 * rest - non-control keys
 */
#define INPUT_REPORT_KEYS_MAX_LEN (1 + 1 + KEY_PRESS_MAX)

typedef struct {
    scan_code_t scan_code;
    bool is_pressed;
} kb_key_t;

typedef struct {
    uint8_t ctrl_keys; /* Current keys state */
    uint8_t pad;       // Padding for alignment for msg queue
    uint8_t keys_state[KEY_PRESS_MAX];
} kb_state_t;

void print_kb_state(kb_state_t *kb_state);
uint8_t get_ctrl_scan_code(uint8_t ctrl_mask, uint8_t bit_num);
int kb_hid_get_state(kb_state_t *kb_state);

int kb_hid_set_key_evt(kb_key_t *key_evt);
void kb_hid_key_change(uint8_t prev_scan_code, uint8_t new_scan_code);

#endif