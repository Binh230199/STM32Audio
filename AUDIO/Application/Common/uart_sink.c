/*
 * uart_sink.c
 *
 *  Created on: Aug 23, 2025
 *    Author: binhhv.23.1.99@gmail.com
 */

#include "uart_sink.h"
#include <string.h>

/* Forward declarations */
static bool_t uart_sink_log(core_logger_sink_t* self, log_level_t level, const char* message);
static void uart_sink_set_level(core_logger_sink_t* self, log_level_t min_level);
static log_level_t uart_sink_get_level(core_logger_sink_t* self);
static void uart_sink_destroy_impl(core_logger_sink_t* self);

/* UART sink virtual table */
static const log_sink_vtable_t uart_sink_vtable = {
  .log = uart_sink_log,
  .set_level = uart_sink_set_level,
  .get_level = uart_sink_get_level,
  .destroy = uart_sink_destroy_impl
};

bool_t uart_sink_create(uart_sink_t* sink, const uart_sink_config_t* config, log_level_t min_level)
{
  if (sink == NULL || config == NULL || config->huart == NULL) {
    return false;
  }

  /* Initialize base sink */
  log_sink_init(&sink->base, &uart_sink_vtable, "UART", min_level);

  /* Initialize UART specific fields */
  sink->huart = config->huart;
  sink->timeout_ms = config->timeout_ms;
  sink->use_dma = config->use_dma;

  return true;
}

void uart_sink_destroy(uart_sink_t* sink)
{
  if (sink != NULL && sink->base.vtable != NULL && sink->base.vtable->destroy != NULL) {
    sink->base.vtable->destroy(&sink->base);
  }
}

void uart_sink_set_timeout(uart_sink_t* sink, uint32_t timeout_ms)
{
  if (sink != NULL) {
    sink->timeout_ms = timeout_ms;
  }
}

UART_HandleTypeDef* uart_sink_get_handle(const uart_sink_t* sink)
{
  if (sink != NULL) {
    return sink->huart;
  }
  return NULL;
}

/* Virtual table implementations */

static bool_t uart_sink_log(core_logger_sink_t* self, log_level_t level, const char* message)
{
  if (self == NULL || message == NULL) {
    return false;
  }

  /* Check if should log this level */
  if (!log_sink_should_log(self, level)) {
    return true; /* Not an error, just filtered */
  }

  uart_sink_t* uart_sink = (uart_sink_t*)self;

  if (uart_sink->huart == NULL) {
    return false;
  }

  /* Get message length */
  size_t message_len = strlen(message);
  if (message_len == 0) {
    return true; /* Empty message is OK */
  }

  /* Transmit via UART */
  HAL_StatusTypeDef status;
  if (uart_sink->use_dma) {
    /* Use DMA transmission (non-blocking) */
    status = HAL_UART_Transmit_DMA(uart_sink->huart, (uint8_t*)message, (uint16_t)message_len);
  } else {
    /* Use blocking transmission */
    status = HAL_UART_Transmit(uart_sink->huart, (uint8_t*)message, (uint16_t)message_len, uart_sink->timeout_ms);
  }

  return (status == HAL_OK);
}

static void uart_sink_set_level(core_logger_sink_t* self, log_level_t min_level)
{
  if (self != NULL) {
    self->min_level = min_level;
  }
}

static log_level_t uart_sink_get_level(core_logger_sink_t* self)
{
  if (self != NULL) {
    return self->min_level;
  }
  return LOG_LEVEL_COUNT; /* Invalid level */
}

static void uart_sink_destroy_impl(core_logger_sink_t* self)
{
  if (self != NULL) {
    uart_sink_t* uart_sink = (uart_sink_t*)self;

    /* Reset UART specific fields */
    uart_sink->huart = NULL;
    uart_sink->timeout_ms = 0;
    uart_sink->use_dma = false;
  }
}
