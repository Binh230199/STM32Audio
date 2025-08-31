/*
 * audio_recorder.c
 *
 *  Created on: Aug 31, 2025
 *    Author: binhhv.23.1.99@gmail.com
 */

#include "audio_recorder.h"
#include "Core/Logger/core_logger.h"
#include "Core/RTOS/ThreadPool/core_thread.h"
#include <string.h>

/* Global audio recorder instance */
static audio_recorder_t g_audio_recorder = {0};

/* External DFSDM handles from dfsdm.c */
extern DFSDM_Filter_HandleTypeDef hdfsdm1_filter0;
extern DFSDM_Filter_HandleTypeDef hdfsdm1_filter1;

/**
 * @brief Initialize audio recorder system
 */
void audio_recorder_init(void) {
  LOGI("Initializing Audio Recorder...");

  /* Initialize audio data structure */
  memset(&g_audio_recorder.audio_data, 0, sizeof(audio_data_t));
  g_audio_recorder.audio_data.buffer_state = AUDIO_BUFFER_EMPTY;
  g_audio_recorder.is_recording = false;

  /* Create synchronization objects */
  g_audio_recorder.audio_semaphore = osSemaphoreNew(1, 0, NULL);
  if (g_audio_recorder.audio_semaphore == NULL) {
    LOGE("Failed to create audio semaphore");
    return;
  }

  g_audio_recorder.audio_queue = osMessageQueueNew(10, sizeof(audio_buffer_state_t), NULL);
  if (g_audio_recorder.audio_queue == NULL) {
    LOGE("Failed to create audio queue");
    return;
  }

  /* Audio recorder initialization complete */
  LOGI("Audio Recorder initialized successfully");
}

/**
 * @brief Start audio recording
 */
void audio_recorder_start(void) {
  if (g_audio_recorder.is_recording) {
    LOGW("Audio recording already started");
    return;
  }

  LOGI("Starting audio recording...");

  /* Configure DFSDM filter channels */
  if (HAL_DFSDM_FilterConfigRegChannel(&hdfsdm1_filter0, DFSDM_CHANNEL_0, DFSDM_CONTINUOUS_CONV_ON) != HAL_OK) {
    LOGE("Failed to configure left DFSDM channel");
    return;
  }

  if (HAL_DFSDM_FilterConfigRegChannel(&hdfsdm1_filter1, DFSDM_CHANNEL_3, DFSDM_CONTINUOUS_CONV_ON) != HAL_OK) {
    LOGE("Failed to configure right DFSDM channel");
    return;
  }

  /* Start DFSDM conversions with DMA */
  if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter1,
                     g_audio_recorder.audio_data.right_buffer,
                     AUDIO_BUFFER_SIZE) != HAL_OK) {
    LOGE("Failed to start right DFSDM filter");
    return;
  }

  if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0,
                     g_audio_recorder.audio_data.left_buffer,
                     AUDIO_BUFFER_SIZE) != HAL_OK) {
    LOGE("Failed to start left DFSDM filter");
    return;
  }

  g_audio_recorder.is_recording = true;
  LOGI("Audio recording started successfully");
}

/**
 * @brief Stop audio recording
 */
void audio_recorder_stop(void) {
  if (!g_audio_recorder.is_recording) {
    LOGW("Audio recording not active");
    return;
  }

  LOGI("Stopping audio recording...");

  /* Stop DFSDM conversions */
  HAL_DFSDM_FilterRegularStop_DMA(&hdfsdm1_filter0);
  HAL_DFSDM_FilterRegularStop_DMA(&hdfsdm1_filter1);

  g_audio_recorder.is_recording = false;
  LOGI("Audio recording stopped");
}

/**
 * @brief Deinitialize audio recorder
 */
void audio_recorder_deinit(void) {
  LOGI("Deinitializing Audio Recorder...");

  /* Stop recording if active */
  audio_recorder_stop();

  /* Clean up synchronization objects */
  if (g_audio_recorder.audio_semaphore) {
    osSemaphoreDelete(g_audio_recorder.audio_semaphore);
  }

  if (g_audio_recorder.audio_queue) {
    osMessageQueueDelete(g_audio_recorder.audio_queue);
  }

  LOGI("Audio Recorder deinitialized");
}

