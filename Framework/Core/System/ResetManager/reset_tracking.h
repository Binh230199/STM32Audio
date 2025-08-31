/*
 * system_reset_tracking.h
 *
 * MCU-agnostic reset reason tracking library for RTOS frameworks.
 *
 * Features:
 * - Simple API for tracking reset reasons
 * - Platform hooks for custom persistence implementation
 * - No dependencies on specific MCU families
 * - RTOS-friendly design
 *
 *  Created on: Aug 23, 2025
 *      Author: binhhv.23.1.99@gmail.com
 */

#ifndef CORE_SYSTEMMANAGEMENT_SYSTEM_RESET_TRACKING_H_
#define CORE_SYSTEMMANAGEMENT_SYSTEM_RESET_TRACKING_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  NORMAL_BOOT = 0,
  NMI_RESET,
  HARDFAULT_RESET,
  MEMMANAGE_RESET,
  BUSFAULT_RESET,
  USAGEFAULT_RESET,
  DEBUGMON_RESET,
  IWDG_RESET,
  WWDG_RESET,
  SOFTWARE_RESET,
  EXTERNAL_RESET,
  POWER_ON_RESET,
  LOW_POWER_RESET,
  FIREWALL_RESET,
  OPTION_BYTE_RESET,
  FWUPDATE_RESET,
  UNKNOWN_RESET
} e_reset_reason;

/**
 * Initialize reset tracking system.
 *
 * Call this early in boot sequence before any code that might clear
 * hardware reset flags. This function will:
 * 1. Try to read persisted reset reason (if platform supports it)
 * 2. Fall back to hardware detection (if available)
 * 3. Default to NORMAL_BOOT if no other method works
 *
 * @note This is safe to call multiple times
 */
void system_reset_tracking_init(void);

/**
 * Get the detected reset reason.
 *
 * @return Current reset reason enum value
 */
e_reset_reason system_reset_tracking_get_reason(void);

/**
 * Convert reset reason to human-readable string.
 *
 * @param reason Reset reason enum value
 * @return Constant string representation (never NULL)
 */
const char* system_reset_tracking_reason_to_string(e_reset_reason reason);

/**
 * Set reset reason and request persistence.
 *
 * This is typically called from fault handlers before triggering
 * a system reset. The reason will be cached in RAM and a persistence
 * request will be made via platform hooks.
 *
 * @param reason Reset reason to set and persist
 */
void system_reset_tracking_set_reason(e_reset_reason reason);

/**
 * Clear stored reset reason.
 *
 * Clears both RAM cache and any persistent storage via platform hooks.
 */
void system_reset_tracking_clear(void);

/* ========================================================================
 * PLATFORM INTEGRATION HOOKS
 *
 * Implement these functions in your platform code to enable persistence.
 * Default weak implementations are provided that do nothing.
 * ======================================================================== */

/**
 * Platform hook: Write reset reason to persistent storage.
 *
 * Implement this to store the reset reason across power cycles.
 * Common implementations use:
 * - RTC backup registers
 * - Dedicated flash sector
 * - Battery-backed SRAM
 *
 * @param reason Reset reason to store
 * @return true if successfully stored, false otherwise
 */
bool platform_reset_persist_write(e_reset_reason reason);

/**
 * Platform hook: Read reset reason from persistent storage.
 *
 * @param reason Pointer to store the read reset reason
 * @return true if valid reason was read, false if no data or invalid
 */
bool platform_reset_persist_read(e_reset_reason *reason);

/**
 * Platform hook: Clear persistent storage.
 */
void platform_reset_persist_clear(void);

/**
 * Platform hook: Detect reset reason from hardware flags.
 *
 * Implement this to read MCU-specific reset status registers.
 * This is called during init if no persisted reason is found.
 *
 * @param reason Pointer to store the detected reset reason
 * @return true if reason was detected from hardware, false otherwise
 */
bool platform_reset_detect_hardware(e_reset_reason *reason);

#ifdef __cplusplus
}
#endif


#endif /* CORE_SYSTEMMANAGEMENT_SYSTEM_RESET_TRACKING_H_ */
