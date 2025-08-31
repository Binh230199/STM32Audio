/*
 * audio_recorder.h
 *
 *  Created on: Aug 31, 2025
 *      Author: binhhv.23.1.99@gmail.com
 */

#ifndef APPLICATION_AUDIORECORDER_AUDIO_RECORDER_H_
#define APPLICATION_AUDIORECORDER_AUDIO_RECORDER_H_

#include "main.h"
#include "cmsis_os.h"
#include "dfsdm.h"
#include "dma.h"
#include "Core/Header/core_types.h"

/* Audio Configuration */
#define AUDIO_BUFFER_SIZE       2048
#define AUDIO_CHANNELS          2
#define AUDIO_SAMPLE_RATE       16000
#define AUDIO_BUFFER_COUNT      2

/* Audio Buffer States */
typedef enum {
    AUDIO_BUFFER_EMPTY = 0,
    AUDIO_BUFFER_HALF_FULL,
    AUDIO_BUFFER_FULL
} audio_buffer_state_t;

/* Audio Data Structure */
typedef struct {
    int32_t left_buffer[AUDIO_BUFFER_SIZE];
    int32_t right_buffer[AUDIO_BUFFER_SIZE];
    int16_t playback_buffer[AUDIO_BUFFER_SIZE * 2]; // Interleaved L/R
    audio_buffer_state_t buffer_state;
    uint32_t buffer_index;
} audio_data_t;

/* Audio Recorder Handle */
typedef struct {
    audio_data_t audio_data;
    osSemaphoreId_t audio_semaphore;
    osMessageQueueId_t audio_queue;
    bool_t is_recording;
} audio_recorder_t;

/* Function Prototypes */
void audio_recorder_init(void);
void audio_recorder_start(void);
void audio_recorder_stop(void);
void audio_recorder_deinit(void);

/* Callback functions (called from HAL) */
void audio_recorder_half_complete_callback(void);
void audio_recorder_full_complete_callback(void);

/* Task functions */
void audio_recorder_task(void *argument);
void audio_processor_task(void *argument);

#endif /* APPLICATION_AUDIORECORDER_AUDIO_RECORDER_H_ */
