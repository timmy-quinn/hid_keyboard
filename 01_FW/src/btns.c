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

#define KEY_MATRIX_OUT_COUNT 4
#define KEY_MATRIX_IN_COUNT 4
#define SLEEP_TIME_MS 2

#define ADV_ACC 0xF0
#define ADV_BEGIN 0xF1
#define SCAN_BEGIN 0xF2
#define SCAN_ACCEPT 0xF3

static struct k_work_delayable btn_scan_work;
static size_t scan_col;

static const struct gpio_dt_spec pwr_pin = GPIO_DT_SPEC_GET(DT_NODELABEL(pwr), gpios);
static const struct gpio_dt_spec usr_btn = GPIO_DT_SPEC_GET(DT_NODELABEL(button0), gpios);

static const struct gpio_dt_spec key_mtrx_out[KEY_MATRIX_OUT_COUNT] = {
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout0), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout1), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout2), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout3), gpios)};

static const struct gpio_dt_spec key_mtrx_in[KEY_MATRIX_IN_COUNT] = {
    GPIO_DT_SPEC_GET(DT_NODELABEL(swin0), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swin1), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swin2), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swin3), gpios)};

kb_key_t key_mtrx[KEY_MATRIX_OUT_COUNT][KEY_MATRIX_IN_COUNT] = {
    {{ADV_ACC, 0}, {ADV_BEGIN, 0}, {SCAN_BEGIN, 0}, {SCAN_ACCEPT, 0}},
    {{KEY_E, 0}, {KEY_F, 0}, {KEY_G, 0}, {KEY_H, 0}},
    {{KEY_I, 0}, {KEY_J, 0}, {KEY_K, 0}, {KEY_L, 0}},
    {{KEY_N, 0}, {KEY_O, 0}, {KEY_P, 0}, {KEY_LSHIFT, 0}}};

static void key_gpios_init() {
    for (uint16_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++) {
        gpio_pin_configure_dt(&key_mtrx_out[i], GPIO_OUTPUT);
    }
    gpio_pin_configure_dt(&pwr_pin, GPIO_OUTPUT);

    for (uint16_t i = 0; i < KEY_MATRIX_IN_COUNT; i++) {
        gpio_pin_configure_dt(&key_mtrx_in[i], GPIO_INPUT);
    }

    gpio_pin_configure_dt(&usr_btn, GPIO_INPUT);

    for (uint16_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++) {
        gpio_pin_set_dt(&key_mtrx_out[i], 0);
    }
    gpio_pin_set_dt(&pwr_pin, 1);
}

static void special_key_handler(kb_key_t *key) {
    if (!key->is_pressed) {
        return;
    }
    switch (key->scan_code) {
        case (ADV_ACC):
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
    int btn_pressed;
    bool key_changed = 0;

    gpio_pin_set_dt(&key_mtrx_out[scan_col], 0);
    if (scan_col < KEY_MATRIX_OUT_COUNT - 1) {
        scan_col++;
    } else {
        scan_col = 0;
    }
    gpio_pin_set_dt(&key_mtrx_out[scan_col], 1);

    for (uint16_t j = 0; j < KEY_MATRIX_IN_COUNT; j++) {
        btn_pressed = gpio_pin_get(key_mtrx_in[j].port, key_mtrx_in[j].pin);
        if (key_mtrx[j][scan_col].is_pressed != btn_pressed) {
            key_mtrx[j][scan_col].is_pressed = btn_pressed;

            if (key_mtrx[j][scan_col].scan_code >= ADV_ACC &&
                key_mtrx[j][scan_col].scan_code <= SCAN_ACCEPT) {
                special_key_handler(&key_mtrx[j][scan_col]);
            } else {
                kb_hid_set_key_evt(&(key_mtrx[j][scan_col]));
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
