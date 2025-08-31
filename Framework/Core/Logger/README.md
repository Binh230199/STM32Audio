# Logger Sink Pattern Implementation

## Overview

This implementation adds the **Sink Pattern** to the existing logger system, allowing multiple output destinations while maintaining backward compatibility with the original UART-only logging.

## Features

- **Multiple Sinks**: Support for UART, SPI, and custom sinks
- **Thread-Safe**: Protected by existing mutex system
- **Backward Compatible**: Original logger API unchanged
- **Runtime Configuration**: Switch between sink and legacy modes
- **Individual Filtering**: Each sink can have different log levels
- **Zero-Copy**: Efficient message passing to sinks

## Architecture

```
Application
     ↓
Logger Core (with Sink Registry)
     ↓
[UART Sink] [SPI Sink] [Custom Sink] ...
     ↓            ↓           ↓
   UART         SPI     Custom Output
```

## Quick Start

### 1. Basic Setup

```c
#include "logger_sink_example.h"

int main(void) {
    /* HAL initialization */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART3_UART_Init();
    MX_SPI1_Init();

    /* Initialize logger with sinks */
    logger_sink_example_init();

    /* Start RTOS */
    osKernelStart();

    return 0;
}
```

### 2. Manual Setup

```c
#include "Core/Logger/core_logger.h"
#include "uart_sink.h"
#include "spi_sink.h"

void setup_logger_manually(void) {
    /* Initialize logger */
    core_logger_init();

    /* Create UART sink */
    static uart_sink_t uart_sink;
    uart_sink_config_t uart_config = {
        .huart = &huart3,
        .timeout_ms = 1000,
        .use_dma = false
    };
    uart_sink_create(&uart_sink, &uart_config, LOG_PRIORITY_DEBUG);
    core_logger_register_sink((struct core_logger_sink*)&uart_sink);

    /* Create SPI sink */
    static spi_sink_t spi_sink;
    spi_sink_config_t spi_config = {
        .hspi = &hspi1,
        .cs_port = GPIOA,
        .cs_pin = GPIO_PIN_4,
        .timeout_ms = 500,
        .use_dma = false,
        .active_low_cs = true
    };
    spi_sink_create(&spi_sink, &spi_config, LOG_PRIORITY_INFO);
    core_logger_register_sink((struct core_logger_sink*)&spi_sink);

    /* Enable sink mode */
    log_enable_sink_mode(true);
}
```

### 3. Usage (Same as Before)

```c
void application_task(void) {
    APP_LOGI("Application started");
    MGR_LOGW("Warning: Low battery");
    DRV_LOGE("Communication failed");

    /* All messages go to registered sinks */
}
```

## API Reference

### Sink Management

```c
/* Register/unregister sinks */
bool_t core_logger_register_sink(struct core_logger_sink* sink);
void core_logger_unregister_sink(struct core_logger_sink* sink);
void core_logger_clear_all_sinks(void);
uint8_t core_logger_get_sink_count(void);

/* Mode control */
void log_enable_sink_mode(bool_t enable);
bool_t log_is_sink_mode_enabled(void);
```

### UART Sink

```c
/* UART sink configuration */
typedef struct {
    UART_HandleTypeDef* huart;
    uint32_t timeout_ms;
    bool_t use_dma;
} uart_sink_config_t;

/* UART sink functions */
bool_t uart_sink_create(uart_sink_t* sink, const uart_sink_config_t* config, log_level_t min_level);
void uart_sink_destroy(uart_sink_t* sink);
void uart_sink_set_timeout(uart_sink_t* sink, uint32_t timeout_ms);
```

### SPI Sink

```c
/* SPI sink configuration */
typedef struct {
    SPI_HandleTypeDef* hspi;
    GPIO_TypeDef* cs_port;
    uint16_t cs_pin;
    uint32_t timeout_ms;
    bool_t use_dma;
    bool_t active_low_cs;
} spi_sink_config_t;

/* SPI sink functions */
bool_t spi_sink_create(spi_sink_t* sink, const spi_sink_config_t* config, log_level_t min_level);
void spi_sink_destroy(spi_sink_t* sink);
void spi_sink_set_timeout(spi_sink_t* sink, uint32_t timeout_ms);
```

## Creating Custom Sinks

### 1. Define Sink Structure

```c
typedef struct {
    core_logger_sink_t base;        /* Must be first member */
    /* Your custom fields here */
    void* custom_handle;
    uint32_t custom_config;
} custom_sink_t;
```

### 2. Implement Virtual Table

```c
static bool_t custom_sink_log(core_logger_sink_t* self, log_level_t level, log_layer_t layer, const char* message) {
    custom_sink_t* custom = (custom_sink_t*)self;

    if (!log_sink_should_log(self, level)) {
        return true; /* Filtered */
    }

    /* Your custom output logic here */
    return send_to_custom_output(custom->custom_handle, message);
}

static const log_sink_vtable_t custom_sink_vtable = {
    .log = custom_sink_log,
    .set_level = /* standard implementation */,
    .get_level = /* standard implementation */,
    .destroy = custom_sink_destroy
};
```

### 3. Create and Register

```c
bool_t custom_sink_create(custom_sink_t* sink, void* handle, log_level_t min_level) {
    log_sink_init(&sink->base, &custom_sink_vtable, "Custom", min_level);
    sink->custom_handle = handle;
    return true;
}

/* Register with logger */
static custom_sink_t my_custom_sink;
custom_sink_create(&my_custom_sink, my_handle, LOG_PRIORITY_INFO);
core_logger_register_sink((struct core_logger_sink*)&my_custom_sink);
```

## Performance Considerations

- **Memory**: Each sink adds ~24 bytes (structure + vtable pointer)
- **CPU**: Minimal overhead - only active when logging
- **Thread Safety**: Protected by existing logger mutex
- **Fallback**: Falls back to UART if all sinks fail

## Backward Compatibility

- **Original API**: All existing log macros work unchanged
- **Legacy Mode**: Can disable sink mode and use original UART only
- **Configuration**: Original compile-time flags still work
- **Buffer Size**: Uses same LOGGER_BUFFER_SIZE

## Example Projects

See `logger_sink_example.c` for complete working examples including:
- Multiple sink setup
- Runtime mode switching
- Custom sink levels
- Error handling

## File Structure

```
Framework/Core/Logger/
├── logger.h/c              # Original logger (enhanced)
├── sink_interface.h/c      # Base sink interface
├── uart_sink.h/c          # UART sink implementation
├── spi_sink.h/c           # SPI sink implementation
├── logger_sink_example.h/c # Usage examples
└── README.md              # This file
```
