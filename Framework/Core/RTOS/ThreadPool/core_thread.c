/**
 * @file threadpool.c
 * @brief Framework ThreadPool Core Module Implementation
 * @version 1.0.0
 * @date Created on: Aug 24, 2025
 */

#include "core_thread.h"
#include "Core/Logger/core_logger.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include <string.h>
#include <stdio.h>

//==============================================================================
// INTERNAL STRUCTURES
//==============================================================================

/**
 * @brief Task structure for ThreadPool
 */
typedef struct {
  void (*function)(void*);           // Task function
  void* arg;                        // Function argument
  bool_t is_shutdown_signal;        // Shutdown signal
  ThreadPoolPriority priority;      // Task priority
  osPriority_t execution_priority;  // Thread execution priority
} Task;

/**
 * @brief ThreadPool internal structure
 */
typedef struct {
  osMessageQueueId_t task_queue;     // Task queue
  osThreadId_t* worker_threads;      // Worker threads array
  uint32_t thread_count;             // Number of threads
  ThreadPoolState state;             // Current state
  osMutexId_t state_mutex;           // State protection mutex
  volatile uint32_t active_tasks;    // Number of active tasks (volatile for ISR-safe reads)
  osMutexId_t active_tasks_mutex;    // Active tasks counter mutex
  osThreadId_t main_thread;          // Main thread for low-power mode
  bool_t low_power_mode;             // Low-power mode enabled
  ThreadPoolConfig config;           // Configuration
} ThreadPoolInternal;

//==============================================================================
// GLOBAL VARIABLES
//==============================================================================

/**
 * @brief Global ThreadPool instance
 */
static ThreadPoolInternal* g_threadpool = NULL;

//==============================================================================
// PRIVATES FUNCTIONS
//==============================================================================

/**
 * @brief Worker thread function
 */
static void worker_thread_function(void* argument)
{
  ThreadPoolInternal* pool = (ThreadPoolInternal*)argument;
  Task task;
  osStatus_t status;
  osThreadId_t current_thread_id = osThreadGetId();
  const osPriority_t default_priority = osThreadGetPriority(current_thread_id);

  memset(&task, 0, sizeof(Task));

  while (1) {
    // Wait for new task
    status = osMessageQueueGet(pool->task_queue, &task, NULL, osWaitForever);

    if (status == osOK) {
      // Check shutdown signal
      if (task.is_shutdown_signal) {
        break;
      }

      // Increment active task count
      osMutexAcquire(pool->active_tasks_mutex, osWaitForever);
      pool->active_tasks++;
      osMutexRelease(pool->active_tasks_mutex);

      // Adjust thread priority if needed
      bool_t priority_changed = false;
      if (task.execution_priority != default_priority) {
        osThreadSetPriority(current_thread_id, task.execution_priority);
        priority_changed = true;
      }

      LOGD("[%p] Start: active_tasks: %d, queue_size: %d/%d",
          task.function,
          pool->active_tasks,
          osMessageQueueGetCount(pool->task_queue),
          osMessageQueueGetCapacity(pool->task_queue));

      // Execute task
      task.function(task.arg);

      // Restore thread priority
      if (priority_changed) {
        osThreadSetPriority(current_thread_id, default_priority);
      }

      // Decrement active task count
      osMutexAcquire(pool->active_tasks_mutex, osWaitForever);
      pool->active_tasks--;

      // Handle low-power mode
      if (pool->low_power_mode && pool->active_tasks == 0 &&
          pool->state == THREADPOOL_RUNNING && pool->main_thread != NULL) {
        osThreadResume(pool->main_thread);
      }

      osMutexRelease(pool->active_tasks_mutex);

      LOGD("[%p] Stop : active_tasks: %d, queue_size: %d/%d",
          task.function,
          pool->active_tasks,
          osMessageQueueGetCount(pool->task_queue),
          osMessageQueueGetCapacity(pool->task_queue));
    }
  }
}

/**
 * @brief Convert priority to queue priority
 */
static uint8_t convert_priority(ThreadPoolPriority priority)
{
  switch (priority) {
    case THREADPOOL_PRIORITY_LOW:      return 0;
    case THREADPOOL_PRIORITY_NORMAL:   return 1;
    case THREADPOOL_PRIORITY_HIGH:     return 2;
    case THREADPOOL_PRIORITY_CRITICAL: return 3;
    default:                           return 1;
  }
}

/**
 * @brief Create ThreadPool internal structure
 */
