/*
 * MainApplication.c
 *
 *  Created on: Aug 31, 2025
 *      Author: binhhv.23.1.99@gmail.com
 */

#include "MainApplication.h"
#include "gpio.h"
#include "Core/RTOS/ThreadPool/core_thread.h"
#include "Core/Logger/core_logger.h"
#include "Common/uart_sink.h"
#include "AudioRecorder/audio_recorder.h"

static void loggerInit(void) {
  core_logger_init();
  core_logger_set_min_level(LOG_LEVEL_DEBUG);

  static uart_sink_t uart_sink;
  uart_sink_config_t uart_config = {
      .huart = &huart2,
      .timeout_ms = 1000,
      .use_dma = false
  };

  if (uart_sink_create(&uart_sink, &uart_config, LOG_LEVEL_DEBUG)) {
    core_logger_register_sink((core_logger_sink_t*) &uart_sink);
  }
}

static void threadpoolInit(void) {
  ThreadPoolConfig threadpool_config;

  threadpool_config.thread_count = 4;
  threadpool_config.queue_size = 20;
  threadpool_config.default_timeout = 100;
  threadpool_config.stack_size = 4096;
  threadpool_config.low_power_mode = false;
  threadpool_config.default_thread_priority = osPriorityNormal;

  core_thread_init_with_config(&threadpool_config);
}

void ApplicationInit(void) {
  loggerInit();
  threadpoolInit();

  // Initialize Audio Recorder
  audio_recorder_init();
}

static void taskAudioControl(void* arg) {
  LOGI("Audio Control Task started");

  // Wait for system to stabilize
  osDelay(2000);

  // Start audio recording
  audio_recorder_start();
  LOGI("Audio recording started");

//  while (1) {
//    // Audio control logic here
//    // You can add commands to start/stop recording
//    // or switch between recording modes
//    osDelay(1000);
//  }
}

void ApplicationRun(void) {
  LOGI("Run Main application");

  core_thread_add_task(taskAudioControl, NULL, THREADPOOL_PRIORITY_HIGH, osPriorityHigh, 100);

  // Add audio tasks to threadpool
  core_thread_add_task(audio_recorder_task, NULL, THREADPOOL_PRIORITY_HIGH, osPriorityHigh, 200);
  core_thread_add_task(audio_processor_task, NULL, THREADPOOL_PRIORITY_HIGH, osPriorityHigh, 200);
}


