/*
 * core_os.h
 *
 *  Created on: Aug 29, 2025
 *      Author: binhhv.23.1.99@gmail.com
 */

#ifndef CORE_RTOS_OAL_CORE_OS_H_
#define CORE_RTOS_OAL_CORE_OS_H_

#include "cmsis_os2.h"

// Mutex lock with timeout
#define C_MUTEX_LOCK(mtx, timeout) \
do { \
  if (osMutexAcquire(mtx, timeout) != osOK) { \
    return; \
  } \
} while (0)

// Mutex unlock
#define C_MUTEX_UNLOCK(mtx) \
do { \
  if (osMutexRelease(mtx) != osOK) { \
    return; \
  } \
} while (0)

// Mutex lock with timeout and return value
#define C_MUTEX_LOCK_RET(mtx, timeout, ret) \
do { \
  if (osMutexAcquire(mtx, timeout) != osOK) { \
    return (ret); \
  } \
} while (0)

// Mutex unlock with return value
#define C_MUTEX_UNLOCK_RET(mtx, ret) \
do { \
  if (osMutexRelease(mtx) != osOK) { \
    return (ret); \
  } \
} while (0)

#endif /* CORE_RTOS_OAL_CORE_OS_H_ */
