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

static const struct gpio_dt_spec *keyMatrixOut[KEY_MATRIX_OUT_COUNT];
static const struct gpio_dt_spec *keyMatrixIn[KEY_MATRIX_IN_COUNT];
static const struct gpio_dt_spec *testLED; 
uint8_t scanCodeMatrix[KEY_MATRIX_OUT_COUNT][KEY_MATRIX_IN_COUNT]; 

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

void initKeyMatrix() {
    static const struct gpio_dt_spec ledx0_spec = GPIO_DT_SPEC_GET(EXT_LED_0, gpios);
    static const struct gpio_dt_spec ledx1_spec = GPIO_DT_SPEC_GET(EXT_LED_1, gpios);
    static const struct gpio_dt_spec ledx2_spec = GPIO_DT_SPEC_GET(EXT_LED_2, gpios);
    static const struct gpio_dt_spec ledx3_spec = GPIO_DT_SPEC_GET(EXT_LED_3, gpios);
    static const struct gpio_dt_spec ledx4_spec = GPIO_DT_SPEC_GET(EXT_LED_4, gpios);
    static const struct gpio_dt_spec ledx5_spec = GPIO_DT_SPEC_GET(EXT_LED_5, gpios);
    static const struct gpio_dt_spec ledx6_spec = GPIO_DT_SPEC_GET(EXT_LED_6, gpios);
    static const struct gpio_dt_spec ledx7_spec = GPIO_DT_SPEC_GET(EXT_LED_7, gpios);

    static const struct gpio_dt_spec testLed_spec = GPIO_DT_SPEC_GET(TEST_LED, gpios);

    static const struct gpio_dt_spec btnMtrx0_spec = GPIO_DT_SPEC_GET(BTN_MATRIX_0, gpios);  
    static const struct gpio_dt_spec btnMtrx1_spec = GPIO_DT_SPEC_GET(BTN_MATRIX_1, gpios);  
    static const struct gpio_dt_spec btnMtrx2_spec = GPIO_DT_SPEC_GET(BTN_MATRIX_2, gpios);  
    static const struct gpio_dt_spec btnMtrx3_spec = GPIO_DT_SPEC_GET(BTN_MATRIX_3, gpios);  
    static const struct gpio_dt_spec btnMtrx4_spec = GPIO_DT_SPEC_GET(BTN_MATRIX_4, gpios);  
    static const struct gpio_dt_spec btnMtrx5_spec = GPIO_DT_SPEC_GET(BTN_MATRIX_5, gpios);  
    static const struct gpio_dt_spec btnMtrx6_spec = GPIO_DT_SPEC_GET(BTN_MATRIX_6, gpios);  

    keyMatrixOut[0] = &ledx0_spec;
    keyMatrixOut[1] = &ledx1_spec;
    keyMatrixOut[2] = &ledx2_spec;
    keyMatrixOut[3] = &ledx3_spec;
    keyMatrixOut[4] = &ledx4_spec;
    keyMatrixOut[5] = &ledx5_spec;
    keyMatrixOut[6] = &ledx6_spec;
    keyMatrixOut[7] = &ledx7_spec;

    testLED = &testLed_spec; 

    keyMatrixIn[0] = &btnMtrx0_spec; 
    keyMatrixIn[1] = &btnMtrx1_spec; 
    keyMatrixIn[2] = &btnMtrx2_spec; 
    keyMatrixIn[3] = &btnMtrx3_spec; 
    keyMatrixIn[4] = &btnMtrx4_spec; 
    keyMatrixIn[5] = &btnMtrx5_spec; 
    keyMatrixIn[6] = &btnMtrx6_spec; 
    
    gpio_pin_configure_dt(testLED, GPIO_OUTPUT); 
    for(uint16_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++) {
        gpio_pin_configure_dt(keyMatrixOut[i], GPIO_OUTPUT);
    }

    for(uint16_t i = 0; i < KEY_MATRIX_IN_COUNT; i++) {
        gpio_pin_configure_dt(keyMatrixIn[i], GPIO_INPUT);
    }


    // for(uint16_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++) {
    //     gpio_pin_set_dt(keyMatrixOut[i], 1);
    // }
}

void keyMatrixTask() {
    initKeyMatrix(); 
    int isPressed = 0; 


    while(1) {
        for(uint16_t i = 0; i < KEY_MATRIX_OUT_COUNT; i++) {
            gpio_pin_set_dt(keyMatrixOut[i], 1);
            for(uint16_t j = 0; j < KEY_MATRIX_IN_COUNT; j++) {
                isPressed = gpio_pin_get(keyMatrixIn[j]->port, keyMatrixIn[j]->pin);
                if(isPressed) {
                    gpio_pin_set_dt(testLED, 1); 
                }
            }
            k_msleep(SLEEP_TIME_MS); 
            gpio_pin_set_dt(keyMatrixOut[i], 0);
            gpio_pin_set_dt(testLED, 0); 
        }
    }
}
