#pragma once
#include "board.h"
#include "foc/foc_core.h"
#include "project_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int16_t pole_pairs;
    uint32_t ang_offset;
    uint32_t a_max;
    uint32_t a_min;
    uint32_t b_max;
    uint32_t b_min;
} encoder_t;

typedef void (*ecnoder_callback_t)(uint32_t flag);

#if ENCODER_TYPE == ENCODER_MT6835_SPI_ABZ
#include "mt6835_spi_abz.h"
#else
#error "unsupport encoder"
#endif

static inline float encoder_get_eleAngle(const encoder_t *self, uint32_t raw_ang)
{
#ifdef FAST_SIN_2PIX
    return ((int32_t)(raw_ang - self->ang_offset) * self->pole_pairs) * (1 / 65536.0f);
#else
    return ((int32_t)(raw_ang - self->ang_offset) * self->pole_pairs) * (2.0f * F_PI / 65536.0f);
#endif
}

static inline void encoder_get_eleAngle_sincos(const encoder_t *self, uint32_t raw_ang, foc_sin_cos_t *sincos)
{
    foc_sin_cos(encoder_get_eleAngle(self, raw_ang), sincos);
}

#ifdef __cplusplus
}
#endif