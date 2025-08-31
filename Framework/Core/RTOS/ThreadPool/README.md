# ThreadPool Core Module

ThreadPool core module cung cấp giải pháp multi-threading hiệu suất cao cho RTOS applications.

## Core API

### Initialization
```c
// Initialize with default config
bool_t core_thread_init(void);

// Initialize with custom config
bool_t core_thread_init_with_config(const ThreadPoolConfig* config);

// Get default config for customization
void core_thread_get_default_config(ThreadPoolConfig* config);

// Cleanup
void core_thread_deinit(bool_t wait_for_tasks);
```

### Task Management (Single API)
```c
// Add task with full priority control
ThreadPoolError core_thread_add_task(void (*function)(void*), void* arg,
                                                  ThreadPoolPriority queue_priority,
                                                  osPriority_t execution_priority,
                                                  uint32_t timeout_ms);
```

### Monitoring
```c
// Get active task count
uint32_t core_thread_get_active_tasks_count(void);

// Check if idle
bool_t core_thread_is_idle(void);

// Low-power integration
void core_thread_wait_and_suspend(void);

// Get current state
ThreadPoolState core_thread_get_state(void);
```

## Usage Example

```c
#include "Core/RTOS/ThreadPool/core_thread.h"

void my_task(void* arg) {
    int* data = (int*)arg;
    // Process data...
}

int main() {
    // Initialize
    core_thread_init();

    // Add task
    int data = 42;
    core_thread_add_task(my_task, &data,
                                     THREADPOOL_PRIORITY_HIGH,
                                     osPriorityHigh1,
                                     1000);

    // Monitor
    while (!core_thread_is_idle()) {
        osDelay(100);
    }

    // Cleanup
    core_thread_deinit(true);
}
```

## Configuration

```c
typedef struct {
  uint32_t thread_count;                 // Number of worker threads
  uint32_t queue_size;                   // Task queue size
  uint32_t default_timeout;              // Default timeout (ms)
  bool_t low_power_mode;                 // Enable low-power integration
  osPriority_t default_thread_priority;  // Default thread priority
} ThreadPoolConfig;
```

Override defaults:
```c
#define THREADPOOL_THREAD_COUNT   4
#define THREADPOOL_QUEUE_SIZE     20
#define THREADPOOL_TASK_TIMEOUT   100
```

## Priority Levels

- `THREADPOOL_PRIORITY_CRITICAL` - System critical tasks
- `THREADPOOL_PRIORITY_HIGH` - Time-sensitive operations
- `THREADPOOL_PRIORITY_NORMAL` - Regular tasks
- `THREADPOOL_PRIORITY_LOW` - Background operations

## Build Integration

Add to Makefile:
```makefile
include Framework/Core/RTOS/ThreadPool/threadpool.mk
```

## Memory Usage

- **Default config**: ~8KB RAM, ~3KB Flash
- **Per task overhead**: ~40 bytes