static ThreadPoolInternal* create_threadpool_internal(const ThreadPoolConfig* config)
{
  if (config == NULL || config->thread_count == 0 || config->queue_size == 0 || config->stack_size == 0) {
    LOGE("Invalid ThreadPool configuration");
    return NULL;
  }

  // Allocate ThreadPool structure
  ThreadPoolInternal* pool = (ThreadPoolInternal*)pvPortMalloc(sizeof(ThreadPoolInternal));
  if (pool == NULL) {
    LOGE("Failed to allocate ThreadPool memory");
    return NULL;
  }

  memset(pool, 0, sizeof(ThreadPoolInternal));
  pool->config = *config;  // Store configuration
  pool->thread_count = config->thread_count;
  pool->state = THREADPOOL_RUNNING;
  pool->low_power_mode = config->low_power_mode;
  pool->active_tasks = 0;

  if (config->low_power_mode) {
    pool->main_thread = osThreadGetId();
  }

  // Create mutexes
  pool->state_mutex = osMutexNew(NULL);
  pool->active_tasks_mutex = osMutexNew(NULL);
  if (pool->state_mutex == NULL || pool->active_tasks_mutex == NULL) {
    LOGE("Failed to create ThreadPool mutexes");
    goto error;
  }

  // Create task queue
  pool->task_queue = osMessageQueueNew(config->queue_size, sizeof(Task), NULL);
  if (pool->task_queue == NULL) {
    LOGE("Failed to create ThreadPool task queue");
    goto error;
  }

  // Allocate worker threads array
  pool->worker_threads = (osThreadId_t*)pvPortMalloc(config->thread_count * sizeof(osThreadId_t));
  if (pool->worker_threads == NULL) {
    LOGE("Failed to allocate worker threads array");
    goto error;
  }

  // Create worker threads
  osThreadAttr_t thread_attr = {
    .name = "Worker",
    .priority = config->default_thread_priority,
    .stack_size = config->stack_size,
  };

  for (uint32_t i = 0; i < config->thread_count; i++) {
    char threadName[20];
    snprintf(threadName, sizeof(threadName), "Worker_%lu", (unsigned long)i);
    thread_attr.name = threadName;

    pool->worker_threads[i] = osThreadNew(worker_thread_function, pool, &thread_attr);
    if (pool->worker_threads[i] == NULL) {
      LOGE("Failed to create worker thread %lu", i);
      goto error;
    }
  }

  LOGI("ThreadPool created: %lu threads, queue size: %lu",
       config->thread_count, config->queue_size);
  return pool;

error:
  if (pool) {
    if (pool->task_queue) osMessageQueueDelete(pool->task_queue);
    if (pool->state_mutex) osMutexDelete(pool->state_mutex);
    if (pool->active_tasks_mutex) osMutexDelete(pool->active_tasks_mutex);
    if (pool->worker_threads) vPortFree(pool->worker_threads);
    vPortFree(pool);
  }
  return NULL;
}

/**
 * @brief Destroy ThreadPool internal structure
 */
static void destroy_threadpool_internal(ThreadPoolInternal* pool, bool_t wait_for_tasks)
{
  if (pool == NULL) return;

  LOGI("Destroying ThreadPool (wait: %s)", wait_for_tasks ? "true" : "false");

  // Set shutdown state
  if (pool->state_mutex) {
    osMutexAcquire(pool->state_mutex, osWaitForever);
    pool->state = THREADPOOL_SHUTTING_DOWN;
    osMutexRelease(pool->state_mutex);
  }

  // Wait for active tasks if requested
  if (wait_for_tasks && pool->active_tasks_mutex) {
    while (1) {
      osMutexAcquire(pool->active_tasks_mutex, osWaitForever);
      uint32_t active = pool->active_tasks;
      osMutexRelease(pool->active_tasks_mutex);

      if (active == 0) break;
      osDelay(10);
    }
  }

  // Send shutdown signals to worker threads
  if (pool->task_queue && pool->worker_threads) {
    Task shutdown_task = {0};
    shutdown_task.is_shutdown_signal = true;

    for (uint32_t i = 0; i < pool->thread_count; i++) {
      if (pool->worker_threads[i]) {
        osMessageQueuePut(pool->task_queue, &shutdown_task, 255, 100);
      }
    }

    // Wait for threads to finish naturally (like old ThreadPool)
    osDelay(100);
  }

  // Clean up resources
  if (pool->task_queue) osMessageQueueDelete(pool->task_queue);
  if (pool->state_mutex) osMutexDelete(pool->state_mutex);
  if (pool->active_tasks_mutex) osMutexDelete(pool->active_tasks_mutex);
  if (pool->worker_threads) vPortFree(pool->worker_threads);

  pool->state = THREADPOOL_STOPPED;
  vPortFree(pool);
}

//==============================================================================
// PUBLIC API IMPLEMENTATION
//==============================================================================

bool_t core_thread_init(void)
{
  if (g_threadpool != NULL) {
    LOGI("ThreadPool already initialized");
    return true;
  }

  // Get default configuration
  ThreadPoolConfig config;
  core_thread_get_default_config(&config);

  return core_thread_init_with_config(&config);
}

