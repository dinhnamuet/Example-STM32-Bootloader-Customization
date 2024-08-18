#ifndef __SERIAL_H__
#define __SERIAL_H__
#include "stm32l4xx_hal.h"

#define RX_MAX_LEN 		2062
#define UART_BUS_MAX		3 /* max number of  UART bus that device is supported */

#pragma pack(1)
struct serdev_device {
    UART_HandleTypeDef *bus;

    uint8_t rx_buf[RX_MAX_LEN];
    void (*receive_buf)(struct serdev_device *dev, uint8_t *buf, uint32_t len);
};

struct mapping_table {
	UART_HandleTypeDef *bus;
	struct serdev_device *dev;
};

#pragma pack()

HAL_StatusTypeDef serdev_device_register(struct serdev_device *dev);
HAL_StatusTypeDef serial_send(struct serdev_device *dev, uint8_t *buf, uint32_t len);
HAL_StatusTypeDef serial_start_receiving(struct serdev_device *dev);
HAL_StatusTypeDef serdev_device_unregister(struct serdev_device *dev);

#endif
