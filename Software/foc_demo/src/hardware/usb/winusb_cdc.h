#pragma once

#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*usb_read_cb_t)(uint8_t *, uint32_t);
 
void winusb_cdc_init(uint8_t busid, uint32_t reg_base, const char *serial_str);

void usb_cdc_write(void *buf, uint32_t len);
void usb_cdc_set_read_cb(usb_read_cb_t cb);
bool usb_cdc_dtr_isActivate();

#ifdef __cplusplus
}
#endif