bool_t core_thread_init_with_config(const ThreadPoolConfig* config)
{
  if (config == NULL) {
    LOGE("Invalid ThreadPool configuration");
    return false;
  }

  if (g_threadpool != NULL) {
    LOGI("ThreadPool already initialized");
    return true;
  }

  g_threadpool = create_threadpool_internal(config);
  if (g_threadpool == NULL) {
    LOGE("Failed to initialize ThreadPool");
    return false;
  }

  LOGI("ThreadPool initialized successfully");
  return true;
}

void core_thread_get_default_config(ThreadPoolConfig* config)
{
  if (config == NULL) {
    return;
  }

  config->thread_count = THREADPOOL_THREAD_COUNT;
  config->queue_size = THREADPOOL_QUEUE_SIZE;
  config->default_timeout = THREADPOOL_TASK_TIMEOUT;
  config->stack_size = THREADPOOL_STACK_SIZE * 4;
  config->low_power_mode = true;
  config->default_thread_priority = osPriorityNormal;

}

void core_thread_deinit(bool_t wait_for_tasks)
{
  if (g_threadpool == NULL) {
    LOGW("ThreadPool not initialized");
    return;
  }

  destroy_threadpool_internal(g_threadpool, wait_for_tasks);
  g_threadpool = NULL;
  LOGI("ThreadPool deinitialized");
}

ThreadPoolError core_thread_add_task(void (*function)(void*), void* arg,
                                                  ThreadPoolPriority queue_priority,
                                                  osPriority_t execution_priority,
                                                  uint32_t timeout_ms)
{
  if (g_threadpool == NULL) {
    return THREADPOOL_ERROR_NOT_INITIALIZED;
  }

  if (function == NULL) {
    return THREADPOOL_ERROR_INVALID_ARG;
  }

  // Check state
  osMutexAcquire(g_threadpool->state_mutex, osWaitForever);
  ThreadPoolState state = g_threadpool->state;
  osMutexRelease(g_threadpool->state_mutex);

  if (state != THREADPOOL_RUNNING) {
    return THREADPOOL_ERROR_SHUTDOWN;
  }

  Task task = {0};
  task.function = function;
  task.arg = arg;
  task.priority = queue_priority;
  task.execution_priority = execution_priority;

  uint8_t msg_queue_priority = convert_priority(queue_priority);
  osStatus_t status = osMessageQueuePut(g_threadpool->task_queue, &task, msg_queue_priority, timeout_ms);

  switch (status) {
    case osOK:
      return THREADPOOL_SUCCESS;
    case osErrorTimeout:
    case osErrorResource:
      return THREADPOOL_ERROR_QUEUE_FULL;
    default:
      return THREADPOOL_ERROR_QUEUE_FULL;
  }
}

uint32_t core_thread_get_active_tasks_count(void)
{
  if (g_threadpool == NULL || g_threadpool->active_tasks_mutex == NULL) {
    return 0;
  }

  osMutexAcquire(g_threadpool->active_tasks_mutex, osWaitForever);
  uint32_t count = g_threadpool->active_tasks;
  osMutexRelease(g_threadpool->active_tasks_mutex);

  return count;
}

uint32_t core_thread_get_active_tasks_count_wo_mutex(void)
{
  if (g_threadpool == NULL) {
    return 0;
  }

  /*
   * Read volatile counter without mutex. This is intended for ISR use.
   * The value is a snapshot and may be slightly out-of-date, but it's
   * safe to read from interrupt context.
   */
  return g_threadpool->active_tasks;
}

bool_t core_thread_is_idle(void)
{
  return (core_thread_get_active_tasks_count() == 0);
}

void core_thread_wait_and_suspend(void)
{
  if (g_threadpool == NULL || !g_threadpool->low_power_mode) {
    return;
  }

  // Kiểm tra xem có task nào đang chạy không
  osMutexAcquire(g_threadpool->active_tasks_mutex, osWaitForever);
  bool_t has_active_tasks = (g_threadpool->active_tasks > 0);
  osMutexRelease(g_threadpool->active_tasks_mutex);

  // Nếu có task đang chạy và thread hiện tại là main thread, thì suspend
  if (has_active_tasks && g_threadpool->main_thread != NULL) {
    osThreadSuspend(g_threadpool->main_thread);
  }
}

ThreadPoolState core_thread_get_state(void)
{
  if (g_threadpool == NULL || g_threadpool->state_mutex == NULL) {
    return THREADPOOL_STOPPED;
  }

  osMutexAcquire(g_threadpool->state_mutex, osWaitForever);
  ThreadPoolState state = g_threadpool->state;
  osMutexRelease(g_threadpool->state_mutex);

  return state;
}
