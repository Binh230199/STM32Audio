/*
 * sink_interface.h
 *
 *  Created on: Aug 23, 2025
 *    Author: binhhv.23.1.99@gmail.com
 */

#ifndef CORE_LOGGER_CORE_LOGGER_SINK_INTERFACE_H_
#define CORE_LOGGER_CORE_LOGGER_SINK_INTERFACE_H_

#include "Core/Header/core_types.h"
#include "Core/Logger/core_logger.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration */
struct core_logger_sink;

/**
 * @brief Sink virtual table (function pointers for polymorphism)
 */
typedef struct {
  /**
   * @brief Log message to this sink
   * @param self Pointer to sink instance
   * @param level Log level
   * @param message Formatted message to log
   * @return true if successful, false if failed
   */
  bool_t (*log)(struct core_logger_sink* self, log_level_t level, const char* message);

  /**
   * @brief Set minimum log level for this sink
   * @param self Pointer to sink instance
   * @param min_level Minimum level to log
   */
  void (*set_level)(struct core_logger_sink* self, log_level_t min_level);

  /**
   * @brief Get current minimum log level
   * @param self Pointer to sink instance
   * @return Current minimum level
   */
  log_level_t (*get_level)(struct core_logger_sink* self);

  /**
   * @brief Destroy/cleanup sink resources
   * @param self Pointer to sink instance
   */
  void (*destroy)(struct core_logger_sink* self);
} log_sink_vtable_t;

/**
 * @brief Base sink structure
 */
typedef struct core_logger_sink {
  const log_sink_vtable_t* vtable;  /* Virtual table */
  log_level_t min_level;        /* Minimum log level for this sink */
  char_t name[16];          /* Sink name for debugging */
} core_logger_sink_t;

/**
 * @brief Initialize base sink
 * @param sink Pointer to sink to initialize
 * @param vtable Pointer to virtual table
 * @param name Sink name
 * @param min_level Initial minimum log level
 */
void log_sink_init(core_logger_sink_t* sink, const log_sink_vtable_t* vtable,
           const char* name, log_level_t min_level);

/**
 * @brief Check if sink should log this message
 * @param sink Pointer to sink
 * @param level Log level to check
 * @return true if should log, false if should filter
 */
bool_t log_sink_should_log(const core_logger_sink_t* sink, log_level_t level);

#ifdef __cplusplus
}
#endif

#endif /* CORE_LOGGER_CORE_LOGGER_SINK_INTERFACE_H_ */
