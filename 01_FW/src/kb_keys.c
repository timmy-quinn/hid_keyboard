#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <hal/nrf_gpio.h>
#include "utils.h"
#include "scan_codes.h"
#include "kb_keys.h"

#define GPIO_NODE_0 DT_NODELABEL(gpio0)
#define GPIO_NAME_0 DEVICE_DT_NAME(GPIO_NODE_0) 

#define GPIO_NODE_1 DT_NODELABEL(gpio1)
#define GPIO_NAME_1 DEVICE_DT_NAME(GPIO_NODE_1) 

#define KEY_MATRIX_OUT_COUNT 8
#define KEY_MATRIX_IN_COUNT 7
#define SLEEP_TIME_MS 2 

#define EXT_LED_0        DT_NODELABEL(ledx0)
#define EXT_LED_1        DT_NODELABEL(ledx1)
#define EXT_LED_2        DT_NODELABEL(ledx2)
#define EXT_LED_3        DT_NODELABEL(ledx3)
#define EXT_LED_4        DT_NODELABEL(ledx4)
#define EXT_LED_5        DT_NODELABEL(ledx5)
#define EXT_LED_6        DT_NODELABEL(ledx6)
#define EXT_LED_7        DT_NODELABEL(ledx7)

#define TEST_LED        DT_NODELABEL(testled)

#define BTN_MATRIX_0    DT_NODELABEL(btnmtrx0)
#define BTN_MATRIX_1    DT_NODELABEL(btnmtrx1)
#define BTN_MATRIX_2    DT_NODELABEL(btnmtrx2)
#define BTN_MATRIX_3    DT_NODELABEL(btnmtrx3)
#define BTN_MATRIX_4    DT_NODELABEL(btnmtrx4)
#define BTN_MATRIX_5    DT_NODELABEL(btnmtrx5)
#define BTN_MATRIX_6    DT_NODELABEL(btnmtrx6)

typedef struct _kybrd_key_{
    uint8_t scan_code; 
    int state; 
}kybrd_key_t; 

static struct k_work_delayable key_mtrx_scan;
static uint8_t mtrx_row_active; 
static key_change_cb key_cb; 

static const struct gpio_dt_spec test_led = GPIO_DT_SPEC_GET(DT_NODELABEL(testled), gpios);

static const struct gpio_dt_spec key_mtrx_out[KEY_MATRIX_OUT_COUNT] = {
    GPIO_DT_SPEC_GET(DT_NODELABEL(ledx0), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(ledx1), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(ledx2), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(ledx3), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(ledx4), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(ledx5), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(ledx6), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(ledx7), gpios)
};

static const struct gpio_dt_spec key_mtrx_in[KEY_MATRIX_IN_COUNT] = {
    GPIO_DT_SPEC_GET(DT_NODELABEL(btnmtrx0), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(btnmtrx1), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(btnmtrx2), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(btnmtrx3), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(btnmtrx4), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(btnmtrx5), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(btnmtrx6), gpios)
};

kybrd_key_t scan_code_mtrx[KEY_MATRIX_OUT_COUNT][KEY_MATRIX_IN_COUNT] = {
    {{KEY_A, 0}, {KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0}}, 
    {{KEY_A, 0}, {KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0}}, 
    {{KEY_A, 0}, {KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0}}, 
    {{KEY_A, 0}, {KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0}}, 
    {{KEY_A, 0}, {KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0}}, 
    {{KEY_A, 0}, {KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0}}, 
    {{KEY_A, 0}, {KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0}}, 
    {{KEY_A, 0}, {KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0},{KEY_A, 0}} 
}; 


static void key_gpios_init() {
    gpio_pin_configure_dt(&test_led, GPIO_OUTPUT); 
    for(uint16_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++) {
        gpio_pin_configure_dt(&key_mtrx_out[i], GPIO_OUTPUT);
    }

    for(uint16_t i = 0; i < KEY_MATRIX_IN_COUNT; i++) {
        gpio_pin_configure_dt(&key_mtrx_in[i], GPIO_INPUT);
    }

    for(uint16_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++) {
        gpio_pin_set_dt(&key_mtrx_out[i], 0);
    }
    gpio_pin_set_dt(&test_led, 0);
}


static void key_mtrx_scan_fn() {
    int btn_pressed; 

    gpio_pin_set_dt(&key_mtrx_out[mtrx_row_active], 0);
    CIRCULAR_INCREMENT(mtrx_row_active, 0, KEY_MATRIX_OUT_COUNT -1);
    gpio_pin_set_dt(&key_mtrx_out[mtrx_row_active], 1);

    for(uint16_t j = 0; j < KEY_MATRIX_IN_COUNT; j++) {
        btn_pressed = gpio_pin_get(key_mtrx_in[j].port, key_mtrx_in[j].pin); 
        if(scan_code_mtrx[mtrx_row_active][j].state != btn_pressed) {
            gpio_pin_set_dt(&test_led, btn_pressed); 
            if(key_cb) {
                key_cb(&scan_code_mtrx[mtrx_row_active][j].scan_code, btn_pressed);
            }
            scan_code_mtrx[mtrx_row_active][j].state = btn_pressed;    
        }
    }
	k_work_reschedule(&key_mtrx_scan, K_MSEC(SLEEP_TIME_MS));
}

void kb_keys_init(key_change_cb callback) {
    key_cb = callback; 
    key_gpios_init(); 
	k_work_init_delayable(&key_mtrx_scan, key_mtrx_scan_fn);
	k_work_schedule(&key_mtrx_scan, K_NO_WAIT);
}
