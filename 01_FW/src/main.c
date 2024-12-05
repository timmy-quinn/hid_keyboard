#include <zephyr/kernel.h>
// #include <dk_buttons_and_leds.h>
#include <zephyr/drivers/gpio.h>
#include <hal/nrf_gpio.h>

#include "kb_keys.h"
#include "kb_ble.h"

static const struct gpio_dt_spec adv_status_led = GPIO_DT_SPEC_GET(DT_NODELABEL(led1), gpios);
#define ADV_LED_BLINK_INTERVAL  1000

int main(void)
{
	int blink_status = 0;
    gpio_pin_configure_dt(&adv_status_led, GPIO_OUTPUT);
	kb_ble_init();
	kb_keys_init(kb_ble_key_event); 

	for (;;) {
		if (kb_ble_is_adv()) {
			//dk_set_led(ADV_STATUS_LED, (++blink_status) % 2);
    		gpio_pin_set_dt(&adv_status_led, 1);
		} else {
			// dk_set_led_off(ADV_STATUS_LED);
    		gpio_pin_set_dt(&adv_status_led, 0);
		}
		k_sleep(K_MSEC(ADV_LED_BLINK_INTERVAL));
		/* Battery level simulation */
		kb_ble_bas_notify();
	}
}