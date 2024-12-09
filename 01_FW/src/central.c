#include <bluetooth/services/hogp.h>
#include "central.h"

static struct bt_hogp hogp; 

static uint8_t hogp_notify_cb(struct bt_hogp *hogp, struct bt_hogp_rep_info *rep, uint8_t err, const uint8_t *data) {
    uint8_t size = bt_hogp_rep_size(rep); 

    if(!data) {
        return BT_GATT_ITER_STOP; 
    }
    printk("Notification, id: %u, size %u, data", 
        bt_hogp_rep_id(rep), size); 
    
    for(uint8_t i = 0; i < size; ++i) {
        printk(" 0x%x", data[i]); 
    }
    printk("\n"); 
    return BT_GATT_ITER_CONTINUE; 
}

static void hids_on_ready(struct k_work *work) {
    int err; 
    struct bt_hogp_rep_info *rep = NULL; 

    printk("HIDS is ready to wrok\n");

 
    while (NULL != (rep = bt_hogp_rep_next(&hogp, rep))) { // cool trick. Didn't know this syntax worked. rep is compared to NULL
        if(bt_hogp_rep_type(rep) == BT_HIDS_REPORT_TYPE_INPUT) {
            printk("subscript to report id: %u\n", bt_hogp_rep_id(rep));
            err = bt_hogp_rep_subscribe(&hogp, rep, hogp_notify_cb); 
            if(err) {
                printk("subscribe error (%d)\n", err); 
            }
        }
   
    }
}

static K_WORK_DEFINE(hids_ready_work, hids_on_ready); 

static void hogp_ready_cb(struct bt_hogp *hogp) {
    k_work_submit(&hids_ready_work); 
}

/* HIDS client initialization parameters */
const struct bt_hogp_init_params hogp_init_params = {
    .ready_cb = hogp_ready_cb, 
    .prep_error_cb = hogp_prep_fail_cb, 
    .pm_update_cb = hogp_pm_update
} 

int main(void) {
    int err; 
    printk("Starting BLE Central HIDS"); 
    bt_hogp_init(&hogp, &hogp_init_params); 
}