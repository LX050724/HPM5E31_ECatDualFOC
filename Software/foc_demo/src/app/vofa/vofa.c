#include "vofa.h"
#include "hardware/usb/winusb_cdc.h"
#include "project_config.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    const char *name;
    volatile float *tar_val;
    vofa_modif_cb_t cb;
} CmdCallback_t;

#define BUF_NUM (2048 * 2 / sizeof(just_float_data) - 2)

static DMA_ATTR just_float_data buf[BUF_NUM];
static int wbuf_index;
static CmdCallback_t dict[16];
static int dict_len;

void vofa_register_dict(const char *key, float *p, vofa_modif_cb_t cb)
{
    if (dict_len < sizeof(dict) / sizeof(dict[0]))
    {
        dict[dict_len].name = key;
        dict[dict_len].tar_val = p;
        dict[dict_len].cb = cb;
        dict_len++;
    }
}

void vofa_init()
{
    memset(buf, 0, sizeof(buf));
    for (int i = 0; i < BUF_NUM; i++)
    {
        buf[i].tail[0] = 0x00;
        buf[i].tail[1] = 0x00;
        buf[i].tail[2] = 0x80;
        buf[i].tail[3] = 0x7f;
    }
}

just_float_data *vofa_alloc_block()
{
    return usb_cdc_dtr_isActivate() ? &buf[wbuf_index] : NULL;
}

void vofa_push_data()
{
    wbuf_index += 1;
    if (wbuf_index == BUF_NUM)
    {
        wbuf_index = 0;
        usb_cdc_write(&buf[BUF_NUM / 2], sizeof(buf) / 2);
    }
    else if (wbuf_index == BUF_NUM / 2)
    {
        usb_cdc_write(&buf[0], sizeof(buf) / 2);
    }
}

void vofa_read_cb(uint8_t *data, uint32_t len)
{
    char name[64] = {};
    float value = 0;
    char *start = (char *)data;
    for (int i = 0; i < len - 1; i++)
    {
        if (data[i] == ':')
        {
            strncpy(name, start, (char *)data + i - start);
            value = strtof((char *)&data[i + 1], &start);
            i = start - (char *)data;
            if (start[0] == '\n')
                start++;
            for (int index = 0; index < dict_len; index++)
            {
                if (strcmp(dict[index].name, name) == 0)
                {
                    /* 写入数据，设置更改地址偏移量，发送消息 */
                    if (dict[index].cb)
                    {
                        dict[index].cb(value);
                    }
                    else
                    {
                        *(dict[index].tar_val) = value;
                    }
                    break;
                }
            }
        }
    }
}