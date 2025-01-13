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

#define KEY_MATRIX_OUT_COUNT 4
#define KEY_MATRIX_IN_COUNT 4
#define SLEEP_TIME_MS 2 

typedef struct _kybrd_key_{
    uint8_t scan_code; 
    int state; 
}kybrd_key_t; 

static struct k_work_delayable key_mtrx_scan;
static uint8_t mtrx_col_active; 
static key_change_cb key_cb; 
static vv_cb cent_pairing_cb;
static vv_cb periph_pairing_cb;
static vv_cb advertising_cb;
static vv_cb scanning_cb;

static const struct gpio_dt_spec test_led = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);
static const struct gpio_dt_spec pwr_pin = GPIO_DT_SPEC_GET(DT_NODELABEL(pwr), gpios);
static const struct gpio_dt_spec usr_btn = GPIO_DT_SPEC_GET(DT_NODELABEL(button0), gpios);

static const struct gpio_dt_spec key_mtrx_out[KEY_MATRIX_OUT_COUNT] = {
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout0), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout1), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout2), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swout3), gpios)
};

static const struct gpio_dt_spec key_mtrx_in[KEY_MATRIX_IN_COUNT] = {
    GPIO_DT_SPEC_GET(DT_NODELABEL(swin0), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swin1), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swin2), gpios),
    GPIO_DT_SPEC_GET(DT_NODELABEL(swin3), gpios)
};

kybrd_key_t scan_code_mtrx[KEY_MATRIX_OUT_COUNT][KEY_MATRIX_IN_COUNT] = {
    {{PERIPH_PAIRING_ACCEPT, 0}, {ADV_START, 0}, {SCAN_START, 0}, {CENT_PAIRING_ACCEPT, 0}}, 
    {{KEY_E, 0}, {KEY_F, 0},{KEY_G, 0},{KEY_H, 0}}, 
    {{KEY_I, 0}, {KEY_J, 0},{KEY_K, 0},{KEY_L, 0}}, 
    {{KEY_N, 0}, {KEY_O, 0},{KEY_P, 0},{KEY_LSHIFT, 0}} 
}; 


static void key_gpios_init() {
    // gpio_pin_configure_dt(&test_led, GPIO_OUTPUT); 
    for(uint16_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++) {
        gpio_pin_configure_dt(&key_mtrx_out[i], GPIO_OUTPUT);
    }
    gpio_pin_configure_dt(&pwr_pin, GPIO_OUTPUT);


    for(uint16_t i = 0; i < KEY_MATRIX_IN_COUNT; i++) {
        gpio_pin_configure_dt(&key_mtrx_in[i], GPIO_INPUT);
    }
    
    gpio_pin_configure_dt(&usr_btn, GPIO_INPUT);

    for(uint16_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++) {
        gpio_pin_set_dt(&key_mtrx_out[i], 0);
    }
    gpio_pin_set_dt(&pwr_pin, 1);
}


static void key_mtrx_scan_fn() {
    int btn_pressed; 

    gpio_pin_set_dt(&key_mtrx_out[mtrx_col_active], 0);
    if(mtrx_col_active < KEY_MATRIX_OUT_COUNT -1) {
        mtrx_col_active++; 
    }
    else{
        mtrx_col_active = 0; 
    }
    gpio_pin_set_dt(&key_mtrx_out[mtrx_col_active], 1);

    for(uint16_t j = 0; j < KEY_MATRIX_IN_COUNT; j++) {
        btn_pressed = gpio_pin_get(key_mtrx_in[j].port, key_mtrx_in[j].pin); 
        if(scan_code_mtrx[j][mtrx_col_active].state != btn_pressed) {
            //gpio_pin_set_dt(&test_led, btn_pressed); 
            if(scan_code_mtrx[j][mtrx_col_active].scan_code == CENT_PAIRING_ACCEPT && cent_pairing_cb) {
                cent_pairing_cb(); 
            }
            else if(scan_code_mtrx[j][mtrx_col_active].scan_code == PERIPH_PAIRING_ACCEPT && periph_pairing_cb) {
                periph_pairing_cb();
            }
            else if (scan_code_mtrx[j][mtrx_col_active].scan_code == SCAN_START && scanning_cb) {
                scanning_cb();
            }
            else if (scan_code_mtrx[j][mtrx_col_active].scan_code == ADV_START && advertising_cb) {
                advertising_cb();
            }
            else if(key_cb) {
                key_cb(&scan_code_mtrx[j][mtrx_col_active].scan_code, btn_pressed);
            }
            scan_code_mtrx[j][mtrx_col_active].state = btn_pressed;    
        }
    }

    // if(gpio_pin_get(usr_btn.port, usr_btn.pin)) {
    //     printk("Pairing btn pressed\n"); 
    //     periph_pairing_cb(); 
    // }

	k_work_reschedule(&key_mtrx_scan, K_MSEC(SLEEP_TIME_MS));
}

void kb_keys_init(key_change_cb new_key_cb, vv_cb pp_cb, vv_cb a_cb, vv_cb s_cb, vv_cb cp_cb) {
    key_cb = new_key_cb; 
    periph_pairing_cb = pp_cb;
    advertising_cb = a_cb;
    scanning_cb = s_cb;
    cent_pairing_cb = cp_cb;
    
    key_gpios_init(); 
	k_work_init_delayable(&key_mtrx_scan, key_mtrx_scan_fn);
	k_work_schedule(&key_mtrx_scan, K_NO_WAIT);
}
