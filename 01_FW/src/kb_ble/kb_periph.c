#include <assert.h>
#include <bluetooth/services/hids.h>
#include <errno.h>
#include <soc.h>
#include <stddef.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/bluetooth/services/dis.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/spinlock.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>
#include <zephyr/types.h>
// #include <dk_buttons_and_leds.h>

#include "kb_hid_common.h"
#include "kb_periph.h"

/* Current report map construction requires exactly 8 buttons */
BUILD_ASSERT((KEY_CTRL_CODE_MAX - KEY_CTRL_CODE_MIN) + 1 == 8);

/* OUT report internal indexes.
 *
 * This is a position in internal report table and is not related to
 * report ID.
 */
enum {
    OUTPUT_REP_KEYS_IDX = 0
};

/* INPUT report internal indexes.
 *
 * This is a position in internal report table and is not related to
 * report ID.
 */
enum {
    INPUT_REP_KEYS_IDX = 0
};

/* HIDS instance. */
BT_HIDS_DEF(hids_obj,
            OUTPUT_REPORT_MAX_LEN,
            INPUT_REPORT_KEYS_MAX_LEN);

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_GAP_APPEARANCE,
                  (CONFIG_BT_DEVICE_APPEARANCE >> 0) & 0xff,
                  (CONFIG_BT_DEVICE_APPEARANCE >> 8) & 0xff),
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_HIDS_VAL),
                  BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static struct conn_mode {
    struct bt_conn *conn;
    bool in_boot_mode;
} conn_mode[CONFIG_BT_HIDS_MAX_CLIENT_COUNT];

static struct k_work pairing_work;
struct pairing_data_mitm {
    struct bt_conn *conn;
    unsigned int passkey;
};

K_MSGQ_DEFINE(mitm_queue,
              sizeof(struct pairing_data_mitm),
              CONFIG_BT_HIDS_MAX_CLIENT_COUNT,
              4);

static volatile bool is_adv;
struct k_spinlock adv_spinlock;

static void set_adv_state(bool adv) {
    K_SPINLOCK(&adv_spinlock) {
        is_adv = adv;
    }
}

bool kb_periph_is_adv() {
    bool rtn;
    K_SPINLOCK(&adv_spinlock) {
        rtn = is_adv;
    }
    return rtn;
}

void kb_periph_adv_start(void) {
    int err;
    struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
        BT_LE_ADV_OPT_CONNECTABLE |
            BT_LE_ADV_OPT_ONE_TIME,  // test this: will not start advertising after disconnect.
        BT_GAP_ADV_FAST_INT_MIN_2,   // 160*0.625ms=100ms minimal interval for fast advertising
        BT_GAP_ADV_FAST_INT_MAX_2,   // maximum interval for fast adertising
        NULL);

    if (kb_periph_is_adv()) {
        printk("Advertising continued\n");
        return;
    }

    err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd,
                          ARRAY_SIZE(sd));
    if (err) {
        if (err == -EALREADY) {
            printk("Advertising continued\n");
        } else {
            printk("Advertising failed to start (err %d)\n", err);
        }

        return;
    }

    set_adv_state(true);
    printk("Advertising successfully started\n");
}

static void pairing_process(struct k_work *work) {
    int err;
    struct pairing_data_mitm pairing_data;

    char addr[BT_ADDR_LE_STR_LEN];

    err = k_msgq_peek(&mitm_queue, &pairing_data);
    if (err) {
        return;
    }

    bt_addr_le_to_str(bt_conn_get_dst(pairing_data.conn),
                      addr, sizeof(addr));

    printk("Passkey for %s: %06u\n", addr, pairing_data.passkey);
    printk("Press Button 1 to confirm, Button 2 to reject.\n");
}

