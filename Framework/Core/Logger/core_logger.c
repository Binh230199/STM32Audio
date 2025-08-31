/*
 * logger.c
 *
 *  Created on: Jul 12, 2024
 *      Author: binhhv.23.1.99@gmail.com
 */

#include "core_logger.h"
#include "core_logger_sink_interface.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#if LOGGER_USE_RTOS
/**
 * @brief Logger semaphore (RTOS only)
 */
static osSemaphoreId_t bsem_logger;
#endif

/**
 * @brief Logger instance - simplified structure
 */
static logger_t logger = {{0}, LOG_LEVEL_DEBUG, {NULL}, 0};

/**
 * @brief Layer names for prefixing logs (removed)
 */

/**
 * @brief Initialize logger
 */
void core_logger_init(void)
{
#if LOGGER_USE_RTOS
  bsem_logger = osSemaphoreNew(1, 1, NULL); // must be available: 1
#endif
  logger.min_level = LOG_LEVEL_DEBUG; // Allow all levels by default
}

/**
 * @brief Set minimum log level
 */
void core_logger_set_min_level(log_level_t min_level)
{
  if (min_level < LOG_LEVEL_COUNT) {
    logger.min_level = min_level;
  }
}

/**
 * @brief Get current minimum log level
 */
log_level_t core_logger_get_min_level(void)
{
  return logger.min_level;
}

/**
 * @brief Register a sink with the logger
 * @param sink Pointer to sink to register
 * @return true if successful, false if failed
 */
bool_t core_logger_register_sink(struct core_logger_sink* sink)
{
  if (sink == NULL || logger.sink_count >= LOGGER_MAX_SINKS) {
    return false;
  }

  /* Check if sink already registered */
  for (uint8_t i = 0; i < logger.sink_count; i++) {
    if (logger.sinks[i] == sink) {
      return true; /* Already registered */
    }
  }

  /* Add sink to array */
  logger.sinks[logger.sink_count] = sink;
  logger.sink_count++;

  return true;
}

/**
 * @brief Unregister a sink from the logger
 * @param sink Pointer to sink to unregister
 */
void core_logger_unregister_sink(struct core_logger_sink* sink)
{
  if (sink == NULL) {
    return;
  }

  /* Find and remove sink */
  for (uint8_t i = 0; i < logger.sink_count; i++) {
    if (logger.sinks[i] == sink) {
      /* Shift remaining sinks down */
      for (uint8_t j = i; j < logger.sink_count - 1; j++) {
        logger.sinks[j] = logger.sinks[j + 1];
      }
      logger.sink_count--;
      logger.sinks[logger.sink_count] = NULL;
      break;
    }
  }
}

/**
 * @brief Clear all registered sinks
 */
void core_logger_clear_all_sinks(void)
{
  for (uint8_t i = 0; i < LOGGER_MAX_SINKS; i++) {
    logger.sinks[i] = NULL;
  }
  logger.sink_count = 0;
}

/**
 * @brief Get number of registered sinks
 * @return Number of sinks
 */
uint8_t core_logger_get_sink_count(void)
{
  return logger.sink_count;
}

/**
 * @brief Legacy functions for backward compatibility
 */

/**
 * @brief Log write function - simplified implementation
 *
 * @param level Log level
 * @param fmt Format string
 * @param ... Arguments
 */
void core_logger_write(log_level_t level, const char *fmt, ...)
{
  // Check if level meets minimum threshold
  if (level < logger.min_level)
  {
    return;
  }

  const char *log_level_string[] = {"[D]", "[I]", "[W]", "[E]"};

#if LOGGER_USE_RTOS
  if (osSemaphoreAcquire(bsem_logger, osWaitForever) != osOK)
  {
    // Handle mutex acquisition failure
    goto end; // @suppress("Goto statement used")
  }
#endif

  // Bounds check for level
  if (level >= sizeof(log_level_string) / sizeof(log_level_string[0]))
  {
    level = sizeof(log_level_string) / sizeof(log_level_string[0]) - 1;
  }

  // Prepend log level and newline
  size_t size_pre = (size_t)snprintf(logger.buffer, LOGGER_BUFFER_SIZE,
                                     "\n%s", log_level_string[level]);

  if (size_pre >= LOGGER_BUFFER_SIZE)
  {
    // Handle buffer overflow
#if LOGGER_USE_RTOS
    (void)osSemaphoreRelease(bsem_logger);
#endif
    goto end; // @suppress("Goto statement used")
  }

  va_list args;
  va_start(args, fmt);
  // Format message using vsnprintf
  size_t size_msg = (size_t)vsnprintf(logger.buffer + size_pre, (uint16_t)(LOGGER_BUFFER_SIZE - size_pre), fmt, args);
  va_end(args);

  if (size_msg + size_pre >= LOGGER_BUFFER_SIZE)
  {
    // Handle buffer overflow
#if LOGGER_USE_RTOS
    (void)osSemaphoreRelease(bsem_logger);
#endif
    goto end; // @suppress("Goto statement used")
  }

  /* Choose output method: sinks only */
  if (logger.sink_count > 0) {
    /* Use sink pattern */
    for (uint8_t i = 0; i < logger.sink_count; i++) {
      struct core_logger_sink* sink = logger.sinks[i];
      if (sink != NULL && sink->vtable != NULL && sink->vtable->log != NULL) {
        (void)sink->vtable->log(sink, level, logger.buffer);
      }
    }
  }
  /* Note: No output if no sinks registered - encourages proper sink usage */

#if LOGGER_USE_RTOS
  (void)osSemaphoreRelease(bsem_logger);
#endif

  end:
  return;
}
