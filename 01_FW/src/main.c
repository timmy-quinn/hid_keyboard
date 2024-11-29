#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>

#include "kb_keys.h"
#include "kb_ble.h"

#define ADV_STATUS_LED DK_LED1
#define ADV_LED_BLINK_INTERVAL  1000

int main(void)
{
	int blink_status = 0;

	kb_ble_init();
	kb_keys_init(kb_ble_key_event); 

	for (;;) {
		if (kb_ble_is_adv()) {
			dk_set_led(ADV_STATUS_LED, (++blink_status) % 2);
		} else {
			dk_set_led_off(ADV_STATUS_LED);
		}
		k_sleep(K_MSEC(ADV_LED_BLINK_INTERVAL));
		/* Battery level simulation */
		kb_ble_bas_notify();
	}
}