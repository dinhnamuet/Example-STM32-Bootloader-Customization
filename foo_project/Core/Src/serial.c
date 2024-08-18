#include "serial.h"
#include <string.h>
#include <stdlib.h>

static struct mapping_table uart_table[UART_BUS_MAX] = {0};

static struct serdev_device *uart_bus_to_dev(UART_HandleTypeDef *bus) {
	for (int i = 0; i < UART_BUS_MAX; i++) {
		if (uart_table[i].bus == bus) {
			return uart_table[i].dev;
		}
	}
	return NULL;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
	struct serdev_device *dev = uart_bus_to_dev(huart);
    if (dev && dev->receive_buf) {
        dev->receive_buf(dev, dev->rx_buf, Size); /* Callback function */
    }
}

HAL_StatusTypeDef serdev_device_register(struct serdev_device *dev) {
	if (!dev || !dev->bus) {
		return HAL_ERROR;
	} else {
		for (int i = 0; i < UART_BUS_MAX; i++) {
			if (!uart_table[i].bus && !uart_table[i].dev) {
				uart_table[i].bus = dev->bus;
				uart_table[i].dev = dev;
				return HAL_OK;
			}
		}
	}
	return HAL_ERROR;
}

HAL_StatusTypeDef serdev_device_unregister(struct serdev_device *dev) {
	for (int i = 0; i < UART_BUS_MAX; i++) {
		if (uart_table[i].dev == dev) {
			uart_table[i].dev = NULL;
			uart_table[i].bus = NULL;
			return HAL_OK;
		}
	}
	return HAL_ERROR;
}

HAL_StatusTypeDef serial_send(struct serdev_device *dev, uint8_t *buf, uint32_t len) {
    return HAL_UART_Transmit(dev->bus, buf, len, 2000);
}

HAL_StatusTypeDef serial_start_receiving(struct serdev_device *dev) {
  memset(dev->rx_buf, 0, RX_MAX_LEN);
  return HAL_UARTEx_ReceiveToIdle_IT(dev->bus, dev->rx_buf, RX_MAX_LEN);
}