void periph_connected(struct bt_conn *conn, uint8_t err) {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err) {
        printk("Failed to connect to %s (%u)\n", addr, err);
        return;
    }

    printk("Connected %s\n", addr);

    err = bt_hids_connected(&hids_obj, conn);

    if (err) {
        printk("Failed to notify HID service about connection\n");
        return;
    }

    for (size_t i = 0; i < CONFIG_BT_HIDS_MAX_CLIENT_COUNT; i++) {
        if (!conn_mode[i].conn) {
            conn_mode[i].conn = conn;
            conn_mode[i].in_boot_mode = false;
            break;
        }
    }

    set_adv_state(false);
}

void periph_disconnected(struct bt_conn *conn, uint8_t reason) {
    int err;
    bool is_any_dev_connected = false;
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Disconnected from %s (reason %u)\n", addr, reason);

    err = bt_hids_disconnected(&hids_obj, conn);

    if (err) {
        printk("Failed to notify HID service about disconnection\n");
    }

    for (size_t i = 0; i < CONFIG_BT_HIDS_MAX_CLIENT_COUNT; i++) {
        if (conn_mode[i].conn == conn) {
            conn_mode[i].conn = NULL;
        } else {
            if (conn_mode[i].conn) {
                is_any_dev_connected = true;
            }
        }
    }

    kb_periph_adv_start();
}

static void hids_outp_rep_handler(struct bt_hids_rep *rep,
                                  struct bt_conn *conn,
                                  bool write) {
    char addr[BT_ADDR_LE_STR_LEN];

    if (!write) {
        printk("Output report read\n");
        return;
    };

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    printk("Output report has been received %s\n", addr);
}

static void hids_boot_kb_outp_rep_handler(struct bt_hids_rep *rep,
                                          struct bt_conn *conn,
                                          bool write) {
    char addr[BT_ADDR_LE_STR_LEN];

    if (!write) {
        printk("Output report read\n");
        return;
    };

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    printk("Boot Keyboard Output report has been received %s\n", addr);
}

static void hids_pm_evt_handler(enum bt_hids_pm_evt evt,
                                struct bt_conn *conn) {
    char addr[BT_ADDR_LE_STR_LEN];
    size_t i;

    for (i = 0; i < CONFIG_BT_HIDS_MAX_CLIENT_COUNT; i++) {
        if (conn_mode[i].conn == conn) {
            break;
        }
    }

    if (i >= CONFIG_BT_HIDS_MAX_CLIENT_COUNT) {
        printk("Cannot find connection handle when processing PM");
        return;
    }

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    switch (evt) {
        case BT_HIDS_PM_EVT_BOOT_MODE_ENTERED:
            printk("Boot mode entered %s\n", addr);
            conn_mode[i].in_boot_mode = true;
            break;

        case BT_HIDS_PM_EVT_REPORT_MODE_ENTERED:
            printk("Report mode entered %s\n", addr);
            conn_mode[i].in_boot_mode = false;
            break;

        default:
            break;
    }
}

