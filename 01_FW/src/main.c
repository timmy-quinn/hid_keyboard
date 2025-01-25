#include "btns.h"
#include "kb_ble/kb_ble.h"
#include "kb_ble/kb_cent.h"
#include "kb_ble/kb_periph.h"

#include <zephyr/settings/settings.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/drivers/gpio.h>
#include <hal/nrf_gpio.h>
#include <zephyr/kernel.h>


#define ADV_LED_BLINK_INTERVAL  1000

static const struct gpio_dt_spec adv_status_led = GPIO_DT_SPEC_GET(DT_NODELABEL(led1), gpios);
static btn_callbacks btn_cb =  {
								.btn_0 = kb_periph_accept_pairing,
								.btn_1 = advertising_start,
								.btn_2 = kb_cent_scan_start,
								.btn_3 = cent_pairing_accept,
								};

int main(void)
{
	int blink_status = 0;
   	gpio_pin_configure_dt(&adv_status_led, GPIO_OUTPUT);

	// bt_enable must come before settings_load(); not 100% sure why, but bt_enable appears 
	// to have a bt_settings_init usring similar macros
	if(ble_init()) {
		return 0;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		printk("loading settings\n");
		settings_load();
	}
	kb_periph_init();
	printk("Intialized HID keyboard device\n");
	kb_cent_init();
	printk("Initialize HOGP\n");

	printk("Bluetooth initialized\n");
	btns_init(&btn_cb); 

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