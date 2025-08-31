/*
 * logger.h
 *
 *  Created on: Jul 12, 2024
 *      Author: binhhv.23.1.99@gmail.com
 */

/**
 * ===============================================================================
 * SIMPLIFIED LOGGER MODULE USER GUIDE
 * ===============================================================================
 *
 * The Simplified Logger Module provides a clean and easy-to-use logging system
 * for both RTOS and bare-metal embedded applications. Key features:
 *
 * - Simple log levels: Debug, Info, Warning, and Error
 * - Global minimum level filtering (no complex layer management)
 * - Multiple output destinations via Sink pattern
 * - Thread-safe operation using RTOS semaphores (when RTOS available)
 * - Automatic RTOS/bare-metal detection and adaptation
 * - Backward compatibility with legacy layer-based macros
 *
 * ===============================================================================
 * 1. BASIC USAGE (Same for RTOS and Bare-metal)
 * ===============================================================================
 *
 * Simple logging macros:
 * - LOGD("Debug message with %d parameters", count);
 * - LOGI("Info message");
 * - LOGW("Warning: %s might cause issues", "condition");
 * - LOGE("Error: Operation failed with code %d", error_code);
 *
 * ===============================================================================
 * 2. RTOS vs BARE-METAL DIFFERENCES
 * ===============================================================================
 *
 * RTOS Mode (auto-detected when cmsis_os2.h is included):
 * - Thread-safe logging with semaphores
 * - Log format: [LEVEL][ThreadName][Function][Line]: Message
 * - Example: [I][TaskMgr][app_init][123]: System started
 *
 * Bare-metal Mode (when RTOS not available):
 * - Simple logging without thread safety overhead
 * - Log format: [LEVEL][Function][Line]: Message
 * - Example: [I][app_init][123]: System started
 *
 * ===============================================================================
 * 3. INITIALIZATION
 * ===============================================================================
 *
 * // Same initialization for both modes
 * void main(void) {
 *   core_logger_init();  // Automatically adapts to RTOS/bare-metal
 *
 *   // Create and register output sink
 *   static uart_sink_t uart_sink;
 *   uart_sink_config_t config = { .huart = &huart2, .timeout_ms = 1000, .use_dma = false };
 *   if (uart_sink_create(&uart_sink, &config, LOG_LEVEL_DEBUG)) {
 *     core_logger_register_sink((core_logger_sink_t*)&uart_sink);
 *   }
 *
 *   LOGI("Logger initialized");
 * }
 *
 * ===============================================================================
 */

#ifndef CORE_LOGGER_LOGGER_H_
#define CORE_LOGGER_LOGGER_H_

#include "Core/Header/core_types.h"

/* Check if RTOS is available */
#ifdef CMSIS_OS2_H_
#include "cmsis_os2.h"
#define LOGGER_USE_RTOS 1
#else
#define LOGGER_USE_RTOS 0
#endif

#define LOG_ENABLE

/* Forward declaration for sink support */
struct core_logger_sink;

/**
 * @brief Log information format
 */
#if LOGGER_USE_RTOS
#define LOG_HEADER "[%16s][%36s][%4d]: "
#define LOG_ARGS osThreadGetName(osThreadGetId()), __func__, __LINE__
#define LOG_APP_ARGS  log_tag, __func__, __LINE__
#else
#define LOG_HEADER "[%s][%d]: "
#define LOG_ARGS __func__, __LINE__
#endif

/**
 * @brief Log priority level
 */
typedef enum
{
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_COUNT
} log_level_t;

/**
 * @brief Maximum number of sinks supported
 */
#define LOGGER_MAX_SINKS (4U)

/**
 * @brief Logger buffer size
 */
#define LOGGER_BUFFER_SIZE (2176U)

/**
 * @brief Logger structure
 */
typedef struct
{
  char_t buffer[LOGGER_BUFFER_SIZE];
  log_level_t min_level;                       // Minimum log level to output

  /* Sink management */
  struct core_logger_sink* sinks[LOGGER_MAX_SINKS];    /* Array of registered sinks */
  uint8_t sink_count;                          /* Number of registered sinks */
} logger_t;

/**
 * @brief Simple log macros (no layer distinction)
 */
#if defined(LOG_ENABLE)
  #define LOGD(message, ...) core_logger_write(LOG_LEVEL_DEBUG, LOG_HEADER message, LOG_ARGS, ##__VA_ARGS__)
#else
  #define LOGD(message, ...) ((void)0)
#endif

#if defined(LOG_ENABLE)
  #define LOGI(message, ...) core_logger_write(LOG_LEVEL_INFO, LOG_HEADER message, LOG_ARGS, ##__VA_ARGS__)
#else
  #define LOGI(message, ...) ((void)0)
#endif

#if defined(LOG_ENABLE)
  #define LOGW(message, ...) core_logger_write(LOG_LEVEL_WARN, LOG_HEADER message, LOG_ARGS, ##__VA_ARGS__)
#else
  #define LOGW(message, ...) ((void)0)
#endif

#if defined(LOG_ENABLE)
  #define LOGE(message, ...) core_logger_write(LOG_LEVEL_ERROR, LOG_HEADER message, LOG_ARGS, ##__VA_ARGS__)
#else
  #define LOGE(message, ...) ((void)0)
#endif

/* Backward compatibility macros */
#if defined(LOG_ENABLE)
#if LOGGER_USE_RTOS
#define APP_LOGD(message, ...) core_logger_write(LOG_LEVEL_DEBUG, LOG_HEADER message, LOG_APP_ARGS, ##__VA_ARGS__)
#define APP_LOGI(message, ...) core_logger_write(LOG_LEVEL_INFO, LOG_HEADER message, LOG_APP_ARGS, ##__VA_ARGS__)
#define APP_LOGW(message, ...) core_logger_write(LOG_LEVEL_WARN, LOG_HEADER message, LOG_APP_ARGS, ##__VA_ARGS__)
#define APP_LOGE(message, ...) core_logger_write(LOG_LEVEL_ERROR, LOG_HEADER message, LOG_APP_ARGS, ##__VA_ARGS__)
#else
#define APP_LOGD(message, ...) ((void)0);
#define APP_LOGI(message, ...) ((void)0);
#define APP_LOGW(message, ...) ((void)0);
#define APP_LOGE(message, ...) ((void)0);
#endif
#else
#define APP_LOGD(message, ...) ((void)0);
#define APP_LOGI(message, ...) ((void)0);
#define APP_LOGW(message, ...) ((void)0);
#define APP_LOGE(message, ...) ((void)0);
#endif
#define MGR_LOGD LOGD
#define MGR_LOGI LOGI
#define MGR_LOGW LOGW
#define MGR_LOGE LOGE

#define DRV_LOGD LOGD
#define DRV_LOGI LOGI
#define DRV_LOGW LOGW
#define DRV_LOGE LOGE

#define MCU_LOGD LOGD
#define MCU_LOGI LOGI
#define MCU_LOGW LOGW
#define MCU_LOGE LOGE

#define CMN_LOGD LOGD
#define CMN_LOGI LOGI
#define CMN_LOGW LOGW
#define CMN_LOGE LOGE

/**
 * @brief Logger initialization and control
 */
void core_logger_init(void);
void core_logger_set_min_level(log_level_t min_level);
log_level_t core_logger_get_min_level(void);

/**
 * @brief Sink management functions
 */
bool_t core_logger_register_sink(struct core_logger_sink* sink);
void core_logger_unregister_sink(struct core_logger_sink* sink);
void core_logger_clear_all_sinks(void);
uint8_t core_logger_get_sink_count(void);

/**
 * @brief Log output function
 */
void core_logger_write(log_level_t level, const char *fmt, ...);

#endif /* CORE_LOGGER_LOGGER_H_ */