static void hid_init(void) {
    int err;
    struct bt_hids_init_param hids_init_obj = {0};
    struct bt_hids_inp_rep *hids_inp_rep;
    struct bt_hids_outp_feat_rep *hids_outp_rep;

    static const uint8_t report_map[] = {
        0x05, 0x01, /* Usage Page (Generic Desktop) */
        0x09, 0x06, /* Usage (Keyboard) */
        0xA1, 0x01, /* Collection (Application) */

    /* Keys */
#if INPUT_REP_KEYS_REF_ID
        0x85, INPUT_REP_KEYS_REF_ID,
#endif
        0x05, 0x07, /* Usage Page (Key Codes) */
        0x19, 0xe0, /* Usage Minimum (224) */
        0x29, 0xe7, /* Usage Maximum (231) */
        0x15, 0x00, /* Logical Minimum (0) */
        0x25, 0x01, /* Logical Maximum (1) */
        0x75, 0x01, /* Report Size (1) */
        0x95, 0x08, /* Report Count (8) */
        0x81, 0x02, /* Input (Data, Variable, Absolute) */

        0x95, 0x01, /* Report Count (1) */
        0x75, 0x08, /* Report Size (8) */
        0x81, 0x01, /* Input (Constant) reserved byte(1) */

        0x95, 0x06, /* Report Count (6) */
        0x75, 0x08, /* Report Size (8) */
        0x15, 0x00, /* Logical Minimum (0) */
        0x25, 0x65, /* Logical Maximum (101) */
        0x05, 0x07, /* Usage Page (Key codes) */
        0x19, 0x00, /* Usage Minimum (0) */
        0x29, 0x65, /* Usage Maximum (101) */
        0x81, 0x00, /* Input (Data, Array) Key array(6 bytes) */

    /* LED */
#if OUTPUT_REP_KEYS_REF_ID
        0x85, OUTPUT_REP_KEYS_REF_ID,
#endif
        0x95, 0x05, /* Report Count (5) */
        0x75, 0x01, /* Report Size (1) */
        0x05, 0x08, /* Usage Page (Page# for LEDs) */
        0x19, 0x01, /* Usage Minimum (1) */
        0x29, 0x05, /* Usage Maximum (5) */
        0x91, 0x02, /* Output (Data, Variable, Absolute), */
        /* Led report */
        0x95, 0x01, /* Report Count (1) */
        0x75, 0x03, /* Report Size (3) */
        0x91, 0x01, /* Output (Data, Variable, Absolute), */
        /* Led report padding */

        0xC0 /* End Collection (Application) */
    };

    hids_init_obj.rep_map.data = report_map;
    hids_init_obj.rep_map.size = sizeof(report_map);

    hids_init_obj.info.bcd_hid = BASE_USB_HID_SPEC_VERSION;
    hids_init_obj.info.b_country_code = 0x00;
    hids_init_obj.info.flags = (BT_HIDS_REMOTE_WAKE |
                                BT_HIDS_NORMALLY_CONNECTABLE);

    hids_inp_rep =
        &hids_init_obj.inp_rep_group_init.reports[INPUT_REP_KEYS_IDX];
    hids_inp_rep->size = INPUT_REPORT_KEYS_MAX_LEN;
    hids_inp_rep->id = INPUT_REP_KEYS_REF_ID;
    hids_init_obj.inp_rep_group_init.cnt++;

    hids_outp_rep =
        &hids_init_obj.outp_rep_group_init.reports[OUTPUT_REP_KEYS_IDX];
    hids_outp_rep->size = OUTPUT_REPORT_MAX_LEN;
    hids_outp_rep->id = OUTPUT_REP_KEYS_REF_ID;
    hids_outp_rep->handler = hids_outp_rep_handler;
    hids_init_obj.outp_rep_group_init.cnt++;

    hids_init_obj.is_kb = true;
    hids_init_obj.boot_kb_outp_rep_handler = hids_boot_kb_outp_rep_handler;
    hids_init_obj.pm_evt_handler = hids_pm_evt_handler;

    err = bt_hids_init(&hids_obj, &hids_init_obj);
    __ASSERT(err == 0, "HIDS initialization failed\n");
}

void periph_auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey) {
    int err;

    struct pairing_data_mitm pairing_data;

    pairing_data.conn = bt_conn_ref(conn);
    pairing_data.passkey = passkey;

    err = k_msgq_put(&mitm_queue, &pairing_data, K_NO_WAIT);
    if (err) {
        printk("Pairing queue is full. Purge previous data.\n");
    }

    /* In the case of multiple pairing requests, trigger
     * pairing confirmation which needed user interaction only
     * once to avoid display information about all devices at
     * the same time. Passkey confirmation for next devices will
     * be proccess from queue after handling the earlier ones.
     */
    if (k_msgq_num_used_get(&mitm_queue) == 1) {
        k_work_submit(&pairing_work);
    }
}

void periph_pairing_failed(struct bt_conn *conn, enum bt_security_err reason) {
    char addr[BT_ADDR_LE_STR_LEN];
    struct pairing_data_mitm pairing_data;

    if (k_msgq_peek(&mitm_queue, &pairing_data) != 0) {
        return;
    }

    if (pairing_data.conn == conn) {
        bt_conn_unref(pairing_data.conn);
        k_msgq_get(&mitm_queue, &pairing_data, K_NO_WAIT);
    }

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Pairing failed conn: %s, reason %d\n", addr, reason);
}

