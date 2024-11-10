#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <hal/nrf_gpio.h>
#include "keys.h"


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

typedef struct pin {
    const struct device *device; 
    uint16_t pinNum; 
} gpioPin_t; 

#define KEY_MATRIX_OUT_COUNT 8
#define KEY_MATRIX_IN_COUNT 7
#define SLEEP_TIME_MS 2

gpioPin_t keyMatrixOut[KEY_MATRIX_OUT_COUNT];
gpioPin_t keyMatrixIn[KEY_MATRIX_IN_COUNT];

uint8_t scanCodeMatrix[KEY_MATRIX_OUT_COUNT][KEY_MATRIX_IN_COUNT]; 



void initKeyMatrix() {
    const struct device *gpio0_dev = device_get_binding(GPIO_NAME_0);
    const struct device *gpio1_dev = device_get_binding(GPIO_NAME_1);

// GPIO outputs
    keyMatrixOut[0].device = gpio0_dev;
    keyMatrixOut[0].pinNum = 0;

    keyMatrixOut[1].device = gpio0_dev;
    keyMatrixOut[1].pinNum = 1;

    keyMatrixOut[2].device = gpio0_dev;
    keyMatrixOut[2].pinNum = 2;

    keyMatrixOut[3].device = gpio0_dev;
    keyMatrixOut[3].pinNum = 3;

    keyMatrixOut[4].device = gpio0_dev;
    keyMatrixOut[4].pinNum = 4;

    keyMatrixOut[5].device = gpio0_dev;
    keyMatrixOut[5].pinNum = 5;

    keyMatrixOut[6].device = gpio0_dev;
    keyMatrixOut[6].pinNum = 6;

    keyMatrixOut[7].device = gpio0_dev;
    keyMatrixOut[7].pinNum = 7;

// // GPIO inputs 
//     keyMatrixIn[0].device = gpio0_dev;
//     keyMatrixIn[0].pinNum = 10; 

//     keyMatrixIn[1].device = gpio1_dev;
//     keyMatrixIn[1].pinNum = 10; 

//     keyMatrixIn[2].device = gpio1_dev;
//     keyMatrixIn[2].pinNum = 13; 

//     keyMatrixIn[3].device = gpio1_dev;
//     keyMatrixIn[3].pinNum = 15; 

//     keyMatrixIn[4].device = gpio0_dev;
//     keyMatrixIn[4].pinNum = 02; 

//     keyMatrixIn[5].device = gpio0_dev;
//     keyMatrixIn[5].pinNum = 29; 

//     keyMatrixIn[6].device = gpio0_dev;
//     keyMatrixIn[6].pinNum = 31; 

    for(uint16_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++) {
        gpio_pin_configure(keyMatrixOut[i].device, keyMatrixOut[i].pinNum, GPIO_OUTPUT  );
    }
    // for(uint17_t i = 0; i < KEY_MATRIX_IN_COUNT; i++) {
    //     gpio_pin_configure(keyMatrixOut[i].device, keyMatrixOut[i].pinNum, GPIO_INPUT);
    // }
}

#define MY_LED_NODE        DT_NODELABEL(leds4)


void keyMatrixTask() {
    // initKeyMatrix(); 
    // while(1) {
    //     for(uint16_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++){
    //         gpio_pin_set(keyMatrixOut[i].device, keyMatrixOut[i].pinNum, 1); 
    //         k_msleep(1000); 
    //         gpio_pin_set(keyMatrixOut[i].device, keyMatrixOut[i].pinNum, 0); 
    //     }
    // }

    static const struct gpio_dt_spec myLed_spec = GPIO_DT_SPEC_GET(MY_LED_NODE, gpios);
    gpio_pin_configure_dt(&myLed_spec, GPIO_OUTPUT); 

    while(1) {
        gpio_pin_set_dt(&myLed_spec, 1); 
        k_msleep(SLEEP_TIME_MS); 
        gpio_pin_set_dt(&myLed_spec, 0); 
        k_msleep(SLEEP_TIME_MS); 
    }
}
