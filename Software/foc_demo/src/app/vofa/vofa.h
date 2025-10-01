#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float data[15];
    uint8_t tail[4];
} just_float_data;

typedef void(*vofa_modif_cb_t)(float);

void vofa_init();
void vofa_register_dict(const char *key, float *p, vofa_modif_cb_t cb);
just_float_data *vofa_alloc_block();
void vofa_push_data();
void vofa_read_cb(uint8_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif
