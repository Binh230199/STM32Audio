/**
 * @file threadpool.h
 * @brief Framework ThreadPool Core Module
 * @version 1.0.0
 * @date Created on: Aug 24, 2025
 */

#ifndef CORE_RTOS_THREADPOOL_CORE_THREAD_H
#define CORE_RTOS_THREADPOOL_CORE_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Core/Header/core_types.h"
#include "cmsis_os2.h"
#include <stdint.h>

//==============================================================================
// THREADPOOL CONFIGURATION
//==============================================================================

/**
 * @brief ThreadPool configuration - can be overridden by projects
 */
#ifndef THREADPOOL_THREAD_COUNT
  #define THREADPOOL_THREAD_COUNT   4    // Default thread count
#endif

#ifndef THREADPOOL_QUEUE_SIZE
  #define THREADPOOL_QUEUE_SIZE     20   // Default queue size
#endif

#ifndef THREADPOOL_TASK_TIMEOUT
  #define THREADPOOL_TASK_TIMEOUT   100  // Default timeout (ms)
#endif

#ifndef THREADPOOL_STACK_SIZE
  #define THREADPOOL_STACK_SIZE     2048  // 2048 words
#endif

//==============================================================================
// THREADPOOL CONFIGURATION STRUCTURE
//==============================================================================

/**
 * @brief ThreadPool configuration structure
 */
typedef struct {
  uint32_t thread_count;                 // Number of worker threads
  uint32_t queue_size;                   // Task queue size
  uint32_t default_timeout;              // Default task timeout (ms)
  uint32_t stack_size;                   // Stack size of each task (words)
  bool_t low_power_mode;                 // Enable low-power integration
  osPriority_t default_thread_priority;  // Default thread priority
} ThreadPoolConfig;

//==============================================================================
// THREADPOOL TYPES
//==============================================================================

/**
 * @brief ThreadPool priority levels
 */
typedef enum {
  THREADPOOL_PRIORITY_LOW,
  THREADPOOL_PRIORITY_NORMAL,
  THREADPOOL_PRIORITY_HIGH,
  THREADPOOL_PRIORITY_CRITICAL
} ThreadPoolPriority;

/**
 * @brief ThreadPool error codes
 */
typedef enum {
  THREADPOOL_SUCCESS,
  THREADPOOL_ERROR_INVALID_ARG,
  THREADPOOL_ERROR_ALLOC,
  THREADPOOL_ERROR_QUEUE_CREATE,
  THREADPOOL_ERROR_THREAD_CREATE,
  THREADPOOL_ERROR_QUEUE_FULL,
  THREADPOOL_ERROR_SHUTDOWN,
  THREADPOOL_ERROR_NOT_INITIALIZED
} ThreadPoolError;

/**
 * @brief ThreadPool states
 */
typedef enum {
  THREADPOOL_RUNNING,
  THREADPOOL_SHUTTING_DOWN,
  THREADPOOL_STOPPED
} ThreadPoolState;

//==============================================================================
// THREADPOOL API
//==============================================================================

/**
 * @brief Initialize ThreadPool with default configuration
 *
 * @return true if successful, false otherwise
 */
bool_t core_thread_init(void);

/**
 * @brief Initialize ThreadPool with custom configuration
 *
 * @param config Custom configuration
 * @return true if successful, false otherwise
 */
bool_t core_thread_init_with_config(const ThreadPoolConfig* config);

/**
 * @brief Get default configuration (for customization)
 *
 * @param config Pointer to config structure to fill
 */
void core_thread_get_default_config(ThreadPoolConfig* config);

/**
 * @brief Deinitialize ThreadPool
 *
 * @param wait_for_tasks true to wait for running tasks to complete
 */
void core_thread_deinit(bool_t wait_for_tasks);

/**
 * @brief Add task with full priority control
 *
 * @param function Task function to execute
 * @param arg Argument to pass to function
 * @param queue_priority Priority for queue ordering
 * @param execution_priority OS thread priority during execution
 * @param timeout_ms Timeout in milliseconds
 * @return ThreadPoolError Operation result
 */
ThreadPoolError core_thread_add_task(void (*function)(void*), void* arg,
                                                  ThreadPoolPriority queue_priority,
                                                  osPriority_t execution_priority,
                                                  uint32_t timeout_ms);

/**
 * @brief Get number of active tasks
 *
 * @return uint32_t Number of active tasks
 */
uint32_t core_thread_get_active_tasks_count(void);

/**
 * @brief Get number of active tasks without taking mutex.
 *
 * This API is intended for use in ISR context or other contexts where
 * acquiring a mutex is not allowed. It returns a snapshot of the internal
 * counter without synchronization. The value may be slightly stale, but
 * it is safe to call from interrupts.
 *
 * @return uint32_t Number of active tasks (approximate)
 */
uint32_t core_thread_get_active_tasks_count_wo_mutex(void);

/**
 * @brief Check if ThreadPool is idle (no active tasks)
 *
 * @return true if idle, false if tasks are running
 */
bool_t core_thread_is_idle(void);

/**
 * @brief Wait and suspend for low-power integration
 *
 * This function is used in idle hook to suspend main thread
 * when ThreadPool has active tasks.
 */
void core_thread_wait_and_suspend(void);

/**
 * @brief Get ThreadPool state
 *
 * @return ThreadPoolState Current state
 */
ThreadPoolState core_thread_get_state(void);

#ifdef __cplusplus
}
#endif

#endif /* CORE_RTOS_THREADPOOL_CORE_THREAD_H */
