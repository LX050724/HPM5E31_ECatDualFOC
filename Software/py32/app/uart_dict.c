#include "py32f002b_ll_usart.h"
#include "py32f002bx5.h"
#include <stdint.h>
#include <stdio.h>

static uint8_t uart_rxbuf[8];
static uint32_t buf_index;

static uint8_t *uart_txptr;
static uint32_t uart_txnum;
static uint8_t *uart_txptr_2;
static uint32_t uart_txnum_2;

static uint8_t uart_txbuf[6];
static uint8_t uart_pdo_buf[16];

uint16_t uart_dict[32];
static uint32_t uart_dict_edit_flag;

void uart_recv_frame(uint8_t *data, uint8_t len);

void uart_rxfifo_push(uint8_t data)
{
    if (buf_index < sizeof(uart_rxbuf))
    {
        uart_rxbuf[buf_index++] = data;
    }
}

void uart_rxfifo_rxidle()
{
    uart_recv_frame(uart_rxbuf, buf_index);
    buf_index = 0;
}

int uart_transmit(uint8_t *buf, uint8_t len)
{
    if (uart_txnum > 0)
    {
        if (uart_txnum_2 == 0)
        {
            uart_txptr_2 = buf;
            uart_txnum_2 = len;
            return 0;
        }
        return -1;
    }

    uart_txptr = buf + 1;
    uart_txnum = len - 1;

    LL_USART_TransmitData8(USART1, buf[0]);
    if (uart_txnum > 0)
    {
        LL_USART_EnableIT_TXE(USART1);
    }

    return 0;
}

void uart_transmit_next_byte()
{
    LL_USART_TransmitData8(USART1, *uart_txptr++);
    if (--uart_txnum == 0)
    {
        if (uart_txnum_2)
        {
            uart_txptr = uart_txptr_2;
            uart_txnum = uart_txnum_2;
            uart_txptr_2 = NULL;
            uart_txnum_2 = 0;
        }
        else
        {
            uart_txptr = NULL;
            LL_USART_DisableIT_TXE(USART1);
        }
    }
}

void uart_recv_frame(uint8_t *data, uint8_t len)
{
    struct
    {
        uint8_t head;
        uint8_t addr;
        uint16_t data;
        uint8_t crc;
    } *data_pack = (void *)data;

    uint8_t check_crc = 0;

    if (len != 5 || data_pack->head != 0x5A)
        return;

    check_crc ^= data[1];
    check_crc ^= data[2];
    check_crc ^= data[3];

    if (check_crc != data_pack->crc)
    {
        LL_USART_TransmitData8(USART1, 0x81);
        return;
    }

    if (data_pack->addr & 0x80)
    {
        // write reg
        uint8_t addr = data_pack->addr & 0x7f;
        uart_dict[addr] = data_pack->data;
        uart_dict_edit_flag |= (1 << addr);
        LL_USART_TransmitData8(USART1, addr);
    }
    else
    {
        // read reg
        data_pack = (void *)uart_txbuf;

        data_pack->head = 0x5A;
        data_pack->addr = data[1];
        data_pack->data = uart_dict[data[1]];
        data_pack->crc = 0 ^ uart_txbuf[1] ^ uart_txbuf[2] ^ uart_txbuf[3];
        uart_transmit(uart_txbuf, 5); // send data
    }
}

uint32_t uart_get_edit_flag()
{
    uint32_t ret = uart_dict_edit_flag;
    uart_dict_edit_flag = 0;
    return ret;
}

int uart_send_pdo(uint8_t start_index, uint8_t num)
{
    uint8_t checksum = 0;
    uart_pdo_buf[0] = 0xA5;
    uart_pdo_buf[1] = (num << 5) | (start_index & 0x1f);

    checksum ^= uart_pdo_buf[1];

    for (int i = 0; i < num; i++)
    {
        uint16_t data = uart_dict[start_index + i];
        checksum ^= uart_pdo_buf[2 + (i * 2)] = data & 0xFF;
        checksum ^= uart_pdo_buf[3 + (i * 2)] = (data >> 8) & 0xFF;
    }

    uart_pdo_buf[2 + (num * 2)] = checksum;
    uart_transmit(uart_pdo_buf, 3 + (num * 2));
    return 0;
}
