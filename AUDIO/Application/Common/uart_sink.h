/*
 * uart_sink.h
 *
 *  Created on: Aug 23, 2025
 *    Author: binhhv.23.1.99@gmail.com
 */

#ifndef CORE_LOGGER_UART_SINK_H_
#define CORE_LOGGER_UART_SINK_H_

#include <stdint.h>
#include "Core/Logger/core_logger_sink_interface.h"
#include "usart.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief UART sink configuration
 */
typedef struct {
  UART_HandleTypeDef* huart;    /* UART handle */
  uint32_t timeout_ms;      /* Transmission timeout */
  bool_t use_dma;         /* Use DMA for transmission */
} uart_sink_config_t;

/**
 * @brief UART sink structure
 */
typedef struct {
  core_logger_sink_t base;        /* Base sink */
  UART_HandleTypeDef* huart;    /* UART handle */
  uint32_t timeout_ms;      /* Transmission timeout */
  bool_t use_dma;         /* Use DMA flag */
} uart_sink_t;

/**
 * @brief Create UART sink
 * @param sink Pointer to UART sink structure
 * @param config UART sink configuration
 * @param min_level Minimum log level
 * @return true if successful, false if failed
 */
bool_t uart_sink_create(uart_sink_t* sink, const uart_sink_config_t* config, log_level_t min_level);

/**
 * @brief Destroy UART sink
 * @param sink Pointer to UART sink
 */
void uart_sink_destroy(uart_sink_t* sink);

/**
 * @brief Set UART timeout
 * @param sink Pointer to UART sink
 * @param timeout_ms Timeout in milliseconds
 */
void uart_sink_set_timeout(uart_sink_t* sink, uint32_t timeout_ms);

/**
 * @brief Get UART handle
 * @param sink Pointer to UART sink
 * @return UART handle or NULL if invalid
 */
UART_HandleTypeDef* uart_sink_get_handle(const uart_sink_t* sink);

#ifdef __cplusplus
}
#endif

#endif /* CORE_LOGGER_UART_SINK_H_ */
