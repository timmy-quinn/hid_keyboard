#include "btns.h"

#include <hal/nrf_gpio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "common.h"
#include "kb_ble/kb_cent.h"
#include "kb_ble/kb_hid_common.h"
#include "kb_ble/kb_periph.h"
#include "scan_codes.h"

#define GPIO_NODE_0 DT_NODELABEL(gpio0)
#define GPIO_NAME_0 DEVICE_DT_NAME(GPIO_NODE_0)

#define GPIO_NODE_1 DT_NODELABEL(gpio1)
#define GPIO_NAME_1 DEVICE_DT_NAME(GPIO_NODE_1)

#define KEY_MATRIX_OUT_COUNT 6
#define KEY_MATRIX_IN_COUNT 5
#define SLEEP_TIME_MS 2


static struct k_work_delayable btn_scan_work;

static const struct gpio_dt_spec pwr_pin = GPIO_DT_SPEC_GET(DT_NODELABEL(pwr), gpios);
static const struct gpio_dt_spec usr_btn = GPIO_DT_SPEC_GET(DT_NODELABEL(button0), gpios);

static const struct gpio_dt_spec key_mtrx_in[KEY_MATRIX_IN_COUNT];
static const struct gpio_dt_spec key_mtrx_out[KEY_MATRIX_OUT_COUNT];

static kb_key_t key_mtrx[KEY_MATRIX_IN_COUNT][KEY_MATRIX_OUT_COUNT];

static const struct gpio_dt_spec key_mtrx_in[KEY_MATRIX_IN_COUNT] = {
    GPIO_DT_SPEC_GET(DT_NODELABEL(swin0), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swin1), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swin2), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swin3), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swin4), gpios),
    };

#define SCAN_BEGIN SCN_CD_RESERVED2 
#define SCAN_ACCEPT SCN_CD_RESERVED3 
#define ADV_BEGIN SCN_CD_RESERVED4 
#define ADV_ACCEPT SCN_CD_RESERVED5

#define IS_BLE_SCN_CD(scn_cd) (((scn_cd) == SCAN_BEGIN) || \
                            ((scn_cd) == SCAN_ACCEPT) || \
                            ((scn_cd) == ADV_BEGIN) || \
                            ((scn_cd) == ADV_ACCEPT) )

#ifdef CONFIG_KB_RIGHT 
// This is because SOMEBODY designed the hardware stupidly. 
// On right board the cols go from left #define SCN_CD_ERR_ROLL_OVER 0x01Uto right, on left board the cols go from right to left
static const struct gpio_dt_spec key_mtrx_out[KEY_MATRIX_OUT_COUNT] = {
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout0), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout1), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout2), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout3), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout4), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout5), gpios),
    };

static kb_key_t key_mtrx[KEY_MATRIX_IN_COUNT][KEY_MATRIX_OUT_COUNT] = {
    {{ADV_ACCEPT, 0}, {ADV_BEGIN, 0}, {SCAN_BEGIN, 0}, {SCAN_ACCEPT, 0}, {KEY_H, 0}, {KEY_H, 0}},
    {{KEY_Y, 0}, {KEY_U, 0}, {KEY_I, 0}, {KEY_O, 0}, {KEY_P, 0}, {KEY_H, 0}},
    {{KEY_H, 0}, {KEY_J, 0}, {KEY_K, 0}, {KEY_L, 0}, {KEY_H, 0}, {KEY_H, 0}},
    {{KEY_N, 0}, {KEY_M, 0}, {KEY_H, 0}, {KEY_LSHIFT, 0}, {KEY_H, 0}, {KEY_H, 0}},
    {{KEY_LSHIFT, 0}, {KEY_SPACE, 0}, {KEY_RCTRL, 0}, {KEY_I, 0}, {KEY_J, 0}, {KEY_K, 0}},
    };

#elif CONFIG_KB_LEFT
// See, in keyboard left this goes from 5 to 0, whereas in keyboard right it goes from 5 to 0
static const struct gpio_dt_spec key_mtrx_out[KEY_MATRIX_OUT_COUNT] = {
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout5), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout4), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout3), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout2), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout1), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout0), gpios),
    };

