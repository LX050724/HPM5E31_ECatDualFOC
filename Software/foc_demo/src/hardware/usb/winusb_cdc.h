#pragma once

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

void winusb_cdc_init(uint8_t busid, uint32_t reg_base, const char *serial_str);

#ifdef __cplusplus
}
#endif