#define KEY_REPORT_WORKQ_THREAD_STACK_SIZE 1024
#define KEY_REPORT_WORKQ_PRIORITY (-9)

static K_THREAD_STACK_DEFINE(key_report_stack, 1024);

static struct k_work_q key_report_work_q = {0};
struct k_work key_report_work = {0};

/** @brief Function process keyboard state and sends it
 *
 *  @param pstate     The state to be sent
 *  @param boot_mode  Information if boot mode protocol is selected.
 *  @param conn       Connection handler
 *
 *  @return 0 on success or negative error code.
 */
static int key_report_con_send(const kb_state_t *state,
                               bool boot_mode,
                               struct bt_conn *conn) {
    int err = 0;
    uint8_t data[INPUT_REPORT_KEYS_MAX_LEN];
    uint8_t *key_data;
    const uint8_t *key_state;
    size_t n;

    data[0] = state->ctrl_keys;
    data[1] = 0;
    key_data = &data[2];
    key_state = state->keys_state;

    for (n = 0; n < KEY_PRESS_MAX; ++n) {
        *key_data++ = *key_state++;
    }
    if (boot_mode) {
        err = bt_hids_boot_kb_inp_rep_send(&hids_obj, conn, data,
                                           sizeof(data), NULL);
    } else {
        err = bt_hids_inp_rep_send(&hids_obj, conn,
                                   INPUT_REP_KEYS_IDX, data,
                                   sizeof(data), NULL);
    }
    return err;
}

static void key_report_send(struct k_work *work) {
    int err;
    static kb_state_t state = {0};
    err = kb_hid_get_state(&state);
    if (!err) {
        printk("key report send: ");
        print_kb_state(&state);
    } else {
        printk("get from queue failed");
    }

    for (size_t i = 0; i < CONFIG_BT_HIDS_MAX_CLIENT_COUNT; i++) {
        if (conn_mode[i].conn) {
            int err;

            err = key_report_con_send(&state,
                                      conn_mode[i].in_boot_mode,
                                      conn_mode[i].conn);
            if (err) {
                printk("Key report send error: %d\n", err);
            }
        }
    }
}

static void num_comp_reply(bool accept) {
    struct pairing_data_mitm pairing_data;
    struct bt_conn *conn;

    if (k_msgq_get(&mitm_queue, &pairing_data, K_NO_WAIT) != 0) {
        return;
    }

    conn = pairing_data.conn;

    if (accept) {
        bt_conn_auth_passkey_confirm(conn);
        printk("Numeric Match, conn %p\n", conn);
    } else {
        bt_conn_auth_cancel(conn);
        printk("Numeric Reject, conn %p\n", conn);
    }

    bt_conn_unref(pairing_data.conn);

    if (k_msgq_num_used_get(&mitm_queue)) {
        k_work_submit(&pairing_work);
    }
}

// move to
void kb_periph_bas_notify(void) {
    uint8_t battery_level = bt_bas_get_battery_level();

    battery_level--;

    if (!battery_level) {
        battery_level = 100U;
    }

    bt_bas_set_battery_level(battery_level);
}

void kb_periph_accept_pairing() {
    if (k_msgq_num_used_get(&mitm_queue)) {
        num_comp_reply(true);
    }
}

void kb_periph_hid_notify() {
    k_work_submit_to_queue(&key_report_work_q, &key_report_work);
}

void kb_periph_init() {
    printk("HIDs initalizing\n");
    hid_init();

    k_work_init(&pairing_work, pairing_process);
    printk("initialized pairing work");

    k_work_queue_start(&key_report_work_q, key_report_stack,
                       K_THREAD_STACK_SIZEOF(key_report_stack), KEY_REPORT_WORKQ_PRIORITY,
                       NULL);
    k_work_init(&key_report_work, key_report_send);
    printk("initialized key report work queue");
}