static kb_key_t key_mtrx[KEY_MATRIX_IN_COUNT][KEY_MATRIX_OUT_COUNT] = {
    {{SCN_CD_H, 0}, {SCN_CD_H, 0}, {SCAN_ACCEPT, 0}, {SCAN_BEGIN, 0}, {ADV_BEGIN, 0}, {ADV_ACCEPT, 0}},
    {{SCN_CD_H, 0}, {SCN_CD_Q, 0}, {SCN_CD_W, 0}, {SCN_CD_E, 0}, {SCN_CD_R, 0}, {SCN_CD_T, 0}},
    {{SCN_CD_H, 0}, {SCN_CD_A, 0}, {SCN_CD_S, 0}, {SCN_CD_D, 0}, {SCN_CD_F, 0}, {SCN_CD_G, 0}},
    {{SCN_CD_H, 0}, {SCN_CD_Z, 0}, {SCN_CD_X, 0}, {SCN_CD_V, 0}, {SCN_CD_B, 0}, {SCN_CD_C, 0}},
    {{SCN_CD_I, 0}, {SCN_CD_J, 0}, {SCN_CD_K, 0}, {SCN_CD_RET, 0}, {SCN_CD_SPACE, 0}, {SCN_CD_H, 0}},
    };
#endif

static void key_gpios_init() {
    for (size_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++) {
        gpio_pin_configure_dt(&key_mtrx_out[i], GPIO_OUTPUT);
    }
    gpio_pin_configure_dt(&pwr_pin, GPIO_OUTPUT);

    for (size_t i = 0; i < KEY_MATRIX_IN_COUNT; i++) {
        gpio_pin_configure_dt(&key_mtrx_in[i], GPIO_INPUT);
    }

    gpio_pin_configure_dt(&usr_btn, GPIO_INPUT);

    for (size_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++) {
        gpio_pin_set_dt(&key_mtrx_out[i], 0);
    }
    gpio_pin_set_dt(&pwr_pin, 1);
}

static void special_key_handler(kb_key_t *key) {
    if (!key->is_pressed) {
        return;
    }
    switch (key->scan_code) {
        case (ADV_ACCEPT):
            kb_periph_accept_pairing();
            break;
        case (ADV_BEGIN):
            kb_periph_adv_start();
            break;
        case (SCAN_BEGIN):
            kb_cent_scan_start();
            break;
        case (SCAN_ACCEPT):
            cent_pairing_accept();
            break;
        default:
            break;
    }
}

static void btn_scan() {
    static size_t scan_col = 0;
    volatile int btn_pressed;
    bool key_changed = 0;

    gpio_pin_set_dt(&key_mtrx_out[scan_col], 0);
    if (scan_col < KEY_MATRIX_OUT_COUNT - 1) {
        scan_col++;
    } else {
        scan_col = 0;
    }
    gpio_pin_set_dt(&key_mtrx_out[scan_col], 1);

    k_sleep(K_MSEC(SLEEP_TIME_MS));

    for (uint16_t row = 0; row < KEY_MATRIX_IN_COUNT; row++) {
        btn_pressed = gpio_pin_get(key_mtrx_in[row].port, key_mtrx_in[row].pin);
        if (key_mtrx[row][scan_col].is_pressed != btn_pressed) {
            key_mtrx[row][scan_col].is_pressed = btn_pressed;

            if (IS_BLE_SCN_CD(key_mtrx[row][scan_col].scan_code)) {
                special_key_handler(&key_mtrx[row][scan_col]);
            } else {
                kb_hid_set_key_evt(&(key_mtrx[row][scan_col]));
                key_changed = true;
            }
        }
    }

    if (key_changed) {
        kb_periph_hid_notify();
    }

    k_work_reschedule(&btn_scan_work, K_MSEC(SLEEP_TIME_MS));
}

void btns_init() {
    key_gpios_init();
    k_work_init_delayable(&btn_scan_work, btn_scan);
    k_work_schedule(&btn_scan_work, K_NO_WAIT);
}
