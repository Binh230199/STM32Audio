/*
 * MainApplication.c
 *
 *  Created on: Aug 31, 2025
 *      Author: binhhv.23.1.99@gmail.com
 */

#include "MainApplication.h"
#include "gpio.h"
#include "Core/RTOS/ThreadPool/core_thread.h"

void ApplicationInit(void) {
  ThreadPoolConfig threadpool_config;

  threadpool_config.thread_count = 4;
  threadpool_config.queue_size = 20;
  threadpool_config.default_timeout = 100;
  threadpool_config.stack_size = 512;
  threadpool_config.low_power_mode = false;
  threadpool_config.default_thread_priority = osPriorityNormal;

  core_thread_init_with_config(&threadpool_config);
}

static void taskBlink100ms(void* arg) {
  while (1) {
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_0);
    osDelay(100);
  }
}

static void taskBlink200ms(void* arg) {
  while (1) {
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_1);
    osDelay(200);
  }
}

static void taskBlink500ms(void* arg) {
  while (1) {
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_2);
    osDelay(500);
  }
}

static void taskBlink1000ms(void* arg) {
  while (1) {
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
    osDelay(1000);
  }
}

void ApplicationRun(void) {
  core_thread_add_task(taskBlink100ms, NULL, THREADPOOL_PRIORITY_NORMAL, osPriorityNormal, 100);
  core_thread_add_task(taskBlink200ms, NULL, THREADPOOL_PRIORITY_NORMAL, osPriorityNormal, 100);
  core_thread_add_task(taskBlink500ms, NULL, THREADPOOL_PRIORITY_NORMAL, osPriorityNormal, 100);
  core_thread_add_task(taskBlink1000ms, NULL, THREADPOOL_PRIORITY_NORMAL, osPriorityNormal, 100);
}


