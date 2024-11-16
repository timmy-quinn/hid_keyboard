#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <hal/nrf_gpio.h>
#include "keys.h"
#include "scan_codes.h"

/* 
* NRF dongle pins
* side 1 
* 0.13
* 0.15
* 0.17
* 0.20
* 0.22
* 1.24
* 1.00
* 0.09
* 0.10 
* 
* side 2 
* 1.10
* 1.13
* 1.15
* 0.02
* 0.29
* 0.31
* 
* If all pins are used, we have 56 pins available
*/

#define GPIO_NODE_0 DT_NODELABEL(gpio0)
#define GPIO_NAME_0 DEVICE_DT_NAME(GPIO_NODE_0) 

#define GPIO_NODE_1 DT_NODELABEL(gpio1)
#define GPIO_NAME_1 DEVICE_DT_NAME(GPIO_NODE_1) 

#define KEY_MATRIX_OUT_COUNT 8
#define KEY_MATRIX_IN_COUNT 7
#define SLEEP_TIME_MS 2 

static const struct gpio_dt_spec *key_mtrx_out[KEY_MATRIX_OUT_COUNT];
static const struct gpio_dt_spec *key_mtrx_in[KEY_MATRIX_IN_COUNT];
static struct k_work_delayable key_mtrx_scan;
static const struct gpio_dt_spec *test_led;

typedef struct _kybrd_key_{
    uint8_t scan_code; 
    int state; 
}kybrd_key_t; 

#define KEY_RELEASED 1
#define KEY_PRESSED 1
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

static void key_gpios_init() {

    static const struct gpio_dt_spec ledx0_spec = GPIO_DT_SPEC_GET(EXT_LED_0, gpios);
    static const struct gpio_dt_spec ledx1_spec = GPIO_DT_SPEC_GET(EXT_LED_1, gpios);
    static const struct gpio_dt_spec ledx2_spec = GPIO_DT_SPEC_GET(EXT_LED_2, gpios);
    static const struct gpio_dt_spec ledx3_spec = GPIO_DT_SPEC_GET(EXT_LED_3, gpios);
    static const struct gpio_dt_spec ledx4_spec = GPIO_DT_SPEC_GET(EXT_LED_4, gpios);
    static const struct gpio_dt_spec ledx5_spec = GPIO_DT_SPEC_GET(EXT_LED_5, gpios);
    static const struct gpio_dt_spec ledx6_spec = GPIO_DT_SPEC_GET(EXT_LED_6, gpios);
    static const struct gpio_dt_spec ledx7_spec = GPIO_DT_SPEC_GET(EXT_LED_7, gpios);

    static const struct gpio_dt_spec test_led_spec = GPIO_DT_SPEC_GET(TEST_LED, gpios);

    static const struct gpio_dt_spec btnMtrx0_spec = GPIO_DT_SPEC_GET(BTN_MATRIX_0, gpios);  
    static const struct gpio_dt_spec btnMtrx1_spec = GPIO_DT_SPEC_GET(BTN_MATRIX_1, gpios);  
    static const struct gpio_dt_spec btnMtrx2_spec = GPIO_DT_SPEC_GET(BTN_MATRIX_2, gpios);  
    static const struct gpio_dt_spec btnMtrx3_spec = GPIO_DT_SPEC_GET(BTN_MATRIX_3, gpios);  
    static const struct gpio_dt_spec btnMtrx4_spec = GPIO_DT_SPEC_GET(BTN_MATRIX_4, gpios);  
    static const struct gpio_dt_spec btnMtrx5_spec = GPIO_DT_SPEC_GET(BTN_MATRIX_5, gpios);  
    static const struct gpio_dt_spec btnMtrx6_spec = GPIO_DT_SPEC_GET(BTN_MATRIX_6, gpios);  

    key_mtrx_out[0] = &ledx0_spec;
    key_mtrx_out[1] = &ledx1_spec;
    key_mtrx_out[2] = &ledx2_spec;
    key_mtrx_out[3] = &ledx3_spec;
    key_mtrx_out[4] = &ledx4_spec;
    key_mtrx_out[5] = &ledx5_spec;
    key_mtrx_out[6] = &ledx6_spec;
    key_mtrx_out[7] = &ledx7_spec;

    test_led = &test_led_spec; 

    key_mtrx_in[0] = &btnMtrx0_spec; 
    key_mtrx_in[1] = &btnMtrx1_spec; 
    key_mtrx_in[2] = &btnMtrx2_spec; 
    key_mtrx_in[3] = &btnMtrx3_spec; 
    key_mtrx_in[4] = &btnMtrx4_spec; 
    key_mtrx_in[5] = &btnMtrx5_spec; 
    key_mtrx_in[6] = &btnMtrx6_spec; 
    
    gpio_pin_configure_dt(test_led, GPIO_OUTPUT); 
    for(uint16_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++) {
        gpio_pin_configure_dt(key_mtrx_out[i], GPIO_OUTPUT);
    }

    for(uint16_t i = 0; i < KEY_MATRIX_IN_COUNT; i++) {
        gpio_pin_configure_dt(key_mtrx_in[i], GPIO_INPUT);
    }

    for(uint16_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++) {
        gpio_pin_set_dt(key_mtrx_out[i], 0);
    }
    gpio_pin_set_dt(test_led, 0);
}

#define CIRCULAR_INC(i, min, max) do{if(i < max) {i++;} else {i = min;}}while(0)
static uint8_t mtrx_row_active; 
static key_change_cb key_cb; 

static void key_mtrx_scan_fn() {
    gpio_pin_set_dt(key_mtrx_out[mtrx_row_active], 0);
    // CIRCULAR_INC(mtrx_row_active, 0, ARRAY_SIZE(key_mtrx_out) - 1);
    if(mtrx_row_active < ARRAY_SIZE(key_mtrx_out) - 1) {
        mtrx_row_active++; 
    }
    else {
        mtrx_row_active = 0; 
    }

    gpio_pin_set_dt(key_mtrx_out[mtrx_row_active], 1);
    int btn_pressed; 

    for(uint16_t j = 0; j < KEY_MATRIX_IN_COUNT; j++) {
        btn_pressed = gpio_pin_get(key_mtrx_in[j]->port, key_mtrx_in[j]->pin); 
        // test pin
        if(scan_code_mtrx[mtrx_row_active][j].state != btn_pressed) {
            scan_code_mtrx[mtrx_row_active][j].state = btn_pressed;
            gpio_pin_set_dt(test_led, btn_pressed); 
            if(key_cb) {
                key_cb(&scan_code_mtrx[mtrx_row_active][j].scan_code, btn_pressed);
            }
            scan_code_mtrx[mtrx_row_active][j].state = btn_pressed;    
        }
        
    }

	k_work_reschedule(&key_mtrx_scan, K_MSEC(10));
}

void key_mtrx_init(key_change_cb callback) {
    key_cb = callback; 
    key_gpios_init(); 
	k_work_init_delayable(&key_mtrx_scan, key_mtrx_scan_fn);
	k_work_schedule(&key_mtrx_scan, K_NO_WAIT);
}