/**
 * @brief Audio recorder task - handles DMA callbacks
 */
void audio_recorder_task(void *argument) {
  LOGI("Audio Recorder Task started");

  audio_buffer_state_t buffer_state;

  while (1) {
    /* Wait for audio buffer notification */
    if (osMessageQueueGet(g_audio_recorder.audio_queue, &buffer_state, NULL, osWaitForever) == osOK) {

      switch (buffer_state) {
        case AUDIO_BUFFER_HALF_FULL:
          LOGD("Audio buffer half complete");
          /* Signal processor task to handle first half */
          osSemaphoreRelease(g_audio_recorder.audio_semaphore);
          break;

        case AUDIO_BUFFER_FULL:
          LOGD("Audio buffer full complete");
          /* Signal processor task to handle second half */
          osSemaphoreRelease(g_audio_recorder.audio_semaphore);
          break;

        default:
          LOGW("Unknown buffer state: %d", buffer_state);
          break;
      }
    }
  }
}

/**
 * @brief Audio processor task - processes audio data in threadpool
 */
void audio_processor_task(void *argument) {
  LOGI("Audio Processor Task started");

  while (1) {
    /* Wait for audio data to be ready */
    if (osSemaphoreAcquire(g_audio_recorder.audio_semaphore, osWaitForever) == osOK) {

      if (!g_audio_recorder.is_recording) {
        continue;
      }

      /* Process audio data based on current buffer state */
      uint32_t start_index = 0;
      uint32_t end_index = AUDIO_BUFFER_SIZE / 2;

      if (g_audio_recorder.audio_data.buffer_state == AUDIO_BUFFER_FULL) {
        start_index = AUDIO_BUFFER_SIZE / 2;
        end_index = AUDIO_BUFFER_SIZE;
      }

      /* Convert and interleave audio data */
      for (uint32_t i = start_index; i < end_index; i++) {
        /* Convert 32-bit to 16-bit with saturation */
        int32_t left_sample = g_audio_recorder.audio_data.left_buffer[i] >> 8;
        int32_t right_sample = g_audio_recorder.audio_data.right_buffer[i] >> 8;

        /* Saturate to 16-bit range */
        if (left_sample > 32767) left_sample = 32767;
        else if (left_sample < -32768) left_sample = -32768;

        if (right_sample > 32767) right_sample = 32767;
        else if (right_sample < -32768) right_sample = -32768;

        /* Store interleaved L/R samples */
        g_audio_recorder.audio_data.playback_buffer[2 * i] = (int16_t)left_sample;
        g_audio_recorder.audio_data.playback_buffer[2 * i + 1] = (int16_t)right_sample;
      }

      LOGD("Processed audio samples from %lu to %lu", start_index, end_index);

      /* Here you can add additional processing:
       * - Save to file
       * - Stream over UART/USB
       * - Apply audio effects
       * - Send to playback system
       */
    }
  }
}

/**
 * @brief Callback for DFSDM half transfer complete
 */
void audio_recorder_half_complete_callback(void) {
  audio_buffer_state_t state = AUDIO_BUFFER_HALF_FULL;

  g_audio_recorder.audio_data.buffer_state = AUDIO_BUFFER_HALF_FULL;

  /* Send notification to recorder task */
  osMessageQueuePut(g_audio_recorder.audio_queue, &state, 0, 0);
}

/**
 * @brief Callback for DFSDM transfer complete
 */
void audio_recorder_full_complete_callback(void) {
  audio_buffer_state_t state = AUDIO_BUFFER_FULL;

  g_audio_recorder.audio_data.buffer_state = AUDIO_BUFFER_FULL;

  /* Send notification to recorder task */
  osMessageQueuePut(g_audio_recorder.audio_queue, &state, 0, 0);
}
