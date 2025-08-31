/*
 * system_reset_tracking.c
 *
 * MCU-agnostic reset reason tracking implementation.
 *
 * This implementation provides a simple, portable reset tracking system
 * that works across different STM32 families and other ARM Cortex-M MCUs.
 *
 * Platform integration is done via weak hook functions that can be
 * overridden in platform-specific code.
 *
 *  Created on: Aug 23, 2025
 *    Author: binhhv.23.1.99@gmail.com
 */

#include "Core/System/ResetManager/reset_tracking.h"

/* ========================================================================
 * INTERNAL STATE
 * ======================================================================== */

static e_reset_reason s_current_reason = NORMAL_BOOT;
static bool s_initialized = false;

/* ========================================================================
 * WEAK PLATFORM HOOKS
 *
 * These provide default no-op implementations. Override in platform code
 * to enable actual persistence and hardware detection.
 * ======================================================================== */

__attribute__((weak)) bool platform_reset_persist_write(e_reset_reason reason)
{
  (void)reason;
  return false; /* Default no-op implementation */
}

__attribute__((weak)) bool platform_reset_persist_read(e_reset_reason *reason)
{
  (void)reason;
  return false;
}

__attribute__((weak)) void platform_reset_persist_clear(void)
{
  /* No-op */
}

__attribute__((weak)) bool platform_reset_detect_hardware(e_reset_reason *reason)
{
  (void)reason;
  return false;
}

/* ========================================================================
 * PUBLIC API IMPLEMENTATION
 * ======================================================================== */

void system_reset_tracking_init(void)
{
  if (s_initialized) {
    return; /* Already initialized */
  }

  e_reset_reason detected_reason = NORMAL_BOOT;

  /* 1. Try to read persisted reason first */
  if (platform_reset_persist_read(&detected_reason)) {
    s_current_reason = detected_reason;
    /* Clear persisted data after reading to prevent stale data */
    platform_reset_persist_clear();
  }
  /* 2. Fall back to hardware detection */
  else if (platform_reset_detect_hardware(&detected_reason)) {
    s_current_reason = detected_reason;
  }
  /* 3. Default to NORMAL_BOOT */
  else {
    s_current_reason = NORMAL_BOOT;
  }

  s_initialized = true;
}

e_reset_reason system_reset_tracking_get_reason(void)
{
  return s_current_reason;
}

const char* system_reset_tracking_reason_to_string(e_reset_reason reason)
{
  switch (reason) {
    case NORMAL_BOOT:       return "NORMAL_BOOT";
    case NMI_RESET:         return "NMI_RESET";
    case HARDFAULT_RESET:   return "HARDFAULT_RESET";
    case MEMMANAGE_RESET:   return "MEMMANAGE_RESET";
    case BUSFAULT_RESET:    return "BUSFAULT_RESET";
    case USAGEFAULT_RESET:  return "USAGEFAULT_RESET";
    case DEBUGMON_RESET:    return "DEBUGMON_RESET";
    case IWDG_RESET:        return "IWDG_RESET";
    case WWDG_RESET:        return "WWDG_RESET";
    case SOFTWARE_RESET:    return "SOFTWARE_RESET";
    case EXTERNAL_RESET:    return "EXTERNAL_RESET";
    case POWER_ON_RESET:    return "POWER_ON_RESET";
    case LOW_POWER_RESET:   return "LOW_POWER_RESET";
    case FIREWALL_RESET:    return "FIREWALL_RESET";
    case OPTION_BYTE_RESET: return "OPTION_BYTE_RESET";
    case FWUPDATE_RESET:    return "FWUPDATE_RESET";
    case UNKNOWN_RESET:     return "UNKNOWN_RESET";
    default:                return "INVALID_RESET";
  }
}

void system_reset_tracking_set_reason(e_reset_reason reason)
{
  s_current_reason = reason;

  /* Attempt to persist for next boot */
  (void)platform_reset_persist_write(reason);
}

void system_reset_tracking_clear(void)
{
  s_current_reason = NORMAL_BOOT;
  platform_reset_persist_clear();
}

