#include <hal/nrf_gpio.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>

// Neopixel library 
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/util.h>

#include "btns.h"
#include "kb_ble/kb_ble.h"
#include "kb_ble/kb_cent.h"
#include "kb_ble/kb_periph.h"


#define ADV_LED_BLINK_INTERVAL 1000

static const struct gpio_dt_spec adv_status_led = GPIO_DT_SPEC_GET(DT_NODELABEL(led1), gpios);

#define NEOPIXEL_NODE DT_ALIAS(led_strip)
#define NEOPIXEL_NUM_LEDS DT_PROP(DT_ALIAS(led_strip), chain_length)

#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }
#define NEO_RED RGB(0x0f, 0x00, 0x00)

static const struct led_rgb colors[] = {
	RGB(0x0f, 0x00, 0x00), /* red */
	RGB(0x00, 0x0f, 0x00), /* green */
	RGB(0x00, 0x00, 0x0f), /* blue */
};

struct led_rgb pixels[NEOPIXEL_NUM_LEDS];

static const struct device *neopixels = DEVICE_DT_GET(NEOPIXEL_NODE);
static volatile const struct device *foo;  

int main(void) {
    int blink_status = 0;
    gpio_pin_configure_dt(&adv_status_led, GPIO_OUTPUT);

    foo  = neopixels;

	if (device_is_ready(neopixels)) {
        printk("Device ready\n");
	} else {
        printk("Device not ready\n");
	}

    pixels[0].b = 0x0f;
    pixels[0].g = 0x0f;
    pixels[0].r = 0x0f;

    int rc;
    while(1) {
        printk("Turning LED on...\n");
        rc = led_strip_update_rgb(neopixels, pixels, NEOPIXEL_NUM_LEDS);
        if(rc != 0) {
            printk("error\n"); 

        }
        
        k_sleep(K_MSEC(ADV_LED_BLINK_INTERVAL));
        printk("LED turned on\n");
    }

    // bt_enable must come before settings_load(); not 100% sure why, but bt_enable appears
    // to have a bt_settings_init usring similar macros
    // if (ble_init()) {
    //     return 0;
    // }

    // if (IS_ENABLED(CONFIG_SETTINGS)) {
    //     printk("loading settings\n");
    //     settings_load();
    // }
    // kb_periph_init();
    // printk("Intialized HID keyboard device\n");
    // kb_cent_init();
    // printk("Initialize HOGP\n");

    // printk("Bluetooth initialized\n");
    // btns_init();


    for (;;) {
        if (kb_periph_is_adv()) {
            gpio_pin_set_dt(&adv_status_led, (++blink_status) % 2);
        } else {
            gpio_pin_set_dt(&adv_status_led, 0);
        }
        k_sleep(K_MSEC(ADV_LED_BLINK_INTERVAL));
        /* Battery level simulation */
        kb_periph_bas_notify();
    }
}